#ifndef H_ast_ASTVisitor
#define H_ast_ASTVisitor

#include "pljit/ast/ASTFwd.h"
#include <type_traits>

namespace pljit::ast {

/// Abstract base class for implementing visitors for the AST.
template <bool isConst>
class ASTVisitorBase {
    public:
    /// Destructor
    virtual ~ASTVisitorBase() = default;

    /// Visit methods for the AST.

    using FunctionType = std::conditional_t<isConst, const Function&, Function&>;
    virtual void visit(FunctionType node) = 0;

    using AssignmentStatementType = std::conditional_t<isConst, const AssignmentStatement&, AssignmentStatement&>;
    virtual void visit(AssignmentStatementType node) = 0;

    using ReturnStatementType = std::conditional_t<isConst, const ReturnStatement&, ReturnStatement&>;
    virtual void visit(ReturnStatementType node) = 0;

    using ConstantType = std::conditional_t<isConst, const ConstantLiteral&, ConstantLiteral&>;
    virtual void visit(ConstantType node) = 0;

    using VariableType = std::conditional_t<isConst, const Identifier&, Identifier&>;
    virtual void visit(VariableType node) = 0;

    using UnaryOpType = std::conditional_t<isConst, const UnaryOp&, UnaryOp&>;
    virtual void visit(UnaryOpType node) = 0;

    using BinaryOpType = std::conditional_t<isConst, const BinaryOp&, BinaryOp&>;
    virtual void visit(BinaryOpType node) = 0;
};

/// Non-Const ASTVisitor
using ASTVisitor = ASTVisitorBase<false>;

/// Const ASTVisitor
using ASTConstVisitor = ASTVisitorBase<true>;

} // namespace pljit::ast

#endif
