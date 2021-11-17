#include "pljit/analysis/SemanticAnalysis.h"
#include "pljit/ast/ASTDotVisitor.h"
#include "test/utils/TestUtils.h"
#include <gtest/gtest.h>

namespace pljit::ast {

namespace {

void executeTest(std::string_view code, std::string_view expectedDotGraph) {
    test_utils::ASTEnvironment env(code, test_utils::Optimization::NoOptimization);

    std::ostringstream buffer;
    ASTDotVisitor visitor(buffer, env.symbolTable);
    env.ast->accept(visitor);

    ASSERT_EQ(buffer.str(), expectedDotGraph);
}

} // namespace

TEST(TestASTDotVisitor, SimpleTest) { // NOLINT
    std::string_view code{"BEGIN\n"
                          "RETURN 123"
                          "END.\n"};

    std::string_view expectedASTInDot{"digraph {\n"
                                      "\t0 [label=\"Function\"];\n"
                                      "\t1 [label=\"RETURN\"];\n"
                                      "\t2 [label=\"123\"];\n"
                                      "\t0 -> 1;\n"
                                      "\t1 -> 2;\n"
                                      "}\n"};

    executeTest(code, expectedASTInDot);
}

TEST(TestASTDotVisitor, WeightCalculationi) { // NOLINT
    std::string_view code{"PARAM width, height, depth;\n"
                          "VAR volume;\n"
                          "CONST density = 2400;\n"
                          "\n"
                          "BEGIN\n"
                          "    volume := width * height * depth;\n"
                          "    RETURN density * volume\n"
                          "END."};

    std::string_view expectedASTInDot{"digraph {\n"
                                      "\t0 [label=\"Function\"];\n"
                                      "\t1 [label=\":=\"];\n"
                                      "\t2 [label=\"volume\"];\n"
                                      "\t3 [label=\"*\"];\n"
                                      "\t4 [label=\"width\"];\n"
                                      "\t5 [label=\"*\"];\n"
                                      "\t6 [label=\"height\"];\n"
                                      "\t7 [label=\"depth\"];\n"
                                      "\t8 [label=\"RETURN\"];\n"
                                      "\t9 [label=\"*\"];\n"
                                      "\t10 [label=\"density: 2400\"];\n"
                                      "\t11 [label=\"volume\"];\n"
                                      "\t0 -> 1;\n"
                                      "\t1 -> 2;\n"
                                      "\t1 -> 3;\n"
                                      "\t3 -> 4;\n"
                                      "\t3 -> 5;\n"
                                      "\t5 -> 6;\n"
                                      "\t5 -> 7;\n"
                                      "\t0 -> 8;\n"
                                      "\t8 -> 9;\n"
                                      "\t9 -> 10;\n"
                                      "\t9 -> 11;\n"
                                      "}\n"};

    executeTest(code, expectedASTInDot);
}

TEST(TestASTDotVisitor, MoreComplexTest) { // NOLINT
    std::string_view code{"PARAM a, b;\n"
                          "VAR c;\n"
                          "CONST A = 12002, B = 42;\n"
                          "\n"
                          "BEGIN\n"
                          "    a := -1234 + ((a * b) / 12);\n"
                          "    c := a * A + B;\n"
                          "    RETURN a / c\n"
                          "END.\n"};

    std::string_view expectedASTInDot{"digraph {\n"
                                      "\t0 [label=\"Function\"];\n"
                                      "\t1 [label=\":=\"];\n"
                                      "\t2 [label=\"a\"];\n"
                                      "\t3 [label=\"+\"];\n"
                                      "\t4 [label=\"-\"];\n"
                                      "\t5 [label=\"1234\"];\n"
                                      "\t6 [label=\"/\"];\n"
                                      "\t7 [label=\"*\"];\n"
                                      "\t8 [label=\"a\"];\n"
                                      "\t9 [label=\"b\"];\n"
                                      "\t10 [label=\"12\"];\n"
                                      "\t11 [label=\":=\"];\n"
                                      "\t12 [label=\"c\"];\n"
                                      "\t13 [label=\"+\"];\n"
                                      "\t14 [label=\"*\"];\n"
                                      "\t15 [label=\"a\"];\n"
                                      "\t16 [label=\"A: 12002\"];\n"
                                      "\t17 [label=\"B: 42\"];\n"
                                      "\t18 [label=\"RETURN\"];\n"
                                      "\t19 [label=\"/\"];\n"
                                      "\t20 [label=\"a\"];\n"
                                      "\t21 [label=\"c\"];\n"
                                      "\t0 -> 1;\n"
                                      "\t1 -> 2;\n"
                                      "\t1 -> 3;\n"
                                      "\t3 -> 4;\n"
                                      "\t4 -> 5;\n"
                                      "\t3 -> 6;\n"
                                      "\t6 -> 7;\n"
                                      "\t7 -> 8;\n"
                                      "\t7 -> 9;\n"
                                      "\t6 -> 10;\n"
                                      "\t0 -> 11;\n"
                                      "\t11 -> 12;\n"
                                      "\t11 -> 13;\n"
                                      "\t13 -> 14;\n"
                                      "\t14 -> 15;\n"
                                      "\t14 -> 16;\n"
                                      "\t13 -> 17;\n"
                                      "\t0 -> 18;\n"
                                      "\t18 -> 19;\n"
                                      "\t19 -> 20;\n"
                                      "\t19 -> 21;\n"
                                      "}\n"};

    executeTest(code, expectedASTInDot);
}

} // namespace pljit::ast
