#ifndef H_exec_ExecutionContext
#define H_exec_ExecutionContext

#include "pljit/analysis/SymbolTableFwd.h"
#include <vector>

namespace pljit::exec {

/// A class which wraps the necessary data structures for executing an AST.
struct ExecutionContext {
    /// Constructor
    ExecutionContext(std::vector<int64_t>&& parameterValues,
                     const analysis::SymbolTable& symbolTable);

    /// Vector which maps the parameter id to the current value.
    std::vector<int64_t> parameterValues;

    /// Map for tracking the variable assignments.
    std::vector<int64_t> variableValues;

    /// Symbol table, used for obtaining the constant values.
    const analysis::SymbolTable& symbolTable;

    /// Return value
    int64_t returnValue{};

    /// Runtime errors
    enum class ErrorType {
        NoError,
        DivisionByZero
    };

    /// Error type which might have occurred during the execution.
    ErrorType error{ErrorType::NoError};

    /// Returns true if an error is set, otherwise it returns false.
    bool hasError() const;
};

} // namespace pljit::exec

#endif
