#ifndef H_parse_tree_ParseTree
#define H_parse_tree_ParseTree

#include "pljit/common/References.h"
#include "pljit/parse_tree/ParseTreeFwd.h"
#include "pljit/parse_tree/ParseTreeVisitorFwd.h"
#include <memory>
#include <vector>

namespace pljit::parse_tree {

class ParseTreeNode {
    public:
    // The types of the parse tree node.
    enum class Type {
        // Terminal symbols
        Identifier,
        Literal,
        GenericToken,

        // Non-Terminal symbols
        FunctionDefinition,
        ParameterDeclarations,
        VariableDeclarations,
        ConstantDeclarations,
        DeclaratorList,
        InitDeclaratorList,
        InitDeclarator,
        CompoundStatement,
        StatementList,
        Statement,
        AssignmentExpression,
        AdditiveExpression,
        MultiplicativeExpression,
        UnaryExpression,
        PrimaryExpression,
    };

    /// Destructor
    virtual ~ParseTreeNode() = default;

    /// Returns the type of the node.
    Type getType() const;

    /// Returns a reference to the range in the source code which the node represents.
    common::SourceRangeReference getReference() const;

    /// Accept function for applying the visitor pattern on the parse tree.
    virtual void accept(ParseTreeVisitor& visitor) const = 0;

    protected:
    /// Constructor
    ParseTreeNode(Type type, common::SourceRangeReference ref);

    private:
    Type type;
    common::SourceRangeReference ref;
};

class FlexibleChildrenBase : public ParseTreeNode {
    public:
    using ChildrenType = std::vector<std::unique_ptr<ParseTreeNode>>;

    /// Destructor
    ~FlexibleChildrenBase() override = default;

    protected:
    /// Constructor
    FlexibleChildrenBase(ParseTreeNode::Type type,
                         ChildrenType children,
                         common::SourceRangeReference ref);

    /// Constructor
    FlexibleChildrenBase(ParseTreeNode::Type type, common::SourceRangeReference ref);

    ChildrenType children{};
};

class Identifier : public ParseTreeNode {
    public:
    /// Constructor
    explicit Identifier(common::SourceRangeReference ref);

    /// Destructor
    ~Identifier() override = default;

    /// Accept function for applying the visitor pattern on the parse tree.
    void accept(ParseTreeVisitor& visitor) const final;
};

class Literal : public ParseTreeNode {
    public:
    /// Constructor
    Literal(int64_t value, common::SourceRangeReference ref);

    /// Destructor
    ~Literal() override = default;

    /// Returns the value of the literal.
    int64_t getValue() const;

    /// Accept function for applying the visitor pattern on the parse tree.
    void accept(ParseTreeVisitor& visitor) const final;

    private:
    int64_t value;
};

class GenericToken : public ParseTreeNode {
    public:
    /// Constructor
    explicit GenericToken(common::SourceRangeReference ref);

    /// Destructor
    ~GenericToken() override = default;

    /// Accept function for applying the visitor pattern on the parse tree.
    void accept(ParseTreeVisitor& visitor) const final;
};

/// Production rule:
/// function-definition = [ parameter-declarations ]
///                       [ variable-declarations ]
///                       [ constant-declarations ]
///                       compound-statement
///                       "."
class FunctionDefinition : public ParseTreeNode {
    public:
    /// Constructor
    FunctionDefinition(std::unique_ptr<ParameterDeclarations> paramDeclarations,
                       std::unique_ptr<VariableDeclarations> varDeclarations,
                       std::unique_ptr<ConstantDeclarations> constDeclarations,
                       std::unique_ptr<CompoundStatement> compoundStatement,
                       std::unique_ptr<GenericToken> programTerminator,
                       common::SourceRangeReference ref);

    /// Destructor
    ~FunctionDefinition() override = default;

    /// Returns a pointer to the optional parameter-declarations.
    /// Note: Might be nullptr.
    const ParameterDeclarations* getParameterDeclarations() const;

    /// Returns a pointer to the optional variable-declarations.
    /// Note: Might be nullptr.
    const VariableDeclarations* getVariableDeclarations() const;

    /// Returns a pointer to the optional constant-declarations.
    /// Note: Might be nullptr.
    const ConstantDeclarations* getConstantDeclarations() const;

