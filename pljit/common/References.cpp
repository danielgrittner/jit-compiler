#include "References.h"
#include <cassert>

namespace pljit::common {

SourceLocationReference::SourceLocationReference(const char* ref)
    : ref(ref)
// Constructor
{}

SourceLocationReference::SourceLocationReference(std::string::const_iterator location)
    : ref(location.base())
// Constructor
{}

SourceRangeReference::SourceRangeReference(const char* ref, size_t length)
    : ref(ref),
      length(length)
// Constructor
{
    assert(ref != nullptr);
    assert(length > 0);
}

SourceRangeReference::SourceRangeReference(SourceLocationReference from, SourceLocationReference to)
    : ref(from.ref),
      length(to.ref - from.ref + 1)
// Constructor
{}

SourceRangeReference::SourceRangeReference(SourceLocationReference locationRef)
    : ref(locationRef.ref),
      length(1)
// Constructor
{}

SourceRangeReference::operator std::string_view() const
// Implicit conversion to string_view.
{
    return std::string_view(ref, length);
}

SourceRangeReference SourceRangeReference::extendUntil(SourceLocationReference location) const
// Extends the current range inclusively until the character right to location reference.
{
    assert(ref != nullptr && location.ref != nullptr);
    assert(location.ref >= ref);
    size_t newLength = location.ref - ref + 1;
    return SourceRangeReference(ref, newLength);
}

SourceLocationReference SourceRangeReference::first() const
// Returns the location reference left to the first character.
{
    assert(length > 0);
    return SourceLocationReference(ref);
}

SourceLocationReference SourceRangeReference::last() const
// Returns the location reference left to the last character.
{
    assert(length > 0 && ref != nullptr);
    return SourceLocationReference(ref + (length - 1));
}

} // namespace pljit::common
