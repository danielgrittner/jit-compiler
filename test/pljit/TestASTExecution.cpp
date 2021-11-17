#include "pljit/analysis/SemanticAnalysis.h"
#include "pljit/exec/ExecutionContext.h"
#include "test/utils/TestUtils.h"
#include <gtest/gtest.h>

namespace pljit::ast {

TEST(TestASTExecution, SimpleProgram) { // NOLINT:
    std::string_view code{"BEGIN\n"
                          "   RETURN 123\n"
                          "END.\n"};
    std::vector<int64_t> parameters;
    test_utils::ExpectedResultASTExecTest expected{123,
                                                   exec::ExecutionContext::ErrorType::NoError};

    test_utils::performASTExecutionTest(code,
                                        parameters,
                                        test_utils::Optimization::DeadCodeEliminationAndConstantPropagation,
                                        expected);
}

TEST(TestASTExecution, WeightCalculationOfBlock) { // NOLINT
    std::string_view code{"PARAM width, height, depth;\n"
                          "VAR volume;\n"
                          "CONST density = 2400;\n"
                          "\n"
                          "BEGIN\n"
                          "    volume := width * height * depth;\n"
                          "    RETURN density * volume\n"
                          "END."};
    std::vector<int64_t> parameters{10, 20, 10};
    test_utils::ExpectedResultASTExecTest expected{4'800'000,
                                                   exec::ExecutionContext::ErrorType::NoError};

    test_utils::performASTExecutionTest(code,
                                        parameters,
                                        test_utils::Optimization::DeadCodeEliminationAndConstantPropagation,
                                        expected);
}

TEST(TestASTExecution, TrapezoidalRule) { // NOLINT
    std::string_view code{"PARAM a, b, fa, fb;\n"
                          "VAR h, factor;\n"
                          "CONST TWO = 2;\n"
                          "BEGIN\n"
                          "      h := (b - a) / TWO;\n"
                          "      RETURN (fa + fb) * h\n"
                          "END."};
    std::vector<int64_t> parameters{0, 10, 4, 10};
    test_utils::ExpectedResultASTExecTest expected{70,
                                                   exec::ExecutionContext::ErrorType::NoError};

    test_utils::performASTExecutionTest(code,
                                        parameters,
                                        test_utils::Optimization::DeadCodeEliminationAndConstantPropagation,
                                        expected);
}

TEST(TestASTExecution, RandomCalculations) { // NOLINT
    std::string_view code{"PARAM a, b, c;\n"
                          "VAR u, v, w;\n"
                          "CONST A = 1024, B = 1;\n"
                          "BEGIN\n"
                          "   u := (a + b) * A;\n"
                          "   v := u - (B * 12);\n"
                          "   w := (((A * c) / B) + u) - (v + (1 * a));\n"
                          "   a := u + v + w;\n"
                          "   c := +(-a);\n"
                          "   RETURN c + B\n"
                          "END."};
    std::vector<int64_t> parameters{1, 2, 3};
    test_utils::ExpectedResultASTExecTest expected{-9214,
                                                   exec::ExecutionContext::ErrorType::NoError};

    test_utils::performASTExecutionTest(code,
                                        parameters,
                                        test_utils::Optimization::DeadCodeEliminationAndConstantPropagation,
                                        expected);
}

TEST(TestASTExecution, PolynomialOfDegree4WithHornerSchema) { // NOLINT
    std::string_view code{"PARAM x;\n"
                          "VAR result, t;\n"
                          "CONST A = 1, B = 2, C = 1, D = 3, E = 1;\n"
                          "BEGIN\n"
                          "   result := -E * x;\n"
                          "   result := D + result * x;\n"
                          "   result := -C + result * x;\n"
                          "   result := B + result * x;\n"
                          "   result := A + result * x;\n"
                          "   RETURN result\n"
                          "END."};
    std::vector<int64_t> parameters{1};
    test_utils::ExpectedResultASTExecTest expected{4,
                                                   exec::ExecutionContext::ErrorType::NoError};

    test_utils::performASTExecutionTest(code,
                                        parameters,
                                        test_utils::Optimization::DeadCodeEliminationAndConstantPropagation,
                                        expected);
}

TEST(TestASTExecution, AnotherPolynomial) { // NOLINT
    std::string_view code{"PARAM x;\n"
                          "VAR result;\n"
                          "BEGIN\n"
                          "   result := 1;\n"
                          "   result := result + 2 * x * x;\n"
                          "   result := result + (-3) * x * x * x;\n"
                          "   RETURN result\n"
                          "END."};
    std::vector<int64_t> parameters{2};
    test_utils::ExpectedResultASTExecTest expected{-15,
                                                   exec::ExecutionContext::ErrorType::NoError};

    test_utils::performASTExecutionTest(code,
                                        parameters,
                                        test_utils::Optimization::DeadCodeEliminationAndConstantPropagation,
                                        expected);
}

TEST(TestASTExecution, SomeCalculations) { // NOLINT
    std::string_view code{"VAR x;\n"
                          "BEGIN\n"
                          "     x := 1 + 3 - 2 + 42;\n"
                          "     RETURN x + 12\n"
                          "END."};
    std::vector<int64_t> parameters;
    test_utils::ExpectedResultASTExecTest expected{-28,
                                                   exec::ExecutionContext::ErrorType::NoError};

    test_utils::performASTExecutionTest(code,
                                        parameters,
                                        test_utils::Optimization::DeadCodeEliminationAndConstantPropagation,
                                        expected);
}

TEST(TestASTExecution, SomeCalculations2) { // NOLINT
    std::string_view code{"BEGIN\n"
                          "     RETURN 1 - -2\n"
                          "END."};
    std::vector<int64_t> parameters;
    test_utils::ExpectedResultASTExecTest expected{3,
                                                   exec::ExecutionContext::ErrorType::NoError};

    test_utils::performASTExecutionTest(code,
                                        parameters,
                                        test_utils::Optimization::DeadCodeEliminationAndConstantPropagation,
                                        expected);
}

TEST(TestASTExecution, SomeCalculations3) { // NOLINT
    std::string_view code{"BEGIN\n"
                          "    RETURN 2 * 3 - 2 + 1\n"
                          "END."};
    std::vector<int64_t> parameters;
    test_utils::ExpectedResultASTExecTest expected{3,
                                                   exec::ExecutionContext::ErrorType::NoError};

    test_utils::performASTExecutionTest(code,
                                        parameters,
                                        test_utils::Optimization::DeadCodeEliminationAndConstantPropagation,
                                        expected);
}

TEST(TestASTExecution, DivisionByZero) { // NOLINT
    test_utils::CaptureCout cout;

    std::string_view code{"BEGIN\n"
                          "   RETURN 1 / 0\n"
                          "END.\n"};
    std::vector<int64_t> parameters;
    test_utils::ExpectedResultASTExecTest expected;
    expected.expectedErrorType = exec::ExecutionContext::ErrorType::DivisionByZero;

    test_utils::performASTExecutionTest(code,
                                        parameters,
                                        test_utils::Optimization::DeadCodeEliminationAndConstantPropagation,
                                        expected);

    // The error message is expected twice since we execute the program
    // once optimized and then also unoptimized.
    ASSERT_EQ(cout.stream.str(), "error: division by zero\n"
                                 "error: division by zero\n");
}

} // namespace pljit::ast
