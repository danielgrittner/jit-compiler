#include "pljit/common/SourceCodeManager.h"
#include "pljit/parse_tree/ParseTree.h"
#include "pljit/parser/Parser.h"
#include <gtest/gtest.h>

namespace pljit::parser {

TEST(TestParser, SimpleTest) { // NOLINT
    std::string_view code{"BEGIN\n"
                          "\tRETURN 123\n"
                          "END\n"
                          "."};

    common::SourceCodeManager sourceCodeManager(std::string{code});
    Parser parser(sourceCodeManager);

    auto functionDefinition = parser.parseFunctionDefinition();
    ASSERT_FALSE(functionDefinition == nullptr);

    // function-definition = compound-statement "."
    ASSERT_EQ(functionDefinition->getType(), parse_tree::ParseTreeNode::Type::FunctionDefinition);
    ASSERT_EQ(functionDefinition->getReference(), code);

    ASSERT_FALSE(functionDefinition->getParameterDeclarations());
    ASSERT_FALSE(functionDefinition->getVariableDeclarations());
    ASSERT_FALSE(functionDefinition->getConstantDeclarations());

    ///////////////// Compound statement

    // compound-statement = BEGIN statement-list END
    const auto& compoundStatement = functionDefinition->getCompoundStatement();
    ASSERT_EQ(compoundStatement.getType(), parse_tree::ParseTreeNode::Type::CompoundStatement);
    ASSERT_EQ(compoundStatement.getReference(), code.substr(0, code.size() - 2));

    // Generic token = BEGIN
    ASSERT_EQ(compoundStatement.getBeginKeyword().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(compoundStatement.getBeginKeyword().getReference(), code.substr(0, 5));

    // statement-list = statement
    const auto& statementList = compoundStatement.getStatementList();
    ASSERT_EQ(statementList.getType(), parse_tree::ParseTreeNode::Type::StatementList);
    ASSERT_EQ(statementList.getReference(), code.substr(7, 10));

    const auto& statementListChildren = statementList.getStatementsSeparatedBySemiColon();
    ASSERT_EQ(statementListChildren.size(), 1);

    // statement = RETURN 123
    ASSERT_EQ(statementListChildren[0]->getType(), parse_tree::ParseTreeNode::Type::Statement);
    ASSERT_EQ(statementListChildren[0]->getReference(), code.substr(7, 10));
    const auto& returnStatement = static_cast<parse_tree::Statement&>(*statementListChildren[0]); // NOLINT
    ASSERT_EQ(returnStatement.getStatementType(), parse_tree::Statement::Type::ReturnStatement);

    // generic-token = RETURN
    ASSERT_EQ(returnStatement.getReturnKeyword().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(returnStatement.getReturnKeyword().getReference(), code.substr(7, 6));

    // additive-expression = 123
    ASSERT_EQ(returnStatement.getAdditiveExpression().getType(), parse_tree::ParseTreeNode::Type::AdditiveExpression);
    ASSERT_EQ(returnStatement.getAdditiveExpression().getReference(), code.substr(14, 3));
    const auto& additiveExpr = returnStatement.getAdditiveExpression();
    ASSERT_EQ(additiveExpr.getAdditiveExpressionType(), parse_tree::AdditiveExpression::Type::None);

    ASSERT_EQ(additiveExpr.getMultiplicativeExpression().getType(), parse_tree::ParseTreeNode::Type::MultiplicativeExpression);
    ASSERT_EQ(additiveExpr.getMultiplicativeExpression().getReference(), code.substr(14, 3));
    const auto& mulExpr = additiveExpr.getMultiplicativeExpression();
    ASSERT_EQ(mulExpr.getMultiplicativeExpressionType(), parse_tree::MultiplicativeExpression::Type::None);

    ASSERT_EQ(mulExpr.getUnaryExpression().getType(), parse_tree::ParseTreeNode::Type::UnaryExpression);
    ASSERT_EQ(mulExpr.getUnaryExpression().getReference(), code.substr(14, 3));
    const auto& unaryExpr = mulExpr.getUnaryExpression();
    ASSERT_EQ(unaryExpr.getUnaryExpressionType(), parse_tree::UnaryExpression::Type::Unsigned);

    ASSERT_EQ(unaryExpr.getPrimaryExpression().getType(), parse_tree::ParseTreeNode::Type::PrimaryExpression);
    ASSERT_EQ(unaryExpr.getPrimaryExpression().getReference(), code.substr(14, 3));
    const auto& primaryExpr = unaryExpr.getPrimaryExpression();

    ASSERT_EQ(primaryExpr.getPrimaryExpressionType(), parse_tree::PrimaryExpression::Type::Literal);

    ASSERT_EQ(primaryExpr.getLiteral().getType(), parse_tree::ParseTreeNode::Type::Literal);
    ASSERT_EQ(primaryExpr.getLiteral().getReference(), code.substr(14, 3));
    ASSERT_EQ(primaryExpr.getLiteral().getValue(), 123);

    // Generic token = END
    ASSERT_EQ(compoundStatement.getEndKeyword().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(compoundStatement.getEndKeyword().getReference(), code.substr(18, 3));

    ///////////////// Program terminator

    // Program terminator = "."
    const auto& programTerminator = functionDefinition->getProgramTerminator();
    ASSERT_EQ(programTerminator.getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(programTerminator.getReference(), code.substr(code.size() - 1, 1));
}

TEST(TestParser, ParamVarConstDeclarations) { // NOLINT
    std::string_view code{"PARAM a, b, c;\n"
                          "VAR d, e, f;\n"
                          "CONST g = 10, h = 1234;\n"
                          "\n"
                          "BEGIN\n"
                          "\tRETURN 123\n"
                          "END\n"
                          "."};

    common::SourceCodeManager sourceCodeManager(std::string{code});
    Parser parser(sourceCodeManager);

    auto functionDefinition = parser.parseFunctionDefinition();
    ASSERT_FALSE(functionDefinition == nullptr);

    // function-definition = parameter-declarations variables-declarations constant-declarations compound-statement "."
    ASSERT_EQ(functionDefinition->getType(), parse_tree::ParseTreeNode::Type::FunctionDefinition);
    ASSERT_EQ(functionDefinition->getReference(), code);

    ///////////////// Parameter declarations

    // parameter-declarations = PARAM declarator-list ";"
    ASSERT_TRUE(functionDefinition->getParameterDeclarations());
    const auto& parameterDeclarations = *functionDefinition->getParameterDeclarations();
    ASSERT_EQ(parameterDeclarations.getType(), parse_tree::ParseTreeNode::Type::ParameterDeclarations);
    ASSERT_EQ(parameterDeclarations.getReference(), code.substr(0, 14));

    // generic token = PARAM
    ASSERT_EQ(parameterDeclarations.getParamKeyword().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(parameterDeclarations.getParamKeyword().getReference(), code.substr(0, 5));

    // declarator-list = identifier comma identifier comma identifier
    ASSERT_EQ(parameterDeclarations.getDeclaratorList().getType(), parse_tree::ParseTreeNode::Type::DeclaratorList);
    ASSERT_EQ(parameterDeclarations.getDeclaratorList().getReference(), code.substr(6, 7));

    const auto& declaratorList = parameterDeclarations.getDeclaratorList();
    const auto& declListChildren = declaratorList.getCommaSeparatedIdentifiers();
    ASSERT_EQ(declListChildren.size(), 5);

    // identifier = a
    ASSERT_EQ(declListChildren[0]->getType(), parse_tree::ParseTreeNode::Type::Identifier);
    ASSERT_EQ(declListChildren[0]->getReference(), code.substr(6, 1));

    // generic-token = ","
    ASSERT_EQ(declListChildren[1]->getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(declListChildren[1]->getReference(), code.substr(7, 1));

    // identifier = b
    ASSERT_EQ(declListChildren[2]->getType(), parse_tree::ParseTreeNode::Type::Identifier);
    ASSERT_EQ(declListChildren[2]->getReference(), code.substr(9, 1));

    // generic-token = ","
    ASSERT_EQ(declListChildren[3]->getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(declListChildren[3]->getReference(), code.substr(10, 1));

    // identifier = c
    ASSERT_EQ(declListChildren[4]->getType(), parse_tree::ParseTreeNode::Type::Identifier);
    ASSERT_EQ(declListChildren[4]->getReference(), code.substr(12, 1));

    // generic-token = ";"
    ASSERT_EQ(parameterDeclarations.getSemiColon().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(parameterDeclarations.getSemiColon().getReference(), code.substr(13, 1));

    ///////////////// Identifier declarations

    // variable-declarations = VAR declarator-list ";"
    ASSERT_TRUE(functionDefinition->getVariableDeclarations());
    const auto& variableDeclarations = *functionDefinition->getVariableDeclarations();
    ASSERT_EQ(variableDeclarations.getType(), parse_tree::ParseTreeNode::Type::VariableDeclarations);
    ASSERT_EQ(variableDeclarations.getReference(), code.substr(15, 12));

    // generic-token = VAR
    ASSERT_EQ(variableDeclarations.getVarKeyword().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(variableDeclarations.getVarKeyword().getReference(), code.substr(15, 3));

    // declarator-list = identifier comma identifier comma identifier
    ASSERT_EQ(variableDeclarations.getDeclaratorList().getType(), parse_tree::ParseTreeNode::Type::DeclaratorList);
    ASSERT_EQ(variableDeclarations.getDeclaratorList().getReference(), code.substr(19, 7));

    const auto& declaratorList2 = variableDeclarations.getDeclaratorList();
    const auto& declListChildren2 = declaratorList2.getCommaSeparatedIdentifiers();
    ASSERT_EQ(declListChildren2.size(), 5);

    // Identifier = d
    ASSERT_EQ(declListChildren2[0]->getType(), parse_tree::ParseTreeNode::Type::Identifier);
    ASSERT_EQ(declListChildren2[0]->getReference(), code.substr(19, 1));

    // Generic-token = ","
    ASSERT_EQ(declListChildren2[1]->getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(declListChildren2[1]->getReference(), code.substr(20, 1));

    // Identifier = e
    ASSERT_EQ(declListChildren2[2]->getType(), parse_tree::ParseTreeNode::Type::Identifier);
    ASSERT_EQ(declListChildren2[2]->getReference(), code.substr(22, 1));

    // Generic-token = ","
    ASSERT_EQ(declListChildren2[3]->getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(declListChildren2[3]->getReference(), code.substr(23, 1));

    // Identifier = f
    ASSERT_EQ(declListChildren2[4]->getType(), parse_tree::ParseTreeNode::Type::Identifier);
    ASSERT_EQ(declListChildren2[4]->getReference(), code.substr(25, 1));

    // generic-token = ";"
    ASSERT_EQ(variableDeclarations.getSemiColon().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(variableDeclarations.getSemiColon().getReference(), code.substr(26, 1));

    ///////////////// ConstantLiteral declarations

    // const-declarations = CONST init-declarator-list ";"
    ASSERT_TRUE(functionDefinition->getConstantDeclarations());
    const auto& constantDeclarations = *functionDefinition->getConstantDeclarations();
    ASSERT_EQ(constantDeclarations.getType(), parse_tree::ParseTreeNode::Type::ConstantDeclarations);
    ASSERT_EQ(constantDeclarations.getReference(), code.substr(28, 23));

    // generic-token = CONST
    ASSERT_EQ(constantDeclarations.getConstKeyword().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(constantDeclarations.getConstKeyword().getReference(), code.substr(28, 5));

    // init-declarator-list = init-declarator , init-declarator
    ASSERT_EQ(constantDeclarations.getInitDeclaratorList().getType(), parse_tree::ParseTreeNode::Type::InitDeclaratorList);
    ASSERT_EQ(constantDeclarations.getInitDeclaratorList().getReference(), code.substr(34, 16));

    const auto& initDeclList = constantDeclarations.getInitDeclaratorList();
    const auto& initDeclListChildren = initDeclList.getCommaSeparatedInitDeclarators();
    ASSERT_EQ(initDeclListChildren.size(), 3);

    // init-declarator = g = 10
    ASSERT_EQ(initDeclListChildren[0]->getType(), parse_tree::ParseTreeNode::Type::InitDeclarator);
    ASSERT_EQ(initDeclListChildren[0]->getReference(), code.substr(34, 6));

    const auto& initDeclarator1 = static_cast<parse_tree::InitDeclarator&>(*initDeclListChildren[0]); // NOLINT

    // identifier = g
    ASSERT_EQ(initDeclarator1.getInitTarget().getType(), parse_tree::ParseTreeNode::Type::Identifier);
    ASSERT_EQ(initDeclarator1.getInitTarget().getReference(), code.substr(34, 1));

    // generic-token = "="
    ASSERT_EQ(initDeclarator1.getInitToken().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(initDeclarator1.getInitToken().getReference(), code.substr(36, 1));

    // literal = 10
    ASSERT_EQ(initDeclarator1.getLiteral().getType(), parse_tree::ParseTreeNode::Type::Literal);
    ASSERT_EQ(initDeclarator1.getLiteral().getReference(), code.substr(38, 2));
    ASSERT_EQ(initDeclarator1.getLiteral().getValue(), 10);

    // generic-token = ","
    ASSERT_EQ(initDeclListChildren[1]->getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(initDeclListChildren[1]->getReference(), code.substr(40, 1));

    // init-declarator = h = 1234
    ASSERT_EQ(initDeclListChildren[2]->getType(), parse_tree::ParseTreeNode::Type::InitDeclarator);
    ASSERT_EQ(initDeclListChildren[2]->getReference(), code.substr(42, 8));

    const auto& initDeclarator2 = static_cast<parse_tree::InitDeclarator&>(*initDeclListChildren[2]); // NOLINT

    // identifier = h
    ASSERT_EQ(initDeclarator2.getInitTarget().getType(), parse_tree::ParseTreeNode::Type::Identifier);
    ASSERT_EQ(initDeclarator2.getInitTarget().getReference(), code.substr(42, 1));

    // generic-token = "="
    ASSERT_EQ(initDeclarator2.getInitToken().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(initDeclarator2.getInitToken().getReference(), code.substr(44, 1));

    // literal = 1234
    ASSERT_EQ(initDeclarator2.getLiteral().getType(), parse_tree::ParseTreeNode::Type::Literal);
    ASSERT_EQ(initDeclarator2.getLiteral().getReference(), code.substr(46, 4));
    ASSERT_EQ(initDeclarator2.getLiteral().getValue(), 1234);

    // generic-token = ";"
    ASSERT_EQ(constantDeclarations.getSemiColon().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(constantDeclarations.getSemiColon().getReference(), code.substr(50, 1));

    ///////////////// Compound statement

    // compound-statement = BEGIN statement-list END
    const auto& compoundStatement = functionDefinition->getCompoundStatement();
    ASSERT_EQ(compoundStatement.getType(), parse_tree::ParseTreeNode::Type::CompoundStatement);
    ASSERT_EQ(compoundStatement.getReference(), code.substr(53, 21));

    // Generic token = BEGIN
    ASSERT_EQ(compoundStatement.getBeginKeyword().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(compoundStatement.getBeginKeyword().getReference(), code.substr(53, 5));

    // statement-list = statement
    ASSERT_EQ(compoundStatement.getStatementList().getType(), parse_tree::ParseTreeNode::Type::StatementList);
    ASSERT_EQ(compoundStatement.getStatementList().getReference(), code.substr(60, 10));
    const auto& statementList = compoundStatement.getStatementList();

    const auto& statementListChildren = statementList.getStatementsSeparatedBySemiColon();
    ASSERT_EQ(statementListChildren.size(), 1);

    // statement = RETURN 123
    ASSERT_EQ(statementListChildren[0]->getType(), parse_tree::ParseTreeNode::Type::Statement);
    ASSERT_EQ(statementListChildren[0]->getReference(), code.substr(60, 10));
    const auto& returnStatement = static_cast<parse_tree::Statement&>(*statementListChildren[0]); // NOLINT
    ASSERT_EQ(returnStatement.getStatementType(), parse_tree::Statement::Type::ReturnStatement);

    // generic-token = RETURN
    ASSERT_EQ(returnStatement.getReturnKeyword().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(returnStatement.getReturnKeyword().getReference(), code.substr(60, 6));

    // additive-expression = 123
    ASSERT_EQ(returnStatement.getAdditiveExpression().getType(), parse_tree::ParseTreeNode::Type::AdditiveExpression);
    ASSERT_EQ(returnStatement.getAdditiveExpression().getReference(), code.substr(67, 3));
    const auto& additiveExpr = returnStatement.getAdditiveExpression();
    ASSERT_EQ(additiveExpr.getAdditiveExpressionType(), parse_tree::AdditiveExpression::Type::None);
    ASSERT_EQ(additiveExpr.getMultiplicativeExpression().getType(), parse_tree::ParseTreeNode::Type::MultiplicativeExpression);
    ASSERT_EQ(additiveExpr.getMultiplicativeExpression().getReference(), code.substr(67, 3));
    const auto& mulExpr = additiveExpr.getMultiplicativeExpression();
    ASSERT_EQ(mulExpr.getMultiplicativeExpressionType(), parse_tree::MultiplicativeExpression::Type::None);
    ASSERT_EQ(mulExpr.getUnaryExpression().getType(), parse_tree::ParseTreeNode::Type::UnaryExpression);
    ASSERT_EQ(mulExpr.getUnaryExpression().getReference(), code.substr(67, 3));
    const auto& unaryExpr = mulExpr.getUnaryExpression();
    ASSERT_EQ(unaryExpr.getUnaryExpressionType(), parse_tree::UnaryExpression::Type::Unsigned);
    ASSERT_EQ(unaryExpr.getPrimaryExpression().getType(), parse_tree::ParseTreeNode::Type::PrimaryExpression);
    ASSERT_EQ(unaryExpr.getPrimaryExpression().getReference(), code.substr(67, 3));
    const auto& primaryExpr = unaryExpr.getPrimaryExpression();
    ASSERT_EQ(primaryExpr.getPrimaryExpressionType(), parse_tree::PrimaryExpression::Type::Literal);
    ASSERT_EQ(primaryExpr.getLiteral().getType(), parse_tree::ParseTreeNode::Type::Literal);
    ASSERT_EQ(primaryExpr.getLiteral().getReference(), code.substr(67, 3));
    ASSERT_EQ(primaryExpr.getLiteral().getValue(), 123);

    // Generic token = END
    ASSERT_EQ(compoundStatement.getEndKeyword().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(compoundStatement.getEndKeyword().getReference(), code.substr(71, 3));

    ///////////////// Program terminator

    // Program terminator = "."
    const auto& programTerminator = functionDefinition->getProgramTerminator();
    ASSERT_EQ(programTerminator.getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(programTerminator.getReference(), code.substr(code.size() - 1, 1));
}

TEST(TestParser, ParamConstDeclarations) { // NOLINT
    std::string_view code{"PARAM a, b;\n"
                          "CONST g = 10;\n"
                          "\n"
                          "BEGIN\n"
                          "\tRETURN 123\n"
                          "END\n"
                          "."};

    common::SourceCodeManager sourceCodeManager(std::string{code});
    Parser parser(sourceCodeManager);

    auto functionDefinition = parser.parseFunctionDefinition();
    ASSERT_FALSE(functionDefinition == nullptr);

    // function-definition = parameter-declarations constant-declarations compound-statement "."
    ASSERT_EQ(functionDefinition->getType(), parse_tree::ParseTreeNode::Type::FunctionDefinition);
    ASSERT_EQ(functionDefinition->getReference(), code);

    ASSERT_FALSE(functionDefinition->getVariableDeclarations());

    ///////////////// Parameter declarations

    // parameter-declarations = PARAM declarator-list ";"
    ASSERT_TRUE(functionDefinition->getParameterDeclarations());
    const auto& parameterDeclarations = *functionDefinition->getParameterDeclarations();
    ASSERT_EQ(parameterDeclarations.getType(), parse_tree::ParseTreeNode::Type::ParameterDeclarations);
    ASSERT_EQ(parameterDeclarations.getReference(), code.substr(0, 11));

    // generic token = PARAM
    ASSERT_EQ(parameterDeclarations.getParamKeyword().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(parameterDeclarations.getParamKeyword().getReference(), code.substr(0, 5));

    // declarator-list = identifier comma identifier
    ASSERT_EQ(parameterDeclarations.getDeclaratorList().getType(), parse_tree::ParseTreeNode::Type::DeclaratorList);
    ASSERT_EQ(parameterDeclarations.getDeclaratorList().getReference(), code.substr(6, 4));

    const auto& declaratorList = parameterDeclarations.getDeclaratorList();
    const auto& declListChildren = declaratorList.getCommaSeparatedIdentifiers();
    ASSERT_EQ(declListChildren.size(), 3);

    // identifier = a
    ASSERT_EQ(declListChildren[0]->getType(), parse_tree::ParseTreeNode::Type::Identifier);
    ASSERT_EQ(declListChildren[0]->getReference(), code.substr(6, 1));

    // generic-token = ","
    ASSERT_EQ(declListChildren[1]->getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(declListChildren[1]->getReference(), code.substr(7, 1));

    // identifier = b
    ASSERT_EQ(declListChildren[2]->getType(), parse_tree::ParseTreeNode::Type::Identifier);
    ASSERT_EQ(declListChildren[2]->getReference(), code.substr(9, 1));

    // generic-token = ";"
    ASSERT_EQ(parameterDeclarations.getSemiColon().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(parameterDeclarations.getSemiColon().getReference(), code.substr(10, 1));

    ///////////////// ConstantLiteral declarations

    // const-declarations = CONST init-declarator-list ";"
    ASSERT_TRUE(functionDefinition->getConstantDeclarations());
    const auto& constantDeclarations = *functionDefinition->getConstantDeclarations();
    ASSERT_EQ(constantDeclarations.getType(), parse_tree::ParseTreeNode::Type::ConstantDeclarations);
    ASSERT_EQ(constantDeclarations.getReference(), code.substr(12, 13));

    // generic-token = CONST
    ASSERT_EQ(constantDeclarations.getConstKeyword().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(constantDeclarations.getConstKeyword().getReference(), code.substr(12, 5));

    // init-declarator-list = init-declarator
    ASSERT_EQ(constantDeclarations.getInitDeclaratorList().getType(), parse_tree::ParseTreeNode::Type::InitDeclaratorList);
    ASSERT_EQ(constantDeclarations.getInitDeclaratorList().getReference(), code.substr(18, 6));

    const auto& initDeclList = constantDeclarations.getInitDeclaratorList();
    const auto& initDeclListChildren = initDeclList.getCommaSeparatedInitDeclarators();
    ASSERT_EQ(initDeclListChildren.size(), 1);

    // init-declarator = g = 10
    ASSERT_EQ(initDeclListChildren[0]->getType(), parse_tree::ParseTreeNode::Type::InitDeclarator);
    ASSERT_EQ(initDeclListChildren[0]->getReference(), code.substr(18, 6));

    const auto& initDeclarator = static_cast<const parse_tree::InitDeclarator&>(*initDeclListChildren[0]); // NOLINT

    // identifier = g
    ASSERT_EQ(initDeclarator.getInitTarget().getType(), parse_tree::ParseTreeNode::Type::Identifier);
    ASSERT_EQ(initDeclarator.getInitTarget().getReference(), code.substr(18, 1));

    // generic-token = "="
    ASSERT_EQ(initDeclarator.getInitToken().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(initDeclarator.getInitToken().getReference(), code.substr(20, 1));

    // literal = 10
    ASSERT_EQ(initDeclarator.getLiteral().getType(), parse_tree::ParseTreeNode::Type::Literal);
    ASSERT_EQ(initDeclarator.getLiteral().getReference(), code.substr(22, 2));
    ASSERT_EQ(initDeclarator.getLiteral().getValue(), 10);

    // generic-token = ";"
    ASSERT_EQ(constantDeclarations.getSemiColon().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(constantDeclarations.getSemiColon().getReference(), code.substr(24, 1));

    // NOTE: compound statement and program terminator were omitted since it is the same as
    //       the first two tests.
}

TEST(TestParser, VarConstDeclarations) { // NOLINT
    std::string_view code{"VAR a, b;\n"
                          "CONST g = 10;\n"
                          "\n"
                          "BEGIN\n"
                          "\tRETURN 123\n"
                          "END\n"
                          "."};

    common::SourceCodeManager sourceCodeManager(std::string{code});
    Parser parser(sourceCodeManager);

    auto functionDefinition = parser.parseFunctionDefinition();
    ASSERT_FALSE(functionDefinition == nullptr);

    // function-definition = parameter-declarations constant-declarations compound-statement "."
    ASSERT_EQ(functionDefinition->getType(), parse_tree::ParseTreeNode::Type::FunctionDefinition);
    ASSERT_EQ(functionDefinition->getReference(), code);

    ASSERT_FALSE(functionDefinition->getParameterDeclarations());

    ///////////////// Identifier declarations

    // variable-declarations = VAR declarator-list ";"
    ASSERT_TRUE(functionDefinition->getVariableDeclarations());
    const auto& variableDeclarations = *functionDefinition->getVariableDeclarations();
    ASSERT_EQ(variableDeclarations.getType(), parse_tree::ParseTreeNode::Type::VariableDeclarations);
    ASSERT_EQ(variableDeclarations.getReference(), code.substr(0, 9));

    // generic-token = VAR
    ASSERT_EQ(variableDeclarations.getVarKeyword().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(variableDeclarations.getVarKeyword().getReference(), code.substr(0, 3));

    // declarator-list = identifier comma identifier
    ASSERT_EQ(variableDeclarations.getDeclaratorList().getType(), parse_tree::ParseTreeNode::Type::DeclaratorList);
    ASSERT_EQ(variableDeclarations.getDeclaratorList().getReference(), code.substr(4, 4));

    const auto& declaratorList = variableDeclarations.getDeclaratorList();
    const auto& declListChildren = declaratorList.getCommaSeparatedIdentifiers();
    ASSERT_EQ(declListChildren.size(), 3);

    // Identifier = a
    ASSERT_EQ(declListChildren[0]->getType(), parse_tree::ParseTreeNode::Type::Identifier);
    ASSERT_EQ(declListChildren[0]->getReference(), code.substr(4, 1));

    // Generic-token = ","
    ASSERT_EQ(declListChildren[1]->getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(declListChildren[1]->getReference(), code.substr(5, 1));

    // Identifier = b
    ASSERT_EQ(declListChildren[2]->getType(), parse_tree::ParseTreeNode::Type::Identifier);
    ASSERT_EQ(declListChildren[2]->getReference(), code.substr(7, 1));

    // generic-token = ";"
    ASSERT_EQ(variableDeclarations.getSemiColon().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(variableDeclarations.getSemiColon().getReference(), code.substr(8, 1));

    ///////////////// ConstantLiteral declarations

    // const-declarations = CONST init-declarator-list ";"
    ASSERT_TRUE(functionDefinition->getConstantDeclarations());
    const auto& constantDeclarations = *functionDefinition->getConstantDeclarations();
    ASSERT_EQ(constantDeclarations.getType(), parse_tree::ParseTreeNode::Type::ConstantDeclarations);
    ASSERT_EQ(constantDeclarations.getReference(), code.substr(10, 13));

    // generic-token = CONST
    ASSERT_EQ(constantDeclarations.getConstKeyword().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(constantDeclarations.getConstKeyword().getReference(), code.substr(10, 5));

    // init-declarator-list = init-declarator
    ASSERT_EQ(constantDeclarations.getInitDeclaratorList().getType(), parse_tree::ParseTreeNode::Type::InitDeclaratorList);
    ASSERT_EQ(constantDeclarations.getInitDeclaratorList().getReference(), code.substr(16, 6));

    const auto& initDeclList = constantDeclarations.getInitDeclaratorList();
    const auto& initDeclListChildren = initDeclList.getCommaSeparatedInitDeclarators();
    ASSERT_EQ(initDeclListChildren.size(), 1);

    // init-declarator = g = 10
    ASSERT_EQ(initDeclListChildren[0]->getType(), parse_tree::ParseTreeNode::Type::InitDeclarator);
    ASSERT_EQ(initDeclListChildren[0]->getReference(), code.substr(16, 6));

    const auto& initDeclarator = static_cast<const parse_tree::InitDeclarator&>(*initDeclListChildren[0]); // NOLINT

    // identifier = g
    ASSERT_EQ(initDeclarator.getInitTarget().getType(), parse_tree::ParseTreeNode::Type::Identifier);
    ASSERT_EQ(initDeclarator.getInitTarget().getReference(), code.substr(16, 1));

    // generic-token = "="
    ASSERT_EQ(initDeclarator.getInitToken().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(initDeclarator.getInitToken().getReference(), code.substr(18, 1));

    // literal = 10
    ASSERT_EQ(initDeclarator.getLiteral().getType(), parse_tree::ParseTreeNode::Type::Literal);
    ASSERT_EQ(initDeclarator.getLiteral().getReference(), code.substr(20, 2));
    ASSERT_EQ(initDeclarator.getLiteral().getValue(), 10);

    // generic-token = ";"
    ASSERT_EQ(constantDeclarations.getSemiColon().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(constantDeclarations.getSemiColon().getReference(), code.substr(22, 1));

    // NOTE: compound statement and program terminator were omitted since it is the same as
    //       the first two tests.
}

TEST(TestParser, VarDeclarations) { // NOLINT
    std::string_view code{"VAR a, b;\n"
                          "\n"
                          "BEGIN\n"
                          "\tRETURN 123\n"
                          "END\n"
                          "."};

    common::SourceCodeManager sourceCodeManager(std::string{code});
    Parser parser(sourceCodeManager);

    auto functionDefinition = parser.parseFunctionDefinition();
    ASSERT_FALSE(functionDefinition == nullptr);

    // function-definition = parameter-declarations constant-declarations compound-statement "."
    ASSERT_EQ(functionDefinition->getType(), parse_tree::ParseTreeNode::Type::FunctionDefinition);
    ASSERT_EQ(functionDefinition->getReference(), code);

    ASSERT_FALSE(functionDefinition->getParameterDeclarations());
    ASSERT_FALSE(functionDefinition->getConstantDeclarations());

    ///////////////// Identifier declarations

    // variable-declarations = VAR declarator-list ";"
    ASSERT_TRUE(functionDefinition->getVariableDeclarations());
    const auto& variableDeclarations = *functionDefinition->getVariableDeclarations();
    ASSERT_EQ(variableDeclarations.getType(), parse_tree::ParseTreeNode::Type::VariableDeclarations);
    ASSERT_EQ(variableDeclarations.getReference(), code.substr(0, 9));

    // generic-token = VAR
    ASSERT_EQ(variableDeclarations.getVarKeyword().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(variableDeclarations.getVarKeyword().getReference(), code.substr(0, 3));

    // declarator-list = identifier comma identifier
    ASSERT_EQ(variableDeclarations.getDeclaratorList().getType(), parse_tree::ParseTreeNode::Type::DeclaratorList);
    ASSERT_EQ(variableDeclarations.getDeclaratorList().getReference(), code.substr(4, 4));

    const auto& declaratorList = variableDeclarations.getDeclaratorList();
    const auto& declListChildren = declaratorList.getCommaSeparatedIdentifiers();
    ASSERT_EQ(declListChildren.size(), 3);

    // Identifier = a
    ASSERT_EQ(declListChildren[0]->getType(), parse_tree::ParseTreeNode::Type::Identifier);
    ASSERT_EQ(declListChildren[0]->getReference(), code.substr(4, 1));

    // Generic-token = ","
    ASSERT_EQ(declListChildren[1]->getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(declListChildren[1]->getReference(), code.substr(5, 1));

    // Identifier = b
    ASSERT_EQ(declListChildren[2]->getType(), parse_tree::ParseTreeNode::Type::Identifier);
    ASSERT_EQ(declListChildren[2]->getReference(), code.substr(7, 1));

    // generic-token = ";"
    ASSERT_EQ(variableDeclarations.getSemiColon().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(variableDeclarations.getSemiColon().getReference(), code.substr(8, 1));

    // NOTE: compound statement and program terminator were omitted since it is the same as
    //       the first two tests.
}

TEST(TestParser, LongStatementList) { // NOLINT
    std::string_view code{"CONST A = 10;\n"
                          "BEGIN\n"
                          "z := (x + A - 1) / -y;\n"
                          "x := (z * (y + 12));\n"
                          "RETURN +x\n"
                          "END."};

    common::SourceCodeManager sourceCodeManager(std::string{code});
    Parser parser(sourceCodeManager);

    auto functionDefinition = parser.parseFunctionDefinition();
    ASSERT_FALSE(functionDefinition == nullptr);

    // function-definition = parameter-declarations constant-declarations compound-statement "."
    ASSERT_EQ(functionDefinition->getType(), parse_tree::ParseTreeNode::Type::FunctionDefinition);
    ASSERT_EQ(functionDefinition->getReference(), code);

    ASSERT_FALSE(functionDefinition->getParameterDeclarations());
    ASSERT_FALSE(functionDefinition->getVariableDeclarations());

    ///////////////// ConstantLiteral declarations

    // const-declarations = CONST init-declarator-list ";"
    ASSERT_TRUE(functionDefinition->getConstantDeclarations());
    const auto& constantDeclarations = *functionDefinition->getConstantDeclarations();
    ASSERT_EQ(constantDeclarations.getType(), parse_tree::ParseTreeNode::Type::ConstantDeclarations);
    ASSERT_EQ(constantDeclarations.getReference(), code.substr(0, 13));

    // generic-token = CONST
    ASSERT_EQ(constantDeclarations.getConstKeyword().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(constantDeclarations.getConstKeyword().getReference(), code.substr(0, 5));

    // init-declarator-list = init-declarator
    ASSERT_EQ(constantDeclarations.getInitDeclaratorList().getType(), parse_tree::ParseTreeNode::Type::InitDeclaratorList);
    ASSERT_EQ(constantDeclarations.getInitDeclaratorList().getReference(), code.substr(6, 6));

    const auto& initDeclList = constantDeclarations.getInitDeclaratorList();
    const auto& initDeclListChildren = initDeclList.getCommaSeparatedInitDeclarators();
    ASSERT_EQ(initDeclListChildren.size(), 1);

    // init-declarator = A = 10
    ASSERT_EQ(initDeclListChildren[0]->getType(), parse_tree::ParseTreeNode::Type::InitDeclarator);
    ASSERT_EQ(initDeclListChildren[0]->getReference(), code.substr(6, 6));

    const auto& initDeclarator = static_cast<const parse_tree::InitDeclarator&>(*initDeclListChildren[0]); // NOLINT

    // identifier = A
    ASSERT_EQ(initDeclarator.getInitTarget().getType(), parse_tree::ParseTreeNode::Type::Identifier);
    ASSERT_EQ(initDeclarator.getInitTarget().getReference(), code.substr(6, 1));

    // generic-token = "="
    ASSERT_EQ(initDeclarator.getInitToken().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(initDeclarator.getInitToken().getReference(), code.substr(8, 1));

    // literal = 10
    ASSERT_EQ(initDeclarator.getLiteral().getType(), parse_tree::ParseTreeNode::Type::Literal);
    ASSERT_EQ(initDeclarator.getLiteral().getReference(), code.substr(10, 2));
    ASSERT_EQ(initDeclarator.getLiteral().getValue(), 10);

    // generic-token = ";"
    ASSERT_EQ(constantDeclarations.getSemiColon().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(constantDeclarations.getSemiColon().getReference(), code.substr(12, 1));

    ///////////////// Compound statement

    // compound-statement = BEGIN statement-list END
    const auto& compoundStatement = functionDefinition->getCompoundStatement();
    ASSERT_EQ(compoundStatement.getType(), parse_tree::ParseTreeNode::Type::CompoundStatement);
    ASSERT_EQ(compoundStatement.getReference(), code.substr(14, 63));

    // generic-token = BEGIN
    ASSERT_EQ(compoundStatement.getBeginKeyword().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(compoundStatement.getBeginKeyword().getReference(), code.substr(14, 5));

    // statement-list = statement ";" statement ";" statement
    ASSERT_EQ(compoundStatement.getStatementList().getType(), parse_tree::ParseTreeNode::Type::StatementList);
    ASSERT_EQ(compoundStatement.getStatementList().getReference(), code.substr(20, 53));

    const auto& statementList = compoundStatement.getStatementList();
    const auto& stmtListChildren = statementList.getStatementsSeparatedBySemiColon();
    ASSERT_EQ(stmtListChildren.size(), 5);

    ///// Statement 1: z := (x + A - 1) / -y

    // assignment = z := (x + A - 1) / -y
    ASSERT_EQ(stmtListChildren[0]->getType(), parse_tree::ParseTreeNode::Type::Statement);
    ASSERT_EQ(stmtListChildren[0]->getReference(), code.substr(20, 21));

    const auto& stmt1 = static_cast<const parse_tree::Statement&>(*stmtListChildren[0]); // NOLINT
    ASSERT_EQ(stmt1.getStatementType(), parse_tree::Statement::Type::AssignmentStatement);

    ASSERT_EQ(stmt1.getAssignmentExpression().getType(), parse_tree::ParseTreeNode::Type::AssignmentExpression);
    ASSERT_EQ(stmt1.getAssignmentExpression().getReference(), code.substr(20, 21));

    const auto& assignmentExpr1 = stmt1.getAssignmentExpression();

    // identifier = z
    ASSERT_EQ(assignmentExpr1.getAssignmentTarget().getType(), parse_tree::ParseTreeNode::Type::Identifier);
    ASSERT_EQ(assignmentExpr1.getAssignmentTarget().getReference(), code.substr(20, 1));

    // generic-token = ":="
    ASSERT_EQ(assignmentExpr1.getAssignmentToken().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(assignmentExpr1.getAssignmentToken().getReference(), code.substr(22, 2));

    // additive-expression = (x + A - 1) / -y
    ASSERT_EQ(assignmentExpr1.getAdditiveExpression().getType(), parse_tree::ParseTreeNode::Type::AdditiveExpression);
    ASSERT_EQ(assignmentExpr1.getAdditiveExpression().getReference(), code.substr(25, 16));

    const auto& additiveExpression1 = assignmentExpr1.getAdditiveExpression();
    ASSERT_EQ(additiveExpression1.getAdditiveExpressionType(), parse_tree::AdditiveExpression::Type::None);

    ASSERT_EQ(additiveExpression1.getMultiplicativeExpression().getType(), parse_tree::ParseTreeNode::Type::MultiplicativeExpression);
    ASSERT_EQ(additiveExpression1.getMultiplicativeExpression().getReference(), code.substr(25, 16));

    const auto& mulExpr1 = additiveExpression1.getMultiplicativeExpression();
    ASSERT_EQ(mulExpr1.getMultiplicativeExpressionType(), parse_tree::MultiplicativeExpression::Type::Div);

    // unary-expression = (x + A - 1)
    ASSERT_EQ(mulExpr1.getUnaryExpression().getType(), parse_tree::ParseTreeNode::Type::UnaryExpression);
    ASSERT_EQ(mulExpr1.getUnaryExpression().getReference(), code.substr(25, 11));

    const auto& unaryExpression1 = mulExpr1.getUnaryExpression();

    ASSERT_EQ(unaryExpression1.getUnaryExpressionType(), parse_tree::UnaryExpression::Type::Unsigned);
    ASSERT_EQ(unaryExpression1.getPrimaryExpression().getType(), parse_tree::ParseTreeNode::Type::PrimaryExpression);
    ASSERT_EQ(unaryExpression1.getPrimaryExpression().getReference(), code.substr(25, 11));

    const auto& primaryExpr1 = unaryExpression1.getPrimaryExpression();
    ASSERT_EQ(primaryExpr1.getPrimaryExpressionType(), parse_tree::PrimaryExpression::Type::Parenthesized);

    // generic-token = "("
    ASSERT_EQ(primaryExpr1.getLeftParenthesis().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(primaryExpr1.getLeftParenthesis().getReference(), code.substr(25, 1));

    // additive-expression = x + A - 1
    ASSERT_EQ(primaryExpr1.getAdditiveExpression().getType(), parse_tree::ParseTreeNode::Type::AdditiveExpression);
    ASSERT_EQ(primaryExpr1.getAdditiveExpression().getReference(), code.substr(26, 9));

    const auto& additiveExpr2 = primaryExpr1.getAdditiveExpression();
    ASSERT_EQ(additiveExpr2.getAdditiveExpressionType(), parse_tree::AdditiveExpression::Type::Add);

    // multiplicative-expression = x
    ASSERT_EQ(additiveExpr2.getMultiplicativeExpression().getType(), parse_tree::ParseTreeNode::Type::MultiplicativeExpression);
    ASSERT_EQ(additiveExpr2.getMultiplicativeExpression().getReference(), code.substr(26, 1));
    const auto& mulExpr2 = additiveExpr2.getMultiplicativeExpression();
    ASSERT_EQ(mulExpr2.getMultiplicativeExpressionType(), parse_tree::MultiplicativeExpression::Type::None);

    ASSERT_EQ(mulExpr2.getUnaryExpression().getType(), parse_tree::ParseTreeNode::Type::UnaryExpression);
    ASSERT_EQ(mulExpr2.getUnaryExpression().getReference(), code.substr(26, 1));

    const auto& unaryExpr2 = mulExpr2.getUnaryExpression();
    ASSERT_EQ(unaryExpr2.getUnaryExpressionType(), parse_tree::UnaryExpression::Type::Unsigned);

    ASSERT_EQ(unaryExpr2.getPrimaryExpression().getType(), parse_tree::ParseTreeNode::Type::PrimaryExpression);
    ASSERT_EQ(unaryExpr2.getPrimaryExpression().getReference(), code.substr(26, 1));

    const auto& primaryExpr2 = unaryExpr2.getPrimaryExpression();
    ASSERT_EQ(primaryExpr2.getPrimaryExpressionType(), parse_tree::PrimaryExpression::Type::Identifier);

    // generic-token = +
    ASSERT_EQ(additiveExpr2.getAdditiveOpToken().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(additiveExpr2.getAdditiveOpToken().getReference(), code.substr(28, 1));

    // additive-expression = A - 1
    ASSERT_EQ(additiveExpr2.getAdditiveExpression().getType(), parse_tree::ParseTreeNode::Type::AdditiveExpression);
    ASSERT_EQ(additiveExpr2.getAdditiveExpression().getReference(), code.substr(30, 5));

    const auto& additiveExpr3 = additiveExpr2.getAdditiveExpression();
    ASSERT_EQ(additiveExpr3.getAdditiveExpressionType(), parse_tree::AdditiveExpression::Type::Sub);

    // multiplicative-expression = A
    ASSERT_EQ(additiveExpr3.getMultiplicativeExpression().getType(), parse_tree::ParseTreeNode::Type::MultiplicativeExpression);
    ASSERT_EQ(additiveExpr3.getMultiplicativeExpression().getReference(), code.substr(30, 1));

    const auto& mulExpr3 = additiveExpr3.getMultiplicativeExpression();
    ASSERT_EQ(mulExpr3.getMultiplicativeExpressionType(), parse_tree::MultiplicativeExpression::Type::None);

    ASSERT_EQ(mulExpr3.getUnaryExpression().getType(), parse_tree::ParseTreeNode::Type::UnaryExpression);
    ASSERT_EQ(mulExpr3.getUnaryExpression().getReference(), code.substr(30, 1));

    const auto& unaryExpr3 = mulExpr3.getUnaryExpression();
    ASSERT_EQ(unaryExpr3.getUnaryExpressionType(), parse_tree::UnaryExpression::Type::Unsigned);

    ASSERT_EQ(unaryExpr3.getPrimaryExpression().getType(), parse_tree::ParseTreeNode::Type::PrimaryExpression);
    ASSERT_EQ(unaryExpr3.getPrimaryExpression().getReference(), code.substr(30, 1));

    const auto& primaryExpr3 = unaryExpr3.getPrimaryExpression();
    ASSERT_EQ(primaryExpr3.getPrimaryExpressionType(), parse_tree::PrimaryExpression::Type::Identifier);

    // generic-token = "-"
    ASSERT_EQ(additiveExpr3.getAdditiveOpToken().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(additiveExpr3.getAdditiveOpToken().getReference(), code.substr(32, 1));

    // additive-expression = 1
    ASSERT_EQ(additiveExpr3.getAdditiveExpression().getType(), parse_tree::ParseTreeNode::Type::AdditiveExpression);
    ASSERT_EQ(additiveExpr3.getAdditiveExpression().getReference(), code.substr(34, 1));

    const auto& additiveExpr4 = additiveExpr3.getAdditiveExpression();
    ASSERT_EQ(additiveExpr4.getAdditiveExpressionType(), parse_tree::AdditiveExpression::Type::None);

    ASSERT_EQ(additiveExpr4.getMultiplicativeExpression().getType(), parse_tree::ParseTreeNode::Type::MultiplicativeExpression);
    ASSERT_EQ(additiveExpr4.getMultiplicativeExpression().getReference(), code.substr(34, 1));

    const auto& mulExpr4 = additiveExpr4.getMultiplicativeExpression();
    ASSERT_EQ(mulExpr4.getMultiplicativeExpressionType(), parse_tree::MultiplicativeExpression::Type::None);

    ASSERT_EQ(mulExpr4.getUnaryExpression().getType(), parse_tree::ParseTreeNode::Type::UnaryExpression);
    ASSERT_EQ(mulExpr4.getUnaryExpression().getReference(), code.substr(34, 1));

    const auto& unaryExpr4 = mulExpr4.getUnaryExpression();

    ASSERT_EQ(unaryExpr4.getUnaryExpressionType(), parse_tree::UnaryExpression::Type::Unsigned);
    ASSERT_EQ(unaryExpr4.getReference(), code.substr(34, 1));

    ASSERT_EQ(unaryExpr4.getPrimaryExpression().getType(), parse_tree::ParseTreeNode::Type::PrimaryExpression);
    ASSERT_EQ(unaryExpr4.getPrimaryExpression().getReference(), code.substr(34, 1));

    const auto& primaryExpr4 = unaryExpr4.getPrimaryExpression();

    ASSERT_EQ(primaryExpr4.getPrimaryExpressionType(), parse_tree::PrimaryExpression::Type::Literal);
    ASSERT_EQ(primaryExpr4.getLiteral().getType(), parse_tree::ParseTreeNode::Type::Literal);
    ASSERT_EQ(primaryExpr4.getLiteral().getValue(), 1);

    // generic-token = ")"
    ASSERT_EQ(primaryExpr1.getRightParenthesis().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(primaryExpr1.getRightParenthesis().getReference(), code.substr(35, 1));

    // generic-token = "/"
    ASSERT_EQ(mulExpr1.getMultiplicativeOpToken().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(mulExpr1.getMultiplicativeOpToken().getReference(), code.substr(37, 1));

    // multiplicative-expression = -y
    ASSERT_EQ(mulExpr1.getMultiplicativeExpression().getType(), parse_tree::ParseTreeNode::Type::MultiplicativeExpression);
    ASSERT_EQ(mulExpr1.getMultiplicativeExpression().getReference(), code.substr(39, 2));

    const auto& mulExpr5 = mulExpr1.getMultiplicativeExpression();
    ASSERT_EQ(mulExpr5.getMultiplicativeExpressionType(), parse_tree::MultiplicativeExpression::Type::None);

    ASSERT_EQ(mulExpr5.getUnaryExpression().getType(), parse_tree::ParseTreeNode::Type::UnaryExpression);
    ASSERT_EQ(mulExpr5.getUnaryExpression().getReference(), code.substr(39, 2));

    const auto& unaryExpr5 = mulExpr5.getUnaryExpression();
    ASSERT_EQ(unaryExpr5.getUnaryExpressionType(), parse_tree::UnaryExpression::Type::MinusSign);

    ASSERT_EQ(unaryExpr5.getSignToken().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(unaryExpr5.getSignToken().getReference(), code.substr(39, 1));

    ASSERT_EQ(unaryExpr5.getPrimaryExpression().getType(), parse_tree::ParseTreeNode::Type::PrimaryExpression);
    ASSERT_EQ(unaryExpr5.getPrimaryExpression().getReference(), code.substr(40, 1));

    const auto& primaryExpr5 = unaryExpr5.getPrimaryExpression();
    ASSERT_EQ(primaryExpr5.getPrimaryExpressionType(), parse_tree::PrimaryExpression::Type::Identifier);

    ASSERT_EQ(primaryExpr5.getIdentifier().getType(), parse_tree::ParseTreeNode::Type::Identifier);
    ASSERT_EQ(primaryExpr5.getIdentifier().getReference(), code.substr(40, 1));

    ///// Separator ";"

    ASSERT_EQ(stmtListChildren[1]->getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(stmtListChildren[1]->getReference(), code.substr(41, 1));

    ///// Statement 2: x := (z * (y + 12))

    // assignment = x := (z * (y + 12))
    ASSERT_EQ(stmtListChildren[2]->getType(), parse_tree::ParseTreeNode::Type::Statement);
    ASSERT_EQ(stmtListChildren[2]->getReference(), code.substr(43, 19));

    const auto& stmt2 = static_cast<const parse_tree::Statement&>(*stmtListChildren[2]); // NOLINT
    ASSERT_EQ(stmt2.getStatementType(), parse_tree::Statement::Type::AssignmentStatement);

    ASSERT_EQ(stmt2.getAssignmentExpression().getType(), parse_tree::ParseTreeNode::Type::AssignmentExpression);
    ASSERT_EQ(stmt2.getAssignmentExpression().getReference(), code.substr(43, 19));

    const auto& assignmentExpr2 = stmt2.getAssignmentExpression();

    // identifier = x
    ASSERT_EQ(assignmentExpr2.getAssignmentTarget().getType(), parse_tree::ParseTreeNode::Type::Identifier);
    ASSERT_EQ(assignmentExpr2.getAssignmentTarget().getReference(), code.substr(43, 1));

    // generic-token = ":="
    ASSERT_EQ(assignmentExpr2.getAssignmentToken().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(assignmentExpr2.getAssignmentToken().getReference(), code.substr(45, 2));

    // additive-expression = (z * (y + 12))
    ASSERT_EQ(assignmentExpr2.getAdditiveExpression().getType(), parse_tree::ParseTreeNode::Type::AdditiveExpression);
    ASSERT_EQ(assignmentExpr2.getAdditiveExpression().getReference(), code.substr(48, 14));

    const auto& additiveExpr6 = assignmentExpr2.getAdditiveExpression();
    ASSERT_EQ(additiveExpr6.getAdditiveExpressionType(), parse_tree::AdditiveExpression::Type::None);

    ASSERT_EQ(additiveExpr6.getMultiplicativeExpression().getType(), parse_tree::ParseTreeNode::Type::MultiplicativeExpression);
    ASSERT_EQ(additiveExpr6.getMultiplicativeExpression().getReference(), code.substr(48, 14));

    const auto& mulExpr6 = additiveExpr6.getMultiplicativeExpression();
    ASSERT_EQ(mulExpr6.getMultiplicativeExpressionType(), parse_tree::MultiplicativeExpression::Type::None);

    ASSERT_EQ(mulExpr6.getUnaryExpression().getType(), parse_tree::ParseTreeNode::Type::UnaryExpression);
    ASSERT_EQ(mulExpr6.getUnaryExpression().getReference(), code.substr(48, 14));

    const auto& unaryExpr6 = mulExpr6.getUnaryExpression();
    ASSERT_EQ(unaryExpr6.getUnaryExpressionType(), parse_tree::UnaryExpression::Type::Unsigned);

    ASSERT_EQ(unaryExpr6.getPrimaryExpression().getType(), parse_tree::ParseTreeNode::Type::PrimaryExpression);
    ASSERT_EQ(unaryExpr6.getPrimaryExpression().getReference(), code.substr(48, 14));

    const auto& primaryExpr6 = unaryExpr6.getPrimaryExpression();
    ASSERT_EQ(primaryExpr6.getPrimaryExpressionType(), parse_tree::PrimaryExpression::Type::Parenthesized);

    // generic-token = "("
    ASSERT_EQ(primaryExpr6.getLeftParenthesis().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(primaryExpr6.getLeftParenthesis().getReference(), code.substr(48, 1));

    // additive-expression = z * (y + 12)
    ASSERT_EQ(primaryExpr6.getAdditiveExpression().getType(), parse_tree::ParseTreeNode::Type::AdditiveExpression);
    ASSERT_EQ(primaryExpr6.getAdditiveExpression().getReference(), code.substr(49, 12));

    const auto& additiveExpr7 = primaryExpr6.getAdditiveExpression();
    ASSERT_EQ(additiveExpr7.getAdditiveExpressionType(), parse_tree::AdditiveExpression::Type::None);

    ASSERT_EQ(additiveExpr7.getMultiplicativeExpression().getType(), parse_tree::ParseTreeNode::Type::MultiplicativeExpression);
    ASSERT_EQ(additiveExpr7.getMultiplicativeExpression().getReference(), code.substr(49, 12));

    const auto& mulExpr7 = additiveExpr7.getMultiplicativeExpression();
    ASSERT_EQ(mulExpr7.getMultiplicativeExpressionType(), parse_tree::MultiplicativeExpression::Type::Mul);

    // unary-expression = z
    ASSERT_EQ(mulExpr7.getUnaryExpression().getType(), parse_tree::ParseTreeNode::Type::UnaryExpression);
    ASSERT_EQ(mulExpr7.getUnaryExpression().getReference(), code.substr(49, 1));

    const auto& unaryExpr7 = mulExpr7.getUnaryExpression();
    ASSERT_EQ(unaryExpr7.getUnaryExpressionType(), parse_tree::UnaryExpression::Type::Unsigned);

    ASSERT_EQ(unaryExpr7.getPrimaryExpression().getType(), parse_tree::ParseTreeNode::Type::PrimaryExpression);
    ASSERT_EQ(unaryExpr7.getPrimaryExpression().getReference(), code.substr(49, 1));

    const auto& primaryExpr7 = unaryExpr7.getPrimaryExpression();
    ASSERT_EQ(primaryExpr7.getPrimaryExpressionType(), parse_tree::PrimaryExpression::Type::Identifier);

    ASSERT_EQ(primaryExpr7.getIdentifier().getType(), parse_tree::ParseTreeNode::Type::Identifier);
    ASSERT_EQ(primaryExpr7.getIdentifier().getReference(), code.substr(49, 1));

    // generic-token = "*"
    ASSERT_EQ(mulExpr7.getMultiplicativeOpToken().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(mulExpr7.getMultiplicativeOpToken().getReference(), code.substr(51, 1));

    // multiplicative-expression = (y + 12)
    ASSERT_EQ(mulExpr7.getMultiplicativeExpression().getType(), parse_tree::ParseTreeNode::Type::MultiplicativeExpression);
    ASSERT_EQ(mulExpr7.getMultiplicativeExpression().getReference(), code.substr(53, 8));

    const auto& mulExpr8 = mulExpr7.getMultiplicativeExpression();
    ASSERT_EQ(mulExpr8.getMultiplicativeExpressionType(), parse_tree::MultiplicativeExpression::Type::None);

    ASSERT_EQ(mulExpr8.getUnaryExpression().getType(), parse_tree::ParseTreeNode::Type::UnaryExpression);
    ASSERT_EQ(mulExpr8.getUnaryExpression().getReference(), code.substr(53, 8));

    const auto& unaryExpr8 = mulExpr8.getUnaryExpression();
    ASSERT_EQ(unaryExpr8.getUnaryExpressionType(), parse_tree::UnaryExpression::Type::Unsigned);

    ASSERT_EQ(unaryExpr8.getPrimaryExpression().getType(), parse_tree::ParseTreeNode::Type::PrimaryExpression);
    ASSERT_EQ(unaryExpr8.getPrimaryExpression().getReference(), code.substr(53, 8));

    const auto& primaryExpr8 = unaryExpr8.getPrimaryExpression();
    ASSERT_EQ(primaryExpr8.getPrimaryExpressionType(), parse_tree::PrimaryExpression::Type::Parenthesized);

    // generic-token = "("
    ASSERT_EQ(primaryExpr8.getLeftParenthesis().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(primaryExpr8.getLeftParenthesis().getReference(), code.substr(53, 1));

    // additive-expression = y + 12
    ASSERT_EQ(primaryExpr8.getAdditiveExpression().getType(), parse_tree::ParseTreeNode::Type::AdditiveExpression);
    ASSERT_EQ(primaryExpr8.getAdditiveExpression().getReference(), code.substr(54, 6));

    const auto& additiveExpr8 = primaryExpr8.getAdditiveExpression();
    ASSERT_EQ(additiveExpr8.getAdditiveExpressionType(), parse_tree::AdditiveExpression::Type::Add);

    // multiplicative-expression = y
    ASSERT_EQ(additiveExpr8.getMultiplicativeExpression().getType(), parse_tree::ParseTreeNode::Type::MultiplicativeExpression);
    ASSERT_EQ(additiveExpr8.getMultiplicativeExpression().getReference(), code.substr(54, 1));

    const auto& mulExpr9 = additiveExpr8.getMultiplicativeExpression();
    ASSERT_EQ(mulExpr9.getMultiplicativeExpressionType(), parse_tree::MultiplicativeExpression::Type::None);

    ASSERT_EQ(mulExpr9.getUnaryExpression().getType(), parse_tree::ParseTreeNode::Type::UnaryExpression);
    ASSERT_EQ(mulExpr9.getUnaryExpression().getReference(), code.substr(54, 1));

    const auto& unaryExpr9 = mulExpr9.getUnaryExpression();
    ASSERT_EQ(unaryExpr9.getUnaryExpressionType(), parse_tree::UnaryExpression::Type::Unsigned);

    ASSERT_EQ(unaryExpr9.getPrimaryExpression().getType(), parse_tree::ParseTreeNode::Type::PrimaryExpression);
    ASSERT_EQ(unaryExpr9.getPrimaryExpression().getReference(), code.substr(54, 1));

    const auto& primaryExpr9 = unaryExpr9.getPrimaryExpression();
    ASSERT_EQ(primaryExpr9.getPrimaryExpressionType(), parse_tree::PrimaryExpression::Type::Identifier);

    ASSERT_EQ(primaryExpr9.getIdentifier().getType(), parse_tree::ParseTreeNode::Type::Identifier);
    ASSERT_EQ(primaryExpr9.getIdentifier().getReference(), code.substr(54, 1));

    // generic-token = "+"
    ASSERT_EQ(additiveExpr8.getAdditiveOpToken().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(additiveExpr8.getAdditiveOpToken().getReference(), code.substr(56, 1));

    // additive-expression = 12
    ASSERT_EQ(additiveExpr8.getAdditiveExpression().getType(), parse_tree::ParseTreeNode::Type::AdditiveExpression);
    ASSERT_EQ(additiveExpr8.getAdditiveExpression().getReference(), code.substr(58, 2));

    const auto& additiveExpr10 = additiveExpr8.getAdditiveExpression();
    ASSERT_EQ(additiveExpr10.getAdditiveExpressionType(), parse_tree::AdditiveExpression::Type::None);

    ASSERT_EQ(additiveExpr10.getMultiplicativeExpression().getType(), parse_tree::ParseTreeNode::Type::MultiplicativeExpression);
    ASSERT_EQ(additiveExpr10.getMultiplicativeExpression().getReference(), code.substr(58, 2));

    const auto& mulExpr10 = additiveExpr10.getMultiplicativeExpression();
    ASSERT_EQ(mulExpr10.getMultiplicativeExpressionType(), parse_tree::MultiplicativeExpression::Type::None);

    ASSERT_EQ(mulExpr10.getUnaryExpression().getType(), parse_tree::ParseTreeNode::Type::UnaryExpression);
    ASSERT_EQ(mulExpr10.getUnaryExpression().getReference(), code.substr(58, 2));

    const auto& unaryExpr10 = mulExpr10.getUnaryExpression();
    ASSERT_EQ(unaryExpr10.getUnaryExpressionType(), parse_tree::UnaryExpression::Type::Unsigned);

    ASSERT_EQ(unaryExpr10.getPrimaryExpression().getType(), parse_tree::ParseTreeNode::Type::PrimaryExpression);
    ASSERT_EQ(unaryExpr10.getPrimaryExpression().getReference(), code.substr(58, 2));

    const auto& primaryExpr10 = unaryExpr10.getPrimaryExpression();
    ASSERT_EQ(primaryExpr10.getPrimaryExpressionType(), parse_tree::PrimaryExpression::Type::Literal);

    ASSERT_EQ(primaryExpr10.getLiteral().getType(), parse_tree::ParseTreeNode::Type::Literal);
    ASSERT_EQ(primaryExpr10.getLiteral().getReference(), code.substr(58, 2));
    ASSERT_EQ(primaryExpr10.getLiteral().getValue(), 12);

    // generic-token = ")"
    ASSERT_EQ(primaryExpr8.getRightParenthesis().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(primaryExpr8.getRightParenthesis().getReference(), code.substr(61, 1));

    // generic-token = ")"
    ASSERT_EQ(primaryExpr6.getRightParenthesis().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(primaryExpr6.getRightParenthesis().getReference(), code.substr(61, 1));

    ///// Separator ";"

    ASSERT_EQ(stmtListChildren[3]->getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(stmtListChildren[3]->getReference(), code.substr(62, 1));

    ///// Statement 3: RETURN +x

    // RETURN additive-expression = RETURN +x
    ASSERT_EQ(stmtListChildren[4]->getType(), parse_tree::ParseTreeNode::Type::Statement);
    ASSERT_EQ(stmtListChildren[4]->getReference(), code.substr(64, 9));

    const auto& stmt3 = static_cast<const parse_tree::Statement&>(*stmtListChildren[4]); // NOLINT
    ASSERT_EQ(stmt3.getStatementType(), parse_tree::Statement::Type::ReturnStatement);

    // generic-token = RETURN
    ASSERT_EQ(stmt3.getReturnKeyword().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(stmt3.getReturnKeyword().getReference(), code.substr(64, 6));

    // additive-expression = +x
    ASSERT_EQ(stmt3.getAdditiveExpression().getType(), parse_tree::ParseTreeNode::Type::AdditiveExpression);
    ASSERT_EQ(stmt3.getAdditiveExpression().getReference(), code.substr(71, 2));

    const auto& additiveExpr11 = stmt3.getAdditiveExpression();
    ASSERT_EQ(additiveExpr11.getAdditiveExpressionType(), parse_tree::AdditiveExpression::Type::None);

    ASSERT_EQ(additiveExpr11.getMultiplicativeExpression().getType(), parse_tree::ParseTreeNode::Type::MultiplicativeExpression);
    ASSERT_EQ(additiveExpr11.getMultiplicativeExpression().getReference(), code.substr(71, 2));

    const auto& mulExpr11 = additiveExpr11.getMultiplicativeExpression();
    ASSERT_EQ(mulExpr11.getMultiplicativeExpressionType(), parse_tree::MultiplicativeExpression::Type::None);

    ASSERT_EQ(mulExpr11.getUnaryExpression().getType(), parse_tree::ParseTreeNode::Type::UnaryExpression);
    ASSERT_EQ(mulExpr11.getUnaryExpression().getReference(), code.substr(71, 2));

    const auto& unaryExpr11 = mulExpr11.getUnaryExpression();
    ASSERT_EQ(unaryExpr11.getUnaryExpressionType(), parse_tree::UnaryExpression::Type::PlusSign);

    // generic-token = "+"
    ASSERT_EQ(unaryExpr11.getSignToken().getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(unaryExpr11.getSignToken().getReference(), code.substr(71, 1));

    // primary-expression = x
    ASSERT_EQ(unaryExpr11.getPrimaryExpression().getType(), parse_tree::ParseTreeNode::Type::PrimaryExpression);
    ASSERT_EQ(unaryExpr11.getPrimaryExpression().getReference(), code.substr(72, 1));

    const auto& primaryExpr11 = unaryExpr11.getPrimaryExpression();
    ASSERT_EQ(primaryExpr11.getPrimaryExpressionType(), parse_tree::PrimaryExpression::Type::Identifier);

    ASSERT_EQ(primaryExpr11.getIdentifier().getType(), parse_tree::ParseTreeNode::Type::Identifier);
    ASSERT_EQ(primaryExpr11.getIdentifier().getReference(), code.substr(72, 1));

    ///////////////// Program terminator

    // Program terminator = "."
    const auto& programTerminator = functionDefinition->getProgramTerminator();
    ASSERT_EQ(programTerminator.getType(), parse_tree::ParseTreeNode::Type::GenericToken);
    ASSERT_EQ(programTerminator.getReference(), code.substr(code.size() - 1, 1));
}

} // namespace pljit::parser
