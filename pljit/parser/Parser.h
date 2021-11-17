#ifndef H_parser_Parser
#define H_parser_Parser

#include "pljit/lexer/Lexer.h"
#include "pljit/parse_tree/ParseTreeFwd.h"
#include <memory>

namespace pljit::parser {

/// Parses the tokens returned from the lexer and transforms them into
/// a parse tree. It implements the recursive-descent algorithm internally.
class Parser {
    public:
    /// Constructor
    /// Note: The source code manager must not manage an empty string!
    explicit Parser(const common::SourceCodeManager& sourceCodeManager);

    /// Parses the source code and returns a parse tree.
    /// If an error occurs, a nullptr will be returned.
    std::unique_ptr<parse_tree::FunctionDefinition> parseFunctionDefinition();

    private:
    /// Parses an identifier.
    /// If an error occurs, a nullptr will be returned.
    std::unique_ptr<parse_tree::Identifier> parseIdentifier();

    /// Parses a literal.
    /// If an error occurs, a nullptr will be returned.
    std::unique_ptr<parse_tree::Literal> parseLiteral();

    /// Parses a generic token of an expected token type.
    /// If an error occurs, a nullptr will be returned.
    std::unique_ptr<parse_tree::GenericToken> parseGenericToken(lexer::Token::Type expectedTokenType);

    /// Functions which try to parse a non-terminal symbol using the token stream
    /// from the lexer and construct a corresponding parse tree node.
    /// If an error occurs, a nullptr will be returned.
    std::unique_ptr<parse_tree::ParameterDeclarations> parseParameterDeclarations();
    std::unique_ptr<parse_tree::VariableDeclarations> parseVariableDeclarations();
    std::unique_ptr<parse_tree::ConstantDeclarations> parseConstantDeclarations();
    std::unique_ptr<parse_tree::DeclaratorList> parseDeclaratorList();
    std::unique_ptr<parse_tree::InitDeclaratorList> parseInitDeclaratorList();
    std::unique_ptr<parse_tree::InitDeclarator> parseInitDeclarator();
    std::unique_ptr<parse_tree::CompoundStatement> parseCompoundStatement();
    std::unique_ptr<parse_tree::StatementList> parseStatementList();
    std::unique_ptr<parse_tree::Statement> parseStatement();
    std::unique_ptr<parse_tree::AssignmentExpression> parseAssignmentExpression();
    std::unique_ptr<parse_tree::AdditiveExpression> parseAdditiveExpression();
    std::unique_ptr<parse_tree::MultiplicativeExpression> parseMultiplicativeExpression();
    std::unique_ptr<parse_tree::UnaryExpression> parseUnaryExpression();
    std::unique_ptr<parse_tree::PrimaryExpression> parsePrimaryExpression();

    /// Source code manager for error handling (i.e. printing the context)
    const common::SourceCodeManager& sourceCodeManager;
    /// Lexer for obtaining the tokens.
    lexer::Lexer lexer;

    /// SourceLocationReference to the character of the last processed token.
    common::SourceLocationReference refToLastChar;
};

} // namespace pljit::parser

#endif