    /// Returns a const-reference to the compound statement.
    const CompoundStatement& getCompoundStatement() const;

    /// Returns a const-reference to the program terminator.
    const GenericToken& getProgramTerminator() const;

    /// Accept function for applying the visitor pattern on the parse tree.
    void accept(ParseTreeVisitor& visitor) const final;

    private:
    std::unique_ptr<ParameterDeclarations> parameterDeclarations{};
    std::unique_ptr<VariableDeclarations> variableDeclarations{};
    std::unique_ptr<ConstantDeclarations> constantDeclarations{};
    std::unique_ptr<CompoundStatement> compoundStatement{};
    std::unique_ptr<GenericToken> programTerminator{};
};

/// Base class for productions of the form: <...> declarator-list ";"
class DeclaratorListWrapper : public ParseTreeNode {
    public:
    /// Returns a const-ref to the declarator
    const DeclaratorList& getDeclaratorList() const;

    /// Returns a const-ref to the semi-colon.
    const GenericToken& getSemiColon() const;

    protected:
    /// Constructor
    DeclaratorListWrapper(ParseTreeNode::Type type,
                          std::unique_ptr<DeclaratorList> declaratorList,
                          std::unique_ptr<GenericToken> semiColon,
                          common::SourceRangeReference ref);

    /// Destructor
    ~DeclaratorListWrapper() override = default;

    std::unique_ptr<DeclaratorList> declaratorList;
    std::unique_ptr<GenericToken> semiColon;
};

/// Production rule:
/// parameter-declarations = "PARAM" declarator-list ";"
class ParameterDeclarations : public DeclaratorListWrapper {
    public:
    /// Constructor
    ParameterDeclarations(std::unique_ptr<GenericToken> paramKeyword,
                          std::unique_ptr<DeclaratorList> declaratorList,
                          std::unique_ptr<GenericToken> semiColon,
                          common::SourceRangeReference ref);

    /// Destructor
    ~ParameterDeclarations() override = default;

    /// Returns a const-ref to the PARAM keyword.
    const GenericToken& getParamKeyword() const;

    /// Accept function for applying the visitor pattern on the parse tree.
    void accept(ParseTreeVisitor& visitor) const final;

    private:
    std::unique_ptr<GenericToken> paramKeyword;
};

/// Production rule:
/// variable-declarations = "VAR" declarator-list ";"
class VariableDeclarations : public DeclaratorListWrapper {
    public:
    /// Constructor
    VariableDeclarations(std::unique_ptr<GenericToken> varKeyword,
                         std::unique_ptr<DeclaratorList> declaratorList,
                         std::unique_ptr<GenericToken> semiColon,
                         common::SourceRangeReference ref);

    /// Destructor
    ~VariableDeclarations() override = default;

    /// Returns a const-ref to the VAR keyword.
    const GenericToken& getVarKeyword() const;

    /// Accept function for applying the visitor pattern on the parse tree.
    void accept(ParseTreeVisitor& visitor) const final;

    private:
    std::unique_ptr<GenericToken> varKeyword;
};

/// Production rule:
/// constant-declarations = "CONST" init-declarator-list ";"
class ConstantDeclarations : public ParseTreeNode {
    public:
    /// Constructor
    ConstantDeclarations(std::unique_ptr<GenericToken> constKeyword,
                         std::unique_ptr<InitDeclaratorList> initDeclaratorList,
                         std::unique_ptr<GenericToken> semiColon,
                         common::SourceRangeReference ref);

    /// Destructor
    ~ConstantDeclarations() override = default;

    /// Returns a const-ref to the CONST keyword.
    const GenericToken& getConstKeyword() const;

    /// Returns a const-ref to the init-declarator-list.
    const InitDeclaratorList& getInitDeclaratorList() const;

    /// Returns a const-ref to the semi-colon.
    const GenericToken& getSemiColon() const;

    /// Accept function for applying the visitor pattern on the parse tree.
    void accept(ParseTreeVisitor& visitor) const final;

    private:
    std::unique_ptr<GenericToken> constKeyword;
    std::unique_ptr<InitDeclaratorList> initDeclaratorList;
    std::unique_ptr<GenericToken> semiColon;
};

/// Production rule:
/// declarator-list = identifier { "," identifier }
class DeclaratorList : public FlexibleChildrenBase {
    public:
    /// Constructor
    DeclaratorList(std::vector<std::unique_ptr<ParseTreeNode>> children,
                   common::SourceRangeReference ref);

