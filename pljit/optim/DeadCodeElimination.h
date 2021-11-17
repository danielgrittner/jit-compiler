#ifndef H_optim_DeadCodeElimination
#define H_optim_DeadCodeElimination

#include "pljit/optim/OptimizationPass.h"

namespace pljit::optim {

class DeadCodeElimination : public OptimizationPass {
    public:
    /// Destructor
    ~DeadCodeElimination() override = default;

    /// Visit methods

    void visit(ast::Function& node) final;

    void visit(ast::AssignmentStatement& node) final;

    void visit(ast::ReturnStatement& node) final;

    void visit(ast::ConstantLiteral& node) final;

    void visit(ast::Identifier& node) final;

    void visit(ast::UnaryOp& node) final;

    void visit(ast::BinaryOp& node) final;
};

} // namespace pljit::optim

#endif
