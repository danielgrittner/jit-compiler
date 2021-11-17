#include "Pljit.h"
#include "pljit/analysis/SemanticAnalysis.h"
#include "pljit/analysis/SymbolTable.h"
#include "pljit/ast/AST.h"
#include "pljit/common/SourceCodeManager.h"
#include "pljit/exec/ExecutionContext.h"
#include "pljit/optim/ConstantPropagation.h"
#include "pljit/optim/DeadCodeElimination.h"
#include "pljit/parse_tree/ParseTree.h"
#include "pljit/parser/Parser.h"
#include <cassert>
#include <iostream>

namespace pljit {

namespace {

bool isSourceCodeEmpty(const common::SourceCodeManager& sourceCodeManager)
{
    return sourceCodeManager.getCodeBegin() == sourceCodeManager.getCodeEnd();
}

bool parserError(const std::unique_ptr<parse_tree::FunctionDefinition>& parseTree)
{
    return parseTree == nullptr;
}

bool analysisError(const std::unique_ptr<ast::Function>& ast)
{
    return ast == nullptr;
}

Result compileError()
{
    return Result{-1, ResultCode::CompileError};
}

Result runtimeError()
{
    return Result{-1, ResultCode::RuntimeError};
}

Result errorInvalidFunctionCall()
{
    return Result{-1, ResultCode::InvalidFunctionCall};
}

Result success(int64_t result)
{
    return Result{result, ResultCode::Success};
}

void optimize(ast::Function& ast, const analysis::SymbolTable& symbolTable)
{
    optim::DeadCodeElimination deadCodeElimination;
    ast.accept(deadCodeElimination);

    optim::ConstantPropagation constantPropagation(symbolTable);
    ast.accept(constantPropagation);
}

} // namespace

FunctionHandle Pljit::registerFunction(const std::string& code)
// Registers a PL/0 function.
{
    functions.emplace_front(code);
    return FunctionHandle(functions.begin());
}

void Pljit::FunctionFrame::compile()
// Compiles the function.
{
    if (isSourceCodeEmpty(*sourceCodeManager)) {
        // The source code is empty!
        std::cout << "error: received code string of length 0" << std::endl;
        state = FunctionState::CompileError;
        return;
    }

    // Parsing and lexing
    parser::Parser parser(*sourceCodeManager);
    auto parseTree = parser.parseFunctionDefinition();
    if (parserError(parseTree)) {
        // An error occurred during the compilation!
        state = FunctionState::CompileError;
        return;
    }

    // Semantic analysis
    auto symbolTablePtr = std::make_unique<analysis::SymbolTable>();
    analysis::SemanticAnalysis semanticAnalysis(*sourceCodeManager,
                                                *symbolTablePtr);
    auto ast = semanticAnalysis.analyzeFunction(*parseTree);
    if (analysisError(ast)) {
        // An error occurred during the compilation!
        state = FunctionState::CompileError;
        return;
    }

    // Optimization passes
    optimize(*ast, *symbolTablePtr);

    // We successfully compiled the function! Update the function frame!
    assert(function == nullptr);
    function = std::move(ast);
    symbolTable = std::move(symbolTablePtr);
    state = FunctionState::Compiled;
}

Pljit::FunctionFrame::FunctionFrame(std::string code)
    : sourceCodeManager(std::make_unique<common::SourceCodeManager>(std::move(code)))
// Constructor
{}

Result Pljit::FunctionFrame::execute(std::vector<int64_t>&& parameters)
// Execute a function. If it was not yet compiled, compile it.
{
    // We first need to check whether the function is already compiled.
    bool wasCompiled = false;
    {
        std::shared_lock lck(compileMutex);
        if (state == FunctionState::CompileError) {
            return compileError();
        }
        wasCompiled = state == FunctionState::Compiled;
    }

    if (!wasCompiled) {
        // When we previously checked, the function was not yet compiled.
        std::unique_lock lck(compileMutex);
        // Does this still hold? If yes, start the compilation process.
        if (state == FunctionState::NotCompiled) {
            compile();
        }
        // Did a compile error occur in the compiling thread?
        if (state == FunctionState::CompileError) {
            return compileError();
        }
    }

    if (parameters.size() != symbolTable->getNumberOfParameters()) {
        std::cout << "error: invalid number of parameters provided, expected "
                  << symbolTable->getNumberOfParameters() << " but "
                  << parameters.size() << " were provided" << std::endl;
        return errorInvalidFunctionCall();
    }

    exec::ExecutionContext executionContext(std::move(parameters),
                                            *symbolTable);
    function->execute(executionContext);

    if (executionContext.hasError()) {
        return runtimeError();
    }
    return success(executionContext.returnValue);
}

int64_t cantFail(Result functionResult)
// Wrapper function for safe function calls.
{
    return functionResult.value;
}

FunctionHandle::FunctionHandle(Pljit::FunctionRef functionRef)
    : functionRef(functionRef)
// Constructor
{}

} // namespace pljit
