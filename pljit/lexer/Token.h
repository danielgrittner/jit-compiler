#ifndef H_lexer_Tokens
#define H_lexer_Tokens

#include "pljit/common/References.h"

namespace pljit::lexer {

/// A class which represents a token returned by the lexer.
class Token {
    public:
    /// Defines the supported token types.
    enum class Type {
        Unknown,

        // Keywords
        Param,
        Var,
        Const,
        Begin,
        End,
        Return,

        // Separators
        Comma,
        SemiColon,
        Assignment,
        Init,
        LeftParenthesis,
        RightParenthesis,
        ProgramTerminator,

        // Operators
        OpPlus,
        OpMinus,
        OpMul,
        OpDiv,

        // Others
        Identifier,
        Literal,

        // Error
        LexerError
    };

    /// Constructor
    Token(Type tokenType, common::SourceRangeReference ref);

    /// Returns the token type.
    Type getTokenType() const;

    /// Returns a reference of the token into the source code.
    common::SourceRangeReference getReference() const;

    /// Returns true if it is an error token.
    bool hasError() const;

    private:
    Type tokenType;
    /// Note: A SourceRangeReference can represent both, single- and multi-character
    ///       tokens.
    common::SourceRangeReference ref;
};

} // namespace pljit::lexer

#endif
