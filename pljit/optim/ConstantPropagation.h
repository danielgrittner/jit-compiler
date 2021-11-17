#ifndef H_optim_ConstantPropagation
#define H_optim_ConstantPropagation

#include "pljit/analysis/SymbolTableFwd.h"
#include "pljit/ast/AST.h"
#include "pljit/optim/OptimizationPass.h"
#include <optional>
#include <unordered_map>

namespace pljit::optim {

class ConstantPropagation : public OptimizationPass {
    public:
    /// Constructor
    explicit ConstantPropagation(const analysis::SymbolTable& symbolTable);

    /// Destructor
    ~ConstantPropagation() override = default;

    /// Visit methods

    void visit(ast::Function& node) final;

    void visit(ast::AssignmentStatement& node) final;

    void visit(ast::ReturnStatement& node) final;

    void visit(ast::ConstantLiteral& node) final;

    void visit(ast::Identifier& node) final;

    void visit(ast::UnaryOp& node) final;

    void visit(ast::BinaryOp& node) final;

    private:
    /// Optional for storing the result of evaluating a constant expression.
    /// This variable is used to propagate the evaluated constant expression
    /// results up the tree.
    std::optional<int64_t> constantResultFromLastCall{};

    struct VariableAssignment {
        int64_t value{};
        /// True if a variable currently has a constant value assigned.
        /// False if a variable currently has not a constant value assigned.
        bool hasConstantAssignment{false};
    };

    using IdentifierKey = std::pair<size_t, ast::Identifier::Type>;

    /// Hash functor for IdentifierKey
    struct IdentifierKeyHash {
        size_t operator()(const IdentifierKey& key) const {
            /// Hash function inspired by boost::hash_combine.
            auto h1 = std::hash<size_t>()(key.first);
            auto h2 = std::hash<size_t>()(static_cast<size_t>(key.second));
            return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
        }
    };

    /// Map in which we track whether a variable has a constant value
    /// assigned and its value.
    std::unordered_map<IdentifierKey, VariableAssignment, IdentifierKeyHash> variableTable;

    /// Symbol table for obtaining the constant values of Const variables.
    const analysis::SymbolTable& symbolTable;
};

} // namespace pljit::optim

#endif