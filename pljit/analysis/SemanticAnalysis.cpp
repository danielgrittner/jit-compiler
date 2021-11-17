#include "SemanticAnalysis.h"
#include "pljit/analysis/SymbolTable.h"
#include "pljit/ast/AST.h"
#include "pljit/common/SourceCodeManager.h"
#include "pljit/parse_tree/ParseTree.h"
#include <cassert>

namespace pljit::analysis {

namespace {

template <typename ASTNodeT>
std::unique_ptr<ASTNodeT> error()
{
    return nullptr;
}

template <typename ASTNodeT>
bool hasError(const std::unique_ptr<ASTNodeT>& node)
{
    return node == nullptr;
}

bool declarationSuccess()
{
    return true;
}

bool declarationError()
{
    return false;
}

std::vector<std::unique_ptr<ast::Statement>> statementsError()
{
    return {};
}

bool haveStatementsError(const std::vector<std::unique_ptr<ast::Statement>>& stmts)
{
    return stmts.empty();
}

} // namespace

SemanticAnalysis::SemanticAnalysis(const common::SourceCodeManager& sourceCodeManager,
                                   SymbolTable& symbolTable)
    : sourceCodeManager(sourceCodeManager),
      symbolTable(symbolTable)
// Constructor
{}

std::unique_ptr<ast::Function> SemanticAnalysis::analyzeFunction(
    const parse_tree::FunctionDefinition& node)
// Semantically analyzes the parse tree and builds up an AST.
{
    const auto* paramDeclarations = node.getParameterDeclarations();
    if (paramDeclarations != nullptr && !registerParameterDeclarations(*paramDeclarations)) {
        // An error occurred during the analysis of the parameter declaration.
        return error<ast::Function>();
    }

    const auto* varDeclarations = node.getVariableDeclarations();
    if (varDeclarations != nullptr && !registerVariableDeclarations(*varDeclarations)) {
        // An error occurred during the analysis of the variable declaration.
        return error<ast::Function>();
    }

    const auto* constDeclarations = node.getConstantDeclarations();
    if (constDeclarations != nullptr && !registerConstantDeclarations(*constDeclarations)) {
        // An error occurred during the analysis of the constant declaration.
        return error<ast::Function>();
    }

    auto statements = analyzeStatements(node.getCompoundStatement());
    if (haveStatementsError(statements)) {
        // An error occurred during the analysis of the statements.
        return error<ast::Function>();
    }

    if (!containsReturnStatement) {
        // The given function does not contain a return statement!
        const auto& endKeyword = node.getCompoundStatement().getEndKeyword();
        sourceCodeManager.printContext(endKeyword.getReference(),
                                       "error: function does not "
                                       "contain a return-statement");
        return error<ast::Function>();
    }

    return std::make_unique<ast::Function>(std::move(statements));
}

bool SemanticAnalysis::registerDeclaratorList(const parse_tree::DeclaratorList& node,
                                              ast::Identifier::Type symbolType)
// Registers all symbols with the given type declared in the declarator-list.
{
    for (const auto& child : node.getCommaSeparatedIdentifiers()) {
        if (child->getType() == parse_tree::ParseTreeNode::Type::GenericToken) {
            // Skip the commas!
            assert(child->getReference() == std::string_view{","});
            continue;
        }

        // Register the parameter in the symbol table.
        assert(child->getType() == parse_tree::ParseTreeNode::Type::Identifier);
        auto result = symbolTable.registerSymbol(symbolType, child->getReference());
        if (!result.newlyRegistered) {
            // The symbol was already registered, hence, we found a duplicate declaration!
            sourceCodeManager.printContext(child->getReference(),
                                           "error: duplicate declaration of identifier");
            sourceCodeManager.printContext(result.entry.declarationRef,
                                           "note: already declared here");
            return declarationError();
        }
    }

    return declarationSuccess();
}

bool SemanticAnalysis::registerParameterDeclarations(const parse_tree::ParameterDeclarations& node)
// Registers all parameters in the symbol table.
{
    return registerDeclaratorList(node.getDeclaratorList(), ast::Identifier::Type::Parameter);
}

bool SemanticAnalysis::registerVariableDeclarations(const parse_tree::VariableDeclarations& node)
// Registers all variables in the symbol table.
{
    return registerDeclaratorList(node.getDeclaratorList(), ast::Identifier::Type::Variable);
}

bool SemanticAnalysis::registerConstantDeclarations(const parse_tree::ConstantDeclarations& node)
// Registers all constants in the symbol table. Moreover, stores the value assigned to a constant.
{
    const auto& initDeclaratorList = node.getInitDeclaratorList();

    for (const auto& child : initDeclaratorList.getCommaSeparatedInitDeclarators()) {
        if (child->getType() == parse_tree::ParseTreeNode::Type::GenericToken) {
            // Skip the commas!
            assert(child->getReference() == std::string_view{","});
            continue;
        }

        assert(child->getType() == parse_tree::ParseTreeNode::Type::InitDeclarator);
        const auto& initDeclarator = static_cast<parse_tree::InitDeclarator&>(*child); // NOLINT

        // First, we check whether the symbol of the constant was already declared. If not,
        // we register its symbol in the symbol table and store its value.
        const auto& identifier = initDeclarator.getInitTarget();
        auto result = symbolTable.registerSymbol(ast::Identifier::Type::Constant,
                                                 identifier.getReference(),
                                                 initDeclarator.getLiteral().getValue());

        if (!result.newlyRegistered) {
            // The symbol was already registered, hence, we found a duplicate declaration!
            sourceCodeManager.printContext(identifier.getReference(),
                                           "error: duplicate declaration of identifier");
            sourceCodeManager.printContext(result.entry.declarationRef,
                                           "note: already declared here");
            return declarationError();
        }
    }

    return declarationSuccess();
}

std::vector<std::unique_ptr<ast::Statement>> SemanticAnalysis::analyzeStatements(
    const parse_tree::CompoundStatement& node)
// Semantically analyzes the compound statement which means analyzing the statement list.
{
    std::vector<std::unique_ptr<ast::Statement>> statements;

    const auto& statementList = node.getStatementList();

    for (const auto& child : statementList.getStatementsSeparatedBySemiColon()) {
        if (child->getType() == parse_tree::ParseTreeNode::Type::GenericToken) {
            // Skip the semi-colons!
            assert(child->getReference() == std::string_view{";"});
            continue;
        }

        // We have a statement which we now need to analyze!
        assert(child->getType() == parse_tree::ParseTreeNode::Type::Statement);
        auto astStatement = analyzeStatement(static_cast<parse_tree::Statement&>(*child)); // NOLINT
        if (hasError(astStatement)) {
            return statementsError();
        }
        statements.push_back(std::move(astStatement));
    }

    assert(statements.size() == 1 || statements.size() ==
               (statementList.getStatementsSeparatedBySemiColon().size() - statements.size() + 1));

    return statements;
}

std::unique_ptr<ast::Statement> SemanticAnalysis::analyzeStatement(const parse_tree::Statement& node)
// Semantically analyzes a statement of the parse tree.
{
    auto statementType = node.getStatementType();

    if (statementType == parse_tree::Statement::Type::ReturnStatement) {
        // We found a return statement!
        containsReturnStatement = true;

        const auto& additiveExpression = node.getAdditiveExpression();

        auto astExpr = analyzeExpression(additiveExpression);
        if (hasError(astExpr)) {
            return error<ast::Statement>();
        }

        return std::make_unique<ast::ReturnStatement>(std::move(astExpr));
    }

    // We must have an assignment statement!
    assert(statementType == parse_tree::Statement::Type::AssignmentStatement);

    // We need to unfold the assignment-expression into its identifier and its additive-expression.
    const auto& assignmentExpr = node.getAssignmentExpression();

    // We first analyze the additive expression.
    const auto& additiveExpression = assignmentExpr.getAdditiveExpression();

    auto astExpr = analyzeExpression(additiveExpression);
    if (hasError(astExpr)) {
        return error<ast::Statement>();
    }

    // Now, we need to analyze the assignment target.

    const auto& identifier = assignmentExpr.getAssignmentTarget();
    std::string_view identifierSymbol = identifier.getReference();

    // Check whether the identifier was declared.
    auto lookUpResultOpt = symbolTable.lookUpSymbol(identifierSymbol);
    if (!lookUpResultOpt) {
        // Trying to assign to an undeclared identifier!
        sourceCodeManager.printContext(identifier.getReference(),
                                       "error: use of undeclared identifier");
        return error<ast::Statement>();
    }

    // Ensure that the identifier is not a constant.
    auto lookUpResult = lookUpResultOpt.value();
    if (lookUpResult.symbolType == ast::Identifier::Type::Constant) {
        // Trying to assign to a constant!
        sourceCodeManager.printContext(identifier.getReference(),
                                       "error: trying to assign to an "
                                       "identifier declared 'CONST'");
        sourceCodeManager.printContext(lookUpResult.declarationRef,
                                       "note: declared as 'CONST' here");
        return error<ast::Statement>();
    }

    // Create the assignment target.
    auto assignmentTarget =
        std::make_unique<ast::Identifier>(lookUpResult.symbolType, lookUpResult.symbolId);

    // Is the assignment target a variable?
    if (lookUpResult.symbolType == ast::Identifier::Type::Variable) {
        // Remember that the variable now has a value assigned! It is now safe to use
        // in case it wasn't before.
        initializedVariables.insert(lookUpResult.symbolId);
    }

    return std::make_unique<ast::AssignmentStatement>(std::move(assignmentTarget), std::move(astExpr));
}

std::unique_ptr<ast::Expression> SemanticAnalysis::analyzeExpression(
    const parse_tree::AdditiveExpression& node)
// Semantically analyzes an additive-expression.
{
    // The first child is always a multiplicative expression and does always exist.
    auto mulExpr = analyzeExpression(node.getMultiplicativeExpression());
    if (hasError(mulExpr)) {
        return error<ast::Expression>();
    }

    auto addExprType = node.getAdditiveExpressionType();

    if (addExprType == parse_tree::AdditiveExpression::Type::None) {
        // The expression is just a multiplicative expression!
        return mulExpr;
    }

    // The expression must be either an addition or subtraction!
    assert(addExprType == parse_tree::AdditiveExpression::Type::Add ||
           addExprType == parse_tree::AdditiveExpression::Type::Sub);

    // Determine the sign of the additive-expression.
    auto binaryOpType = node.getAdditiveExpressionType() == parse_tree::AdditiveExpression::Type::Add
                        ? ast::BinaryOp::Type::Add : ast::BinaryOp::Type::Sub;

    // Analyze the last child which is an additive-expression.
    auto addExpr = analyzeExpression(node.getAdditiveExpression());
    if (hasError(addExpr)) {
        return error<ast::Expression>();
    }

    return std::make_unique<ast::BinaryOp>(std::move(mulExpr), binaryOpType, std::move(addExpr));
}

std::unique_ptr<ast::Expression> SemanticAnalysis::analyzeExpression(
    const parse_tree::MultiplicativeExpression& node)
// Semantically analyzes a multiplicative-expression.
{
    // The first child is always a unary expression and does always exist.
    auto unaryExpr = analyzeExpression(node.getUnaryExpression());
    if (hasError(unaryExpr)) {
        return error<ast::Expression>();
    }

    auto mulExprType = node.getMultiplicativeExpressionType();

    if (mulExprType == parse_tree::MultiplicativeExpression::Type::None) {
        // The expression is just a unary expression!
        return unaryExpr;
    }

    // The expression must be either a multiplication or division!
    assert(mulExprType == parse_tree::MultiplicativeExpression::Type::Mul ||
           mulExprType == parse_tree::MultiplicativeExpression::Type::Div);

    // Determine the operation type of the multiplicative-expression.
    auto binaryOpType = node.getMultiplicativeExpressionType()
                        == parse_tree::MultiplicativeExpression::Type::Mul
                        ? ast::BinaryOp::Type::Mul : ast::BinaryOp::Type::Div;

    // Analyze the last child which is again a multiplicative-expression.
    auto mulExpr = analyzeExpression(node.getMultiplicativeExpression());
    if (hasError(mulExpr)) {
        return error<ast::Expression>();
    }

    return std::make_unique<ast::BinaryOp>(std::move(unaryExpr), binaryOpType, std::move(mulExpr));
}

std::unique_ptr<ast::Expression> SemanticAnalysis::analyzeExpression(
    const parse_tree::UnaryExpression& node)
// Semantically analyzes a unary-expression.
{
    auto primaryExpr = analyzeExpression(node.getPrimaryExpression());
    if (hasError(primaryExpr)) {
        return error<ast::Expression>();
    }

    auto unaryExprType = node.getUnaryExpressionType();

    if (node.getUnaryExpressionType() == parse_tree::UnaryExpression::Type::Unsigned) {
        // The unary expression does not have a sign!
        return primaryExpr;
    }

    // The unary expression must have a sign!
    assert(unaryExprType == parse_tree::UnaryExpression::Type::PlusSign ||
           unaryExprType == parse_tree::UnaryExpression::Type::MinusSign);

    auto unaryOpType = node.getUnaryExpressionType() == parse_tree::UnaryExpression::Type::PlusSign ?
                       ast::UnaryOp::Type::PlusSign : ast::UnaryOp::Type::MinusSign;

    return std::make_unique<ast::UnaryOp>(unaryOpType, std::move(primaryExpr));
}

std::unique_ptr<ast::Expression> SemanticAnalysis::analyzeExpression(
    const parse_tree::PrimaryExpression& node)
// Semantically analyzes a primary-expression.
{
    switch (node.getPrimaryExpressionType()) {
        case parse_tree::PrimaryExpression::Type::Literal:
            return analyzeExpression(node.getLiteral());

        case parse_tree::PrimaryExpression::Type::Identifier:
            return analyzeExpression(node.getIdentifier()); // NOLINT

        case parse_tree::PrimaryExpression::Type::Parenthesized:
            // In the case of a parenthesized primary expression, we are only interested
            // in the additive-expression. The parenthesis do not matter.
            return analyzeExpression(node.getAdditiveExpression());

        default:
            // This case should never be reached since all cases are handled,
            // but clang-tidy still complains...
            __builtin_unreachable();
    }
}

std::unique_ptr<ast::Expression> SemanticAnalysis::analyzeExpression(
    const parse_tree::Identifier& node)
// Semantically analyzes an identifier.
{
    std::string_view symbolString = node.getReference();
    auto lookUpResultOp = symbolTable.lookUpSymbol(symbolString);

    if (!lookUpResultOp) {
        // We found an undeclared identifier!
        sourceCodeManager.printContext(node.getReference(),
                                       "error: use of undeclared identifier");
        return error<ast::Expression>();
    }

    auto lookUpResult = lookUpResultOp.value();

    if (lookUpResult.symbolType == ast::Identifier::Type::Variable) {
        // We have a variable! For variables, we need to check
        // whether they already have a value assigned to them.
        if (!initializedVariables.contains(lookUpResult.symbolId)) {
            // Trying to use an uninitialized variable!
            sourceCodeManager.printContext(node.getReference(),
                                           "error: use of uninitialized identifier");
            return error<ast::Expression>();
        }
    }

    return std::make_unique<ast::Identifier>(lookUpResult.symbolType, lookUpResult.symbolId);
}

std::unique_ptr<ast::Expression> SemanticAnalysis::analyzeExpression( // NOLINT
    const parse_tree::Literal& node)
// Semantically analyzes a literal.
{
    return std::make_unique<ast::ConstantLiteral>(node.getValue());
}

} // namespace pljit::analysis