    /// Destructor
    ~DeclaratorList() override = default;

    /// Returns a const-ref to the comma-separated identifiers.
    const ChildrenType& getCommaSeparatedIdentifiers() const;

    /// Accept function for applying the visitor pattern on the parse tree.
    void accept(ParseTreeVisitor& visitor) const final;
};

/// Production rule:
/// init-declarator-list = init-declarator { "," init-declarator }
class InitDeclaratorList : public FlexibleChildrenBase {
    public:
    /// Constructor
    InitDeclaratorList(std::vector<std::unique_ptr<ParseTreeNode>> children,
                       common::SourceRangeReference ref);

    /// Destructor
    ~InitDeclaratorList() override = default;

    /// Returns a const-ref to the comma-separated init-declarators.
    const ChildrenType& getCommaSeparatedInitDeclarators() const;

    /// Accept function for applying the visitor pattern on the parse tree.
    void accept(ParseTreeVisitor& visitor) const final;
};

/// Production rule:
/// init-declarator = identifier "=" literal
class InitDeclarator : public ParseTreeNode {
    public:
    /// Constructor
    InitDeclarator(std::unique_ptr<Identifier> identifier,
                   std::unique_ptr<GenericToken> initToken,
                   std::unique_ptr<Literal> literal,
                   common::SourceRangeReference ref);

    /// Destructor
    ~InitDeclarator() override = default;

    /// Returns a const-ref to the init-target.
    const Identifier& getInitTarget() const;

    /// Returns a const-ref to the init token.
    const GenericToken& getInitToken() const;

    /// Returns a const-ref to the literal.
    const Literal& getLiteral() const;

    /// Accept function for applying the visitor pattern on the parse tree.
    void accept(ParseTreeVisitor& visitor) const final;

    private:
    std::unique_ptr<Identifier> identifier;
    std::unique_ptr<GenericToken> initToken;
    std::unique_ptr<Literal> literal;
};

/// Production rule:
/// compound-statement = "BEGIN" statement-list "END"
class CompoundStatement : public ParseTreeNode {
    public:
    /// Constructor
    CompoundStatement(std::unique_ptr<GenericToken> beginKeyword,
                      std::unique_ptr<StatementList> statementList,
                      std::unique_ptr<GenericToken> endKeyword,
                      common::SourceRangeReference ref);

    /// Destructor
    ~CompoundStatement() override = default;

    /// Returns a const-ref to the BEGIN keyword.
    const GenericToken& getBeginKeyword() const;

    /// Returns a const-ref to the statement-list.
    const StatementList& getStatementList() const;

    /// Returns a const-ref to the END keyword.
    const GenericToken& getEndKeyword() const;

    /// Accept function for applying the visitor pattern on the parse tree.
    void accept(ParseTreeVisitor& visitor) const final;

    private:
    std::unique_ptr<GenericToken> beginKeyword;
    std::unique_ptr<StatementList> statementList;
    std::unique_ptr<GenericToken> endKeyword;
};

/// Production rule:
/// statement-list = statement { ";" statement }
class StatementList : public FlexibleChildrenBase {
    public:
    /// Constructor
    StatementList(std::vector<std::unique_ptr<ParseTreeNode>> children,
                  common::SourceRangeReference ref);

    /// Destructor
    ~StatementList() override = default;

    /// Returns a const-ref to the statements separated by semi-colons.
    const ChildrenType& getStatementsSeparatedBySemiColon() const;

    /// Accept function for applying the visitor pattern on the parse tree.
    void accept(ParseTreeVisitor& visitor) const final;
};

/// Production rule:
/// statement = assignment-expression | "RETURN" additive-expression
class Statement : public FlexibleChildrenBase {
    public:
    /// Different types of statements.
    enum class Type {
        AssignmentStatement,
        ReturnStatement
    };

    /// Constructor for an assignment statement
    Statement(std::unique_ptr<AssignmentExpression> assignmentExpression,
              common::SourceRangeReference ref);

