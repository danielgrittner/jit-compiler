#ifndef H_lexer_Lexer
#define H_lexer_Lexer

#include "pljit/common/SourceCodeManager.h"
#include "pljit/common/References.h"
#include "pljit/lexer/Token.h"
#include <optional>

namespace pljit::lexer {

/// A class which performs lexical analysis on source code in a stream-like
/// fashion.
class Lexer {
    public:
    /// Constructor
    explicit Lexer(const common::SourceCodeManager& manager);

    /// Returns true if there are still tokens left.
    /// Note: Returns true even if the next token might be an illegal token.
    bool hasNext() const;

    /// Returns the next token. If an illegal token was encountered, the
    /// token type is set to "LexerError".
    Token next();

    /// Peeks the next token.
    /// If the next token is illegal, the token type is set to "LexerError".
    Token peek();

    private:
    /// Advances current extendUntil either a non-whitespace character or the end
    /// is reached.
    void trimLeadingWhitespace();

    /// Determines the token type of a single character token. If it isn't
    /// a defined single character token type, then the "Unknown" token type is
    /// returned.
    static Token::Type determineSingleCharacterTokenType(char c);

    /// Determines the token type of an alpha character (i.e. a-z and A-Z)
    /// token type of the category keyword. If none of the keywords match,
    /// then the token must be an identifier.
    static Token::Type determineAlphaCharTokenType(common::SourceRangeReference ref);

    /// Source code manager
    const common::SourceCodeManager& sourceCodeManager;

    /// Iterators for iterating over the characters of the source code.
    common::SourceCodeManager::SourceCodeIterator current;
    common::SourceCodeManager::SourceCodeIterator end;

    /// Cache, needed for the peek functionality.
    std::optional<Token> tokenCache;
};

} // namespace pljit::lexer

#endif
