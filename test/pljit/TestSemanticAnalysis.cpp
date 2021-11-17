#include "pljit/analysis/SemanticAnalysis.h"
#include "pljit/ast/AST.h"
#include "test/utils/TestUtils.h"
#include <gtest/gtest.h>

namespace pljit::analysis {

namespace {

void executeErrorTest(std::string_view code, std::string_view expectedErrorMsg) {
    test_utils::CaptureCout cout;
    test_utils::ASTEnvironment env(code, test_utils::Optimization::NoOptimization);
    ASSERT_TRUE(env.ast == nullptr);
    ASSERT_EQ(cout.stream.str(), expectedErrorMsg);
}

} // namespace

TEST(TestSemanticAnalysis, SimpleTest) { // NOLINT
    std::string_view code{"PARAM a, b;\n"
                          "BEGIN\n"
                          "   RETURN a * b\n"
                          "END."};

    test_utils::ASTEnvironment env(code, test_utils::Optimization::NoOptimization);
    ASSERT_FALSE(env.ast == nullptr);
    const auto& ast = env.ast;
    const auto& symbolTable = env.symbolTable;

    test_utils::checkASTParameter(symbolTable, 0, "a");
    test_utils::checkASTParameter(symbolTable, 1, "b");

    const auto& statements = ast->getStatements();
    ASSERT_EQ(statements.size(), 1);

    ////////////// Statement 1: RETURN a * b

    ASSERT_EQ(statements[0]->getType(), ast::ASTNode::Type::ReturnStatement);
    const auto& returnStatement = static_cast<ast::ReturnStatement&>(*statements[0]); // NOLINT

    const auto& expression = returnStatement.getExpression();
    ASSERT_EQ(expression.getType(), ast::ASTNode::Type::BinaryOp);

    const auto& binaryOp = static_cast<const ast::BinaryOp&>(expression); // NOLINT
    ASSERT_EQ(binaryOp.getBinaryOpType(), ast::BinaryOp::Type::Mul);

    // Identifier: a
    const auto& lhsExpression = binaryOp.getLhsExpression();
    ASSERT_EQ(lhsExpression.getType(), ast::ASTNode::Type::Identifier);
    const auto& lhs = static_cast<const ast::Identifier&>(lhsExpression); // NOLINT
    ASSERT_EQ(lhs.getIdentifierType(), ast::Identifier::Type::Parameter);
    ASSERT_EQ(lhs.getId(), 0);

    // Identifier: b
    const auto& rhsExpression = binaryOp.getRhsExpression();
    ASSERT_EQ(rhsExpression.getType(), ast::ASTNode::Type::Identifier);
    const auto& rhs = static_cast<const ast::Identifier&>(rhsExpression); // NOLINT
    ASSERT_EQ(rhs.getIdentifierType(), ast::Identifier::Type::Parameter);
    ASSERT_EQ(rhs.getId(), 1);
}

TEST(TestSemanticAnalysis, ComplexTest) { // NOLINT
    std::string_view code{"PARAM a, b;\n"
                          "VAR x, y;\n"
                          "CONST D = 42, F = 39, G = 1024;\n"
                          "BEGIN\n"
                          "      x := (a - D) / (b - (F * G));\n"
                          "      y := x * (D + a);\n"
                          "      a := -x;\n"
                          "      RETURN +a\n"
                          "END.\n"};

    test_utils::ASTEnvironment env(code, test_utils::Optimization::NoOptimization);
    ASSERT_FALSE(env.ast == nullptr);
    const auto& ast = env.ast;
    const auto& symbolTable = env.symbolTable;

    test_utils::checkASTParameter(symbolTable, 0, "a");
    test_utils::checkASTParameter(symbolTable, 1, "b");
    test_utils::checkASTVariable(symbolTable, 0, "x");
    test_utils::checkASTVariable(symbolTable, 1, "y");
    test_utils::checkASTConstant(symbolTable, 0, "D", 42);
    test_utils::checkASTConstant(symbolTable, 1, "F", 39);
    test_utils::checkASTConstant(symbolTable, 2, "G", 1024);

    const auto& statements = ast->getStatements();
    ASSERT_EQ(statements.size(), 4);

    ////////////// Statement 1: x := (a - D) / (b - (F * G))

    ASSERT_EQ(statements[0]->getType(), ast::ASTNode::Type::AssignmentStatement);
    const auto& stmt1 = static_cast<const ast::AssignmentStatement&>(*statements[0]); // NOLINT

    // Assignment target: x
    const auto& stmt1AssignmentTarget = stmt1.getAssignmentTarget();
    ASSERT_EQ(stmt1AssignmentTarget.getType(), ast::ASTNode::Type::Identifier);
    const auto& target1 = static_cast<const ast::Identifier&>(stmt1AssignmentTarget); // NOLINT
    ASSERT_EQ(target1.getIdentifierType(), ast::Identifier::Type::Variable);
    ASSERT_EQ(target1.getId(), 0);

    // Expression: (a - D) / (b - (F * G))
    const auto& expr1 = stmt1.getExpression();
    ASSERT_EQ(expr1.getType(), ast::ASTNode::Type::BinaryOp);
    const auto& binOp1 = static_cast<const ast::BinaryOp&>(expr1); // NOLINT
    ASSERT_EQ(binOp1.getBinaryOpType(), ast::BinaryOp::Type::Div);

    // Expression: a - D
    const auto& expr2 = binOp1.getLhsExpression();
    ASSERT_EQ(expr2.getType(), ast::ASTNode::Type::BinaryOp);
    const auto& binOp2 = static_cast<const ast::BinaryOp&>(expr2); // NOLINT
    ASSERT_EQ(binOp2.getBinaryOpType(), ast::BinaryOp::Type::Sub);

    // Expression: a
    const auto& expr3 = binOp2.getLhsExpression();
    ASSERT_EQ(expr3.getType(), ast::ASTNode::Type::Identifier);
    const auto& var1 = static_cast<const ast::Identifier&>(expr3); // NOLINT
    ASSERT_EQ(var1.getIdentifierType(), ast::Identifier::Type::Parameter);
    ASSERT_EQ(var1.getId(), 0);

    // Expression: D (= 42)
    const auto& expr4 = binOp2.getRhsExpression();
    ASSERT_EQ(expr4.getType(), ast::ASTNode::Type::Identifier);
    const auto& const1 = static_cast<const ast::Identifier&>(expr4); // NOLINT
    ASSERT_EQ(const1.getIdentifierType(), ast::Identifier::Type::Constant);
    ASSERT_EQ(const1.getId(), 0);

    // Expression: b - (F * G)
    const auto& expr5 = binOp1.getRhsExpression();
    ASSERT_EQ(expr5.getType(), ast::ASTNode::Type::BinaryOp);
    const auto& binOp3 = static_cast<const ast::BinaryOp&>(expr5); // NOLINT
    ASSERT_EQ(binOp3.getBinaryOpType(), ast::BinaryOp::Type::Sub);

    // Expression: b
    const auto& expr6 = binOp3.getLhsExpression();
    ASSERT_EQ(expr6.getType(), ast::ASTNode::Type::Identifier);
    const auto& var2 = static_cast<const ast::Identifier&>(expr6); // NOLINT
    ASSERT_EQ(var2.getIdentifierType(), ast::Identifier::Type::Parameter);
    ASSERT_EQ(var2.getId(), 1);

    // Expression: F * G
    const auto& expr7 = binOp3.getRhsExpression();
    ASSERT_EQ(expr7.getType(), ast::ASTNode::Type::BinaryOp);
    const auto& binOp4 = static_cast<const ast::BinaryOp&>(expr7); // NOLINT
    ASSERT_EQ(binOp4.getBinaryOpType(), ast::BinaryOp::Type::Mul);

    // Expression: F (= 39)
    const auto& expr8 = binOp4.getLhsExpression();
    ASSERT_EQ(expr8.getType(), ast::ASTNode::Type::Identifier);
    const auto& const2 = static_cast<const ast::Identifier&>(expr8); // NOLINT
    ASSERT_EQ(const2.getIdentifierType(), ast::Identifier::Type::Constant);
    ASSERT_EQ(const2.getId(), 1);

    // Expression: G (= 1024)
    const auto& expr9 = binOp4.getRhsExpression();
    ASSERT_EQ(expr9.getType(), ast::ASTNode::Type::Identifier);
    const auto& const3 = static_cast<const ast::Identifier&>(expr9); // NOLINT
    ASSERT_EQ(const3.getIdentifierType(), ast::Identifier::Type::Constant);
    ASSERT_EQ(const3.getId(), 2);

    ////////////// Statement 2: y := x * (D + a)

    ASSERT_EQ(statements[1]->getType(), ast::ASTNode::Type::AssignmentStatement);
    const auto& stmt2 = static_cast<const ast::AssignmentStatement&>(*statements[1]); // NOLINT

    // Assignment target: y
    const auto& stmt2AssignmentTarget = stmt2.getAssignmentTarget();
    ASSERT_EQ(stmt2AssignmentTarget.getType(), ast::ASTNode::Type::Identifier);
    const auto& target2 = static_cast<const ast::Identifier&>(stmt2AssignmentTarget); // NOLINT
    ASSERT_EQ(target2.getIdentifierType(), ast::Identifier::Type::Variable);
    ASSERT_EQ(target2.getId(), 1);

    // Expression: x * (D + a)
    const auto& expr10 = stmt2.getExpression();
    ASSERT_EQ(expr10.getType(), ast::ASTNode::Type::BinaryOp);
    const auto& binOp5 = static_cast<const ast::BinaryOp&>(expr10); // NOLINT
    ASSERT_EQ(binOp5.getBinaryOpType(), ast::BinaryOp::Type::Mul);

    // Expression: x
    const auto& expr11 = binOp5.getLhsExpression();
    ASSERT_EQ(expr11.getType(), ast::ASTNode::Type::Identifier);
    const auto& var3 = static_cast<const ast::Identifier&>(expr11); // NOLINT
    ASSERT_EQ(var3.getIdentifierType(), ast::Identifier::Type::Variable);
    ASSERT_EQ(var3.getId(), 0);

    // Expression: D + a
    const auto& expr12 = binOp5.getRhsExpression();
    ASSERT_EQ(expr12.getType(), ast::ASTNode::Type::BinaryOp);
    const auto& binOp6 = static_cast<const ast::BinaryOp&>(expr12); // NOLINT
    ASSERT_EQ(binOp6.getBinaryOpType(), ast::BinaryOp::Type::Add);

    // Expression: D (= 42)
    const auto& expr13 = binOp6.getLhsExpression();
    ASSERT_EQ(expr13.getType(), ast::ASTNode::Type::Identifier);
    const auto& const4 = static_cast<const ast::Identifier&>(expr13); // NOLINT
    ASSERT_EQ(const4.getIdentifierType(), ast::Identifier::Type::Constant);
    ASSERT_EQ(const4.getId(), 0);

    // Expression: a
    const auto& expr14 = binOp6.getRhsExpression();
    ASSERT_EQ(expr14.getType(), ast::ASTNode::Type::Identifier);
    const auto& var4 = static_cast<const ast::Identifier&>(expr14); // NOLINT
    ASSERT_EQ(var4.getIdentifierType(), ast::Identifier::Type::Parameter);
    ASSERT_EQ(var4.getId(), 0);

    ////////////// Statement 3: a := -x

    ASSERT_EQ(statements[2]->getType(), ast::ASTNode::Type::AssignmentStatement);
    const auto& stmt3 = static_cast<const ast::AssignmentStatement&>(*statements[2]); // NOLINT

    // Assignment target: a
    const auto& stmt3AssignmentTarget = stmt3.getAssignmentTarget();
    ASSERT_EQ(stmt3AssignmentTarget.getType(), ast::ASTNode::Type::Identifier);
    const auto& target3 = static_cast<const ast::Identifier&>(stmt3AssignmentTarget); // NOLINT
    ASSERT_EQ(target3.getIdentifierType(), ast::Identifier::Type::Parameter);
    ASSERT_EQ(target3.getId(), 0);

    // Expression: -x
    const auto& expr15 = stmt3.getExpression();
    ASSERT_EQ(expr15.getType(), ast::ASTNode::Type::UnaryOp);
    const auto& unaryOp1 = static_cast<const ast::UnaryOp&>(expr15); // NOLINT
    ASSERT_EQ(unaryOp1.getUnaryOpType(), ast::UnaryOp::Type::MinusSign);

    // Expression: x
    const auto& expr16 = unaryOp1.getExpression();
    ASSERT_EQ(expr16.getType(), ast::ASTNode::Type::Identifier);
    const auto& var5 = static_cast<const ast::Identifier&>(expr16); // NOLINT
    ASSERT_EQ(var5.getIdentifierType(), ast::Identifier::Type::Variable);
    ASSERT_EQ(var5.getId(), 0);

    ////////////// Statement 4: RETURN +a

    ASSERT_EQ(statements[3]->getType(), ast::ASTNode::Type::ReturnStatement);
    const auto& stmt4 = static_cast<const ast::ReturnStatement&>(*statements[3]); // NOLINT

    // Expression: +a
    const auto& expr17 = stmt4.getExpression();
    ASSERT_EQ(expr17.getType(), ast::ASTNode::Type::UnaryOp);
    const auto& unaryOp2 = static_cast<const ast::UnaryOp&>(expr17); // NOLINT
    ASSERT_EQ(unaryOp2.getUnaryOpType(), ast::UnaryOp::Type::PlusSign);

    // Expression: a
    const auto& expr18 = unaryOp2.getExpression();
    ASSERT_EQ(expr18.getType(), ast::ASTNode::Type::Identifier);
    const auto& var6 = static_cast<const ast::Identifier&>(expr18); // NOLINT
    ASSERT_EQ(var6.getIdentifierType(), ast::Identifier::Type::Parameter);
    ASSERT_EQ(var6.getId(), 0);
}

TEST(TestSemanticAnalysisError, IdentifierDeclaredTwice1) { // NOLINT
    std::string_view code{"PARAM a, b, a;\n"
                          "VAR x, y;\n"
                          "CONST D = 42, F = 39, G = 1024;\n"
                          "BEGIN\n"
                          "  RETURN 1\n"
                          "END.\n"};
    std::string_view expectedErrorMsg{"1:13: error: duplicate declaration of identifier\n"
                                      "PARAM a, b, a;\n"
                                      "            ^\n"
                                      "1:7: note: already declared here\n"
                                      "PARAM a, b, a;\n"
                                      "      ^\n"};
    executeErrorTest(code, expectedErrorMsg);
}

TEST(TestSemanticAnalysisError, IdentifierDeclaredTwice2) { // NOLINT
    std::string_view code{"PARAM a, b, c;\n"
                          "VAR x, y;\n"
                          "CONST D = 42, a = 39, G = 1024;\n"
                          "BEGIN\n"
                          "  RETURN 1\n"
                          "END.\n"};
    std::string_view expectedErrorMsg{"3:15: error: duplicate declaration of identifier\n"
                                      "CONST D = 42, a = 39, G = 1024;\n"
                                      "              ^\n"
                                      "1:7: note: already declared here\n"
                                      "PARAM a, b, c;\n"
                                      "      ^\n"};
    executeErrorTest(code, expectedErrorMsg);
}

TEST(TestSemanticAnalysisError, IdentifierDeclaredTwice3) { // NOLINT
    std::string_view code{"PARAM a, b, c;\n"
                          "VAR x, y;\n"
                          "CONST D = 42, D = 39, G = 1024;\n"
                          "BEGIN\n"
                          "  RETURN 1\n"
                          "END.\n"};
    std::string_view expectedErrorMsg{"3:15: error: duplicate declaration of identifier\n"
                                      "CONST D = 42, D = 39, G = 1024;\n"
                                      "              ^\n"
                                      "3:7: note: already declared here\n"
                                      "CONST D = 42, D = 39, G = 1024;\n"
                                      "      ^\n"};
    executeErrorTest(code, expectedErrorMsg);
}

TEST(TestSemanticAnalysisError, IdentifierDeclaredTwice4) { // NOLINT
    std::string_view code{"PARAM a, b, c;\n"
                          "VAR a, y;\n"
                          "CONST D = 42, E = 39, G = 1024;\n"
                          "BEGIN\n"
                          "  RETURN 1\n"
                          "END.\n"};
    std::string_view expectedErrorMsg{"2:5: error: duplicate declaration of identifier\n"
                                      "VAR a, y;\n"
                                      "    ^\n"
                                      "1:7: note: already declared here\n"
                                      "PARAM a, b, c;\n"
                                      "      ^\n"};
    executeErrorTest(code, expectedErrorMsg);
}

TEST(TestSemanticAnalysisError, AssigningValueToConstant) { // NOLINT
    std::string_view code{"CONST D = 42;\n"
                          "BEGIN\n"
                          "  D := 12;\n"
                          "  RETURN D\n"
                          "END.\n"};
    std::string_view expectedErrorMsg{"3:3: error: trying to assign to an identifier declared 'CONST'\n"
                                      "  D := 12;\n"
                                      "  ^\n"
                                      "1:7: note: declared as 'CONST' here\n"
                                      "CONST D = 42;\n"
                                      "      ^\n"};
    executeErrorTest(code, expectedErrorMsg);
}

TEST(TestSemanticAnalysisError, UseOfUndeclaredIdentifier1) { // NOLINT
    std::string_view code{"BEGIN\n"
                          "  a := 12;\n"
                          "  RETURN a\n"
                          "END.\n"};
    std::string_view expectedErrorMsg{"2:3: error: use of undeclared identifier\n"
                                      "  a := 12;\n"
                                      "  ^\n"};
    executeErrorTest(code, expectedErrorMsg);
}

TEST(TestSemanticAnalysisError, UseOfUndeclaredIdentifier2) { // NOLINT
    std::string_view code{"VAR a;\n"
                          "BEGIN\n"
                          "  a := 12 + X;\n"
                          "  RETURN a\n"
                          "END.\n"};
    std::string_view expectedErrorMsg{"3:13: error: use of undeclared identifier\n"
                                      "  a := 12 + X;\n"
                                      "            ^\n"};
    executeErrorTest(code, expectedErrorMsg);
}

TEST(TestSemanticAnalysisError, UseOfUninitializedIdentifier) { // NOLINT
    std::string_view code{"VAR a;\n"
                          "BEGIN\n"
                          "  RETURN a\n"
                          "END.\n"};
    std::string_view expectedErrorMsg{"3:10: error: use of uninitialized identifier\n"
                                      "  RETURN a\n"
                                      "         ^\n"};
    executeErrorTest(code, expectedErrorMsg);
}

TEST(TestSemanticAnalysisError, MissingReturnStatement) { // NOLINT
    std::string_view code{"VAR a;\n"
                          "BEGIN\n"
                          "  a := 1 + 3;\n"
                          "  a := 4\n"
                          "END.\n"};
    std::string_view expectedErrorMsg{"5:1: error: function does not contain a return-statement\n"
                                      "END.\n"
                                      "^~~\n"};
    executeErrorTest(code, expectedErrorMsg);
}

} // namespace pljit::analysis

