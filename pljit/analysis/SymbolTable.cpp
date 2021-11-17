#include "SymbolTable.h"
#include <cassert>

namespace pljit::analysis {

namespace {

constexpr bool SYMBOL_ALREADY_EXISTS = false;
constexpr bool SYMBOL_NEWLY_INSERTED = true;

std::optional<std::string_view> lookUpString(size_t symbolId,
                                             const std::vector<std::string_view>& vec)
{
    return symbolId < vec.size() ? std::optional(vec[symbolId]) : std::nullopt;
}

} // namespace

SymbolTable::RegistrationResult SymbolTable::registerSymbol(ast::Identifier::Type symbolType,
                                                            common::SourceRangeReference declarationRef,
                                                            int64_t constantValue)
// Registers a symbol if it does not yet exist. Otherwise the existing entry is returned.
{
    if (auto it = symbolStrToSymbolEntryMapping.find(declarationRef);
        it != symbolStrToSymbolEntryMapping.end()) {
        // The symbol already exists!
        return {it->second, SYMBOL_ALREADY_EXISTS};
    }

    // The symbol does not yet exist. Hence, we insert it.

    size_t nextSymbolId;
    if (symbolType == ast::Identifier::Type::Parameter) {
        nextSymbolId = nextParameterId++;
        parameterIdToStringMapping.push_back(declarationRef);
    } else if (symbolType == ast::Identifier::Type::Variable) {
        nextSymbolId = nextVariableId++;
        variableIdToStringMapping.push_back(declarationRef);
    } else {
        nextSymbolId = nextConstantId++;
        constantIdToStringMapping.push_back(declarationRef);
        assert(constantValue != -1); // A constant value should always be >= 0.
        constantValues.push_back(constantValue);
    }

    SymbolEntry newEntry{nextSymbolId, declarationRef, symbolType};
    symbolStrToSymbolEntryMapping.emplace(declarationRef, newEntry);

    return {newEntry, SYMBOL_NEWLY_INSERTED};
}

std::optional<SymbolTable::SymbolEntry> SymbolTable::lookUpSymbol(
    std::string_view symbolStr) const
// Looks up a symbol and returns its symbol entry if it exists.
{
    auto it = symbolStrToSymbolEntryMapping.find(symbolStr);
    return it != symbolStrToSymbolEntryMapping.end()
        ? std::optional(it->second) : std::nullopt;
}

std::optional<std::string_view> SymbolTable::lookUpSymbolName(ast::Identifier::Type symbolType,
                                                              size_t symbolId) const
// For a given symbolId and symbolType the corresponding symbol string is looked up.
{
    if (symbolType == ast::Identifier::Type::Parameter) {
        return lookUpString(symbolId, parameterIdToStringMapping);
    }

    if (symbolType == ast::Identifier::Type::Variable) {
        return lookUpString(symbolId, variableIdToStringMapping);
    }

    assert(symbolType == ast::Identifier::Type::Constant);
    return lookUpString(symbolId, constantIdToStringMapping);
}

int64_t SymbolTable::getConstantValue(size_t constantId) const
// Gets the value for a registered constant.
{
    assert(constantId < constantValues.size());
    return constantValues[constantId];
}

size_t SymbolTable::getNumberOfParameters() const
// Returns the number of registered parameters.
{
    return parameterIdToStringMapping.size();
}

size_t SymbolTable::getNumberOfVariables() const
// Returns the number of registered variables.
{
    return variableIdToStringMapping.size();
}

} // namespace pljit::analysis
