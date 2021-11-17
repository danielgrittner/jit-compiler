#include "DeadCodeElimination.h"
#include "pljit/ast/AST.h"
#include <cassert>

namespace pljit::optim {

void DeadCodeElimination::visit(ast::Function& node)
{
    auto& statements = node.getStatements();

    // We iterate until we found the first return statement.
    size_t statementId = 0;
    for (; statementId < statements.size(); ++statementId) {
        if (statements[statementId]->getType() == ast::ASTNode::Type::ReturnStatement) {
            break;
        }
    }

    // We cut off all the statements behind the first return statement.
    assert(statementId < statements.size());
    statements.resize(statementId + 1);
}

void DeadCodeElimination::visit(ast::AssignmentStatement& /*node*/) {}

void DeadCodeElimination::visit(ast::ReturnStatement& /*node*/) {}

void DeadCodeElimination::visit(ast::ConstantLiteral& /*node*/) {}

void DeadCodeElimination::visit(ast::Identifier& /*node*/) {}

void DeadCodeElimination::visit(ast::UnaryOp& /*node*/) {}

void DeadCodeElimination::visit(ast::BinaryOp& /*node*/) {}

} // namespace pljit::optim
