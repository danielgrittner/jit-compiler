#ifndef H_test_TestUtils
#define H_test_TestUtils

#include "pljit/analysis/SymbolTable.h"
#include "pljit/ast/ASTFwd.h"
#include "pljit/common/SourceCodeManager.h"
#include "pljit/exec/ExecutionContext.h"
#include <iostream>
#include <sstream>

namespace pljit::test_utils {

/// Taken from Sheet 2, Exercise 2, test_simplevm.cpp
/// Class for capturing the output to cout.
class CaptureCout {
    private:
    std::streambuf* sbuf;

    public:
    std::stringstream stream;

    /// Constructor
    CaptureCout();

    /// Destructor
    ~CaptureCout();

    /// Copy constructor/assignment
    CaptureCout(const CaptureCout& other) = delete;
    CaptureCout& operator=(const CaptureCout& other) = delete;

    /// Move constructor/assignment
    CaptureCout(CaptureCout&& other) noexcept = delete;
    CaptureCout& operator=(CaptureCout&& other) noexcept = delete;
};

void checkASTParameter(const analysis::SymbolTable& symbolTable, size_t id,
                       std::string_view name);

void checkASTVariable(const analysis::SymbolTable& symbolTable, size_t id,
                      std::string_view name);

void checkASTConstant(const analysis::SymbolTable& symbolTable, size_t id,
                      std::string_view name, int64_t constantValue);

enum class Optimization {
    DeadCodeElimination,
    ConstantPropagation,
    DeadCodeEliminationAndConstantPropagation,
    NoOptimization
};

struct ASTEnvironment {
    common::SourceCodeManager sourceCodeManager;
    analysis::SymbolTable symbolTable;
    std::unique_ptr<ast::Function> ast;

    ASTEnvironment(std::string_view code, Optimization optimization);

    void optimize(Optimization optimization) const;
};

struct ExpectedResultASTExecTest {
    int64_t expectedReturnValue{};
    exec::ExecutionContext::ErrorType expectedErrorType{};
};

void performASTExecutionTest(std::string_view code,
                             const std::vector<int64_t>& parameters,
                             Optimization optimization,
                             ExpectedResultASTExecTest expected);

} // namespace pljit::test_utils

#endif
