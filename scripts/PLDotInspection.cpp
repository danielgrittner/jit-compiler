#include "pljit/analysis/SemanticAnalysis.h"
#include "pljit/analysis/SymbolTable.h"
#include "pljit/ast/ASTDotVisitor.h"
#include "pljit/common/SourceCodeManager.h"
#include "pljit/optim/ConstantPropagation.h"
#include "pljit/optim/DeadCodeElimination.h"
#include "pljit/parse_tree/ParseTree.h"
#include "pljit/parse_tree/ParseTreeDotVisitor.h"
#include "pljit/parser/Parser.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>

enum class ExecutionType {
    ParseTree,
    AST,
    ASTWithDeadCodeElimination,
    ASTWithConstProp,
    ASTWithDeadCodeEliminationAndConstProp
};

/// This script allows to inspect the parse tree or ASTs of PL programs. It
/// takes as input file a PL program and writes the parse tree or AST in the DOT
/// format to the output file.
/// The output file could then be visualized with xdot: "xdot <dot-file>".
///
/// - Printing the parse tree:
///             ./<script-executable> -P <infile> <outfile>
///
/// - Printing the AST without optimization passes:
///             ./<script-executable> -A <infile> <outfile>
///
/// - Printing the AST with dead code elimination:
///             ./<script-executable> -Ad <infile> <outfile>
///
/// - Printing the AST with constant propagation:
///             ./<script-executable> -Ac <infile> <outfile>
///
/// - Printing the AST with constant propagation and dead code elimination:
///             ./<script-executable> -Acd <infile> <outfile>
///
int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "USAGE: " << argv[0] << " [-P|-A|-Ad|-Ac|-Acd] <infile> <outfile>" << std::endl;
        return 1;
    }

    ExecutionType execType;
    if (std::strcmp(argv[1], "-P") == 0) {
        execType = ExecutionType::ParseTree;
    } else if (std::strcmp(argv[1], "-A") == 0) {
        execType = ExecutionType::AST;
    } else if (std::strcmp(argv[1], "-Ad") == 0) {
        execType = ExecutionType::ASTWithDeadCodeElimination;
    } else if (std::strcmp(argv[1], "-Ac") == 0) {
        execType = ExecutionType::ASTWithConstProp;
    } else if (std::strcmp(argv[1], "-Acd") == 0) {
        execType = ExecutionType::ASTWithDeadCodeEliminationAndConstProp;
    } else {
        std::cerr << "Could not recognize flag " << argv[1] << std::endl;
        return 1;
    }

    std::ifstream in(argv[2]);
    if (!in.is_open()) {
        std::cerr << "Failed to open file " << argv[2] << std::endl;
        return 1;
    }

    std::ofstream out(argv[3]);
    if (!out.is_open()) {
        std::cerr << "Failed to open file " << argv[3] << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << in.rdbuf();
    auto code = buffer.str();

    pljit::common::SourceCodeManager sourceCodeManager(code);
    pljit::parser::Parser parser(sourceCodeManager);
    auto parseTree = parser.parseFunctionDefinition();
    if (parseTree == nullptr) {
        return 1;
    }

    switch (execType) {
        case ExecutionType::ParseTree: {
            pljit::parse_tree::ParseTreeDotVisitor visitor(out);
            parseTree->accept(visitor);
            break;
        }

        case ExecutionType::AST: {
            pljit::analysis::SymbolTable symbolTable;
            pljit::analysis::SemanticAnalysis semanticAnalysis(sourceCodeManager, symbolTable);
            auto ast = semanticAnalysis.analyzeFunction(*parseTree);
            if (ast == nullptr) {
                return 1;
            }

            pljit::ast::ASTDotVisitor visitor(out, symbolTable);
            ast->accept(visitor);
            break;
        }

        case ExecutionType::ASTWithDeadCodeElimination: {
            pljit::analysis::SymbolTable symbolTable;
            pljit::analysis::SemanticAnalysis semanticAnalysis(sourceCodeManager, symbolTable);
            auto ast = semanticAnalysis.analyzeFunction(*parseTree);
            if (ast == nullptr) {
                return 1;
            }

            pljit::optim::DeadCodeElimination optimizationPass1;
            ast->accept(optimizationPass1);

            pljit::ast::ASTDotVisitor visitor(out, symbolTable);
            ast->accept(visitor);
            break;
        }

        case ExecutionType::ASTWithConstProp: {
            pljit::analysis::SymbolTable symbolTable;
            pljit::analysis::SemanticAnalysis semanticAnalysis(sourceCodeManager, symbolTable);
            auto ast = semanticAnalysis.analyzeFunction(*parseTree);
            if (ast == nullptr) {
                return 1;
            }

            pljit::optim::ConstantPropagation optimizationPass2(symbolTable);
            ast->accept(optimizationPass2);

            pljit::ast::ASTDotVisitor visitor(out, symbolTable);
            ast->accept(visitor);
            break;
        }

        case ExecutionType::ASTWithDeadCodeEliminationAndConstProp: {
            pljit::analysis::SymbolTable symbolTable;
            pljit::analysis::SemanticAnalysis semanticAnalysis(sourceCodeManager, symbolTable);
            auto ast = semanticAnalysis.analyzeFunction(*parseTree);
            if (ast == nullptr) {
                return 1;
            }

            pljit::optim::DeadCodeElimination optimizationPass1;
            ast->accept(optimizationPass1);

            pljit::optim::ConstantPropagation optimizationPass2(symbolTable);
            ast->accept(optimizationPass2);

            pljit::ast::ASTDotVisitor visitor(out, symbolTable);
            ast->accept(visitor);
            break;
        }
    }

    return 0;
}