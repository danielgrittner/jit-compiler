#ifndef H_parse_tree_ParseTreeVisitor
#define H_parse_tree_ParseTreeVisitor

#include "pljit/parse_tree/ParseTreeFwd.h"

namespace pljit::parse_tree {

/// Visitor base class which can be used on a parse tree.
class ParseTreeVisitor {
    public:
    virtual ~ParseTreeVisitor() = default;

    virtual void visit(const Identifier& node) = 0;

    virtual void visit(const Literal& node) = 0;

    virtual void visit(const GenericToken& node) = 0;

    virtual void visit(const FunctionDefinition& node) = 0;

    virtual void visit(const ParameterDeclarations& node) = 0;

    virtual void visit(const VariableDeclarations& node) = 0;

    virtual void visit(const ConstantDeclarations& node) = 0;

    virtual void visit(const DeclaratorList& node) = 0;

    virtual void visit(const InitDeclaratorList& node) = 0;

    virtual void visit(const InitDeclarator& node) = 0;

    virtual void visit(const CompoundStatement& node) = 0;

    virtual void visit(const StatementList& node) = 0;

    virtual void visit(const Statement& node) = 0;

    virtual void visit(const AssignmentExpression& node) = 0;

    virtual void visit(const AdditiveExpression& node) = 0;

    virtual void visit(const MultiplicativeExpression& node) = 0;

    virtual void visit(const UnaryExpression& node) = 0;

    virtual void visit(const PrimaryExpression& node) = 0;
};

} // namespace pljit::parse_tree

#endif