#ifndef H_ast_SemanticAnalysis
#define H_ast_SemanticAnalysis

#include "pljit/analysis/SymbolTableFwd.h"
#include "pljit/ast/AST.h"
#include "pljit/common/SourceCodeManagerFwd.h"
#include "pljit/parse_tree/ParseTreeFwd.h"
#include <memory>
#include <unordered_set>
#include <vector>

namespace pljit::analysis {

/// A class which performs semantic analysis on the parse tree returned
/// from the parser.
class SemanticAnalysis {
    public:
    /// Constructor
    SemanticAnalysis(const common::SourceCodeManager& sourceCodeManager,
                     SymbolTable& symbolTable);

    /// Semantically analyzes the parse tree and builds up an AST.
    /// If an error occurs, a nullptr will be returned.
    std::unique_ptr<ast::Function> analyzeFunction(const parse_tree::FunctionDefinition& node);

    private:
    /// Registers all symbols with the given type declared in the declarator-list.
    /// If an error occurs, false will be returned.
    bool registerDeclaratorList(const parse_tree::DeclaratorList& node,
                                ast::Identifier::Type symbolType);

    /// Registers all parameters in the symbol table and checks for duplicate
    /// declarations.
    /// If an error occurs, false will be returned.
    bool registerParameterDeclarations(const parse_tree::ParameterDeclarations& node);

    /// Registers all variables in the symbol table and checks for duplicate
    /// declarations.
    /// If an error occurs, false will be returned.
    bool registerVariableDeclarations(const parse_tree::VariableDeclarations& node);

    /// Registers all constants in the symbol table and checks for duplicate
    /// declarations. Moreover, it stores the value assigned to the constants.
    /// If an error occurs, false will be returned.
    bool registerConstantDeclarations(const parse_tree::ConstantDeclarations& node);

    /// Semantically analyzes the compound statement which means analyzing
    /// the statement list.
    /// If an error occurs, an empty vector will be returned.
    std::vector<std::unique_ptr<ast::Statement>> analyzeStatements(const parse_tree::CompoundStatement& node);

    /// Semantically analyzes a single statement of the parse tree and
    /// transforms it into a statement node of the AST.
    /// If an error occurs, a nullptr will be returned.
    std::unique_ptr<ast::Statement> analyzeStatement(const parse_tree::Statement& node);

    /// Semantically analyzes an expression.
    /// If an error occurs, a nullptr will be returned.
    std::unique_ptr<ast::Expression> analyzeExpression(const parse_tree::AdditiveExpression& node);
    std::unique_ptr<ast::Expression> analyzeExpression(const parse_tree::MultiplicativeExpression& node);
    std::unique_ptr<ast::Expression> analyzeExpression(const parse_tree::UnaryExpression& node);
    std::unique_ptr<ast::Expression> analyzeExpression(const parse_tree::PrimaryExpression& node);
    std::unique_ptr<ast::Expression> analyzeExpression(const parse_tree::Identifier& node);
    std::unique_ptr<ast::Expression> analyzeExpression(const parse_tree::Literal& node);

    /// Source code manager
    const common::SourceCodeManager& sourceCodeManager;

    /// Symbol table
    SymbolTable& symbolTable;

    /// Flag whether a return statement was found during the analysis.
    bool containsReturnStatement{false};

    /// Set which contains the symbol ids of the variables which were already initialized.
    std::unordered_set<size_t> initializedVariables{};
};

} // namespace pljit::analysis

#endif
