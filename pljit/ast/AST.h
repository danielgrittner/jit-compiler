#ifndef H_ast_AST
#define H_ast_AST

#include "pljit/ast/ASTFwd.h"
#include "pljit/ast/ASTVisitor.h"
#include "pljit/exec/ExecutionContextFwd.h"
#include <memory>
#include <vector>

namespace pljit::ast {

class ASTNode {
    public:
    /// All possible types of ASTNodes
    enum class Type {
        // Function type
        Function,

        // Statement types
        AssignmentStatement,
        ReturnStatement,

        // Expression types
        ConstantLiteral,
        Identifier,
        UnaryOp,
        BinaryOp,
    };

    /// Destructor
    virtual ~ASTNode() = default;

    /// Returns the type of the ASTNode.
    Type getType() const;

    /// Accept function for applying the visitor pattern on the AST.
    virtual void accept(ASTConstVisitor& visitor) const = 0;
    virtual void accept(ASTVisitor& visitor) = 0;

    protected:
    /// Constructor
    explicit ASTNode(Type type);

    private:
    Type type;
};

class ExecutableNode : public ASTNode {
    public:
    /// Destructor
    ~ExecutableNode() override = default;

    /// Execution function.
    virtual void execute(exec::ExecutionContext& context) const = 0;

    protected:
    /// Constructor
    explicit ExecutableNode(ASTNode::Type type);
};

class Function : public ExecutableNode {
    public:
    /// Constructor
    explicit Function(std::vector<std::unique_ptr<Statement>> statements);

    /// Destructor
    ~Function() override = default;

    /// Returns a const-reference to the statements.
    const std::vector<std::unique_ptr<Statement>>& getStatements() const;

    /// Returns a reference to the statements.
    std::vector<std::unique_ptr<Statement>>& getStatements();

    /// Accept function for applying the visitor pattern on the AST.
    void accept(ASTConstVisitor& visitor) const final;
    void accept(ASTVisitor& visitor) final;

    /// Execution function.
    void execute(exec::ExecutionContext& context) const final;

    private:
    std::vector<std::unique_ptr<Statement>> statements;
};

class Statement : public ExecutableNode {
    public:
    /// Destructor
    ~Statement() override = default;

    /// Returns a reference to the expression of the statement.
    const Expression& getExpression() const;
    Expression& getExpression();

    /// Set a new expression.
    void setExpression(std::unique_ptr<Expression> newExpression);

    protected:
    /// Constructor
    Statement(Type type, std::unique_ptr<Expression> expression);

    std::unique_ptr<Expression> expression;
};

class AssignmentStatement : public Statement {
    public:
    /// Constructor
    AssignmentStatement(std::unique_ptr<Identifier> target,
                        std::unique_ptr<Expression> expression);

    /// Destructor
    ~AssignmentStatement() override = default;

    /// Returns a const-reference to the assignment target.
    const Identifier& getAssignmentTarget() const;

    /// Accept function for applying the visitor pattern on the AST.
    void accept(ASTConstVisitor& visitor) const final;
    void accept(ASTVisitor& visitor) final;

    /// Execution function.
    void execute(exec::ExecutionContext& context) const final;

    private:
    std::unique_ptr<Identifier> assignmentTarget;
};

class ReturnStatement : public Statement {
    public:
    /// Constructor
    explicit ReturnStatement(std::unique_ptr<Expression> expression);

    /// Destructor
    ~ReturnStatement() override = default;

    /// Accept function for applying the visitor pattern on the AST.
    void accept(ASTConstVisitor& visitor) const final;
    void accept(ASTVisitor& visitor) final;

    /// Execution function.
    void execute(exec::ExecutionContext& context) const final;
};

class Expression : public ASTNode {
    public:
    /// Destructor
    ~Expression() override = default;

    /// Function which evaluates an expression.
    virtual int64_t evaluate(exec::ExecutionContext& context) const = 0;

    protected:
    /// Constructor
    explicit Expression(Type type);
};

class ConstantLiteral : public Expression {
    public:
    /// Constructor
    explicit ConstantLiteral(int64_t value);

    /// Destructor
    ~ConstantLiteral() override = default;

    /// Returns the value of the constant.
    int64_t getValue() const;

    /// Accept function for applying the visitor pattern on the AST.
    void accept(ASTConstVisitor& visitor) const final;
    void accept(ASTVisitor& visitor) final;

    /// Function which evaluates an expression.
    int64_t evaluate(exec::ExecutionContext& context) const final;

    private:
    int64_t value;
};

class Identifier : public Expression {
    public:
    /// Identifier types
    enum class Type {
        Parameter,
        Variable,
        Constant
    };

    /// Constructor
    Identifier(Type identifierType, size_t id);

    /// Destructor
    ~Identifier() override = default;

    /// Returns the identifier type.
    Type getIdentifierType() const;

    /// Returns the id.
    size_t getId() const;

    /// Accept function for applying the visitor pattern on the AST.
    void accept(ASTConstVisitor& visitor) const final;
    void accept(ASTVisitor& visitor) final;

    /// Function which evaluates an expression.
    int64_t evaluate(exec::ExecutionContext& context) const final;

    private:
    Type identifierType;
    size_t id;
};

class UnaryOp : public Expression {
    public:
    /// Unary operation types
    enum class Type {
        PlusSign,
        MinusSign
    };

    /// Constructor
    UnaryOp(Type unaryOpType, std::unique_ptr<Expression> expression);

    /// Destructor
    ~UnaryOp() override = default;

    /// Returns the type of UnaryOp.
    Type getUnaryOpType() const;

    /// Returns a reference to the expression.
    const Expression& getExpression() const;
    Expression& getExpression();

    /// Accept function for applying the visitor pattern on the AST.
    void accept(ASTConstVisitor& visitor) const final;
    void accept(ASTVisitor& visitor) final;

    /// Function which evaluates an expression.
    int64_t evaluate(exec::ExecutionContext& context) const final;

    private:
    Type unaryOpType;
    std::unique_ptr<Expression> expression;
};

class BinaryOp : public Expression {
    public:
    /// Binary operation types
    enum class Type {
        Add,
        Sub,
        Mul,
        Div
    };

    /// Constructor
    BinaryOp(std::unique_ptr<Expression> lhsExpression,
             Type binaryOpType,
             std::unique_ptr<Expression> rhsExpression);

    /// Destructor
    ~BinaryOp() override = default;

    /// Returns the type of BinaryOp.
    Type getBinaryOpType() const;

    /// Returns a reference to the lhs expression.
    const Expression& getLhsExpression() const;
    Expression& getLhsExpression();

    /// Set a new lhs expression.
    void setLhsExpression(std::unique_ptr<Expression> newLhsExpression);

    /// Returns a reference to the rhs expression.
    const Expression& getRhsExpression() const;
    Expression& getRhsExpression();

    /// Set a new rhs expression.
    void setRhsExpression(std::unique_ptr<Expression> newRhsExpression);

    /// Accept function for applying the visitor pattern on the AST.
    void accept(ASTConstVisitor& visitor) const final;
    void accept(ASTVisitor& visitor) final;

    /// Function which evaluates an expression.
    int64_t evaluate(exec::ExecutionContext& context) const final;

    private:
    Type binaryOpType;
    std::unique_ptr<Expression> lhsExpression;
    std::unique_ptr<Expression> rhsExpression;
};

} // namespace pljit::ast

#endif