    /// Constructor for a return statement
    Statement(std::unique_ptr<GenericToken> returnKeyword,
              std::unique_ptr<AdditiveExpression> additiveExpression,
              common::SourceRangeReference ref);

    /// Destructor
    ~Statement() override = default;

    /// Returns the statement type.
    Type getStatementType() const;

    /// Returns a const-ref to the assignment-expression.
    /// Note: only valid if statement type is ReturnStatement.
    const AssignmentExpression& getAssignmentExpression() const;

    /// Returns a const-ref to the RETURN keyword.
    /// Note: only valid if statement type is AssignmentStatement.
    const GenericToken& getReturnKeyword() const;

    /// Returns a const-ref to the additive-expression.
    /// Note: only valid if statement type is AssignmentStatement.
    const AdditiveExpression& getAdditiveExpression() const;

    /// Accept function for applying the visitor pattern on the parse tree.
    void accept(ParseTreeVisitor& visitor) const final;

    private:
    Type statementType;
};

/// Production rule:
/// assignment-expression = identifier ":=" additive-expression
class AssignmentExpression : public ParseTreeNode {
    public:
    /// Constructor
    AssignmentExpression(std::unique_ptr<Identifier> identifier,
                         std::unique_ptr<GenericToken> assignmentToken,
                         std::unique_ptr<AdditiveExpression> additiveExpression,
                         common::SourceRangeReference ref);

    /// Destructor
    ~AssignmentExpression() override = default;

    /// Returns a const-ref to the assignment target.
    const Identifier& getAssignmentTarget() const;

    /// Returns a const-ref to the assignment token.
    const GenericToken& getAssignmentToken() const;

    /// Returns a const-ref to the additive-expression.
    const AdditiveExpression& getAdditiveExpression() const;

    /// Accept function for applying the visitor pattern on the parse tree.
    void accept(ParseTreeVisitor& visitor) const final;

    private:
    std::unique_ptr<Identifier> identifier;
    std::unique_ptr<GenericToken> assignmentToken;
    std::unique_ptr<AdditiveExpression> additiveExpression;
};

/// Production rule:
/// additive-expression = multiplicative-expression [ ("+" | "-") additive-expression]
class AdditiveExpression : public FlexibleChildrenBase {
    public:
    /// Defines the different types of additive expressions
    enum class Type {
        Add,
        Sub,
        None
    };

    /// Constructor for a multiplicative expression.
    AdditiveExpression(std::unique_ptr<MultiplicativeExpression> multiplicativeExpression,
                       common::SourceRangeReference ref);

    /// Constructor for an additive expression.
    AdditiveExpression(std::unique_ptr<MultiplicativeExpression> multiplicativeExpression,
                       std::unique_ptr<GenericToken> op,
                       std::unique_ptr<AdditiveExpression> additiveExpression,
                       common::SourceRangeReference ref);

    /// Destructor
    ~AdditiveExpression() override = default;

    /// Returns the type of the additive expression.
    Type getAdditiveExpressionType() const;

    /// Returns a const-ref to the multiplicative-expression.
    const MultiplicativeExpression& getMultiplicativeExpression() const;

    /// Returns a const-ref to the additive operator token (i.e. + or -).
    /// Note: only valid if additive expression type is Add or Sub.
    const GenericToken& getAdditiveOpToken() const;

    /// Returns a const-ref to the additive-expression.
    /// Note: only valid if additive expression type is Add or Sub.
    const AdditiveExpression& getAdditiveExpression() const;

    /// Accept function for applying the visitor pattern on the parse tree.
    void accept(ParseTreeVisitor& visitor) const final;

    private:
    Type additiveExpressionType;
};

/// Production rule:
/// multiplicative-expression = unary-expression [ ("*" | "/") multiplicative-expression]
class MultiplicativeExpression : public FlexibleChildrenBase {
    public:
    /// Defines the different types of multiplicative expressions
    enum class Type {
        Mul,
        Div,
        None
    };

    /// Constructor for an unary expression.
    MultiplicativeExpression(std::unique_ptr<UnaryExpression> unaryExpression,
                             common::SourceRangeReference ref);

    /// Constructor for a multiplicative expression.
    MultiplicativeExpression(std::unique_ptr<UnaryExpression> unaryExpression,
                             std::unique_ptr<GenericToken> op,
                             std::unique_ptr<MultiplicativeExpression> multiplicativeExpression,
                             common::SourceRangeReference ref);

