#include "ExecutionContext.h"
#include "pljit/analysis/SymbolTable.h"

namespace pljit::exec {

ExecutionContext::ExecutionContext(std::vector<int64_t>&& parameterValues,
                                   const analysis::SymbolTable& symbolTable)
    : parameterValues(std::move(parameterValues)),
      variableValues(symbolTable.getNumberOfVariables()),
      symbolTable(symbolTable)
// Constructor
{}

bool ExecutionContext::hasError() const
// Returns true if an error is set, otherwise it returns false.
{
    return error != ErrorType::NoError;
}

} // namespace pljit::exec
