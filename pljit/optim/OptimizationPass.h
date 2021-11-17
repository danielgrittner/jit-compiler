#ifndef H_optim_OptimizationPass
#define H_optim_OptimizationPass

#include "pljit/ast/ASTVisitor.h"

namespace pljit::optim {

/// An abstract class which for applying optimization passes on the AST
/// using the visitor pattern.
class OptimizationPass : public ast::ASTVisitor {
    public:
    /// Destructor
    ~OptimizationPass() override = default;
};

} // namespace pljit::optim

#endif