    /// Destructor
    ~MultiplicativeExpression() override = default;

    /// Returns the type of the multiplicative expression.
    Type getMultiplicativeExpressionType() const;

    /// Returns a const-ref to the unary-expression.
    const UnaryExpression& getUnaryExpression() const;

    /// Returns a const-ref to the multiplicative operator (i.e. * or /).
    /// Note: only valid if additive expression type is Mul or Div.
    const GenericToken& getMultiplicativeOpToken() const;

    /// Returns a const-ref to the multiplicative-expression.
    /// Note: only valid if additive expression type is Mul or Div.
    const MultiplicativeExpression& getMultiplicativeExpression() const;

    /// Accept function for applying the visitor pattern on the parse tree.
    void accept(ParseTreeVisitor& visitor) const final;

    private:
    Type multiplicativeExpressionType;
};

/// Production rule:
/// unary-expression = ["+" | "-"] primary-expression
class UnaryExpression : public FlexibleChildrenBase {
    public:
    /// Defines the different types of unary expressions
    enum class Type {
        PlusSign,
        MinusSign,
        Unsigned
    };

    /// Constructor for an unsigned unary expression.
    UnaryExpression(std::unique_ptr<PrimaryExpression> primaryExpression,
                    common::SourceRangeReference ref);

    /// Constructor for a signed unary expression.
    UnaryExpression(std::unique_ptr<GenericToken> sign,
                    std::unique_ptr<PrimaryExpression> primaryExpression,
                    common::SourceRangeReference ref);

    /// Destructor
    ~UnaryExpression() override = default;

    /// Returns the type of the unary expression.
    Type getUnaryExpressionType() const;

    /// Returns a const-ref to the sign token.
    /// Note: only valid if unary expression type is PlusSign or MinusSign.
    const GenericToken& getSignToken() const;

    /// Returns a const-ref to the primary-expression.
    const PrimaryExpression& getPrimaryExpression() const;

    /// Accept function for applying the visitor pattern on the parse tree.
    void accept(ParseTreeVisitor& visitor) const final;

    private:
    Type unaryExpressionType;
};

/// Production rule:
/// primary-expression = identifier
///                    | literal
///                    | "(" additive-expression ")"
class PrimaryExpression : public FlexibleChildrenBase {
    public:
    /// Defines the different types of primary expressions
    enum class Type {
        Identifier,
        Literal,
        Parenthesized
    };

    /// Constructor for an identifier
    PrimaryExpression(std::unique_ptr<Identifier> identifier,
                      common::SourceRangeReference ref);

    /// Constructor for a literal
    PrimaryExpression(std::unique_ptr<Literal> literal,
                      common::SourceRangeReference ref);

    /// Constructor for a parenthesized primary expression
    PrimaryExpression(std::unique_ptr<GenericToken> leftParenthesis,
                      std::unique_ptr<AdditiveExpression> additiveExpression,
                      std::unique_ptr<GenericToken> rightParenthesis,
                      common::SourceRangeReference ref);

    /// Destructor
    ~PrimaryExpression() override = default;

    /// Returns the type of the primary expression.
    Type getPrimaryExpressionType() const;

    /// Returns a const-ref to the identifier.
    /// Note: only valid if primary expression type is Identifier.
    const Identifier& getIdentifier() const;

    /// Returns a const-ref to the Literal.
    /// Note: only valid if primary expression type is Literal.
    const Literal& getLiteral() const;

    /// Returns a const-ref to the left parenthesis.
    /// Note: only valid if primary expression type is Parenthesized.
    const GenericToken& getLeftParenthesis() const;

    /// Returns a const-ref to the additive-expression.
    /// NOte: only valid if primary expression is Parenthesized.
    const AdditiveExpression& getAdditiveExpression() const;

    /// Returns a const-ref to the right parenthesis.
    /// Note: only valid if primary expression type is Parenthesized.
    const GenericToken& getRightParenthesis() const;

    /// Accept function for applying the visitor pattern on the parse tree.
    void accept(ParseTreeVisitor& visitor) const final;

    private:
    Type primaryExpressionType;
};

} // namespace pljit::parse_tree

#endif