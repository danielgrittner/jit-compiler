#include "Lexer.h"
#include <cassert>

namespace pljit::lexer {

namespace {

bool isWhitespace(char c)
{
    return c == ' ' || c == '\n' || c == '\t';
}

bool isLiteralChar(char c)
{
    return '0' <= c && c <= '9';
}

bool isAlphaChar(char c)
{
    return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
}

bool isLegalChar(char c)
{
    return ('(' <= c && c <= ';') || c == '=' || // covers separators, operators, and numbers
        isAlphaChar(c) || // covers a-z and A-Z
        isWhitespace(c); // covers whitespaces
}

} // namespace

Lexer::Lexer(const common::SourceCodeManager& manager)
    : sourceCodeManager(manager),
      current(sourceCodeManager.getCodeBegin()),
      end(sourceCodeManager.getCodeEnd())
// Constructor
{
    // We always ensure that current points to a non-whitespace character.
    trimLeadingWhitespace();
}

bool Lexer::hasNext() const
// Returns true if there are still tokens left and false otherwise.
{
    return current != end || tokenCache.has_value();
}

Token Lexer::next()
// Returns the next token.
{
    if (tokenCache.has_value()) {
        // If an error occurred during peek(), we also want to return the
        // error here as well.
        if (tokenCache->getTokenType() == Token::Type::LexerError) {
            return tokenCache.value();
        }
        // peek() was previously called, hence, we have the next token
        // already in the token cache.
        auto token = tokenCache.value();
        tokenCache = std::nullopt; // Invalidate the cache.
        return token;
    }

    assert(hasNext());
    // Current should always point to the first non-whitespace character if the
    // end was not yet reached.
    assert(!isWhitespace(*current));

    // Determine the token type and obtain a range reference.
    auto firstChar = *current;
    common::SourceLocationReference startRef(current);

    if (!isLegalChar(firstChar)) {
        // The next char is an invalid character. Hence, we stop the compilation.
        sourceCodeManager.printContext(startRef, "error: illegal character");
        // Store the error in the token cache to not let the caller continue.
        tokenCache = Token(Token::Type::LexerError, common::SourceRangeReference(startRef));
        return tokenCache.value();
    }

    // Move to the next char.
    ++current;

    // Do we have a single character type?
    auto tokenType = determineSingleCharacterTokenType(firstChar);
    if (tokenType != Token::Type::Unknown) {
        // We have a single character token!

        // Advance the current position until the next non-whitespace character is
        // found or the end is reached.
        trimLeadingWhitespace();

        return Token(tokenType, common::SourceRangeReference(startRef));
    }

    // The token must be a multi-character token (if valid of course)!

    auto lastChar = firstChar;
    auto endRef = startRef;

    bool isLiteral = isLiteralChar(firstChar);
    tokenType = isLiteral ? Token::Type::Literal : Token::Type::Unknown;

    for (; current != end; ++current) {
        char currentChar = *current;

        if (!isLegalChar(currentChar)) {
            // The current char is an illegal character. Hence, we stop the compilation.
            common::SourceLocationReference ref(current);
            sourceCodeManager.printContext(ref, "error: illegal character");
            // Store the error in the token cache to not let the caller continue.
            tokenCache = Token(Token::Type::LexerError, common::SourceRangeReference(ref));
            return tokenCache.value();
        }

        // Edge case: assignment operator
        if (lastChar == ':') {
            if (currentChar != '=') {
                // We have an illegal token type. We stop the compilation.
                common::SourceLocationReference currentRef(current);
                common::SourceRangeReference rangeRef(startRef, currentRef);
                sourceCodeManager.printContext(rangeRef, "error: unknown multi-character token");
                // Store the error in the token cache to not let the caller continue.
                tokenCache = Token(Token::Type::LexerError, rangeRef);
                return tokenCache.value();
            }
            // We found an assignment token!
            tokenType = Token::Type::Assignment;
            endRef = common::SourceLocationReference(current);
            ++current;
            break;
        }

        if (!isLiteral && !isAlphaChar(currentChar)) {
            // We are done. We found a character which cannot be part of an identifier or a keyword.
            break;
        }

        if (isLiteral && !isLiteralChar(currentChar)) {
            // We are done parsing the literal.
            break;
        }

        // Update the endRef.
        lastChar = *current;
        endRef = common::SourceLocationReference(current);
    }

    // Determine the token type based on the given information in case the token is
    // still unknown.
    common::SourceRangeReference ref(startRef, endRef);
    tokenType = tokenType == Token::Type::Unknown ? determineAlphaCharTokenType(ref) : tokenType;

    // Advance the current position until the next non-whitespace character is
    // found or the end is reached.
    trimLeadingWhitespace();

    assert(tokenType != Token::Type::Unknown);
    return Token(tokenType, ref);
}

Token Lexer::peek()
// Peeks the next token.
{
    if (tokenCache.has_value()) {
        // Since the last peek, there was no next() call.
        return tokenCache.value();
    }
    // The cache is empty, hence, we calculate the next token and cache it.
    tokenCache = next();
    return tokenCache.value();
}

void Lexer::trimLeadingWhitespace()
// Trims leading whitespace.
{
    for (; current != end && isWhitespace(*current); ++current);
}

Token::Type Lexer::determineSingleCharacterTokenType(char c)
// Determines the type of a single character token.
{
    switch (c) {
        case ',':
            return Token::Type::Comma;

        case ';':
            return Token::Type::SemiColon;

        case '=':
            return Token::Type::Init;

        case '(':
            return Token::Type::LeftParenthesis;

        case ')':
            return Token::Type::RightParenthesis;

        case '.':
            return Token::Type::ProgramTerminator;

        case '+':
            return Token::Type::OpPlus;

        case '-':
            return Token::Type::OpMinus;

        case '*':
            return Token::Type::OpMul;

        case '/':
            return Token::Type::OpDiv;

        default:
            return Token::Type::Unknown;
    }
}

Token::Type Lexer::determineAlphaCharTokenType(common::SourceRangeReference ref)
// Determines the type of a keyword token.
{
    std::string_view codeString = ref;
    assert(!codeString.empty());

    if (codeString == "PARAM") {
        return Token::Type::Param;
    }

    if (codeString == "VAR") {
        return Token::Type::Var;
    }

    if (codeString == "CONST") {
        return Token::Type::Const;
    }

    if (codeString == "BEGIN") {
        return Token::Type::Begin;
    }

    if (codeString == "END") {
        return Token::Type::End;
    }

    if (codeString == "RETURN") {
        return Token::Type::Return;
    }

    // The multi-character token could not be identified as a keyword,
    // hence, it must be an identifier.
    return Token::Type::Identifier;
}

} // namespace pljit::lexer
