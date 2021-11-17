#ifndef H_common_References
#define H_common_References

#include "pljit/common/ReferencesFwd.h"
#include "pljit/common/SourceCodeManagerFwd.h"
#include <string>

namespace pljit::common {

/// Represents a single location in the source code.
class SourceLocationReference {
    private:
    /// The SourceCodeManager must be a friend, in order to be able to resolve
    /// a location.
    friend SourceCodeManager;
    /// The SourceRangeReference needs to be able to construct location references
    /// from a char pointer.
    friend SourceRangeReference;

    /// Points to the char right to the location reference.
    const char* ref;

    /// Constructor
    explicit SourceLocationReference(const char* ref);

    public:
    /// Constructor
    explicit SourceLocationReference(std::string::const_iterator location);
};

/// Represents a range of characters from the source code.
class SourceRangeReference {
    private:
    /// The SourceCodeManager must be a friend, in order to be able to print the context
    /// (due to the line number).
    friend SourceCodeManager;

    const char* ref;
    size_t length;

    /// Constructor
    SourceRangeReference(const char* ref, size_t length);

    public:
    /// Constructor
    SourceRangeReference(SourceLocationReference from, SourceLocationReference to);

    /// Constructor for a range reference of length 1
    explicit SourceRangeReference(SourceLocationReference locationRef);

    /// Implicit conversion to string_view.
    operator std::string_view() const; // NOLINT

    /// Extends the current range reference until the given location.
    SourceRangeReference extendUntil(SourceLocationReference location) const;

    /// Returns the location reference where the first character of the
    /// current range is right to it.
    SourceLocationReference first() const;

    /// Returns the location reference where the last character of the
    /// current range is right to it.
    SourceLocationReference last() const;
};

} // namespace pljit::common

#endif