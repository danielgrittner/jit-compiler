set(TEST_SOURCES
        Tester.cpp

        # Tests
        pljit/TestLexer.cpp
        pljit/TestParser.cpp
        pljit/TestParserErrors.cpp
        pljit/TestParseTreeDotVisitor.cpp
        pljit/TestSemanticAnalysis.cpp
        pljit/TestASTDotVisitor.cpp
        pljit/TestASTExecution.cpp
        pljit/TestDeadCodeElimination.cpp
        pljit/TestConstantPropagation.cpp
        pljit/TestPljit.cpp

        # Utils
        utils/TestUtils.cpp
        )

find_package(Threads REQUIRED)

add_executable(tester ${TEST_SOURCES})
target_link_libraries(tester PUBLIC
    pljit
    GTest::GTest
    Threads::Threads)
