#include "pljit/Pljit.h"
#include "test/utils/TestUtils.h"
#include <thread>
#include <type_traits>
#include <gtest/gtest.h>

namespace pljit {

TEST(TestFunctionHandle, TriviallyCopyable) { // NOLINT
    ASSERT_TRUE(std::is_trivially_copyable_v<FunctionHandle>);
}

TEST(TestPljitSingleThreaded, SimpleTest) { // NOLINT
    std::string code{"BEGIN\n"
                     "RETURN 1\n"
                     "END."};

    Pljit pljit;
    auto func = pljit.registerFunction(code);
    auto result = func();
    ASSERT_EQ(result.resultCode, ResultCode::Success);
    ASSERT_EQ(result.value, 1);
}

TEST(TestPljitSingleThreaded, ParameterTest) { // NOLINT
    std::string code{"PARAM a, b;\n"
                     "BEGIN\n"
                     "RETURN a + b\n"
                     "END."};

    Pljit pljit;
    auto func = pljit.registerFunction(code);
    auto result = func(1, 2);
    ASSERT_EQ(result.resultCode, ResultCode::Success);
    ASSERT_EQ(result.value, 3);
}

TEST(TestPljitSingleThreaded, ParameterTest2) { // NOLINT
    std::string code{"PARAM a, b, c;\n"
                     "BEGIN\n"
                     "RETURN a + b - c\n"
                     "END."};

    Pljit pljit;
    auto func = pljit.registerFunction(code);
    auto result = func(1, 2, 3);
    ASSERT_EQ(result.resultCode, ResultCode::Success);
    ASSERT_EQ(result.value, 0);
}

TEST(TestPljitSingleThreaded, WrongParameterCount) { // NOLINT
    test_utils::CaptureCout cout;

    std::string code{"PARAM a, b, c;\n"
                     "BEGIN\n"
                     "RETURN a + b - c\n"
                     "END."};

    Pljit pljit;
    auto func = pljit.registerFunction(code);
    auto result = func(1, 2);

    ASSERT_EQ(result.resultCode, ResultCode::InvalidFunctionCall);
    ASSERT_EQ(cout.stream.str(), "error: invalid number of parameters "
                                 "provided, expected 3 but 2 were provided\n");
}

TEST(TestPljitSingleThreaded, RuntimeError) { // NOLINT
    test_utils::CaptureCout cout;

    std::string code{"PARAM a, b;\n"
                     "BEGIN\n"
                     "   RETURN a / b\n"
                     "END.\n"};

    Pljit pljit;
    auto func = pljit.registerFunction(code);
    auto result = func(1, 0);

    ASSERT_EQ(result.resultCode, ResultCode::RuntimeError);
    ASSERT_EQ(cout.stream.str(), "error: division by zero\n");
}

TEST(TestPljitSingleThreaded, CompileError) { // NOLINT
    test_utils::CaptureCout cout;

    std::string code{"PARAM a, b;\n"
                     "VAR c;\n"
                     "BEGIN\n"
                     "   RETURN c * (a + b)\n"
                     "END.\n"};

    Pljit pljit;
    auto func = pljit.registerFunction(code);
    auto result = func(1, 2);

    ASSERT_EQ(result.resultCode, ResultCode::CompileError);
    ASSERT_EQ(cout.stream.str(), "4:11: error: use of uninitialized identifier\n"
                                 "   RETURN c * (a + b)\n"
                                 "          ^\n");
}

TEST(TestPljitSingleThreaded, CantFailTest) { // NOLINT
    std::string code{"BEGIN\n"
                     "RETURN 1\n"
                     "END."};

    Pljit pljit;
    auto func = pljit.registerFunction(code);
    ASSERT_EQ(cantFail(func()), 1);
}

TEST(TestPljitSingleThreaded, EmptyCodeString) { // NOLINT
    test_utils::CaptureCout cout;

    std::string code;

    Pljit pljit;
    auto func = pljit.registerFunction(code);
    auto result = func();

    ASSERT_EQ(result.resultCode, ResultCode::CompileError);
    ASSERT_EQ(cout.stream.str(), "error: received code string of length 0\n");
}

TEST(TestPljitMultiThreaded, MultipleThreadsSameFunction) { // NOLINT
    std::string code{"PARAM a, b;\n"
                     "VAR c;\n"
                     "BEGIN\n"
                     "  c := a + b;\n"
                     "RETURN c * 2\n"
                     "END."};
    Pljit pljit;
    auto func = pljit.registerFunction(code);

    std::vector<std::thread> threadPool;
    for (int64_t i = 0; i < 10; i++) {
        threadPool.emplace_back([func, i]() {
            for (unsigned j = 0; j < 100; j++) {
                auto result = func(i, 2 * i);
                ASSERT_EQ(result.resultCode, ResultCode::Success);
                ASSERT_EQ(result.value, 6 * i);
            }
        });
    }

    for (auto& thread : threadPool) {
        thread.join();
    }
}

TEST(TestPljitMultiThreaded, MultipleThreadsSameFunctionOneFailingThread) { // NOLINT
    // We ignore everything printed to std::cout to not spam the console.
    std::cout.setstate(std::ios_base::failbit);

    std::string code{"PARAM a, b;\n"
                     "VAR c;\n"
                     "BEGIN\n"
                     "  c := a / b;\n"
                     "RETURN c * 2\n"
                     "END."};
    Pljit pljit;
    auto func = pljit.registerFunction(code);

    std::vector<std::thread> threadPool;
    threadPool.reserve(10);
    for (int64_t i = 0; i < 10; i++) {
        threadPool.emplace_back([func, i]() {
            for (unsigned j = 0; j < 100; j++) {
                auto result = func(2 * i, i);

                if (i == 0) {
                    ASSERT_EQ(result.resultCode, ResultCode::RuntimeError);
                } else {
                    ASSERT_EQ(result.resultCode, ResultCode::Success);
                    ASSERT_EQ(result.value, 4);
                }
            }
        });
    }

    for (auto& thread : threadPool) {
        thread.join();
    }

    std::cout.clear();
}

TEST(TestPljitMultiThreaded, MultipleRegistrations) { // NOLINT
    Pljit pljit;

    const unsigned numberOfRegisteredFunctions = 2;

    std::vector<FunctionHandle> functions;
    for (unsigned i = 0; i < numberOfRegisteredFunctions; i++) {
        std::string function{"BEGIN\n"
                             "RETURN " + std::to_string(i) + "\n"
                             "END."};
        functions.push_back(pljit.registerFunction(function));
    }

    std::vector<std::thread> threadPool;
    for (int64_t i = 0; i < 20; i++) {
        auto function = functions[i % numberOfRegisteredFunctions];
        threadPool.emplace_back([function, i, numberOfRegisteredFunctions]() {
            auto result = function();
            ASSERT_EQ(result.resultCode, ResultCode::Success);
            ASSERT_EQ(result.value, (i % numberOfRegisteredFunctions));
        });
    }

    for (auto& thread : threadPool) {
        thread.join();
    }
}

TEST(TestPljitMultiThreaded, MultipleRegistrationsWithOnlyCompilerErrors) { // NOLINT
    // We ignore everything printed to std::cout to not spam the console.
    std::cout.setstate(std::ios_base::failbit);

    Pljit pljit;

    const unsigned numberOfRegisteredFunctions = 2;

    std::vector<FunctionHandle> functions;
    for (unsigned i = 0; i < numberOfRegisteredFunctions; i++) {
        // Function with missing END and program terminator.
        std::string function{"BEGIN\n"
                             "RETURN " + std::to_string(i) + "\n"};
        functions.push_back(pljit.registerFunction(function));
    }

    std::vector<std::thread> threadPool;
    for (int64_t i = 0; i < 20; i++) {
        auto function = functions[i % numberOfRegisteredFunctions];
        threadPool.emplace_back([function]() {
          auto result = function();
          ASSERT_EQ(result.resultCode, ResultCode::CompileError);
        });
    }

    for (auto& thread : threadPool) {
        thread.join();
    }
}

TEST(TestPljitMultiThreaded, MultipleRegistrationsWithOnlyInvalidFunctionCalls) { // NOLINT
    // We ignore everything printed to std::cout to not spam the console.
    std::cout.setstate(std::ios_base::failbit);

    Pljit pljit;

    const unsigned numberOfRegisteredFunctions = 2;

    std::vector<FunctionHandle> functions;
    for (unsigned i = 0; i < numberOfRegisteredFunctions; i++) {
        // Function with missing END and program terminator.
        std::string function{"PARAM a, b;\n"
                             "BEGIN\n"
                             "RETURN " + std::to_string(i) + "\n"
                             "END."};
        functions.push_back(pljit.registerFunction(function));
    }

    std::vector<std::thread> threadPool;
    for (int64_t i = 0; i < 20; i++) {
        auto function = functions[i % numberOfRegisteredFunctions];
        threadPool.emplace_back([function]() {
          auto result = function();
          ASSERT_EQ(result.resultCode, ResultCode::InvalidFunctionCall);
        });
    }

    for (auto& thread : threadPool) {
        thread.join();
    }
}

TEST(TestPljitMultiThreaded, FuzzyTest) { // NOLINT
    // We ignore everything printed to std::cout to not spam the console.
    std::cout.setstate(std::ios_base::failbit);

    Pljit pljit;

    std::vector<FunctionHandle> functions;

    // 1. Valid function
    std::string function1{"PARAM a, b; BEGIN RETURN 0 END."};
    functions.push_back(pljit.registerFunction(function1));

    // 2. Compiler error
    std::string function2{"VAR a; BEGIN RETURN a END."};
    functions.push_back(pljit.registerFunction(function2));

    // 3. (Potential) Runtime error
    std::string function3{"PARAM a, b; BEGIN RETURN a / b END."};
    functions.push_back(pljit.registerFunction(function3));

    std::vector<std::thread> threadPool;
    for (int64_t i = 0; i < 30; i++) {
        unsigned functionId = i % 3;
        auto function = functions[functionId];

        if (functionId == 0) {
            // 1. Valid function
            threadPool.emplace_back([function]() {
              for (unsigned j = 0; j < 5; j++) {
                  if (j % 2 == 0) {
                    auto result = function();
                    ASSERT_EQ(result.resultCode, ResultCode::InvalidFunctionCall);
                  } else {
                    auto result = function(1, 2);
                    ASSERT_EQ(result.resultCode, ResultCode::Success);
                    ASSERT_EQ(result.value, 0);
                  }
              }
            });
        } else if (functionId == 1) {
            // 2. Compiler error
            threadPool.emplace_back([function]() {
               for (unsigned j = 0; j < 5; j++) {
                   auto result = function(2);
                   ASSERT_EQ(result.resultCode, ResultCode::CompileError);
               }
            });
        } else {
            // 3. (Potential) Runtime error
            threadPool.emplace_back([function]() {
              for (unsigned j = 0; j < 5; j++) {
                  if (j % 2 == 0) {
                      auto result = function(1, 0);
                      ASSERT_EQ(result.resultCode, ResultCode::RuntimeError);
                  } else {
                      auto result = function(4, 2);
                      ASSERT_EQ(result.resultCode, ResultCode::Success);
                      ASSERT_EQ(result.value, 2);
                  }
              }
            });
        }
    }

    for (auto& thread : threadPool) {
        thread.join();
    }
}

} // namespace pljit
