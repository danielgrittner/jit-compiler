#ifndef H_ast_ASTDotVisitor
#define H_ast_ASTDotVisitor

#include "pljit/analysis/SymbolTableFwd.h"
#include "pljit/ast/ASTVisitor.h"
#include <ostream>
#include <vector>

namespace pljit::ast {

class ASTDotVisitor : public ASTConstVisitor {
    public:
    /// Constructor
    ASTDotVisitor(std::ostream& out, const analysis::SymbolTable& symbolTable);

    /// Destructor
    ~ASTDotVisitor() override = default;

    /// Visit methods for the AST.
    /// Note: It is also possible to print subtrees of an AST.

    void visit(const Function& node) final;

    void visit(const AssignmentStatement& node) final;

    void visit(const ReturnStatement& node) final;

    void visit(const ConstantLiteral& node) final;

    void visit(const Identifier& node) final;

    void visit(const UnaryOp& node) final;

    void visit(const BinaryOp& node) final;

    private:
    /// When visiting a new node, we take the edge from the current node
    /// with label id currentLabelId to an unvisited node whose label
    /// id will be labels.size(). This function stores the edges.
    void addEdgeToNextNode(unsigned currentLabelId);

    /// Handles the next visit, i.e. adding a new edge and visiting the next node.
    void handleNextVisit(unsigned currentLabelId, const ASTNode& node);

    /// This function prints the collected information about the tree
    /// in the DOT format if currentDepth == 0. This implies that
    /// the starting node is reached
    void printDotGraphIfStartingNodeIsReachedAgain();

    std::ostream& out;

    const analysis::SymbolTable& symbolTable;

    /// Indicates the current depth of the parse tree. This is needed
    /// in order to know when we reached the starting node again. This
    /// needs to be known for printing the DOT format.
    unsigned currentDepth{};

    /// Vector of labels
    std::vector<std::string> labels;

    struct Edge {
        unsigned from{};
        unsigned to{};
    };

    /// Vector of edges
    std::vector<Edge> edges;
};

} // namespace pljit::ast

#endif
