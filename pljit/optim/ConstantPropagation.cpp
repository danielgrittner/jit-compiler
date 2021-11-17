#include "ConstantPropagation.h"
#include "pljit/analysis/SymbolTable.h"
#include <cassert>

namespace pljit::optim {

namespace {

std::pair<size_t, ast::Identifier::Type> constructKey(const ast::Identifier& node)
{
    assert(node.getIdentifierType() != ast::Identifier::Type::Constant);
    return {node.getId(), node.getIdentifierType()};
}

} // namespace

ConstantPropagation::ConstantPropagation(const analysis::SymbolTable& symbolTable)
    : symbolTable(symbolTable)
// Constructor
{}

void ConstantPropagation::visit(ast::Function& node)
{
    // Iterate over the statements in order to visit them.
    auto& statements = node.getStatements();
    for (auto& statement : statements) {
        statement->accept(*this);
    }
}

void ConstantPropagation::visit(ast::AssignmentStatement& node)
{
    // Determine the assignment target key.
    auto assignmentTargetKey = constructKey(node.getAssignmentTarget());

    // Visit the expression of the assignment.
    node.getExpression().accept(*this);

    // Can the expression be transformed into a constant?
    if (constantResultFromLastCall) {
        // Yes, it can!
        auto result = constantResultFromLastCall.value();

        // Since we now can't further propagate the constant result up,
        // we materialize the constant here.
        node.setExpression(std::make_unique<ast::ConstantLiteral>(result));

        // Remember that the variable with the key assignmentTargetKey is a
        // constant as well as its value!
        auto it = variableTable.emplace(assignmentTargetKey, VariableAssignment{});
        it.first->second.hasConstantAssignment = true;
        it.first->second.value = result;

        // Reset the optional because we cannot propagate the constant
        // value further up.
        constantResultFromLastCall = std::nullopt;
    } else {
        // The assignment did not yield to an assignment of a constant.
        // Hence, we need to remember that from this moment on, the variable
        // with the key assignmentTargetKey is currently not a constant!
        auto it = variableTable.emplace(assignmentTargetKey, VariableAssignment{});
        it.first->second.hasConstantAssignment = false;
    }
}

void ConstantPropagation::visit(ast::ReturnStatement& node)
{
    // Visit the expression of the return expression.
    node.getExpression().accept(*this);

    // Can the expression be transformed into a constant?
    if (constantResultFromLastCall) {
        // Yes, it can!
        auto result = constantResultFromLastCall.value();

        // Since we now can't further propagate the constant result up,
        // we materialize the constant here.
        node.setExpression(std::make_unique<ast::ConstantLiteral>(result));

        // Reset the optional because we cannot propagate the constant
        // value further up.
        constantResultFromLastCall = std::nullopt;
    }
}

void ConstantPropagation::visit(ast::ConstantLiteral& node)
{
    // Propagate the constant value up, such that the parent knows that
    // it gets a constant value.
    constantResultFromLastCall = node.getValue();
}

void ConstantPropagation::visit(ast::Identifier& node)
{
    if (node.getIdentifierType() == ast::Identifier::Type::Constant) {
        // We have a Const variable!
        constantResultFromLastCall = symbolTable.getConstantValue(node.getId());
        return;
    }

    auto it = variableTable.find(constructKey(node));
    // Is the variable/parameter currently a constant?
    if (it != variableTable.end() && it->second.hasConstantAssignment) {
        // The variable/parameter is currently a constant!
        constantResultFromLastCall = it->second.value;
    }
}

void ConstantPropagation::visit(ast::UnaryOp& node)
{
    // Visit the child expression.
    node.getExpression().accept(*this);

    // In case the child expression evaluated to a constant and the unary expression
    // has a minus sign, then we need to update the constant value.
    if (constantResultFromLastCall && node.getUnaryOpType() == ast::UnaryOp::Type::MinusSign) {
        // If we have a negative sign, the new constant is:
        constantResultFromLastCall = -constantResultFromLastCall.value();
    }
    // otherwise we can just propagate the result of the child expression up.
}

void ConstantPropagation::visit(ast::BinaryOp& node)
{
    // Visit the left child expression.
    node.getLhsExpression().accept(*this);
    auto leftConstantResultOpt = constantResultFromLastCall;
    constantResultFromLastCall = std::nullopt;

    // Visit the right child expression.
    node.getRhsExpression().accept(*this);
    auto rightConstantResultOpt = constantResultFromLastCall;
    constantResultFromLastCall = std::nullopt;

    if (leftConstantResultOpt && rightConstantResultOpt) {
        // We can transform the whole binary operation into a constant!

        // Precompute the binary operation and propagate its result up!
        // The parent will transform the whole binary operation into a constant!

        auto lhsValue = leftConstantResultOpt.value();
        auto rhsValue = rightConstantResultOpt.value();

        switch (node.getBinaryOpType()) {
            case ast::BinaryOp::Type::Add:
                constantResultFromLastCall = lhsValue + rhsValue;
                break;

            case ast::BinaryOp::Type::Sub:
                constantResultFromLastCall = lhsValue - rhsValue;
                break;

            case ast::BinaryOp::Type::Mul:
                constantResultFromLastCall = lhsValue * rhsValue;
                break;

            case ast::BinaryOp::Type::Div:
                if (rhsValue == 0) {
                    // If we have a division by 0 error, we leave it as is
                    // during optimization.
                    constantResultFromLastCall = std::nullopt;
                } else {
                    constantResultFromLastCall = lhsValue / rhsValue;
                }
                break;
        }
        return;
    }

    // The whole binary operation cannot be evaluated to a constant.
    // Therefore, we now must check whether either the lhs or rhs
    // did yield a constant. If this is the case, we must materialize
    // the constant result here.

    if (leftConstantResultOpt) {
        auto result = leftConstantResultOpt.value();
        // Update the expression of the unary op node!
        node.setLhsExpression(std::make_unique<ast::ConstantLiteral>(result));
        return;
    }

    if (rightConstantResultOpt) {
        auto result = rightConstantResultOpt.value();
        // Update the expression of the unary op node!
        node.setRhsExpression(std::make_unique<ast::ConstantLiteral>(result));
    }
}

} // namespace pljit::optim
