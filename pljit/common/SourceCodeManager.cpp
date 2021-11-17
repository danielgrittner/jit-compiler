#include "SourceCodeManager.h"
#include <iomanip>
#include <iostream>

namespace pljit::common {

SourceCodeManager::SourceCodeManager(std::string code)
    : sourceCode(std::move(code))
// Constructor
{}

void SourceCodeManager::printContext(SourceLocationReference location, std::string_view message) const
// Prints the context with a message for a given reference.
{
    SourceRangeReference rangeRef(location);
    printContext(rangeRef, message);
}

void SourceCodeManager::printContext(SourceRangeReference ref, std::string_view message) const
// Prints the range with a message for a given range reference.
{
    auto locationInfo = resolveLocation(ref.first());

    // Print the first line containing some meta information and the message.
    std::cout << locationInfo.lineNumber
              << ":"
              << locationInfo.lineOffset
              << ": "
              << message
              << std::endl;

    // Print the actual line, i.e. the context.
    const auto* lineStart = sourceCode.data() + locationInfo.indexLineStart;
    std::string_view line(lineStart, locationInfo.lineLength);
    std::cout << line << std::endl;

    // Underline the referenced section.
    std::cout << std::setw(static_cast<int>(locationInfo.lineOffset))
              << "^"
              << std::setfill('~')
              << std::setw(static_cast<int>(ref.length))
              << "\n"
              << std::setfill(' '); // Reset the fill character.
}

SourceCodeManager::SourceCodeIterator SourceCodeManager::getCodeBegin() const
// Returns an iterator referring to the first character.
{
    return sourceCode.cbegin();
}

SourceCodeManager::SourceCodeIterator SourceCodeManager::getCodeEnd() const
// Returns an iterator referring to one-past-the-end character.
{
    return sourceCode.cend();
}

SourceCodeManager::SourceCodeLocation SourceCodeManager::resolveLocation(
    SourceLocationReference ref) const
// Resolves a (range) reference into its line and line offset position.
{
    size_t currentLine = 1;
    size_t startOfCurrentLine = 0;

    // Find the start of the line and the line number.
    size_t stopIndex = ref.ref - sourceCode.data();
    for (size_t i = 0; i < stopIndex; ++i) {
        if (sourceCode[i] == '\n') {
            currentLine++;
            startOfCurrentLine = i + 1;
        }
    }

    // Find the length of the line.
    size_t lineLength = stopIndex - startOfCurrentLine + 1;
    for (size_t i = stopIndex + 1; i < sourceCode.size() && sourceCode[i] != '\n'; ++i) {
        ++lineLength;
    }

    return {currentLine, stopIndex - startOfCurrentLine + 1, startOfCurrentLine, lineLength};
}

} // namespace pljit::common

