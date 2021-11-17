#include "pljit/analysis/SemanticAnalysis.h"
#include "pljit/ast/AST.h"
#include "pljit/exec/ExecutionContext.h"
#include "test/utils/TestUtils.h"
#include <gtest/gtest.h>

namespace pljit::optim {

TEST(TestDeadCodeElimination, SimpleTest) { // NOLINT
    std::string_view code{"BEGIN\n"
                          "     RETURN 123;\n"
                          "     RETURN 123;\n"
                          "     RETURN 123;\n"
                          "     RETURN 123\n"
                          "END."};

    test_utils::ASTEnvironment env(code, test_utils::Optimization::DeadCodeElimination);
    const auto& ast = env.ast;

    const auto& statements = ast->getStatements();
    ASSERT_EQ(statements.size(), 1);

    ////////////// Statement 1: RETURN 123

    ASSERT_EQ(statements[0]->getType(), ast::ASTNode::Type::ReturnStatement);
    const auto& returnStatement = static_cast<ast::ReturnStatement&>(*statements[0]); // NOLINT

    const auto& expression = returnStatement.getExpression();
    ASSERT_EQ(expression.getType(), ast::ASTNode::Type::ConstantLiteral);
    const auto& constant = static_cast<const ast::ConstantLiteral&>(expression); // NOLINT
    ASSERT_EQ(constant.getValue(), 123);
}

TEST(TestDeadCodeElimination, MoreComplexTest) { // NOLINT
    std::string_view code{"PARAM a, b;\n"
                          "VAR c, d;\n"
                          "BEGIN\n"
                          "     c := a;\n"
                          "     RETURN c;\n"
                          "     d := b;\n"
                          "     RETURN d;"
                          "     RETURN 42\n"
                          "END."};

    test_utils::ASTEnvironment env(code, test_utils::Optimization::DeadCodeElimination);
    const auto& ast = env.ast;

    const auto& statements = ast->getStatements();
    ASSERT_EQ(statements.size(), 2);

    ////////////// Statement 1: c := a

    ASSERT_EQ(statements[0]->getType(), ast::ASTNode::Type::AssignmentStatement);
    const auto& assignmentStatement = static_cast<ast::AssignmentStatement&>(*statements[0]); // NOLINT

    const auto& assignmentTarget = assignmentStatement.getAssignmentTarget();
    ASSERT_EQ(assignmentTarget.getType(), ast::ASTNode::Type::Identifier);
    ASSERT_EQ(assignmentTarget.getIdentifierType(), ast::Identifier::Type::Variable);

    const auto& expression = assignmentStatement.getExpression();
    ASSERT_EQ(expression.getType(), ast::ASTNode::Type::Identifier);
    const auto& identifier = static_cast<const ast::Identifier&>(expression); // NOLINT
    ASSERT_EQ(identifier.getIdentifierType(), ast::Identifier::Type::Parameter);

    ////////////// Statement 2: RETURN c

    ASSERT_EQ(statements[1]->getType(), ast::ASTNode::Type::ReturnStatement);
    const auto& returnStatement = static_cast<ast::ReturnStatement&>(*statements[1]); // NOLINT

    const auto& expression2 = returnStatement.getExpression();
    ASSERT_EQ(expression2.getType(), ast::ASTNode::Type::Identifier);
    const auto& variable = static_cast<const ast::Identifier&>(expression2); // NOLINT
    ASSERT_EQ(variable.getIdentifierType(), ast::Identifier::Type::Variable);
}

TEST(TestDeadCodeElimination, ExecutionTest) { // NOLINT
    std::string_view code{"PARAM a, b;\n"
                          "VAR c;\n"
                          "BEGIN\n"
                          "    c := a * b;\n"
                          "    RETURN c;\n"
                          "    c := c + a * b;\n"
                          "    RETURN c\n"
                          "END.\n"};
    std::vector<int64_t> parameters{1, 2};
    test_utils::ExpectedResultASTExecTest expected{2,
                                                   exec::ExecutionContext::ErrorType::NoError};
    test_utils::performASTExecutionTest(code, parameters,
                                        test_utils::Optimization::DeadCodeElimination,
                                        expected);
}

} // namespace pljit::optim
