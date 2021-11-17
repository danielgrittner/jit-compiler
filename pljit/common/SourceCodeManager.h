#ifndef H_common_SourceCodeManager
#define H_common_SourceCodeManager

#include "pljit/common/References.h"
#include <string>
#include <string_view>

namespace pljit::common {

/// This class manages source code and implements useful methods for
/// interacting with the source code.
class SourceCodeManager {
    public:
    /// Constructor
    explicit SourceCodeManager(std::string sourceCode);

    /// Prints the context with a message for a given reference.
    void printContext(SourceLocationReference location, std::string_view message) const;

    /// Prints the range with a message for a given range reference.
    void printContext(SourceRangeReference ref, std::string_view message) const;

    using SourceCodeIterator = std::string::const_iterator;

    /// Get begin iterator for iterating over the source code.
    SourceCodeIterator getCodeBegin() const;

    /// Get end iterator for iterating over the source code.
    SourceCodeIterator getCodeEnd() const;

    private:
    /// Useful information about a location in the source code.
    struct SourceCodeLocation {
        size_t lineNumber{};
        size_t lineOffset{};
        size_t indexLineStart{};
        size_t lineLength{};
    };

    /// Resolves a location reference into its line number and line offset.
    /// Additionally, it also calculates some metadata, such as the
    /// index of the line from the location as well as the length
    /// of the line in which the SourceLocationReference is located in (both
    /// required for printContext).
    SourceCodeLocation resolveLocation(SourceLocationReference location) const;

    /// String view which contains the original source code.
    const std::string sourceCode;
};

} // namespace pljit::common

#endif
