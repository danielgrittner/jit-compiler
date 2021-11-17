#include "ParseTree.h"
#include "pljit/parse_tree/ParseTreeVisitor.h"
#include <cassert>

namespace pljit::parse_tree {

namespace {

constexpr std::string_view PLUS = "+";
constexpr std::string_view MINUS = "-";
constexpr std::string_view MUL = "*";
constexpr std::string_view DIV = "/";

} // namespace

ParseTreeNode::Type ParseTreeNode::getType() const
// Returns the type of the node.
{
    return type;
}

common::SourceRangeReference ParseTreeNode::getReference() const
// Returns a reference to the range in the source code which the node represents.
{
    return ref;
}

ParseTreeNode::ParseTreeNode(Type type, common::SourceRangeReference ref)
    : type(type),
      ref(ref)
// Constructor
{}

FlexibleChildrenBase::FlexibleChildrenBase(ParseTreeNode::Type type,
                                           ChildrenType children,
                                           common::SourceRangeReference ref)
    : ParseTreeNode(type, ref),
      children(std::move(children))
// Constructor
{}

FlexibleChildrenBase::FlexibleChildrenBase(ParseTreeNode::Type type,
                                           common::SourceRangeReference ref)
    : ParseTreeNode(type, ref)
// Constructor
{}

Identifier::Identifier(common::SourceRangeReference ref)
    : ParseTreeNode(ParseTreeNode::Type::Identifier, ref)
// Constructor
{}

void Identifier::accept(ParseTreeVisitor& visitor) const
// Accept function for applying the visitor pattern on the parse tree.
{
    visitor.visit(*this);
}

Literal::Literal(int64_t value, common::SourceRangeReference ref)
    : ParseTreeNode(ParseTreeNode::Type::Literal, ref),
      value(value)
// Constructor
{}

int64_t Literal::getValue() const
// Returns the value of the literal.
{
    return value;
}

void Literal::accept(ParseTreeVisitor& visitor) const
// Accept function for applying the visitor pattern on the parse tree.
{
    visitor.visit(*this);
}

GenericToken::GenericToken(common::SourceRangeReference ref)
    : ParseTreeNode(ParseTreeNode::Type::GenericToken, ref)
// Constructor
{}

void GenericToken::accept(ParseTreeVisitor& visitor) const
// Accept function for applying the visitor pattern on the parse tree.
{
    visitor.visit(*this);
}

FunctionDefinition::FunctionDefinition(std::unique_ptr<ParameterDeclarations> paramDeclarations,
                                       std::unique_ptr<VariableDeclarations> varDeclarations,
                                       std::unique_ptr<ConstantDeclarations> constDeclarations,
                                       std::unique_ptr<CompoundStatement> compoundStatement,
                                       std::unique_ptr<GenericToken> programTerminator,
                                       common::SourceRangeReference ref)
    : ParseTreeNode(ParseTreeNode::Type::FunctionDefinition, ref),
      parameterDeclarations(std::move(paramDeclarations)),
      variableDeclarations(std::move(varDeclarations)),
      constantDeclarations(std::move(constDeclarations)),
      compoundStatement(std::move(compoundStatement)),
      programTerminator(std::move(programTerminator))
// Constructor
{}

const ParameterDeclarations* FunctionDefinition::getParameterDeclarations() const
// Returns a pointer to the optional parameter-declarations.
{
    return parameterDeclarations.get();
}

const VariableDeclarations* FunctionDefinition::getVariableDeclarations() const
// Returns a pointer to the optional variable-declarations.
{
    return variableDeclarations.get();
}

const ConstantDeclarations* FunctionDefinition::getConstantDeclarations() const
// Returns a pointer to the optional constant-declarations.
{
    return constantDeclarations.get();
}

const CompoundStatement& FunctionDefinition::getCompoundStatement() const
// Returns a const-reference to the compound statement.
{
    assert(compoundStatement != nullptr);
    return *compoundStatement;
}

const GenericToken& FunctionDefinition::getProgramTerminator() const
// Returns a const-reference to the generic token.
{
    assert(programTerminator != nullptr);
    return *programTerminator;
}

void FunctionDefinition::accept(ParseTreeVisitor& visitor) const
// Accept function for applying the visitor pattern on the parse tree.
{
    visitor.visit(*this);
}

DeclaratorListWrapper::DeclaratorListWrapper(ParseTreeNode::Type type,
                                             std::unique_ptr<DeclaratorList> declaratorList,
                                             std::unique_ptr<GenericToken> semiColon,
                                             common::SourceRangeReference ref)
    : ParseTreeNode(type, ref),
      declaratorList(std::move(declaratorList)),
      semiColon(std::move(semiColon))
// Constructor
{}

const DeclaratorList& DeclaratorListWrapper::getDeclaratorList() const
// Returns a const-ref to the declarator-list.
{
    return *declaratorList;
}

const GenericToken& DeclaratorListWrapper::getSemiColon() const
// Returns a const-ref to the semi-colon.
{
    return *semiColon;
}

ParameterDeclarations::ParameterDeclarations(std::unique_ptr<GenericToken> paramKeyword,
                                             std::unique_ptr<DeclaratorList> declaratorList,
                                             std::unique_ptr<GenericToken> semiColon,
                                             common::SourceRangeReference ref)
    : DeclaratorListWrapper(ParseTreeNode::Type::ParameterDeclarations,
                            std::move(declaratorList),
                            std::move(semiColon),
                            ref),
      paramKeyword(std::move(paramKeyword))
// Constructor
{}

void ParameterDeclarations::accept(ParseTreeVisitor& visitor) const
// Accept function for applying the visitor pattern on the parse tree.
{
    visitor.visit(*this);
}

const GenericToken& ParameterDeclarations::getParamKeyword() const
// Returns a const-ref to the PARAM keyword.
{
    return *paramKeyword;
}

VariableDeclarations::VariableDeclarations(std::unique_ptr<GenericToken> varKeyword,
                                           std::unique_ptr<DeclaratorList> declaratorList,
                                           std::unique_ptr<GenericToken> semiColon,
                                           common::SourceRangeReference ref)
    : DeclaratorListWrapper(ParseTreeNode::Type::VariableDeclarations,
                            std::move(declaratorList),
                            std::move(semiColon),
                            ref),
      varKeyword(std::move(varKeyword))
// Constructor
{}

const GenericToken& VariableDeclarations::getVarKeyword() const
// Returns a const-ref to the VAR keyword.
{
    return *varKeyword;
}

void VariableDeclarations::accept(ParseTreeVisitor& visitor) const
// Accept function for applying the visitor pattern on the parse tree.
{
    visitor.visit(*this);
}

ConstantDeclarations::ConstantDeclarations(std::unique_ptr<GenericToken> constKeyword,
                                           std::unique_ptr<InitDeclaratorList> initDeclaratorList,
                                           std::unique_ptr<GenericToken> semiColon,
                                           common::SourceRangeReference ref)
    : ParseTreeNode(ParseTreeNode::Type::ConstantDeclarations, ref),
      constKeyword(std::move(constKeyword)),
      initDeclaratorList(std::move(initDeclaratorList)),
      semiColon(std::move(semiColon))
// Constructor
{}

const GenericToken& ConstantDeclarations::getConstKeyword() const
// Returns a const-ref to the CONST keyword.
{
    return *constKeyword;
}

const InitDeclaratorList& ConstantDeclarations::getInitDeclaratorList() const
// Returns a const-ref to the init-declarator-list.
{
    return *initDeclaratorList;
}

const GenericToken& ConstantDeclarations::getSemiColon() const
// Returns a const-ref to the semi-colon.
{
    return *semiColon;
}

void ConstantDeclarations::accept(ParseTreeVisitor& visitor) const
// Accept function for applying the visitor pattern on the parse tree.
{
    visitor.visit(*this);
}

DeclaratorList::DeclaratorList(ChildrenType children, common::SourceRangeReference ref)
    : FlexibleChildrenBase(ParseTreeNode::Type::DeclaratorList,
                           std::move(children),
                           ref)
// Constructor
{}

const FlexibleChildrenBase::ChildrenType& DeclaratorList::getCommaSeparatedIdentifiers() const
// Returns a const-ref to the comma-separated identifiers.
{
    return children;
}

void DeclaratorList::accept(ParseTreeVisitor& visitor) const
// Accept function for applying the visitor pattern on the parse tree.
{
    visitor.visit(*this);
}

InitDeclaratorList::InitDeclaratorList(std::vector<std::unique_ptr<ParseTreeNode>> children,
                                       common::SourceRangeReference ref)
    : FlexibleChildrenBase(ParseTreeNode::Type::InitDeclaratorList,
                           std::move(children),
                           ref)
// Constructor
{}

const FlexibleChildrenBase::ChildrenType& InitDeclaratorList::getCommaSeparatedInitDeclarators() const
// Returns a const-ref to the comma-separated init-declarators.
{
    return children;
}

void InitDeclaratorList::accept(ParseTreeVisitor& visitor) const
// Accept function for applying the visitor pattern on the parse tree.
{
    visitor.visit(*this);
}

InitDeclarator::InitDeclarator(std::unique_ptr<Identifier> identifier,
                               std::unique_ptr<GenericToken> initToken,
                               std::unique_ptr<Literal> literal,
                               common::SourceRangeReference ref)
    : ParseTreeNode(ParseTreeNode::Type::InitDeclarator, ref),
      identifier(std::move(identifier)),
      initToken(std::move(initToken)),
      literal(std::move(literal))
// Constructor
{}

const Identifier& InitDeclarator::getInitTarget() const
// Returns a const-ref to the init-target.
{
    return *identifier;
}

const GenericToken& InitDeclarator::getInitToken() const
// Returns a const-ref to the init token.
{
    return *initToken;
}

const Literal& InitDeclarator::getLiteral() const
// Returns a const-ref to the literal.
{
    return *literal;
}

void InitDeclarator::accept(ParseTreeVisitor& visitor) const
// Accept function for applying the visitor pattern on the parse tree.
{
    visitor.visit(*this);
}

CompoundStatement::CompoundStatement(std::unique_ptr<GenericToken> beginKeyword,
                                     std::unique_ptr<StatementList> statementList,
                                     std::unique_ptr<GenericToken> endKeyword,
                                     common::SourceRangeReference ref)
    : ParseTreeNode(ParseTreeNode::Type::CompoundStatement, ref),
      beginKeyword(std::move(beginKeyword)),
      statementList(std::move(statementList)),
      endKeyword(std::move(endKeyword))
// Constructor
{}

const GenericToken& CompoundStatement::getBeginKeyword() const
// Returns a const-ref to the BEGIN keyword.
{
    return *beginKeyword;
}

const StatementList& CompoundStatement::getStatementList() const
// Returns a const-ref to the statement-list.
{
    return *statementList;
}

const GenericToken& CompoundStatement::getEndKeyword() const
// Returns a const-ref to the END keyword.
{
    return *endKeyword;
}

void CompoundStatement::accept(ParseTreeVisitor& visitor) const
// Accept function for applying the visitor pattern on the parse tree.
{
    visitor.visit(*this);
}

StatementList::StatementList(ChildrenType children, common::SourceRangeReference ref)
    : FlexibleChildrenBase(ParseTreeNode::Type::StatementList,
                           std::move(children),
                           ref)
// Constructor
{}

const FlexibleChildrenBase::ChildrenType& StatementList::getStatementsSeparatedBySemiColon() const
// Returns a const-ref to the statements separated by semi-colons.
{
    return children;
}

void StatementList::accept(ParseTreeVisitor& visitor) const
// Accept function for applying the visitor pattern on the parse tree.
{
    visitor.visit(*this);
}

Statement::Statement(std::unique_ptr<AssignmentExpression> assignmentExpression,
                     common::SourceRangeReference ref)
    : FlexibleChildrenBase(ParseTreeNode::Type::Statement, ref),
      statementType(Type::AssignmentStatement)
// Constructor for an assignment statement
{
    children.push_back(std::move(assignmentExpression));
}

Statement::Statement(std::unique_ptr<GenericToken> returnKeyword,
                     std::unique_ptr<AdditiveExpression> additiveExpression,
                     common::SourceRangeReference ref)
    : FlexibleChildrenBase(ParseTreeNode::Type::Statement, ref),
      statementType(Type::ReturnStatement)
// Constructor for a return statement
{
    children.push_back(std::move(returnKeyword));
    children.push_back(std::move(additiveExpression));
}

const AssignmentExpression& Statement::getAssignmentExpression() const
// Returns a const-ref to the assignment-expression.
{
    assert(statementType == Type::AssignmentStatement);
    assert(children.size() == 1);
    assert(children[0]->getType() == ParseTreeNode::Type::AssignmentExpression);
    return static_cast<const AssignmentExpression&>(*children[0]); // NOLINT
}

const GenericToken& Statement::getReturnKeyword() const
// Returns a const-ref to the RETURN keyword.
{
    assert(statementType == Type::ReturnStatement);
    assert(children.size() == 2);
    assert(children[0]->getType() == ParseTreeNode::Type::GenericToken);
    return static_cast<const GenericToken&>(*children[0]); // NOLINT
}

const AdditiveExpression& Statement::getAdditiveExpression() const
// Returns a const-ref to the additive-expression.
{
    assert(statementType == Type::ReturnStatement);
    assert(children.size() == 2);
    assert(children[1]->getType() == ParseTreeNode::Type::AdditiveExpression);
    return static_cast<const AdditiveExpression&>(*children[1]); // NOLINT
}

Statement::Type Statement::getStatementType() const
// Returns the statement type.
{
    return statementType;
}

void Statement::accept(ParseTreeVisitor& visitor) const
// Accept function for applying the visitor pattern on the parse tree.
{
    visitor.visit(*this);
}

AssignmentExpression::AssignmentExpression(std::unique_ptr<Identifier> identifier,
                                           std::unique_ptr<GenericToken> assignmentToken,
                                           std::unique_ptr<AdditiveExpression> additiveExpression,
                                           common::SourceRangeReference ref)
    : ParseTreeNode(ParseTreeNode::Type::AssignmentExpression, ref),
      identifier(std::move(identifier)),
      assignmentToken(std::move(assignmentToken)),
      additiveExpression(std::move(additiveExpression))
// Constructor
{}

const Identifier& AssignmentExpression::getAssignmentTarget() const
// Returns a const-ref to the assignment target.
{
    return *identifier;
}

const GenericToken& AssignmentExpression::getAssignmentToken() const
// Returns a const-ref to the assignment token.
{
    return *assignmentToken;
}

const AdditiveExpression& AssignmentExpression::getAdditiveExpression() const
// Returns a const-ref to the additive-expression.
{
    return *additiveExpression;
}

void AssignmentExpression::accept(ParseTreeVisitor& visitor) const
// Accept function for applying the visitor pattern on the parse tree.
{
    visitor.visit(*this);
}

AdditiveExpression::AdditiveExpression(std::unique_ptr<MultiplicativeExpression> multiplicativeExpression,
                                       common::SourceRangeReference ref)
    : FlexibleChildrenBase(ParseTreeNode::Type::AdditiveExpression, ref),
      additiveExpressionType(Type::None)
// Constructor for a multiplicative expression.
{
    children.push_back(std::move(multiplicativeExpression));
}

AdditiveExpression::AdditiveExpression(std::unique_ptr<MultiplicativeExpression> multiplicativeExpression,
                                       std::unique_ptr<GenericToken> op,
                                       std::unique_ptr<AdditiveExpression> additiveExpression,
                                       common::SourceRangeReference ref)
    : FlexibleChildrenBase(ParseTreeNode::Type::AdditiveExpression, ref),
      additiveExpressionType(op->getReference() == PLUS ? Type::Add : Type::Sub)
// Constructor for an additive expression.
{
    assert((additiveExpressionType == Type::Add && op->getReference() == PLUS) ||
           (additiveExpressionType == Type::Sub && op->getReference() == MINUS));
    children.push_back(std::move(multiplicativeExpression));
    children.push_back(std::move(op));
    children.push_back(std::move(additiveExpression));
}

AdditiveExpression::Type AdditiveExpression::getAdditiveExpressionType() const
// Returns the type of the additive expression.
{
    return additiveExpressionType;
}

const MultiplicativeExpression& AdditiveExpression::getMultiplicativeExpression() const
// Returns a const-ref to the multiplicative-expression.
{
    assert(children.size() == 1 || children.size() == 3);
    assert(children[0]->getType() == ParseTreeNode::Type::MultiplicativeExpression);
    return static_cast<const MultiplicativeExpression&>(*children[0]); // NOLINT
}

const GenericToken& AdditiveExpression::getAdditiveOpToken() const
// Returns a const-ref to the additive operator token (i.e. + or -).
{
    assert(children.size() == 3);
    assert(children[1]->getType() == ParseTreeNode::Type::GenericToken);
    assert(additiveExpressionType == Type::Add || additiveExpressionType == Type::Sub);
    return static_cast<const GenericToken&>(*children[1]); // NOLINT
}

const AdditiveExpression& AdditiveExpression::getAdditiveExpression() const
// Returns a const-ref to the additive-expression.
{
    assert(children.size() == 3);
    assert(children[2]->getType() == ParseTreeNode::Type::AdditiveExpression);
    assert(additiveExpressionType == Type::Add || additiveExpressionType == Type::Sub);
    return static_cast<const AdditiveExpression&>(*children[2]); // NOLINT
}

void AdditiveExpression::accept(ParseTreeVisitor& visitor) const
// Accept function for applying the visitor pattern on the parse tree.
{
    visitor.visit(*this);
}

MultiplicativeExpression::MultiplicativeExpression(std::unique_ptr<UnaryExpression> unaryExpression,
                                                   common::SourceRangeReference ref)
    : FlexibleChildrenBase(ParseTreeNode::Type::MultiplicativeExpression, ref),
      multiplicativeExpressionType(Type::None)
// Constructor for an unary expression.
{
    children.push_back(std::move(unaryExpression));
}

MultiplicativeExpression::MultiplicativeExpression(std::unique_ptr<UnaryExpression> unaryExpression,
                                                   std::unique_ptr<GenericToken> op,
                                                   std::unique_ptr<MultiplicativeExpression> multiplicativeExpression,
                                                   common::SourceRangeReference ref)
    : FlexibleChildrenBase(ParseTreeNode::Type::MultiplicativeExpression, ref),
      multiplicativeExpressionType(op->getReference() == MUL ? Type::Mul : Type::Div)
// Constructor for a multiplicative expression.
{
    assert((multiplicativeExpressionType == Type::Mul && op->getReference() == MUL) ||
           (multiplicativeExpressionType == Type::Div && op->getReference() == DIV));
    children.push_back(std::move(unaryExpression));
    children.push_back(std::move(op));
    children.push_back(std::move(multiplicativeExpression));
}

MultiplicativeExpression::Type MultiplicativeExpression::getMultiplicativeExpressionType() const
// Returns the type of the multiplicative expression.
{
    return multiplicativeExpressionType;
}

const UnaryExpression& MultiplicativeExpression::getUnaryExpression() const
// Returns a const-ref to the unary-expression.
{
    assert(children.size() == 1 || children.size() == 3);
    assert(children[0]->getType() == ParseTreeNode::Type::UnaryExpression);
    return static_cast<const UnaryExpression&>(*children[0]); // NOLINT
}

const GenericToken& MultiplicativeExpression::getMultiplicativeOpToken() const
// Returns a const-ref to the multiplicative operator (i.e. * or /).
{
    assert(children.size() == 3);
    assert(children[1]->getType() == ParseTreeNode::Type::GenericToken);
    assert(multiplicativeExpressionType == Type::Mul || multiplicativeExpressionType == Type::Div);
    return static_cast<const GenericToken&>(*children[1]); // NOLINT
}

const MultiplicativeExpression& MultiplicativeExpression::getMultiplicativeExpression() const
// Returns a const-ref to the multiplicative-expression.
{
    assert(children.size() == 3);
    assert(children[2]->getType() == ParseTreeNode::Type::MultiplicativeExpression);
    assert(multiplicativeExpressionType == Type::Mul || multiplicativeExpressionType == Type::Div);
    return static_cast<const MultiplicativeExpression&>(*children[2]); // NOLINT
}

void MultiplicativeExpression::accept(ParseTreeVisitor& visitor) const
// Accept function for applying the visitor pattern on the parse tree.
{
    visitor.visit(*this);
}

UnaryExpression::UnaryExpression(std::unique_ptr<PrimaryExpression> primaryExpression,
                                 common::SourceRangeReference ref)
    : FlexibleChildrenBase(ParseTreeNode::Type::UnaryExpression, ref),
      unaryExpressionType(Type::Unsigned)
// Constructor for an unsigned unary expression.
{
    children.push_back(std::move(primaryExpression));
}

UnaryExpression::UnaryExpression(std::unique_ptr<GenericToken> sign,
                                 std::unique_ptr<PrimaryExpression> primaryExpression,
                                 common::SourceRangeReference ref)
    : FlexibleChildrenBase(ParseTreeNode::Type::UnaryExpression, ref),
      unaryExpressionType(sign->getReference() == PLUS ? Type::PlusSign : Type::MinusSign)
// Constructor for a signed unary expression.
{
    assert((unaryExpressionType == Type::PlusSign && sign->getReference() == PLUS) ||
           (unaryExpressionType == Type::MinusSign && sign->getReference() == MINUS));
    children.push_back(std::move(sign));
    children.push_back(std::move(primaryExpression));
}

UnaryExpression::Type UnaryExpression::getUnaryExpressionType() const
// Returns the type of the unary expression.
{
    return unaryExpressionType;
}

const GenericToken& UnaryExpression::getSignToken() const
// Returns a const-ref to the sign token.
{
    assert(children.size() == 2);
    assert(children[0]->getType() == ParseTreeNode::Type::GenericToken);
    assert(unaryExpressionType == Type::PlusSign || unaryExpressionType == Type::MinusSign);
    return static_cast<const GenericToken&>(*children[0]); // NOLINT
}

const PrimaryExpression& UnaryExpression::getPrimaryExpression() const
// Returns a const-ref to the primary-expression.
{
    assert(children.size() == 1 || children.size() == 2);
    assert(children.back()->getType() == ParseTreeNode::Type::PrimaryExpression);
    return static_cast<const PrimaryExpression&>(*children.back()); // NOLINT
}

void UnaryExpression::accept(ParseTreeVisitor& visitor) const
// Accept function for applying the visitor pattern on the parse tree.
{
    visitor.visit(*this);
}

PrimaryExpression::PrimaryExpression(std::unique_ptr<Identifier> identifier,
                                     common::SourceRangeReference ref)
    : FlexibleChildrenBase(ParseTreeNode::Type::PrimaryExpression, ref),
      primaryExpressionType(Type::Identifier)
// Constructor for an identifier.
{
    children.push_back(std::move(identifier));
}

PrimaryExpression::PrimaryExpression(std::unique_ptr<Literal> literal,
                                     common::SourceRangeReference ref)
    : FlexibleChildrenBase(ParseTreeNode::Type::PrimaryExpression, ref),
      primaryExpressionType(Type::Literal)
// Constructor for a literal.
{
    children.push_back(std::move(literal));
}

PrimaryExpression::PrimaryExpression(std::unique_ptr<GenericToken> leftParenthesis,
                                     std::unique_ptr<AdditiveExpression> additiveExpression,
                                     std::unique_ptr<GenericToken> rightParenthesis,
                                     common::SourceRangeReference ref)
    : FlexibleChildrenBase(ParseTreeNode::Type::PrimaryExpression, ref),
      primaryExpressionType(Type::Parenthesized)
// Constructor for a parenthesized primary expression.
{
    children.push_back(std::move(leftParenthesis));
    children.push_back(std::move(additiveExpression));
    children.push_back(std::move(rightParenthesis));
}

PrimaryExpression::Type PrimaryExpression::getPrimaryExpressionType() const
// Returns the type of the primary expression.
{
    return primaryExpressionType;
}

const Identifier& PrimaryExpression::getIdentifier() const
// Returns a const-ref to the Identifier.
{
    assert(primaryExpressionType == Type::Identifier);
    assert(children.size() == 1);
    assert(children[0]->getType() == ParseTreeNode::Type::Identifier);
    return static_cast<const Identifier&>(*children[0]); // NOLINT
}

const Literal& PrimaryExpression::getLiteral() const
// Returns a const-ref to the Literal.
{
    assert(primaryExpressionType == Type::Literal);
    assert(children.size() == 1);
    assert(children[0]->getType() == ParseTreeNode::Type::Literal);
    return static_cast<const Literal&>(*children[0]); // NOLINT
}

const GenericToken& PrimaryExpression::getLeftParenthesis() const
// Returns a const-ref to the left parenthesis.
{
    assert(primaryExpressionType == Type::Parenthesized);
    assert(children.size() == 3);
    assert(children[0]->getType() == ParseTreeNode::Type::GenericToken);
    return static_cast<const GenericToken&>(*children[0]); // NOLINT
}

const AdditiveExpression& PrimaryExpression::getAdditiveExpression() const
// returns a const-ref to the additive-expression.
{
    assert(primaryExpressionType == Type::Parenthesized);
    assert(children.size() == 3);
    assert(children[1]->getType() == ParseTreeNode::Type::AdditiveExpression);
    return static_cast<const AdditiveExpression&>(*children[1]); // NOLINT
}

const GenericToken& PrimaryExpression::getRightParenthesis() const
// Returns a const-ref to the right parenthesis.
{
    assert(primaryExpressionType == Type::Parenthesized);
    assert(children.size() == 3);
    assert(children[2]->getType() == ParseTreeNode::Type::GenericToken);
    return static_cast<const GenericToken&>(*children[2]); // NOLINT
}

void PrimaryExpression::accept(ParseTreeVisitor& visitor) const
// Accept function for applying the visitor pattern on the parse tree.
{
    visitor.visit(*this);
}

} // namespace pljit::parse_tree
