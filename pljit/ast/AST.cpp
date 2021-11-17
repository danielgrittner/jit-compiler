#include "AST.h"
#include "pljit/analysis/SymbolTable.h"
#include "pljit/exec/ExecutionContext.h"
#include <cassert>
#include <iostream>

namespace pljit::ast {

ASTNode::Type ASTNode::getType() const
// Returns the type of the ASTNode.
{
    return type;
}

ASTNode::ASTNode(Type type)
    : type(type)
// Constructor
{}

ExecutableNode::ExecutableNode(ASTNode::Type type)
    : ASTNode(type)
// Constructor
{}

Function::Function(std::vector<std::unique_ptr<Statement>> statements)
    : ExecutableNode(ASTNode::Type::Function),
      statements(std::move(statements))
// Constructor
{}

const std::vector<std::unique_ptr<Statement>>& Function::getStatements() const
// Returns a const-reference to the statements.
{
    return statements;
}

std::vector<std::unique_ptr<Statement>>& Function::getStatements()
// Returns a reference to the statements.
{
    return statements;
}

void Function::accept(ASTConstVisitor& visitor) const
// Accept function for applying the visitor pattern on the AST.
{
    visitor.visit(*this);
}

void Function::accept(ASTVisitor& visitor)
// Accept function for applying the visitor pattern on the AST.
{
    visitor.visit(*this);
}

void Function::execute(exec::ExecutionContext& context) const
// Execution function.
{
    for (const auto& stmt : statements) {
        stmt->execute(context);
        // Did an error occur?
        if (context.hasError()) {
            // Yes! Hence, stop the execution!
            break;
        }

        // Did we reach a return statement?
        if (stmt->getType() == ASTNode::Type::ReturnStatement) {
            // Yes! Therefore, stop!
            break;
        }
    }
}

Statement::Statement(Type type, std::unique_ptr<Expression> expression)
    : ExecutableNode(type),
      expression(std::move(expression))
// Constructor
{}

const Expression& Statement::getExpression() const
// Returns a reference to the expression of the statement.
{
    assert(expression != nullptr);
    return *expression;
}

Expression& Statement::getExpression()
// Returns a reference to the expression of the statement.
{
    assert(expression != nullptr);
    return *expression;
}

void Statement::setExpression(std::unique_ptr<Expression> newExpression)
// Set a new expression.
{
    expression = std::move(newExpression);
}

AssignmentStatement::AssignmentStatement(std::unique_ptr<Identifier> target,
                                         std::unique_ptr<Expression> expression)
    : Statement(ASTNode::Type::AssignmentStatement, std::move(expression)),
      assignmentTarget(std::move(target))
// Constructor
{
    assert(assignmentTarget->getIdentifierType() == Identifier::Type::Variable ||
           assignmentTarget->getIdentifierType() == Identifier::Type::Parameter);
}

const Identifier& AssignmentStatement::getAssignmentTarget() const
// Returns a const-reference to the assignment target.
{
    assert(assignmentTarget != nullptr);
    return *assignmentTarget;
}

void AssignmentStatement::accept(ASTConstVisitor& visitor) const
// Accept function for applying the visitor pattern on the AST.
{
    visitor.visit(*this);
}

void AssignmentStatement::accept(ASTVisitor& visitor)
// Accept function for applying the visitor pattern on the AST.
{
    visitor.visit(*this);
}

void AssignmentStatement::execute(exec::ExecutionContext& context) const
// Execution function.
{
    int64_t result = expression->evaluate(context);
    if (!context.hasError()) {
        // If no error occurred, we update the value of the assignment
        // target.
        if (assignmentTarget->getIdentifierType() == Identifier::Type::Variable) {
            context.variableValues[assignmentTarget->getId()] = result;
        } else {
            assert(assignmentTarget->getIdentifierType() == Identifier::Type::Parameter);
            context.parameterValues[assignmentTarget->getId()] = result;
        }
    }
}

ReturnStatement::ReturnStatement(std::unique_ptr<Expression> expression)
    : Statement(ASTNode::Type::ReturnStatement, std::move(expression))
// Constructor
{}

void ReturnStatement::accept(ASTConstVisitor& visitor) const
// Accept function for applying the visitor pattern on the AST.
{
    visitor.visit(*this);
}

void ReturnStatement::accept(ASTVisitor& visitor)
// Accept function for applying the visitor pattern on the AST.
{
    visitor.visit(*this);
}

void ReturnStatement::execute(exec::ExecutionContext& context) const
// Execution function.
{
    int64_t result = expression->evaluate(context);
    if (!context.hasError()) {
        // If no error occurred, we update the return value.
        context.returnValue = result;
    }
}

Expression::Expression(Type type)
    : ASTNode(type)
// Constructor
{}

ConstantLiteral::ConstantLiteral(int64_t value)
    : Expression(ASTNode::Type::ConstantLiteral),
      value(value)
// Constructor
{}

int64_t ConstantLiteral::getValue() const
// Returns the value of the constant.
{
    return value;
}

void ConstantLiteral::accept(ASTConstVisitor& visitor) const
// Accept function for applying the visitor pattern on the AST.
{
    visitor.visit(*this);
}

void ConstantLiteral::accept(ASTVisitor& visitor)
// Accept function for applying the visitor pattern on the AST.
{
    visitor.visit(*this);
}

int64_t ConstantLiteral::evaluate(exec::ExecutionContext& /*context*/) const
// Function which evaluates an expression.
{
    return value;
}

Identifier::Identifier(Type identifierType, size_t id)
    : Expression(ASTNode::Type::Identifier),
      identifierType(identifierType),
      id(id)
// Constructor
{}

Identifier::Type Identifier::getIdentifierType() const
// Returns the identifier type.
{
    return identifierType;
}

size_t Identifier::getId() const
// Returns the variable id.
{
    return id;
}

void Identifier::accept(ASTConstVisitor& visitor) const
// Accept function for applying the visitor pattern on the AST.
{
    visitor.visit(*this);
}

void Identifier::accept(ASTVisitor& visitor)
// Accept function for applying the visitor pattern on the AST.
{
    visitor.visit(*this);
}

int64_t Identifier::evaluate(exec::ExecutionContext& context) const
// Function which evaluates an expression.
{
    switch (identifierType) {
        case Type::Parameter:
            return context.parameterValues[id];

        case Type::Variable:
            return context.variableValues[id];

        case Type::Constant:
            return context.symbolTable.getConstantValue(id);

        default:
            // This case should never be reached since all cases are handled,
            // but clang-tidy still complains...
            __builtin_unreachable();
    }
}

UnaryOp::UnaryOp(Type unaryOpType, std::unique_ptr<Expression> expression)
    : Expression(ASTNode::Type::UnaryOp),
      unaryOpType(unaryOpType),
      expression(std::move(expression))
// Constructor
{}

UnaryOp::Type UnaryOp::getUnaryOpType() const
// Returns the type of the UnaryOp.
{
    return unaryOpType;
}

const Expression& UnaryOp::getExpression() const
// Returns a reference to the expression.
{
    assert(expression != nullptr);
    return *expression;
}

Expression& UnaryOp::getExpression()
// Returns a reference to the expression.
{
    assert(expression != nullptr);
    return *expression;
}

void UnaryOp::accept(ASTConstVisitor& visitor) const
// Accept function for applying the visitor pattern on the AST.
{
    visitor.visit(*this);
}

void UnaryOp::accept(ASTVisitor& visitor)
// Accept function for applying the visitor pattern in order to optimize an AST.
{
    visitor.visit(*this);
}

int64_t UnaryOp::evaluate(exec::ExecutionContext& context) const
// Function which evaluates an expression.
{
    auto result = expression->evaluate(context);
    if (context.hasError()) {
        // An error occurred, hence, we stop.
        return 0;
    }

    if (unaryOpType == Type::MinusSign) {
        return -result;
    }

    assert(unaryOpType == Type::PlusSign);
    return result;
}

BinaryOp::BinaryOp(std::unique_ptr<Expression> lhsExpression,
                   Type binaryOpType,
                   std::unique_ptr<Expression> rhsExpression)
    : Expression(ASTNode::Type::BinaryOp),
      binaryOpType(binaryOpType),
      lhsExpression(std::move(lhsExpression)),
      rhsExpression(std::move(rhsExpression))
// Constructor
{}

BinaryOp::Type BinaryOp::getBinaryOpType() const
// Returns the type of BinaryOp.
{
    return binaryOpType;
}

const Expression& BinaryOp::getLhsExpression() const
// Returns a reference to the lhs expression.
{
    assert(lhsExpression != nullptr);
    return *lhsExpression;
}

Expression& BinaryOp::getLhsExpression()
// Returns a reference to the lhs expression.
{
    assert(lhsExpression != nullptr);
    return *lhsExpression;
}

void BinaryOp::setLhsExpression(std::unique_ptr<Expression> newLhsExpression)
// Set a new lhs expression.
{
    lhsExpression = std::move(newLhsExpression);
}

const Expression& BinaryOp::getRhsExpression() const
// Returns a reference to the rhs expression.
{
    assert(rhsExpression != nullptr);
    return *rhsExpression;
}

Expression& BinaryOp::getRhsExpression()
// Returns a reference to the rhs expression.
{
    assert(rhsExpression != nullptr);
    return *rhsExpression;
}

void BinaryOp::setRhsExpression(std::unique_ptr<Expression> newRhsExpression)
// Set a new rhs expression.
{
    rhsExpression = std::move(newRhsExpression);
}

void BinaryOp::accept(ASTConstVisitor& visitor) const
// Accept function for applying the visitor pattern on the AST.
{
    visitor.visit(*this);
}

void BinaryOp::accept(ASTVisitor& visitor)
// Accept function for applying the visitor pattern on the AST.
{
    visitor.visit(*this);
}

int64_t BinaryOp::evaluate(exec::ExecutionContext& context) const
// Function which evaluates an expression.
{
    int64_t lhs = lhsExpression->evaluate(context);
    if (context.hasError()) {
        // An error occurred, hence, we stop.
        return 0;
    }

    int64_t rhs = rhsExpression->evaluate(context);
    if (context.hasError()) {
        // An error occurred, hence, we stop.
        return 0;
    }

    switch (binaryOpType) {
        case Type::Add:
            return lhs + rhs;

        case Type::Sub:
            return lhs - rhs;

        case Type::Mul:
            return lhs * rhs;

        case Type::Div:
            if (rhs == 0) {
                // Error! Division by zero!
                context.error = exec::ExecutionContext::ErrorType::DivisionByZero;
                std::cout << "error: division by zero" << std::endl;
                return 0;
            }
            return lhs / rhs;

        default:
            // This case should never be reached since all cases are handled,
            // but clang-tidy still complains...
            __builtin_unreachable();
    }
}

} // namespace pljit::ast
