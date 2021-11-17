#ifndef H_parse_tree_ParseTreeDotVisitor
#define H_parse_tree_ParseTreeDotVisitor

#include "pljit/parse_tree/ParseTreeFwd.h"
#include "pljit/parse_tree/ParseTreeVisitor.h"
#include <memory>
#include <ostream>
#include <string_view>
#include <vector>

namespace pljit::parse_tree {

/// Prints the parse tree in the DOT format.
class ParseTreeDotVisitor : public ParseTreeVisitor {
    public:
    /// Constructor
    explicit ParseTreeDotVisitor(std::ostream& out);

    /// Destructor
    ~ParseTreeDotVisitor() override = default;

    /// visit functions for the parse tree.
    /// Note: It is also possible to print subtrees of the parse trees.

    void visit(const Identifier& node) final;

    void visit(const Literal& node) final;

    void visit(const GenericToken& node) final;

    void visit(const FunctionDefinition& node) final;

    void visit(const ParameterDeclarations& node) final;

    void visit(const VariableDeclarations& node) final;

    void visit(const ConstantDeclarations& node) final;

    void visit(const DeclaratorList& node) final;

    void visit(const InitDeclaratorList& node) final;

    void visit(const InitDeclarator& node) final;

    void visit(const CompoundStatement& node) final;

    void visit(const StatementList& node) final;

    void visit(const Statement& node) final;

    void visit(const AssignmentExpression& node) final;

    void visit(const AdditiveExpression& node) final;

    void visit(const MultiplicativeExpression& node) final;

    void visit(const UnaryExpression& node) final;

    void visit(const PrimaryExpression& node) final;

    private:
    /// When visiting a new node, we take the edge from the current node
    /// with label id currentLabelId to an unvisited node whose label
    /// id will be labels.size(). This function stores the edges.
    void addEdgeToNextNode(unsigned currentLabelId);

    /// Handles the printing of declarator-list and semi-colon for classes
    /// which derive from DeclaratorListWrapper.
    void visitDeclaratorListWrapper(unsigned currentLabelId, const DeclaratorListWrapper& node);

    /// General visit function for nodes which have variable length children.
    void visitVariableLengthChildren(const std::vector<std::unique_ptr<ParseTreeNode>>& children,
                                     std::string_view label);

    /// Handles the next visit, i.e. adding a new edge and visiting the next node.
    void handleNextVisit(unsigned currentLabelId, const ParseTreeNode& node);

    /// This function prints the collected information about the tree
    /// in the DOT format if currentDepth == 0. This implies that
    /// the starting node is reached
    void printDotGraphIfStartingNodeIsReachedAgain();

    std::ostream& out;

    /// Indicates the current depth of the parse tree. This is needed
    /// in order to know when we reached the starting node again. This
    /// ensures that also subtrees of a parse tree can be printed.
    unsigned currentDepth{};

    /// Vector of labels
    std::vector<std::string_view> labels;

    struct Edge {
        unsigned from{};
        unsigned to{};
    };

    /// Vector of edges
    std::vector<Edge> edges;
};

} // namespace pljit::parse_tree

#endif
