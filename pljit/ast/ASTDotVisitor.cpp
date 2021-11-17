#include "ASTDotVisitor.h"
#include "pljit/analysis/SymbolTable.h"
#include "pljit/ast/AST.h"
#include <cassert>

namespace pljit::ast {

ASTDotVisitor::ASTDotVisitor(std::ostream& out, const analysis::SymbolTable& symbolTable)
    : out(out),
      symbolTable(symbolTable)
// Constructor
{}

void ASTDotVisitor::visit(const Function& node)
{
    unsigned myLabelId = labels.size();
    labels.emplace_back("Function");

    ++currentDepth;

    for (const auto& stmt : node.getStatements()) {
        handleNextVisit(myLabelId, *stmt);
    }

    --currentDepth;

    printDotGraphIfStartingNodeIsReachedAgain();
}

void ASTDotVisitor::visit(const AssignmentStatement& node)
{
    unsigned myLabelId = labels.size();
    labels.emplace_back(":=");

    ++currentDepth;

    handleNextVisit(myLabelId, node.getAssignmentTarget());
    handleNextVisit(myLabelId, node.getExpression());

    --currentDepth;

    printDotGraphIfStartingNodeIsReachedAgain();
}

void ASTDotVisitor::visit(const ReturnStatement& node)
{
    unsigned myLabelId = labels.size();
    labels.emplace_back("RETURN");

    ++currentDepth;

    handleNextVisit(myLabelId, node.getExpression());

    --currentDepth;

    printDotGraphIfStartingNodeIsReachedAgain();
}

void ASTDotVisitor::visit(const ConstantLiteral& node)
{
    labels.emplace_back(std::to_string(node.getValue()));
    printDotGraphIfStartingNodeIsReachedAgain();
}

void ASTDotVisitor::visit(const Identifier& node)
{
    auto symbolNameOpt = symbolTable.lookUpSymbolName(node.getIdentifierType(),
                                                      node.getId());
    assert(symbolNameOpt);
    auto symbolName = symbolNameOpt.value();
    if (node.getIdentifierType() == Identifier::Type::Constant) {
        int64_t constValue = symbolTable.getConstantValue(node.getId());
        labels.emplace_back(std::string{symbolName} +
                            ": " + std::to_string(constValue));
    } else {
        labels.emplace_back(symbolName);
    }
    printDotGraphIfStartingNodeIsReachedAgain();
}

void ASTDotVisitor::visit(const UnaryOp& node)
{
    unsigned myLabelId = labels.size();
    switch (node.getUnaryOpType()) {
        case UnaryOp::Type::PlusSign:
            labels.emplace_back("+");
            break;

        case UnaryOp::Type::MinusSign:
            labels.emplace_back("-");
            break;
    }

    ++currentDepth;

    handleNextVisit(myLabelId, node.getExpression());

    --currentDepth;

    printDotGraphIfStartingNodeIsReachedAgain();
}

void ASTDotVisitor::visit(const BinaryOp& node)
{
    unsigned myLabelId = labels.size();
    switch (node.getBinaryOpType()) {
        case BinaryOp::Type::Add:
            labels.emplace_back("+");
            break;

        case BinaryOp::Type::Sub:
            labels.emplace_back("-");
            break;

        case BinaryOp::Type::Mul:
            labels.emplace_back("*");
            break;

        case BinaryOp::Type::Div:
            labels.emplace_back("/");
            break;
    }

    ++currentDepth;

    handleNextVisit(myLabelId, node.getLhsExpression());
    handleNextVisit(myLabelId, node.getRhsExpression());

    --currentDepth;

    printDotGraphIfStartingNodeIsReachedAgain();
}

void ASTDotVisitor::addEdgeToNextNode(unsigned int currentLabelId)
// Add a new edge from the current node to the next unvisited node.
{
    unsigned nextLabelId = labels.size();
    edges.push_back({currentLabelId, nextLabelId});
}

void ASTDotVisitor::handleNextVisit(unsigned int currentLabelId,
                                    const ASTNode& node)
// Handles the next visit, i.e. adding a new edge and visiting the next node.
{
    addEdgeToNextNode(currentLabelId);
    node.accept(*this);
}

void ASTDotVisitor::printDotGraphIfStartingNodeIsReachedAgain()
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

} // namespace pljit::ast
