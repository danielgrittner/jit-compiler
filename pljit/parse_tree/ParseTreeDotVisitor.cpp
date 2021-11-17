#include "ParseTreeDotVisitor.h"
#include "pljit/parse_tree/ParseTree.h"

namespace pljit::parse_tree {

ParseTreeDotVisitor::ParseTreeDotVisitor(std::ostream& out)
    : out(out)
// Constructor
{}

void ParseTreeDotVisitor::visit(const Identifier& node)
{
    labels.push_back(node.getReference());
    printDotGraphIfStartingNodeIsReachedAgain();
}

void ParseTreeDotVisitor::visit(const Literal& node)
{
    labels.push_back(node.getReference());
    printDotGraphIfStartingNodeIsReachedAgain();
}

void ParseTreeDotVisitor::visit(const GenericToken& node)
{
    labels.push_back(node.getReference());
    printDotGraphIfStartingNodeIsReachedAgain();
}

void ParseTreeDotVisitor::visit(const FunctionDefinition& node)
{
    unsigned myLabelId = labels.size();
    labels.emplace_back("function-definition");

    ++currentDepth;

    const auto* paramDeclarations = node.getParameterDeclarations();
    if (paramDeclarations != nullptr) {
        handleNextVisit(myLabelId, *paramDeclarations);
    }

    const auto* varDeclarations = node.getVariableDeclarations();
    if (varDeclarations != nullptr) {
        handleNextVisit(myLabelId, *varDeclarations);
    }

    const auto* constDeclarations = node.getConstantDeclarations();
    if (constDeclarations != nullptr) {
        handleNextVisit(myLabelId, *constDeclarations);
    }

    handleNextVisit(myLabelId, node.getCompoundStatement());
    handleNextVisit(myLabelId, node.getProgramTerminator());

    --currentDepth;

    printDotGraphIfStartingNodeIsReachedAgain();
}

void ParseTreeDotVisitor::visit(const ParameterDeclarations& node)
{
    unsigned myLabelId = labels.size();
    labels.emplace_back("parameter-declarations");

    ++currentDepth;

    handleNextVisit(myLabelId, node.getParamKeyword());
    visitDeclaratorListWrapper(myLabelId, node);

    --currentDepth;

    printDotGraphIfStartingNodeIsReachedAgain();
}

void ParseTreeDotVisitor::visit(const VariableDeclarations& node)
{
    unsigned myLabelId = labels.size();
    labels.emplace_back("variable-declarations");

    ++currentDepth;

    handleNextVisit(myLabelId, node.getVarKeyword());
    visitDeclaratorListWrapper(myLabelId, node);

    --currentDepth;

    printDotGraphIfStartingNodeIsReachedAgain();
}

void ParseTreeDotVisitor::visit(const ConstantDeclarations& node)
{
    unsigned myLabelId = labels.size();
    labels.emplace_back("constant-declarations");

    ++currentDepth;

    handleNextVisit(myLabelId, node.getConstKeyword());
    handleNextVisit(myLabelId, node.getInitDeclaratorList());
    handleNextVisit(myLabelId, node.getSemiColon());

    --currentDepth;

    printDotGraphIfStartingNodeIsReachedAgain();
}

void ParseTreeDotVisitor::visit(const DeclaratorList& node)
{
    visitVariableLengthChildren(node.getCommaSeparatedIdentifiers(), "declarator-list");
}

void ParseTreeDotVisitor::visit(const InitDeclaratorList& node)
{
    visitVariableLengthChildren(node.getCommaSeparatedInitDeclarators(), "init-declarator-list");
}

void ParseTreeDotVisitor::visit(const InitDeclarator& node)
{
    unsigned myLabelId = labels.size();
    labels.emplace_back("init-declarator");

    ++currentDepth;

    handleNextVisit(myLabelId, node.getInitTarget());
    handleNextVisit(myLabelId, node.getInitToken());
    handleNextVisit(myLabelId, node.getLiteral());

    --currentDepth;

    printDotGraphIfStartingNodeIsReachedAgain();
}

void ParseTreeDotVisitor::visit(const CompoundStatement& node)
{
    unsigned myLabelId = labels.size();
    labels.emplace_back("compound-statement");

    ++currentDepth;

    handleNextVisit(myLabelId, node.getBeginKeyword());
    handleNextVisit(myLabelId, node.getStatementList());
    handleNextVisit(myLabelId, node.getEndKeyword());

    --currentDepth;

    printDotGraphIfStartingNodeIsReachedAgain();
}

void ParseTreeDotVisitor::visit(const StatementList& node)
{
    visitVariableLengthChildren(node.getStatementsSeparatedBySemiColon(), "statement-list");
}

void ParseTreeDotVisitor::visit(const Statement& node)
{
    unsigned myLabelId = labels.size();
    labels.emplace_back("statement");

    ++currentDepth;

    if (node.getStatementType() == Statement::Type::AssignmentStatement) {
        handleNextVisit(myLabelId, node.getAssignmentExpression());
    } else {
        handleNextVisit(myLabelId, node.getReturnKeyword());
        handleNextVisit(myLabelId, node.getAdditiveExpression());
    }

    --currentDepth;

    printDotGraphIfStartingNodeIsReachedAgain();
}

void ParseTreeDotVisitor::visit(const AssignmentExpression& node)
{
    unsigned myLabelId = labels.size();
    labels.emplace_back("assignment-expression");

    ++currentDepth;

    handleNextVisit(myLabelId, node.getAssignmentTarget());
    handleNextVisit(myLabelId, node.getAssignmentToken());
    handleNextVisit(myLabelId, node.getAdditiveExpression());

    --currentDepth;

    printDotGraphIfStartingNodeIsReachedAgain();
}

void ParseTreeDotVisitor::visit(const AdditiveExpression& node)
{
    unsigned myLabelId = labels.size();
    labels.emplace_back("additive-expression");

    ++currentDepth;

    handleNextVisit(myLabelId, node.getMultiplicativeExpression());
    if (node.getAdditiveExpressionType() != AdditiveExpression::Type::None) {
        handleNextVisit(myLabelId, node.getAdditiveOpToken());
        handleNextVisit(myLabelId, node.getAdditiveExpression());
    }

    --currentDepth;

    printDotGraphIfStartingNodeIsReachedAgain();
}

void ParseTreeDotVisitor::visit(const MultiplicativeExpression& node)
{
    unsigned myLabelId = labels.size();
    labels.emplace_back("multiplicative-expression");

    ++currentDepth;

    handleNextVisit(myLabelId, node.getUnaryExpression());
    if (node.getMultiplicativeExpressionType() != MultiplicativeExpression::Type::None) {
        handleNextVisit(myLabelId, node.getMultiplicativeOpToken());
        handleNextVisit(myLabelId, node.getMultiplicativeExpression());
    }

    --currentDepth;

    printDotGraphIfStartingNodeIsReachedAgain();
}

void ParseTreeDotVisitor::visit(const UnaryExpression& node)
{
    unsigned myLabelId = labels.size();
    labels.emplace_back("unary-expression");

    ++currentDepth;

    if (node.getUnaryExpressionType() != UnaryExpression::Type::Unsigned) {
        handleNextVisit(myLabelId, node.getSignToken());
    }
    handleNextVisit(myLabelId, node.getPrimaryExpression());

    --currentDepth;

    printDotGraphIfStartingNodeIsReachedAgain();
}

void ParseTreeDotVisitor::visit(const PrimaryExpression& node)
{
    unsigned myLabelId = labels.size();
    labels.emplace_back("primary-expression");

    ++currentDepth;

    switch (node.getPrimaryExpressionType()) {
        case PrimaryExpression::Type::Identifier:
            handleNextVisit(myLabelId, node.getIdentifier());
            break;

        case PrimaryExpression::Type::Literal:
            handleNextVisit(myLabelId, node.getLiteral());
            break;

        case PrimaryExpression::Type::Parenthesized:
            handleNextVisit(myLabelId, node.getLeftParenthesis());
            handleNextVisit(myLabelId, node.getAdditiveExpression());
            handleNextVisit(myLabelId, node.getRightParenthesis());
            break;
    }

    --currentDepth;

    printDotGraphIfStartingNodeIsReachedAgain();
}

void ParseTreeDotVisitor::addEdgeToNextNode(unsigned int currentLabelId)
// Add a new edge from the current node to the next unvisited node.
{
    unsigned nextLabelId = labels.size();
    edges.push_back({currentLabelId, nextLabelId});
}

void ParseTreeDotVisitor::visitDeclaratorListWrapper(unsigned int currentLabelId,
                                                     const DeclaratorListWrapper& node)
// Handles the printing of declarator-list and semi-colon.
{
    handleNextVisit(currentLabelId, node.getDeclaratorList());
    handleNextVisit(currentLabelId, node.getSemiColon());
}

void ParseTreeDotVisitor::visitVariableLengthChildren(const std::vector<std::unique_ptr<ParseTreeNode>>& children,
                                                      std::string_view label)
// General visit function for nodes which have variable length children.
{
    unsigned myLabelId = labels.size();
    labels.emplace_back(label);

    ++currentDepth;

    for (const auto& child : children) {
        handleNextVisit(myLabelId, *child);
    }

    --currentDepth;

    printDotGraphIfStartingNodeIsReachedAgain();
}

void ParseTreeDotVisitor::handleNextVisit(unsigned int current, const ParseTreeNode& node)
// Handles the next visit, i.e. adding a new edge and visiting the next node.
{
    addEdgeToNextNode(current);
    node.accept(*this);
}

void ParseTreeDotVisitor::printDotGraphIfStartingNodeIsReachedAgain()
// Prints the DOT graph if currentDepth == 0.
{
    if (currentDepth != 0) {
        return;
    }

    out << "digraph {\n";
    // Print the labels.
    for (unsigned i = 0; i < labels.size(); ++i) {
        out << "\t" << i << " [label=\"" << labels[i] << "\"];\n";
    }
    // Print the edges.
    for (const auto [from, to] : edges) {
        out << "\t" << from << " -> " << to << ";\n";
    }
    out << "}\n";

    // We make the visitor re-usable.
    labels.clear();
    edges.clear();
}

} // namespace pljit::parse_tree
