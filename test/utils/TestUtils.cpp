#include "TestUtils.h"
#include "pljit/analysis/SemanticAnalysis.h"
#include "pljit/exec/ExecutionContext.h"
#include "pljit/optim/ConstantPropagation.h"
#include "pljit/optim/DeadCodeElimination.h"
#include "pljit/parse_tree/ParseTree.h"
#include "pljit/parser/Parser.h"
#include <gtest/gtest.h>

namespace pljit::test_utils {

CaptureCout::CaptureCout()
    : sbuf(std::cout.rdbuf())
// Constructor
{
    std::cout.rdbuf(stream.rdbuf());
}

CaptureCout::~CaptureCout()
// Destructor
{
    std::cout.rdbuf(sbuf);
}

namespace {

void checkIdentifier(const analysis::SymbolTable& symbolTable, size_t id,
                     std::string_view name, ast::Identifier::Type symbolType) {
    auto lookUpSymbolOpt = symbolTable.lookUpSymbolName(symbolType, id);
    ASSERT_TRUE(lookUpSymbolOpt);
    auto lookUpSymbol = lookUpSymbolOpt.value();
    ASSERT_EQ(lookUpSymbol, name);

    auto lookUpResultOpt = symbolTable.lookUpSymbol(lookUpSymbol);
    ASSERT_TRUE(lookUpResultOpt);
    auto lookUpResult = lookUpResultOpt.value();

    ASSERT_EQ(lookUpResult.symbolId, id);
    ASSERT_EQ(lookUpResult.symbolType, symbolType);
    ASSERT_EQ(lookUpResult.declarationRef, name);
}

} // namespace

void checkASTParameter(const analysis::SymbolTable& symbolTable, size_t id,
                       std::string_view name) {
    checkIdentifier(symbolTable, id, name, ast::Identifier::Type::Parameter);
}

void checkASTVariable(const analysis::SymbolTable& symbolTable, size_t id,
                      std::string_view name) {
    checkIdentifier(symbolTable, id, name, ast::Identifier::Type::Variable);
}

void checkASTConstant(const analysis::SymbolTable& symbolTable, size_t id,
                      std::string_view name, int64_t constantValue) {
    checkIdentifier(symbolTable, id, name, ast::Identifier::Type::Constant);
    ASSERT_EQ(symbolTable.getConstantValue(id), constantValue);
}

ASTEnvironment::ASTEnvironment(std::string_view code, Optimization optimization)
    : sourceCodeManager(std::string{code})
{
    parser::Parser parser(sourceCodeManager);
    auto parseTree = parser.parseFunctionDefinition();

    analysis::SemanticAnalysis semanticAnalysis(sourceCodeManager, symbolTable);
    ast = semanticAnalysis.analyzeFunction(*parseTree);

    optimize(optimization);
}

void ASTEnvironment::optimize(Optimization optimization) const
{
    switch (optimization) {
        case Optimization::DeadCodeElimination: {
            optim::DeadCodeElimination optimizer;
            ast->accept(optimizer);
            break;
        }

        case Optimization::ConstantPropagation: {
            optim::ConstantPropagation optimizer(symbolTable);
            ast->accept(optimizer);
            break;
        }

        case Optimization::DeadCodeEliminationAndConstantPropagation: {
            optim::DeadCodeElimination optimizer;
            ast->accept(optimizer);

            optim::ConstantPropagation optimizer2(symbolTable);
            ast->accept(optimizer);
            break;
        }

        case Optimization::NoOptimization:
            break;
    }
}

void performASTExecutionTest(std::string_view code,
                             const std::vector<int64_t>& parameters,
                             Optimization optimization,
                             ExpectedResultASTExecTest expected) {
    ASTEnvironment env(code, Optimization::NoOptimization);

    // Execution before optimization
    std::vector<int64_t> parameters1(parameters);
    exec::ExecutionContext executionContextBeforeOptimization(std::move(parameters1), env.symbolTable);
    env.ast->execute(executionContextBeforeOptimization);

    ASSERT_EQ(executionContextBeforeOptimization.error, expected.expectedErrorType);
    if (expected.expectedErrorType == exec::ExecutionContext::ErrorType::NoError) {
        ASSERT_EQ(executionContextBeforeOptimization.returnValue, expected.expectedReturnValue);
    }

    // Optimization and execution after optimization
    env.optimize(optimization);

    std::vector<int64_t> parameters2(parameters);
    exec::ExecutionContext executionContextAfterOptimization(std::move(parameters2), env.symbolTable);
    env.ast->execute(executionContextAfterOptimization);

    ASSERT_EQ(executionContextAfterOptimization.error, expected.expectedErrorType);
    if (expected.expectedErrorType == exec::ExecutionContext::ErrorType::NoError) {
        ASSERT_EQ(executionContextAfterOptimization.returnValue, expected.expectedReturnValue);
    }
}

} // namespace pljit::test_utils
