#include "pljit/analysis/SemanticAnalysis.h"
#include "pljit/ast/AST.h"
#include "pljit/exec/ExecutionContext.h"
#include "test/utils/TestUtils.h"
#include <gtest/gtest.h>

namespace pljit::optim {

TEST(TestConstantPropagation, SimpleTest) { // NOLINT
    std::string_view code{"VAR x;\n"
                          "BEGIN\n"
                          "     x := 1 + 3 - 2 + 42;\n"
                          "     RETURN x + 12\n"
                          "END."};
    /*
     Should be transformed into:
     VAR x;
     BEGIN
        x := -40;
        RETURN -28
     END.

     Explanation:
     Due to right-to-left associativity and equal operator precedence,
     we can rewrite the statement to (1 + (3 - (2 + 42))) = -40
     */

    test_utils::ASTEnvironment env(code, test_utils::Optimization::ConstantPropagation);
    const auto& ast = env.ast;
    const auto& symbolTable = env.symbolTable;

    test_utils::checkASTVariable(symbolTable, 0, "x");

    const auto& statements = ast->getStatements();
    ASSERT_EQ(statements.size(), 2);

    ////////////// Statement 1: x := -40

    ASSERT_EQ(statements[0]->getType(), ast::ASTNode::Type::AssignmentStatement);
    const auto& assignmentStatement = static_cast<const ast::AssignmentStatement&>(*statements[0]); // NOLINT

    const auto& expression1 = assignmentStatement.getExpression();
    ASSERT_EQ(expression1.getType(), ast::ASTNode::Type::ConstantLiteral);
    const auto& constant1 = static_cast<const ast::ConstantLiteral&>(expression1); // NOLINT
    ASSERT_EQ(constant1.getValue(), -40);

    ////////////// Statement 2: RETURN -28

    ASSERT_EQ(statements[1]->getType(), ast::ASTNode::Type::ReturnStatement);
    const auto& returnStatement = static_cast<const ast::ReturnStatement&>(*statements[1]); // NOLINT

    const auto& expression2 = returnStatement.getExpression();
    ASSERT_EQ(expression2.getType(), ast::ASTNode::Type::ConstantLiteral);
    const auto& constant2 = static_cast<const ast::ConstantLiteral&>(expression2); // NOLINT
    ASSERT_EQ(constant2.getValue(), -28);
}

TEST(TestConstantPropagation, TestWithSubexpressionConstantPropagation) { // NOLINT
    std::string_view code{"PARAM a;\n"
                          "VAR b;\n"
                          "BEGIN\n"
                          "    b := 100 * 1 + 2;\n"
                          "    a := a + b - 23;\n"
                          "    b := a * 100;\n"
                          "    RETURN b\n"
                          "END."};
    /*
     Should be transformed into:
     PARAM a;
     VAR b;
     BEGIN
        b := 102;
        a := a + 79;
        b := a * 100;
        RETURN b
     END.
     */

    test_utils::ASTEnvironment env(code, test_utils::Optimization::ConstantPropagation);
    const auto& ast = env.ast;
    const auto& symbolTable = env.symbolTable;

    test_utils::checkASTParameter(symbolTable, 0, "a");
    test_utils::checkASTVariable(symbolTable, 0, "b");

    const auto& statements = ast->getStatements();
    ASSERT_EQ(statements.size(), 4);

    ////////////// Statement 1: b := 102

    ASSERT_EQ(statements[0]->getType(), ast::ASTNode::Type::AssignmentStatement);
    const auto& stmt1 = static_cast<const ast::AssignmentStatement&>(*statements[0]); // NOLINT

    const auto& expression1 = stmt1.getExpression();
    ASSERT_EQ(expression1.getType(), ast::ASTNode::Type::ConstantLiteral);
    const auto& constant1 = static_cast<const ast::ConstantLiteral&>(expression1); // NOLINT
    ASSERT_EQ(constant1.getValue(), 102);

    ////////////// Statement 2: a := a + 79

    ASSERT_EQ(statements[1]->getType(), ast::ASTNode::Type::AssignmentStatement);
    const auto& stmt2 = static_cast<const ast::AssignmentStatement&>(*statements[1]); // NOLINT

    const auto& expression2 = stmt2.getExpression();
    ASSERT_EQ(expression2.getType(), ast::ASTNode::Type::BinaryOp);
    const auto& binOp1 = static_cast<const ast::BinaryOp&>(expression2); // NOLINT
    ASSERT_EQ(binOp1.getBinaryOpType(), ast::BinaryOp::Type::Add);

    const auto& expression3 = binOp1.getLhsExpression();
    ASSERT_EQ(expression3.getType(), ast::ASTNode::Type::Identifier);
    const auto& param1 = static_cast<const ast::Identifier&>(expression3); // NOLINT
    ASSERT_EQ(param1.getIdentifierType(), ast::Identifier::Type::Parameter);
    ASSERT_EQ(param1.getId(), 0);

    const auto& expression4 = binOp1.getRhsExpression();
    ASSERT_EQ(expression4.getType(), ast::ASTNode::Type::ConstantLiteral);
    const auto& constant2 = static_cast<const ast::ConstantLiteral&>(expression4); // NOLINT
    ASSERT_EQ(constant2.getValue(), 79);

    ////////////// Statement 3: b := a * 100

    ASSERT_EQ(statements[2]->getType(), ast::ASTNode::Type::AssignmentStatement);
    const auto& stmt3 = static_cast<const ast::AssignmentStatement&>(*statements[2]); // NOLINT

    const auto& expression5 = stmt3.getExpression();
    ASSERT_EQ(expression5.getType(), ast::ASTNode::Type::BinaryOp);
    const auto& binOp2 = static_cast<const ast::BinaryOp&>(expression5); // NOLINT
    ASSERT_EQ(binOp2.getBinaryOpType(), ast::BinaryOp::Type::Mul);

    const auto& expression6 = binOp2.getLhsExpression();
    ASSERT_EQ(expression6.getType(), ast::ASTNode::Type::Identifier);
    const auto& param2 = static_cast<const ast::Identifier&>(expression6); // NOLINT
    ASSERT_EQ(param2.getIdentifierType(), ast::Identifier::Type::Parameter);
    ASSERT_EQ(param2.getId(), 0);

    const auto& expression7 = binOp2.getRhsExpression();
    ASSERT_EQ(expression7.getType(), ast::ASTNode::Type::ConstantLiteral);
    const auto& constant3 = static_cast<const ast::ConstantLiteral&>(expression7); // NOLINT
    ASSERT_EQ(constant3.getValue(), 100);

    ////////////// Statement 4: RETURN b

    ASSERT_EQ(statements[3]->getType(), ast::ASTNode::Type::ReturnStatement);
    const auto& stmt4 = static_cast<const ast::ReturnStatement&>(*statements[3]); // NOLINT

    const auto& expression8 = stmt4.getExpression();
    ASSERT_EQ(expression8.getType(), ast::ASTNode::Type::Identifier);
    const auto& var1 = static_cast<const ast::Identifier&>(expression8); // NOLINT
    ASSERT_EQ(var1.getIdentifierType(), ast::Identifier::Type::Variable);
    ASSERT_EQ(var1.getId(), 0);
}

TEST(TestConstantPropagation, EventuallyAllConstants) { // NOLINT
    std::string_view code{"PARAM a, b;\n"
                          "VAR c;\n"
                          "BEGIN\n"
                          "      c := 100;\n"
                          "      a := c;\n"
                          "      b := a;\n"
                          "      RETURN a + b\n"
                          "END."};
    /*
     Should be transformed into:
     PARAM a, b;
     VAR c;
     BEGIN
        c := 100;
        a := 100;
        b := 100;
        RETURN 200
     END.
     */

    test_utils::ASTEnvironment env(code, test_utils::Optimization::ConstantPropagation);
    const auto& ast = env.ast;
    const auto& symbolTable = env.symbolTable;

    test_utils::checkASTParameter(symbolTable, 0, "a");
    test_utils::checkASTParameter(symbolTable, 1, "b");
    test_utils::checkASTVariable(symbolTable, 0, "c");

    const auto& statements = ast->getStatements();
    ASSERT_EQ(statements.size(), 4);

    ////////////// Statement 1: c := 100

    ASSERT_EQ(statements[0]->getType(), ast::ASTNode::Type::AssignmentStatement);
    const auto& stmt1 = static_cast<const ast::AssignmentStatement&>(*statements[0]); // NOLINT

    const auto& expression1 = stmt1.getExpression();
    ASSERT_EQ(expression1.getType(), ast::ASTNode::Type::ConstantLiteral);
    const auto& constant1 = static_cast<const ast::ConstantLiteral&>(expression1); // NOLINT
    ASSERT_EQ(constant1.getValue(), 100);

    ////////////// Statement 2: a := 100

    ASSERT_EQ(statements[1]->getType(), ast::ASTNode::Type::AssignmentStatement);
    const auto& stmt2 = static_cast<const ast::AssignmentStatement&>(*statements[1]); // NOLINT

    const auto& expression2 = stmt2.getExpression();
    ASSERT_EQ(expression2.getType(), ast::ASTNode::Type::ConstantLiteral);
    const auto& constant2 = static_cast<const ast::ConstantLiteral&>(expression2); // NOLINT
    ASSERT_EQ(constant2.getValue(), 100);

    ////////////// Statement 3: b := 100

    ASSERT_EQ(statements[2]->getType(), ast::ASTNode::Type::AssignmentStatement);
    const auto& stmt3 = static_cast<const ast::AssignmentStatement&>(*statements[2]); // NOLINT

    const auto& expression3 = stmt3.getExpression();
    ASSERT_EQ(expression3.getType(), ast::ASTNode::Type::ConstantLiteral);
    const auto& constant3 = static_cast<const ast::ConstantLiteral&>(expression3); // NOLINT
    ASSERT_EQ(constant3.getValue(), 100);

    ////////////// Statement 4: RETURN 200

    ASSERT_EQ(statements[3]->getType(), ast::ASTNode::Type::ReturnStatement);
    const auto& stmt4 = static_cast<const ast::ReturnStatement&>(*statements[3]); // NOLINT

    const auto& expression4 = stmt4.getExpression();
    ASSERT_EQ(expression4.getType(), ast::ASTNode::Type::ConstantLiteral);
    const auto& constant4 = static_cast<const ast::ConstantLiteral&>(expression4); // NOLINT
    ASSERT_EQ(constant4.getValue(), 200);
}

TEST(TestConstantPropagation, AlternatingVariableAndConstantAssignment) { // NOLINT
    std::string_view code{"PARAM a, b;\n"
                          "VAR c, d;\n"
                          "\n"
                          "BEGIN\n"
                          "    c := 1;\n"
                          "    d := 2;\n"
                          "    c := a + b;\n"
                          "    d := c + a + b;\n"
                          "    c := 3;\n"
                          "    d := 4;\n"
                          "    RETURN c + d\n"
                          "END."};
    /*
    Should be transformed into:

        PARAM a, b;
        VAR c, d;

        BEGIN
            c := 1;
            d := 2;
            c := a + b;
            d := c + a + b;
            c := 3;
            d := 4;
            RETURN 7
        END.
     */

    test_utils::ASTEnvironment env(code, test_utils::Optimization::ConstantPropagation);
    const auto& ast = env.ast;
    const auto& symbolTable = env.symbolTable;

    test_utils::checkASTParameter(symbolTable, 0, "a");
    test_utils::checkASTParameter(symbolTable, 1, "b");
    test_utils::checkASTVariable(symbolTable, 0, "c");
    test_utils::checkASTVariable(symbolTable, 1, "d");

    const auto& statements = ast->getStatements();
    ASSERT_EQ(statements.size(), 7);

    ////////////// Statement 1: c := 1

    ASSERT_EQ(statements[0]->getType(), ast::ASTNode::Type::AssignmentStatement);
    const auto& stmt1 = static_cast<const ast::AssignmentStatement&>(*statements[0]); // NOLINT

    const auto& expression1 = stmt1.getExpression();
    ASSERT_EQ(expression1.getType(), ast::ASTNode::Type::ConstantLiteral);
    const auto& constant1 = static_cast<const ast::ConstantLiteral&>(expression1); // NOLINT
    ASSERT_EQ(constant1.getValue(), 1);

    ////////////// Statement 2: d := 2

    ASSERT_EQ(statements[1]->getType(), ast::ASTNode::Type::AssignmentStatement);
    const auto& stmt2 = static_cast<const ast::AssignmentStatement&>(*statements[1]); // NOLINT

    const auto& expression2 = stmt2.getExpression();
    ASSERT_EQ(expression2.getType(), ast::ASTNode::Type::ConstantLiteral);
    const auto& constant2 = static_cast<const ast::ConstantLiteral&>(expression2); // NOLINT
    ASSERT_EQ(constant2.getValue(), 2);

    ////////////// Statement 3: c := a + b

    ASSERT_EQ(statements[2]->getType(), ast::ASTNode::Type::AssignmentStatement);
    const auto& stmt3 = static_cast<const ast::AssignmentStatement&>(*statements[2]); // NOLINT

    const auto& expression3 = stmt3.getExpression();
    ASSERT_EQ(expression3.getType(), ast::ASTNode::Type::BinaryOp);
    const auto& binOp1 = static_cast<const ast::BinaryOp&>(expression3); // NOLINT
    ASSERT_EQ(binOp1.getBinaryOpType(), ast::BinaryOp::Type::Add);

    const auto& expression4 = binOp1.getLhsExpression();
    ASSERT_EQ(expression4.getType(), ast::ASTNode::Type::Identifier);
    const auto& param1 = static_cast<const ast::Identifier&>(expression4); // NOLINT
    ASSERT_EQ(param1.getIdentifierType(), ast::Identifier::Type::Parameter);
    ASSERT_EQ(param1.getId(), 0);

    const auto& expression5 = binOp1.getRhsExpression();
    ASSERT_EQ(expression5.getType(), ast::ASTNode::Type::Identifier);
    const auto& param2 = static_cast<const ast::Identifier&>(expression5); // NOLINT
    ASSERT_EQ(param2.getIdentifierType(), ast::Identifier::Type::Parameter);
    ASSERT_EQ(param2.getId(), 1);

    ////////////// Statement 4: d := c + a + b

    ASSERT_EQ(statements[3]->getType(), ast::ASTNode::Type::AssignmentStatement);
    const auto& stmt4 = static_cast<const ast::AssignmentStatement&>(*statements[3]); // NOLINT

    const auto& expression6 = stmt4.getExpression();
    ASSERT_EQ(expression6.getType(), ast::ASTNode::Type::BinaryOp);
    const auto& binOp2 = static_cast<const ast::BinaryOp&>(expression6); // NOLINT
    ASSERT_EQ(binOp2.getBinaryOpType(), ast::BinaryOp::Type::Add);

    const auto& expression7 = binOp2.getLhsExpression();
    ASSERT_EQ(expression7.getType(), ast::ASTNode::Type::Identifier);
    const auto& var1 = static_cast<const ast::Identifier&>(expression7); // NOLINT
    ASSERT_EQ(var1.getIdentifierType(), ast::Identifier::Type::Variable);
    ASSERT_EQ(var1.getId(), 0);

    const auto& expression8 = binOp2.getRhsExpression();
    ASSERT_EQ(expression8.getType(), ast::ASTNode::Type::BinaryOp);
    const auto& binOp3 = static_cast<const ast::BinaryOp&>(expression8); // NOLINT
    ASSERT_EQ(binOp3.getBinaryOpType(), ast::BinaryOp::Type::Add);

    const auto& expression9 = binOp3.getLhsExpression();
    ASSERT_EQ(expression9.getType(), ast::ASTNode::Type::Identifier);
    const auto& param3 = static_cast<const ast::Identifier&>(expression9); // NOLINT
    ASSERT_EQ(param3.getIdentifierType(), ast::Identifier::Type::Parameter);
    ASSERT_EQ(param3.getId(), 0);

    const auto& expression10 = binOp3.getRhsExpression();
    ASSERT_EQ(expression10.getType(), ast::ASTNode::Type::Identifier);
    const auto& param4 = static_cast<const ast::Identifier&>(expression10); // NOLINT
    ASSERT_EQ(param4.getIdentifierType(), ast::Identifier::Type::Parameter);
    ASSERT_EQ(param4.getId(), 1);

    ////////////// Statement 5: c := 3

    ASSERT_EQ(statements[4]->getType(), ast::ASTNode::Type::AssignmentStatement);
    const auto& stmt5 = static_cast<const ast::AssignmentStatement&>(*statements[4]); // NOLINT

    const auto& expression11 = stmt5.getExpression();
    ASSERT_EQ(expression11.getType(), ast::ASTNode::Type::ConstantLiteral);
    const auto& constant3 = static_cast<const ast::ConstantLiteral&>(expression11); // NOLINT
    ASSERT_EQ(constant3.getValue(), 3);

    ////////////// Statement 6: c := 4

    ASSERT_EQ(statements[5]->getType(), ast::ASTNode::Type::AssignmentStatement);
    const auto& stmt6 = static_cast<ast::AssignmentStatement&>(*statements[5]); // NOLINT

    const auto& expression12 = stmt6.getExpression();
    ASSERT_EQ(expression12.getType(), ast::ASTNode::Type::ConstantLiteral);
    const auto& constant4 = static_cast<const ast::ConstantLiteral&>(expression12); // NOLINT
    ASSERT_EQ(constant4.getValue(), 4);

    ////////////// Statement 6: RETURN 7

    ASSERT_EQ(statements[6]->getType(), ast::ASTNode::Type::ReturnStatement);
    const auto& stmt7 = static_cast<const ast::AssignmentStatement&>(*statements[6]); // NOLINT

    const auto& expression13 = stmt7.getExpression();
    ASSERT_EQ(expression13.getType(), ast::ASTNode::Type::ConstantLiteral);
    const auto& constant5 = static_cast<const ast::ConstantLiteral&>(expression13); // NOLINT
    ASSERT_EQ(constant5.getValue(), 7);
}

TEST(TestConstantPropagation, SubExpressionConstantPropagation) { // NOLINT
    std::string_view code{"PARAM a, b;\n"
                          "VAR x;\n"
                          "\n"
                          "BEGIN\n"
                          "    x := a * (1 + 3 * 4 - 3) - 3 + b - 4 + 2;\n"
                          "    RETURN a + 1 - x + 3 * 2\n"
                          "END."};
    /*
    Should be transformed into:

    PARAM a, b;
    VAR x;

    BEGIN
        x := a * 10 - 3 + b - 6;
        RETURN a + 1 - x + 6
    END.
    */

    test_utils::ASTEnvironment env(code, test_utils::Optimization::ConstantPropagation);
    const auto& ast = env.ast;
    const auto& symbolTable = env.symbolTable;

    test_utils::checkASTParameter(symbolTable, 0, "a");
    test_utils::checkASTParameter(symbolTable, 1, "b");
    test_utils::checkASTVariable(symbolTable, 0, "x");

    const auto& statements = ast->getStatements();
    ASSERT_EQ(statements.size(), 2);

    ////////////// Statement 1: x := a * 10 - 3 + b - 6

    ASSERT_EQ(statements[0]->getType(), ast::ASTNode::Type::AssignmentStatement);
    const auto& stmt1 = static_cast<const ast::AssignmentStatement&>(*statements[0]); // NOLINT

    const auto& expression1 = stmt1.getExpression();
    ASSERT_EQ(expression1.getType(), ast::ASTNode::Type::BinaryOp);
    const auto& binOp1 = static_cast<const ast::BinaryOp&>(expression1); // NOLINT
    ASSERT_EQ(binOp1.getBinaryOpType(), ast::BinaryOp::Type::Sub);

    // a * 10
    const auto& expression2 = binOp1.getLhsExpression();
    ASSERT_EQ(expression2.getType(), ast::ASTNode::Type::BinaryOp);
    const auto& binOp2 = static_cast<const ast::BinaryOp&>(expression2); // NOLINT
    ASSERT_EQ(binOp2.getBinaryOpType(), ast::BinaryOp::Type::Mul);

    // a
    const auto& expression3 = binOp2.getLhsExpression();
    ASSERT_EQ(expression3.getType(), ast::ASTNode::Type::Identifier);
    const auto& param1 = static_cast<const ast::Identifier&>(expression3); // NOLINT
    ASSERT_EQ(param1.getIdentifierType(), ast::Identifier::Type::Parameter);
    ASSERT_EQ(param1.getId(), 0);

    // 10
    const auto& expression4 = binOp2.getRhsExpression();
    ASSERT_EQ(expression4.getType(), ast::ASTNode::Type::ConstantLiteral);
    const auto& constant1 = static_cast<const ast::ConstantLiteral&>(expression4); // NOLINT
    ASSERT_EQ(constant1.getValue(), 10);

    // 3 + b - 6
    const auto& expression5 = binOp1.getRhsExpression();
    ASSERT_EQ(expression5.getType(), ast::ASTNode::Type::BinaryOp);
    const auto& binOp3 = static_cast<const ast::BinaryOp&>(expression5); // NOLINT
    ASSERT_EQ(binOp3.getBinaryOpType(), ast::BinaryOp::Type::Add);

    // 3
    const auto& expression6 = binOp3.getLhsExpression();
    ASSERT_EQ(expression6.getType(), ast::ASTNode::Type::ConstantLiteral);
    const auto& constant2 = static_cast<const ast::ConstantLiteral&>(expression6); // NOLINT
    ASSERT_EQ(constant2.getValue(), 3);

    // b - 6
    const auto& expression7 = binOp3.getRhsExpression();
    ASSERT_EQ(expression7.getType(), ast::ASTNode::Type::BinaryOp);
    const auto& binOp4 = static_cast<const ast::BinaryOp&>(expression7); // NOLINT
    ASSERT_EQ(binOp4.getBinaryOpType(), ast::BinaryOp::Type::Sub);

    // b
    const auto& expression8 = binOp4.getLhsExpression();
    ASSERT_EQ(expression8.getType(), ast::ASTNode::Type::Identifier);
    const auto& param2 = static_cast<const ast::Identifier&>(expression8); // NOLINT
    ASSERT_EQ(param2.getIdentifierType(), ast::Identifier::Type::Parameter);
    ASSERT_EQ(param2.getId(), 1);

    // 6
    const auto& expression9 = binOp4.getRhsExpression();
    ASSERT_EQ(expression9.getType(), ast::ASTNode::Type::ConstantLiteral);
    const auto& constant3 = static_cast<const ast::ConstantLiteral&>(expression9); // NOLINT
    ASSERT_EQ(constant3.getValue(), 6);

    ////////////// Statement 2: RETURN a + 1 - x + 6

    ASSERT_EQ(statements[1]->getType(), ast::ASTNode::Type::ReturnStatement);
    const auto& stmt2 = static_cast<const ast::ReturnStatement&>(*statements[1]); // NOLINT

    const auto& expression10 = stmt2.getExpression();
    ASSERT_EQ(expression10.getType(), ast::ASTNode::Type::BinaryOp);
    const auto& binOp5 = static_cast<const ast::BinaryOp&>(expression10); // NOLINT
    ASSERT_EQ(binOp5.getBinaryOpType(), ast::BinaryOp::Type::Add);

    // a
    const auto& expression11 = binOp5.getLhsExpression();
    ASSERT_EQ(expression11.getType(), ast::ASTNode::Type::Identifier);
    const auto& param3 = static_cast<const ast::Identifier&>(expression11); // NOLINT
    ASSERT_EQ(param3.getIdentifierType(), ast::Identifier::Type::Parameter);
    ASSERT_EQ(param3.getId(), 0);

    // 1 - x + 6
    const auto& expression12 = binOp5.getRhsExpression();
    ASSERT_EQ(expression12.getType(), ast::ASTNode::Type::BinaryOp);
    const auto& binOp6 = static_cast<const ast::BinaryOp&>(expression12); // NOLINT
    ASSERT_EQ(binOp6.getBinaryOpType(), ast::BinaryOp::Type::Sub);

    // 1
    const auto& expression13 = binOp6.getLhsExpression();
    ASSERT_EQ(expression13.getType(), ast::ASTNode::Type::ConstantLiteral);
    const auto& constant4 = static_cast<const ast::ConstantLiteral&>(expression13); // NOLINT
    ASSERT_EQ(constant4.getValue(), 1);

    // x + 6
    const auto& expression14 = binOp6.getRhsExpression();
    ASSERT_EQ(expression14.getType(), ast::ASTNode::Type::BinaryOp);
    const auto& binOp7 = static_cast<const ast::BinaryOp&>(expression14); // NOLINT
    ASSERT_EQ(binOp7.getBinaryOpType(), ast::BinaryOp::Type::Add);

    // x
    const auto& expression15 = binOp7.getLhsExpression();
    ASSERT_EQ(expression15.getType(), ast::ASTNode::Type::Identifier);
    const auto& var1 = static_cast<const ast::Identifier&>(expression15); // NOLINT
    ASSERT_EQ(var1.getIdentifierType(), ast::Identifier::Type::Variable);
    ASSERT_EQ(var1.getId(), 0);

    // 6
    const auto& expression16 = binOp7.getRhsExpression();
    ASSERT_EQ(expression16.getType(), ast::ASTNode::Type::ConstantLiteral);
    const auto& constant5 = static_cast<const ast::ConstantLiteral&>(expression16); // NOLINT
    ASSERT_EQ(constant5.getValue(), 6);
}

TEST(TestConstantPropagation, DivisionByZero) { // NOLINT
    std::string_view code{"BEGIN\n"
                          "    RETURN 1 / 0\n"
                          "END."};

    test_utils::ASTEnvironment env(code, test_utils::Optimization::ConstantPropagation);
    const auto& ast = env.ast;

    const auto& statements = ast->getStatements();
    ASSERT_EQ(statements.size(), 1);

    ////////////// Statement 1: RETURN 1 / 0

    ASSERT_EQ(statements[0]->getType(), ast::ASTNode::Type::ReturnStatement);
    const auto& stmt = static_cast<const ast::ReturnStatement&>(*statements[0]); // NOLINT

    const auto& expression1 = stmt.getExpression();
    ASSERT_EQ(expression1.getType(), ast::ReturnStatement::Type::BinaryOp);
    const auto& binOp = static_cast<const ast::BinaryOp&>(expression1); // NOLINT
    ASSERT_EQ(binOp.getBinaryOpType(), ast::BinaryOp::Type::Div);

    const auto& expression2 = binOp.getLhsExpression();
    ASSERT_EQ(expression2.getType(), ast::ASTNode::Type::ConstantLiteral);
    const auto& constant1 = static_cast<const ast::ConstantLiteral&>(expression2); // NOLINT
    ASSERT_EQ(constant1.getValue(), 1);

    const auto& expression3 = binOp.getRhsExpression();
    ASSERT_EQ(expression3.getType(), ast::ASTNode::Type::ConstantLiteral);
    const auto& constant2 = static_cast<const ast::ConstantLiteral&>(expression3); // NOLINT
    ASSERT_EQ(constant2.getValue(), 0);
}

TEST(TestConstantPropagation, ConstVariables) { // NOLINT
    std::string_view code{"CONST A = 10, B = 10;\n"
                          "BEGIN\n"
                          "    RETURN A + B\n"
                          "END.\n"};
    /*
    Should be transformed into:

    CONST A = 10, B = 10;

    BEGIN
        RETURN A + B
    END.
    */

    test_utils::ASTEnvironment env(code, test_utils::Optimization::ConstantPropagation);
    const auto& ast = env.ast;

    const auto& statements = ast->getStatements();
    ASSERT_EQ(statements.size(), 1);

    ////////////// Statement 1: RETURN 20

    ASSERT_EQ(statements[0]->getType(), ast::ASTNode::Type::ReturnStatement);
    const auto& stmt = static_cast<const ast::ReturnStatement&>(*statements[0]); // NOLINT

    const auto& expression = stmt.getExpression();
    ASSERT_EQ(expression.getType(), ast::ASTNode::Type::ConstantLiteral);
    const auto& constLiteral = static_cast<const ast::ConstantLiteral&>(expression); // NOLINT
    ASSERT_EQ(constLiteral.getValue(), 20);
}

TEST(TestConstantPropagation, MixtureOfConstVariablesAndConstantLiterals) { // NOLINT
    std::string_view code{"PARAM a;\n"
                          "VAR x, y;\n"
                          "CONST A = 10, B = 5;\n"
                          "BEGIN\n"
                          "  x := 10 + 5;\n"
                          "  y := x + A - B;\n"
                          "  x := B * a + A - 2 + B;"
                          "  y := A;\n"
                          "  RETURN y + A + B\n"
                          "END.\n"};
    /*
    Should be transformed into:

    PARAM a;
    VAR x, y;
    CONST A = 10, B = 5;

    BEGIN
        x := 15;
        y := 20;
        x := 5 * a + 3;
        y := 10;
        RETURN 25
    END.
    */

    test_utils::ASTEnvironment env(code, test_utils::Optimization::ConstantPropagation);
    const auto& ast = env.ast;

    const auto& statements = ast->getStatements();
    ASSERT_EQ(statements.size(), 5);

    ////////////// Statement 1: x := 15

    ASSERT_EQ(statements[0]->getType(), ast::ASTNode::Type::AssignmentStatement);
    const auto& stmt1 = static_cast<const ast::AssignmentStatement&>(*statements[0]); // NOLINT

    const auto& assignmentTarget1 = stmt1.getAssignmentTarget();
    ASSERT_EQ(assignmentTarget1.getIdentifierType(), ast::Identifier::Type::Variable);

    const auto& expression1 = stmt1.getExpression();
    ASSERT_EQ(expression1.getType(), ast::ASTNode::Type::ConstantLiteral);
    const auto& constLiteral1 = static_cast<const ast::ConstantLiteral&>(expression1); // NOLINT
    ASSERT_EQ(constLiteral1.getValue(), 15);

    ////////////// Statement 2: y := 20

    ASSERT_EQ(statements[1]->getType(), ast::ASTNode::Type::AssignmentStatement);
    const auto& stmt2 = static_cast<const ast::AssignmentStatement&>(*statements[1]); // NOLINT

    const auto& assignmentTarget2 = stmt2.getAssignmentTarget();
    ASSERT_EQ(assignmentTarget2.getIdentifierType(), ast::Identifier::Type::Variable);

    const auto& expression2 = stmt2.getExpression();
    ASSERT_EQ(expression2.getType(), ast::ASTNode::Type::ConstantLiteral);
    const auto& constLiteral2 = static_cast<const ast::ConstantLiteral&>(expression2); // NOLINT
    ASSERT_EQ(constLiteral2.getValue(), 20);

    ////////////// Statement 3: x := 5 * a + 3

    ASSERT_EQ(statements[2]->getType(), ast::ASTNode::Type::AssignmentStatement);
    const auto& stmt3 = static_cast<const ast::AssignmentStatement&>(*statements[2]); // NOLINT

    const auto& assignmentTarget3 = stmt3.getAssignmentTarget();
    ASSERT_EQ(assignmentTarget3.getIdentifierType(), ast::Identifier::Type::Variable);

    const auto& expression3 = stmt3.getExpression();
    ASSERT_EQ(expression3.getType(), ast::ASTNode::Type::BinaryOp);
    const auto& binOp1 = static_cast<const ast::BinaryOp&>(expression3); // NOLINT
    ASSERT_EQ(binOp1.getBinaryOpType(), ast::BinaryOp::Type::Add);

    const auto& expression4 = binOp1.getLhsExpression();
    ASSERT_EQ(expression4.getType(), ast::ASTNode::Type::BinaryOp);
    const auto& binOp2 = static_cast<const ast::BinaryOp&>(expression4); // NOLINT
    ASSERT_EQ(binOp2.getBinaryOpType(), ast::BinaryOp::Type::Mul);

    const auto& expression5 = binOp2.getLhsExpression();
    ASSERT_EQ(expression5.getType(), ast::ASTNode::Type::ConstantLiteral);
    const auto& constLiteral3 = static_cast<const ast::ConstantLiteral&>(expression5); // NOLINT
    ASSERT_EQ(constLiteral3.getValue(), 5);

    const auto& expression6 = binOp2.getRhsExpression();
    ASSERT_EQ(expression6.getType(), ast::ASTNode::Type::Identifier);
    const auto& param = static_cast<const ast::Identifier&>(expression6); // NOLINT
    ASSERT_EQ(param.getIdentifierType(), ast::Identifier::Type::Parameter);

    const auto& expression7 = binOp1.getRhsExpression();
    ASSERT_EQ(expression7.getType(), ast::ASTNode::Type::ConstantLiteral);
    const auto& constLiteral4 = static_cast<const ast::ConstantLiteral&>(expression7); // NOLINT
    ASSERT_EQ(constLiteral4.getValue(), 3);

    ////////////// Statement 4: y := 10

    ASSERT_EQ(statements[3]->getType(), ast::ASTNode::Type::AssignmentStatement);
    const auto& stmt4 = static_cast<const ast::AssignmentStatement&>(*statements[3]); // NOLINT

    const auto& assignmentTarget4 = stmt4.getAssignmentTarget();
    ASSERT_EQ(assignmentTarget4.getIdentifierType(), ast::Identifier::Type::Variable);

    const auto& expression8 = stmt4.getExpression();
    ASSERT_EQ(expression8.getType(), ast::ASTNode::Type::ConstantLiteral);
    const auto& constLiteral5 = static_cast<const ast::ConstantLiteral&>(expression8); // NOLINT
    ASSERT_EQ(constLiteral5.getValue(), 10);

    ////////////// Statement 5: RETURN 25

    ASSERT_EQ(statements[4]->getType(), ast::ASTNode::Type::ReturnStatement);
    const auto& stmt5 = static_cast<const ast::ReturnStatement&>(*statements[4]); // NOLINT

    const auto& expression9 = stmt5.getExpression();
    ASSERT_EQ(expression9.getType(), ast::ASTNode::Type::ConstantLiteral);
    const auto& constLiteral6 = static_cast<const ast::ConstantLiteral&>(expression9); // NOLINT
    ASSERT_EQ(constLiteral6.getValue(), 25);
}

TEST(TestConstantPropagation, ExecutionTest) { // NOLINT
    std::string_view code{"PARAM a, b;\n"
                          "VAR g;\n"
                          "CONST X = 12, Y = 10;\n"
                          "BEGIN\n"
                          "   g := X + Y;\n"
                          "   a := g * X - Y;\n"
                          "   RETURN a * b + g\n"
                          "END.\n"};
    std::vector<int64_t> parameters{1, 2};
    test_utils::ExpectedResultASTExecTest expected{530,
                                                   exec::ExecutionContext::ErrorType::NoError};
    test_utils::performASTExecutionTest(code, parameters,
                                        test_utils::Optimization::ConstantPropagation,
                                        expected);
}

} // namespace pljit::optim
