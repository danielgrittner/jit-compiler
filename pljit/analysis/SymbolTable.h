#ifndef H_analysis_SymbolTable
#define H_analysis_SymbolTable

#include "pljit/ast/AST.h"
#include "pljit/common/References.h"
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace pljit::analysis {

/// A class which is used to collect all the used symbols during the semantic
/// analysis.
class SymbolTable {
    public:

    struct SymbolEntry {
        size_t symbolId;
        common::SourceRangeReference declarationRef;
        ast::Identifier::Type symbolType;
    };

    struct RegistrationResult {
        SymbolEntry entry;
        bool newlyRegistered;
    };

    /// Registers a symbol if it does net yet exist. If the symbol was already
    /// registered, then the already existing entry is returned. Whether the
    /// entry was newly inserted or already existed, is indicated by the
    /// newlyRegistered flag.
    /// If a Const variable is registered, then the passed constantValue is
    /// stored.
    RegistrationResult registerSymbol(ast::Identifier::Type symbolType,
                                      common::SourceRangeReference declarationRef,
                                      int64_t constantValue = -1);

    /// Looks up a symbol. If it exists, its SymbolEntry is returned. Otherwise
    /// the optional is empty.
    std::optional<SymbolEntry> lookUpSymbol(std::string_view symbolStr) const;

    /// For a given symbolId and symbolType the corresponding symbol string is looked up.
    std::optional<std::string_view> lookUpSymbolName(ast::Identifier::Type symbolType,
                                                     size_t symbolId) const;

    /// Gets the value for a registered constant.
    int64_t getConstantValue(size_t constantId) const;

    /// Returns the number of registered parameters.
    size_t getNumberOfParameters() const;

    /// Returns the number of registered variables.
    size_t getNumberOfVariables() const;

    private:
    /// Indicates the next symbol id for each identifier type.
    size_t nextParameterId{};
    size_t nextVariableId{};
    size_t nextConstantId{};

    /// Mapping between a symbol string and the symbol entries.
    std::unordered_map<std::string_view, SymbolEntry> symbolStrToSymbolEntryMapping{};

    /// Mapping between a symbol id and its symbol string (the index of the
    /// vector represents the symbol id).
    std::vector<std::string_view> parameterIdToStringMapping{};
    std::vector<std::string_view> variableIdToStringMapping{};
    std::vector<std::string_view> constantIdToStringMapping{};

    /// Stores the constant values.
    std::vector<int64_t> constantValues{};
};

} // namespace pljit::analysis

#endif