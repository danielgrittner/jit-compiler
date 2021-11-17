#include "Parser.h"
#include "pljit/parse_tree/ParseTree.h"
#include <cassert>

namespace pljit::parser {

namespace {

common::SourceRangeReference getStartingRangeReferenceForFunctionDef(
    const std::unique_ptr<parse_tree::ParameterDeclarations>& parameterDeclarations,
    const std::unique_ptr<parse_tree::VariableDeclarations>& variableDeclarations,
    const std::unique_ptr<parse_tree::ConstantDeclarations>& constantDeclarations,
    const std::unique_ptr<parse_tree::CompoundStatement>& compoundStatement)
{
    if (parameterDeclarations != nullptr) {
        return parameterDeclarations->getReference();
    }
    if (variableDeclarations != nullptr) {
        return variableDeclarations->getReference();
    }
    if (constantDeclarations != nullptr) {
        return constantDeclarations->getReference();
    }
    return compoundStatement->getReference();
}

/// This function parses an unsigned integer into an int64_t.
int64_t parseLiteralToInt64(std::string_view str)
{
    uint64_t result = 0;
    uint64_t shift = 1;
    size_t offset = str.size();
    for (; offset > 0; --offset, shift *= 10) {
        result += static_cast<uint64_t>(str[offset - 1] - '0') * shift;
    }
    return static_cast<int64_t>(result);
}

constexpr bool NO_TOKEN_LEFT = true;

std::string_view getErrorMessageForToken(lexer::Token::Type tokenType,
                                         bool noTokenLeft = false)
{
    // Note: Only tokens which need an error message were added to the switch
    //       statement.
    switch (tokenType) {
        case lexer::Token::Type::SemiColon:
            return noTokenLeft ? "error: expected ';' afterwards" : "error: expected ';'";

        case lexer::Token::Type::Comma:
            return noTokenLeft ? "error: expected ',' afterwards" : "error: expected ','";

        case lexer::Token::Type::Identifier:
            return noTokenLeft ? "error: expected identifier afterwards" : "error: expected identifier";

        case lexer::Token::Type::Literal:
            return noTokenLeft ? "error: expected literal afterwards" : "error: expected literal";

        case lexer::Token::Type::Init:
            return noTokenLeft ? "error: expected '=' afterwards" : "error: expected '='";

        case lexer::Token::Type::Begin:
            return noTokenLeft ? "error: expected 'BEGIN' afterwards" : "error: expected 'BEGIN'";

        case lexer::Token::Type::End:
            return noTokenLeft ? "error: expected 'END' afterwards" : "error: expected 'END'";

        case lexer::Token::Type::ProgramTerminator:
            return noTokenLeft ? "error: expected '.' afterwards" : "error: expected '.'";

        case lexer::Token::Type::Assignment:
            return noTokenLeft ? "error: expected ':=' afterwards" : "error: expected ':='";

        case lexer::Token::Type::LeftParenthesis:
            return noTokenLeft ? "error: expected '(' afterwards" : "error: expected '('";

        case lexer::Token::Type::RightParenthesis:
            return noTokenLeft ? "error: expected ')' afterwards" : "error: expected ')'";

        default:
            return "unknown error";
    }
}

/// Function for returning errors.
template <typename ParseTreeNodeT>
std::unique_ptr<ParseTreeNodeT> error()
{
    return nullptr;
}

template <typename ParseTreeNodeT>
bool hasError(const std::unique_ptr<ParseTreeNodeT>& node)
{
    return node == nullptr;
}

} // namespace

Parser::Parser(const common::SourceCodeManager& sourceCodeManager)
    : sourceCodeManager(sourceCodeManager),
      lexer(sourceCodeManager),
      refToLastChar(lexer.peek().getReference().first())
// Constructor
{}

std::unique_ptr<parse_tree::FunctionDefinition> Parser::parseFunctionDefinition()
// Parses the source code and returns a parse tree.
{
    // Try to parse the optional parameter declarations. If is is a
    // parameter declaration, then the next token must be the keyword
    // PARAM. If this is the case, then we parse it.
    std::unique_ptr<parse_tree::ParameterDeclarations> parameterDeclarations{};
    if (lexer.hasNext() && lexer.peek().getTokenType() == lexer::Token::Type::Param) {
        parameterDeclarations = parseParameterDeclarations();
        if (hasError(parameterDeclarations)) {
            return error<parse_tree::FunctionDefinition>();
        }

        if (!lexer.hasNext()) {
            sourceCodeManager.printContext(parameterDeclarations->getReference().last(),
                                           "error: expected afterwards either 'VAR', 'CONST', or 'BEGIN'");
            return error<parse_tree::FunctionDefinition>();
        }
    }

    // We now check whether we have variable-declarations.
    // This is the case if the next token is the keyword VAR.
    std::unique_ptr<parse_tree::VariableDeclarations> variableDeclarations{};
    if (lexer.hasNext() && lexer.peek().getTokenType() == lexer::Token::Type::Var) {
        variableDeclarations = parseVariableDeclarations();
        if (hasError(variableDeclarations)) {
            return error<parse_tree::FunctionDefinition>();
        }

        if (!lexer.hasNext()) {
            sourceCodeManager.printContext(variableDeclarations->getReference().last(),
                                           "error: expected afterwards either 'CONST' or 'BEGIN'");
            return error<parse_tree::FunctionDefinition>();
        }
    }

    // And finally, we check whether we have constant-declarations.
    // This is the case if the next token is the keyword CONST.
    std::unique_ptr<parse_tree::ConstantDeclarations> constantDeclarations{};
    if (lexer.hasNext() && lexer.peek().getTokenType() == lexer::Token::Type::Const) {
        constantDeclarations = parseConstantDeclarations();
        if (hasError(constantDeclarations)) {
            return error<parse_tree::FunctionDefinition>();
        }

        if (!lexer.hasNext()) {
            sourceCodeManager.printContext(constantDeclarations->getReference().last(),
                                           "error: expected afterwards 'BEGIN'");
            return error<parse_tree::FunctionDefinition>();
        }
    }

    // Now, parse the compound-statement. This should always exist.
    auto compoundStatement = parseCompoundStatement();
    if (hasError(compoundStatement)) {
        // Failed to parse compound statement.
        return error<parse_tree::FunctionDefinition>();
    }

    // And finally, we need to parse the program terminator. This should also always exist.
    auto programTerminator = parseGenericToken(lexer::Token::Type::ProgramTerminator);
    if (hasError(programTerminator)) {
        // Failed to parse program terminator
        return error<parse_tree::FunctionDefinition>();
    }

    if (lexer.hasNext()) {
        sourceCodeManager.printContext(lexer.peek().getReference(),
                                       "error: expected no tokens after the program terminator");
        return error<parse_tree::FunctionDefinition>();
    }

    auto startingRangeRef = getStartingRangeReferenceForFunctionDef(parameterDeclarations,
                                                                    variableDeclarations,
                                                                    constantDeclarations,
                                                                    compoundStatement);
    auto ref = startingRangeRef.extendUntil(programTerminator->getReference().last());

    return std::make_unique<parse_tree::FunctionDefinition>(std::move(parameterDeclarations),
                                                            std::move(variableDeclarations),
                                                            std::move(constantDeclarations),
                                                            std::move(compoundStatement),
                                                            std::move(programTerminator),
                                                            ref);
}

std::unique_ptr<parse_tree::Identifier> Parser::parseIdentifier()
// Parses an identifier.
{
    if (!lexer.hasNext()) {
        // No token left although expected!
        sourceCodeManager.printContext(refToLastChar,
                                       getErrorMessageForToken(lexer::Token::Type::Identifier,
                                                               NO_TOKEN_LEFT));
        return error<parse_tree::Identifier>();
    }

    auto token = lexer.next();

    if (token.hasError()) {
        // Error during lexing.
        return error<parse_tree::Identifier>();
    }

    if (token.getTokenType() != lexer::Token::Type::Identifier) {
        // Received a different token than expected!
        sourceCodeManager.printContext(token.getReference(),
                                       getErrorMessageForToken(lexer::Token::Type::Identifier));
        return error<parse_tree::Identifier>();
    }

    refToLastChar = token.getReference().last();

    return std::make_unique<parse_tree::Identifier>(token.getReference());
}

std::unique_ptr<parse_tree::Literal> Parser::parseLiteral()
// Parses a literal.
{
    if (!lexer.hasNext()) {
        // No token left although expected!
        sourceCodeManager.printContext(refToLastChar,
                                       getErrorMessageForToken(lexer::Token::Type::Literal,
                                                               NO_TOKEN_LEFT));
        return error<parse_tree::Literal>();
    }

    auto token = lexer.next();

    if (token.hasError()) {
        // Error during lexing.
        return error<parse_tree::Literal>();
    }

    if (token.getTokenType() != lexer::Token::Type::Literal) {
        // Received a different token than expected!
        sourceCodeManager.printContext(token.getReference(),
                                       getErrorMessageForToken(lexer::Token::Type::Literal));
        return error<parse_tree::Literal>();
    }

    refToLastChar = token.getReference().last();

    // Parse the literal into a int64_t.
    int64_t value = parseLiteralToInt64(token.getReference());

    return std::make_unique<parse_tree::Literal>(value, token.getReference());
}

std::unique_ptr<parse_tree::GenericToken> Parser::parseGenericToken(
    lexer::Token::Type expectedTokenType)
// Parses a generic token of an expected token type.
{
    if (!lexer.hasNext()) {
        // No token left although expected!
        sourceCodeManager.printContext(refToLastChar,
                                       getErrorMessageForToken(expectedTokenType, NO_TOKEN_LEFT));
        return error<parse_tree::GenericToken>();
    }

    auto token = lexer.next();

    if (token.hasError()) {
        // Error during lexing.
        return error<parse_tree::GenericToken>();
    }

    if (token.getTokenType() != expectedTokenType) {
        // Received a different token than expected!
        sourceCodeManager.printContext(token.getReference(),
                                       getErrorMessageForToken(expectedTokenType));
        return error<parse_tree::GenericToken>();
    }

    refToLastChar = token.getReference().last();

    return std::make_unique<parse_tree::GenericToken>(token.getReference());
}

std::unique_ptr<parse_tree::ParameterDeclarations> Parser::parseParameterDeclarations()
{
    auto paramKeyword = parseGenericToken(lexer::Token::Type::Param);
    if (hasError(paramKeyword)) {
        return error<parse_tree::ParameterDeclarations>();
    }

    auto declaratorList = parseDeclaratorList();
    if (hasError(declaratorList)) {
        return error<parse_tree::ParameterDeclarations>();
    }

    auto semiColon = parseGenericToken(lexer::Token::Type::SemiColon);
    if (hasError(semiColon)) {
        return error<parse_tree::ParameterDeclarations>();
    }

    auto newRangeRef = paramKeyword->getReference()
                           .extendUntil(semiColon->getReference().last());

    return std::make_unique<parse_tree::ParameterDeclarations>(std::move(paramKeyword),
                                                   std::move(declaratorList),
                                                   std::move(semiColon),
                                                   newRangeRef);
}

std::unique_ptr<parse_tree::VariableDeclarations> Parser::parseVariableDeclarations()
{
    auto varKeyword = parseGenericToken(lexer::Token::Type::Var);
    if (hasError(varKeyword)) {
        return error<parse_tree::VariableDeclarations>();
    }

    auto declaratorList = parseDeclaratorList();
    if (hasError(declaratorList)) {
        return error<parse_tree::VariableDeclarations>();
    }

    auto semiColon = parseGenericToken(lexer::Token::Type::SemiColon);
    if (hasError(semiColon)) {
        return error<parse_tree::VariableDeclarations>();
    }

    auto newRangeRef = varKeyword->getReference()
                           .extendUntil(semiColon->getReference().last());

    return std::make_unique<parse_tree::VariableDeclarations>(std::move(varKeyword),
                                                              std::move(declaratorList),
                                                              std::move(semiColon),
                                                              newRangeRef);
}

std::unique_ptr<parse_tree::ConstantDeclarations> Parser::parseConstantDeclarations()
{
    auto constKeyword = parseGenericToken(lexer::Token::Type::Const);
    if (hasError(constKeyword)) {
        return error<parse_tree::ConstantDeclarations>();
    }

    auto initDeclaratorList = parseInitDeclaratorList();
    if (hasError(initDeclaratorList)) {
        return error<parse_tree::ConstantDeclarations>();
    }

    auto semiColon = parseGenericToken(lexer::Token::Type::SemiColon);
    if (hasError(semiColon)) {
        return error<parse_tree::ConstantDeclarations>();
    }

    auto newRangeRef = constKeyword->getReference()
                           .extendUntil(semiColon->getReference().last());

    return std::make_unique<parse_tree::ConstantDeclarations>(std::move(constKeyword),
                                                              std::move(initDeclaratorList),
                                                              std::move(semiColon),
                                                              newRangeRef);
}

std::unique_ptr<parse_tree::DeclaratorList> Parser::parseDeclaratorList()
{
    std::vector<std::unique_ptr<parse_tree::ParseTreeNode>> children;

    auto firstIdentifier = parseIdentifier();
    if (hasError(firstIdentifier)) {
        return error<parse_tree::DeclaratorList>();
    }
    children.push_back(std::move(firstIdentifier));

    // We can have arbitrarily many identifiers separated by commas. We iterate
    // as long as the peeked token is a comma because this implies more identifiers.
    while (lexer.hasNext() && lexer.peek().getTokenType() == lexer::Token::Type::Comma) {
        // Parse the separator!
        auto comma = parseGenericToken(lexer::Token::Type::Comma);
        if (hasError(comma)) {
            return error<parse_tree::DeclaratorList>();
        }
        children.push_back(std::move(comma));

        // Parse the identifier!
        auto identifier = parseIdentifier();
        if (hasError(identifier)) {
            return error<parse_tree::DeclaratorList>();
        }
        children.push_back(std::move(identifier));
    }

    auto newRangeRef = children.front() != children.back() ?
        children.front()->getReference().extendUntil(children.back()->getReference().last()) :
        children.front()->getReference();

    return std::make_unique<parse_tree::DeclaratorList>(std::move(children), newRangeRef);
}

std::unique_ptr<parse_tree::InitDeclaratorList> Parser::parseInitDeclaratorList()
{
    std::vector<std::unique_ptr<parse_tree::ParseTreeNode>> children;

    auto firstInitDeclarator = parseInitDeclarator();
    if (hasError(firstInitDeclarator)) {
        return error<parse_tree::InitDeclaratorList>();
    }
    children.push_back(std::move(firstInitDeclarator));

    // We can have arbitrarily many init-declarators separated by commas. We iterate
    // as long as the peeked token is a comma because this implies more init-declarators.
    while (lexer.hasNext() && lexer.peek().getTokenType() == lexer::Token::Type::Comma) {
        // Parse the separator!
        auto comma = parseGenericToken(lexer::Token::Type::Comma);
        if (hasError(comma)) {
            return error<parse_tree::InitDeclaratorList>();
        }
        children.push_back(std::move(comma));

        // Parse the init-declarator!
        auto initDeclarator = parseInitDeclarator();
        if (hasError(initDeclarator)) {
            return error<parse_tree::InitDeclaratorList>();
        }
        children.push_back(std::move(initDeclarator));
    }

    auto newRangeRef = children.front() != children.back() ?
        children.front()->getReference().extendUntil(children.back()->getReference().last()) :
        children.front()->getReference();

    return std::make_unique<parse_tree::InitDeclaratorList>(std::move(children), newRangeRef);
}

std::unique_ptr<parse_tree::InitDeclarator> Parser::parseInitDeclarator()
{
    auto identifier = parseIdentifier();
    if (hasError(identifier)) {
        return error<parse_tree::InitDeclarator>();
    }

    auto init = parseGenericToken(lexer::Token::Type::Init);
    if (hasError(init)) {
        return error<parse_tree::InitDeclarator>();
    }

    auto literal = parseLiteral();
    if (hasError(literal)) {
        return error<parse_tree::InitDeclarator>();
    }

    auto newRangeRef = identifier->getReference()
                           .extendUntil(literal->getReference().last());

    return std::make_unique<parse_tree::InitDeclarator>(std::move(identifier),
                                                        std::move(init),
                                                        std::move(literal),
                                                        newRangeRef);
}

std::unique_ptr<parse_tree::CompoundStatement> Parser::parseCompoundStatement()
{
    auto beginKeyword = parseGenericToken(lexer::Token::Type::Begin);
    if (hasError(beginKeyword)) {
        return error<parse_tree::CompoundStatement>();
    }

    auto statementList = parseStatementList();
    if (hasError(statementList)) {
        return error<parse_tree::CompoundStatement>();
    }

    auto endKeyword = parseGenericToken(lexer::Token::Type::End);
    if (hasError(endKeyword)) {
        sourceCodeManager.printContext(beginKeyword->getReference(),
                                       "note: to match this 'BEGIN'");
        return error<parse_tree::CompoundStatement>();
    }

    auto newRangeRef = beginKeyword->getReference()
                           .extendUntil(endKeyword->getReference().last());

    return std::make_unique<parse_tree::CompoundStatement>(std::move(beginKeyword),
                                                           std::move(statementList),
                                                           std::move(endKeyword),
                                                           newRangeRef);
}

std::unique_ptr<parse_tree::StatementList> Parser::parseStatementList()
{
    std::vector<std::unique_ptr<parse_tree::ParseTreeNode>> children;

    auto firstStatement = parseStatement();
    if (hasError(firstStatement)) {
        return error<parse_tree::StatementList>();
    }
    children.push_back(std::move(firstStatement));

    // We can have arbitrarily many statements separated by semi-colons. We iterate
    // as long as the peeked token is a semi-colon because this implies more statements.
    while (lexer.hasNext() && lexer.peek().getTokenType() == lexer::Token::Type::SemiColon) {
        auto semiColon = parseGenericToken(lexer::Token::Type::SemiColon);
        if (hasError(semiColon)) {
            return error<parse_tree::StatementList>();
        }
        children.push_back(std::move(semiColon));

        auto statement = parseStatement();
        if (hasError(statement)) {
            return error<parse_tree::StatementList>();
        }
        children.push_back(std::move(statement));
    }

    auto newRangeRef = children.front() != children.back() ?
        children.front()->getReference().extendUntil(children.back()->getReference().last()) :
        children.front()->getReference();

    return std::make_unique<parse_tree::StatementList>(std::move(children), newRangeRef);
}

std::unique_ptr<parse_tree::Statement> Parser::parseStatement()
{
    if (!lexer.hasNext()) {
        sourceCodeManager.printContext(refToLastChar,
                                       "error: expected statement afterwards");
        return error<parse_tree::Statement>();
    }

    // Check whether we expect a return-statement or an assignment expression.
    if (lexer.peek().getTokenType() == lexer::Token::Type::Return) {
        // We expect a return statement.
        auto returnKeyword = parseGenericToken(lexer::Token::Type::Return);
        if (hasError(returnKeyword)) {
            return error<parse_tree::Statement>();
        }

        auto additiveExpression = parseAdditiveExpression();
        if (hasError(additiveExpression)) {
            return error<parse_tree::Statement>();
        }

        auto newRangeRef = returnKeyword->getReference()
                               .extendUntil(additiveExpression->getReference().last());

        return std::make_unique<parse_tree::Statement>(std::move(returnKeyword),
                                                       std::move(additiveExpression),
                                                       newRangeRef);
    }

    // Can this be an assignment expression?
    if (lexer.peek().getTokenType() != lexer::Token::Type::Identifier) {
        sourceCodeManager.printContext(lexer.peek().getReference(),
                                       "error: expected statement");
        return error<parse_tree::Statement>();
    }

    // We expect an assignment expression.
    assert(lexer.peek().getTokenType() == lexer::Token::Type::Identifier);
    auto assignmentExpression = parseAssignmentExpression();
    if (hasError(assignmentExpression)) {
        return error<parse_tree::Statement>();
    }

    auto newRangeRef = assignmentExpression->getReference();

    return std::make_unique<parse_tree::Statement>(std::move(assignmentExpression),
                                                   newRangeRef);
}

std::unique_ptr<parse_tree::AssignmentExpression> Parser::parseAssignmentExpression()
{
    auto identifier = parseIdentifier();
    if (hasError(identifier)) {
        return error<parse_tree::AssignmentExpression>();
    }

    auto assignment = parseGenericToken(lexer::Token::Type::Assignment);
    if (hasError(assignment)) {
        return error<parse_tree::AssignmentExpression>();
    }

    auto additiveExpression = parseAdditiveExpression();
    if (hasError(additiveExpression)) {
        return error<parse_tree::AssignmentExpression>();
    }

    auto newRangeRef = identifier->getReference()
                           .extendUntil(additiveExpression->getReference().last());

    return std::make_unique<parse_tree::AssignmentExpression>(std::move(identifier),
                                                              std::move(assignment),
                                                              std::move(additiveExpression),
                                                              newRangeRef);
}

std::unique_ptr<parse_tree::AdditiveExpression> Parser::parseAdditiveExpression()
{
    auto multiplicativeExpression = parseMultiplicativeExpression();
    if (hasError(multiplicativeExpression)) {
        return error<parse_tree::AdditiveExpression>();
    }

    // Check whether we have + or -, i.e. an additive expression.
    if (lexer.hasNext() && (lexer.peek().getTokenType() == lexer::Token::Type::OpPlus ||
                            lexer.peek().getTokenType() == lexer::Token::Type::OpMinus)) {
        auto op = parseGenericToken(lexer.peek().getTokenType());
        if (hasError(op)) {
            return error<parse_tree::AdditiveExpression>();
        }

        auto additiveExpression = parseAdditiveExpression();
        if (hasError(additiveExpression)) {
            return error<parse_tree::AdditiveExpression>();
        }

        auto newRangeRef = multiplicativeExpression->getReference()
                               .extendUntil(additiveExpression->getReference().last());

        return std::make_unique<parse_tree::AdditiveExpression>(std::move(multiplicativeExpression),
                                                                std::move(op),
                                                                std::move(additiveExpression),
                                                                newRangeRef);
    }

    // We only have the multiplicative expression.
    auto newRangeRef = multiplicativeExpression->getReference();

    return std::make_unique<parse_tree::AdditiveExpression>(std::move(multiplicativeExpression),
                                                            newRangeRef);
}

std::unique_ptr<parse_tree::MultiplicativeExpression> Parser::parseMultiplicativeExpression()
{
    auto unaryExpression = parseUnaryExpression();
    if (hasError(unaryExpression)) {
        return error<parse_tree::MultiplicativeExpression>();
    }

    // Check whether we have * or /, i.e. a multiplicative expression.
    if (lexer.hasNext() && (lexer.peek().getTokenType() == lexer::Token::Type::OpMul
                            || lexer.peek().getTokenType() == lexer::Token::Type::OpDiv)) {
        auto op = parseGenericToken(lexer.peek().getTokenType());
        if (hasError(op)) {
            return error<parse_tree::MultiplicativeExpression>();
        }

        auto multiplicativeExpression = parseMultiplicativeExpression();
        if (hasError(multiplicativeExpression)) {
            return error<parse_tree::MultiplicativeExpression>();
        }

        auto newRangeRef = unaryExpression->getReference()
                               .extendUntil(multiplicativeExpression->getReference().last());

        return std::make_unique<parse_tree::MultiplicativeExpression>(std::move(unaryExpression),
                                                                      std::move(op),
                                                                      std::move(multiplicativeExpression),
                                                                      newRangeRef);
    }

    // We only have the unary expression.
    auto newRangeRef = unaryExpression->getReference();

    return std::make_unique<parse_tree::MultiplicativeExpression>(std::move(unaryExpression),
                                                                  newRangeRef);
}

std::unique_ptr<parse_tree::UnaryExpression> Parser::parseUnaryExpression()
{
    if (!lexer.hasNext()) {
        sourceCodeManager.printContext(refToLastChar,
                                       "error: expected unary-expression or primary-expression afterwards");
        return error<parse_tree::UnaryExpression>();
    }

    auto nextTokenType = lexer.peek().getTokenType();
    bool hasSign = nextTokenType == lexer::Token::Type::OpPlus || nextTokenType == lexer::Token::Type::OpMinus;

    if (hasSign) {
        auto sign = parseGenericToken(nextTokenType);
        if (hasError(sign)) {
            return error<parse_tree::UnaryExpression>();
        }

        auto primaryExpression = parsePrimaryExpression();
        if (hasError(primaryExpression)) {
            return error<parse_tree::UnaryExpression>();
        }

        auto newRangeRef = sign->getReference()
                               .extendUntil(primaryExpression->getReference().last());

        return std::make_unique<parse_tree::UnaryExpression>(std::move(sign),
                                                             std::move(primaryExpression),
                                                             newRangeRef);
    }

    // We have an unsigned unary expression.
    auto primaryExpression = parsePrimaryExpression();
    if (hasError(primaryExpression)) {
        return error<parse_tree::UnaryExpression>();
    }

    auto newRangeRef = primaryExpression->getReference();

    return std::make_unique<parse_tree::UnaryExpression>(std::move(primaryExpression),
                                                         newRangeRef);
}

std::unique_ptr<parse_tree::PrimaryExpression> Parser::parsePrimaryExpression()
{
    if (!lexer.hasNext()) {
        sourceCodeManager.printContext(refToLastChar,
                                       "error: expected primary-expression afterwards");
        return error<parse_tree::PrimaryExpression>();
    }

    // We peek the next token type and check what kind of primary expression
    // we have.
    auto nextTokenType = lexer.peek().getTokenType();

    // Do we have an identifier as primary-expression?
    if (nextTokenType == lexer::Token::Type::Identifier) {
        auto identifier = parseIdentifier();
        if (hasError(identifier)) {
            return error<parse_tree::PrimaryExpression>();
        }

        auto newRangeRef = identifier->getReference();

        return std::make_unique<parse_tree::PrimaryExpression>(std::move(identifier),
                                                               newRangeRef);
    }

    // Do we have a literal as primary-expression?
    if (nextTokenType == lexer::Token::Type::Literal) {
        auto literal = parseLiteral();
        if (hasError(literal)) {
            return error<parse_tree::PrimaryExpression>();
        }

        auto newRangeRef = literal->getReference();

        return std::make_unique<parse_tree::PrimaryExpression>(std::move(literal),
                                                               newRangeRef);
    }

    // The primary expression must be "( additive-expression )"!

    if (nextTokenType != lexer::Token::Type::LeftParenthesis) {
        // If the next token type is not '(', then this token cannot be a valid
        // primary-expression although expected!
        sourceCodeManager.printContext(lexer.peek().getReference(),
                                       "error: expected primary-expression");
        return error<parse_tree::PrimaryExpression>();
    }

    auto leftParenthesis = parseGenericToken(lexer::Token::Type::LeftParenthesis);
    if (hasError(leftParenthesis)) {
        return error<parse_tree::PrimaryExpression>();
    }

    auto additiveExpression = parseAdditiveExpression();
    if (hasError(additiveExpression)) {
        return error<parse_tree::PrimaryExpression>();
    }

    auto rightParenthesis = parseGenericToken(lexer::Token::Type::RightParenthesis);
    if (hasError(rightParenthesis)) {
        sourceCodeManager.printContext(leftParenthesis->getReference(),
                                       "note: to match this '('");
        return error<parse_tree::PrimaryExpression>();
    }

    auto newRangeRef = leftParenthesis->getReference()
                           .extendUntil(rightParenthesis->getReference().last());

    return std::make_unique<parse_tree::PrimaryExpression>(std::move(leftParenthesis),
                                                           std::move(additiveExpression),
                                                           std::move(rightParenthesis),
                                                           newRangeRef);
}

} // namespace pljit::parser
