#include "Token.h"
#include <cassert>

namespace pljit::lexer {

Token::Token(Type tokenType, common::SourceRangeReference ref)
    : tokenType(tokenType),
      ref(ref)
// Constructor
{
    assert(tokenType != Type::Unknown);
}

Token::Type Token::getTokenType() const
// Returns the token type.
{
    return tokenType;
}

common::SourceRangeReference Token::getReference() const
// Returns a reference of the token into the source code.
{
    return ref;
}

bool Token::hasError() const
// Returns true if it is an error token.
{
    return tokenType == Type::LexerError;
}

} // namespace pljit::lexer
