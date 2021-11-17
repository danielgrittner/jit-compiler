#include "pljit/common/SourceCodeManager.h"
#include "pljit/lexer/Lexer.h"
#include "test/utils/TestUtils.h"
#include <gtest/gtest.h>

namespace pljit::lexer {

bool operator==(const Token& t1, const Token& t2) {
    return t1.getTokenType() == t2.getTokenType() &&
        static_cast<std::string_view>(t1.getReference()) == static_cast<std::string_view>(t2.getReference());
}

namespace {

using ExpectedTokens = std::vector<Token>;

void executeLexerTest(const common::SourceCodeManager& sourceCodeManager,
                      const ExpectedTokens& expectedTokens) {
    Lexer lexer(sourceCodeManager);

    size_t expectedIdx = 0;
    for (; lexer.hasNext() && expectedIdx < expectedTokens.size(); ++expectedIdx) {
        auto nextToken = lexer.next();
        ASSERT_EQ(nextToken, expectedTokens[expectedIdx]);
    }

    ASSERT_EQ(expectedIdx, expectedTokens.size());
    ASSERT_FALSE(lexer.hasNext());
}

void executeLexerErrorTest(const common::SourceCodeManager& sourceCodeManager,
                           std::string_view expectedErrorMessage) {
    test_utils::CaptureCout cout;
    Lexer lexer(sourceCodeManager);

    ASSERT_EQ(lexer.peek().getTokenType(), Token::Type::LexerError);

    auto errorToken = lexer.next();

    ASSERT_EQ(errorToken.getTokenType(), Token::Type::LexerError);
    ASSERT_TRUE(lexer.hasNext());
    ASSERT_EQ(cout.stream.str(), expectedErrorMessage);
}

} // namespace

TEST(TestLexer, LexicalAnalysisEachCategory) { // NOLINT
    std::string code{"PARAM VAR CONST BEGIN END RETURN\n"
                     "\t, ; := = ( ) .\n"
                     "\t+ - * /\n"
                     "1234abc\n"
                     "12 34 ABCDEFG\t\t    \n"};
    common::SourceCodeManager sourceCodeManager(std::move(code));

    ExpectedTokens expectedTokens;
    auto it = sourceCodeManager.getCodeBegin();

    // Keyword: PARAM
    common::SourceLocationReference paramStartRef(it);
    std::advance(it, 4);
    common::SourceLocationReference paramEndRef(it);
    expectedTokens.emplace_back(Token::Type::Param,
                                common::SourceRangeReference(paramStartRef, paramEndRef));

    // Keyword: VAR
    std::advance(it, 2);
    common::SourceLocationReference varStartRef(it);
    std::advance(it, 2);
    common::SourceLocationReference varEndRef(it);
    expectedTokens.emplace_back(Token::Type::Var,
                                common::SourceRangeReference(varStartRef, varEndRef));

    // Keyword: CONST
    std::advance(it, 2);
    common::SourceLocationReference constStartRef(it);
    std::advance(it, 4);
    common::SourceLocationReference constEndRef(it);
    expectedTokens.emplace_back(Token::Type::Const,
                                common::SourceRangeReference(constStartRef, constEndRef));

    // Keyword: BEGIN
    std::advance(it, 2);
    common::SourceLocationReference beginStartRef(it);
    std::advance(it, 4);
    common::SourceLocationReference beginEndRef(it);
    expectedTokens.emplace_back(Token::Type::Begin,
                                common::SourceRangeReference(beginStartRef, beginEndRef));

    // Keyword: END
    std::advance(it, 2);
    common::SourceLocationReference endStartRef(it);
    std::advance(it, 2);
    common::SourceLocationReference endEndRef(it);
    expectedTokens.emplace_back(Token::Type::End,
                                common::SourceRangeReference(endStartRef, endEndRef));

    // Keyword: RETURN
    std::advance(it, 2);
    common::SourceLocationReference returnStartRef(it);
    std::advance(it, 5);
    common::SourceLocationReference returnEndRef(it);
    expectedTokens.emplace_back(Token::Type::Return,
                                common::SourceRangeReference(returnStartRef, returnEndRef));

    // Comma
    std::advance(it, 3);
    common::SourceLocationReference commaRef(it);
    expectedTokens.emplace_back(Token::Type::Comma,
                                common::SourceRangeReference(commaRef));

    // Semi-colon
    std::advance(it, 2);
    common::SourceLocationReference semiColonRef(it);
    expectedTokens.emplace_back(Token::Type::SemiColon,
                                common::SourceRangeReference(semiColonRef));

    // Assignment
    std::advance(it, 2);
    common::SourceLocationReference assignmentStartRef(it);
    std::advance(it, 1);
    common::SourceLocationReference assignmentEndRef(it);
    expectedTokens.emplace_back(Token::Type::Assignment,
                                common::SourceRangeReference(assignmentStartRef, assignmentEndRef));

    // Init
    std::advance(it, 2);
    common::SourceLocationReference initRef(it);
    expectedTokens.emplace_back(Token::Type::Init,
                                common::SourceRangeReference(initRef));

    // (
    std::advance(it, 2);
    common::SourceLocationReference leftParenRef(it);
    expectedTokens.emplace_back(Token::Type::LeftParenthesis,
                                common::SourceRangeReference(leftParenRef));

    // )
    std::advance(it, 2);
    common::SourceLocationReference rightParenRef(it);
    expectedTokens.emplace_back(Token::Type::RightParenthesis,
                                common::SourceRangeReference(rightParenRef));

    // .
    std::advance(it, 2);
    common::SourceLocationReference programTerminatorRef(it);
    expectedTokens.emplace_back(Token::Type::ProgramTerminator,
                                common::SourceRangeReference(programTerminatorRef));

    // +
    std::advance(it, 3);
    common::SourceLocationReference plusRef(it);
    expectedTokens.emplace_back(Token::Type::OpPlus,
                                common::SourceRangeReference(plusRef));

    // -
    std::advance(it, 2);
    common::SourceLocationReference minusRef(it);
    expectedTokens.emplace_back(Token::Type::OpMinus,
                                common::SourceRangeReference(minusRef));

    // *
    std::advance(it, 2);
    common::SourceLocationReference mulRef(it);
    expectedTokens.emplace_back(Token::Type::OpMul,
                                common::SourceRangeReference(mulRef));

    // /
    std::advance(it, 2);
    common::SourceLocationReference divRef(it);
    expectedTokens.emplace_back(Token::Type::OpDiv,
                                common::SourceRangeReference(divRef));

    // Literal: 1234
    std::advance(it, 2);
    common::SourceLocationReference literalStartRef1(it);
    std::advance(it, 3);
    common::SourceLocationReference literalEndRef1(it);
    expectedTokens.emplace_back(Token::Type::Literal,
                                common::SourceRangeReference(literalStartRef1, literalEndRef1));

    // Identifier: abc
    std::advance(it, 1);
    common::SourceLocationReference identifierStartRef1(it);
    std::advance(it, 2);
    common::SourceLocationReference identifierEndRef1(it);
    expectedTokens.emplace_back(Token::Type::Identifier,
                                common::SourceRangeReference(identifierStartRef1,
                                                             identifierEndRef1));

    // Literal: 12
    std::advance(it, 2);
    common::SourceLocationReference literalStartRef2(it);
    std::advance(it, 1);
    common::SourceLocationReference literalEndRef2(it);
    expectedTokens.emplace_back(Token::Type::Literal,
                                common::SourceRangeReference(literalStartRef2,
                                                             literalEndRef2));

    // Literal: 34
    std::advance(it, 2);
    common::SourceLocationReference literalStartRef3(it);
    std::advance(it, 1);
    common::SourceLocationReference literalEndRef3(it);
    expectedTokens.emplace_back(Token::Type::Literal,
                                common::SourceRangeReference(literalStartRef3,
                                                             literalEndRef3));

    // Identifier: ABCDEFG
    std::advance(it, 2);
    common::SourceLocationReference identifierStartRef2(it);
    std::advance(it, 6);
    common::SourceLocationReference identifierEndRef2(it);
    expectedTokens.emplace_back(Token::Type::Identifier,
                                common::SourceRangeReference(identifierStartRef2,
                                                             identifierEndRef2));

    executeLexerTest(sourceCodeManager, expectedTokens);
}

TEST(TestLexer, SimpleLexicalAnalysis) { // NOLINT
    std::string code{"PARAM ab, cd;\n"
                     "BEGIN\n"
                     "     RETURN (ab + cd) / 1234\n"
                     "END\n"
                     "."};
    common::SourceCodeManager sourceCodeManager(std::move(code));

    ExpectedTokens expectedTokens;
    auto it = sourceCodeManager.getCodeBegin();

    // PARAM
    common::SourceLocationReference paramStartRef(it);
    std::advance(it, 4);
    common::SourceLocationReference paramEndRef(it);
    expectedTokens.emplace_back(Token::Type::Param,
                                common::SourceRangeReference(paramStartRef, paramEndRef));

    // Identifier: ab
    std::advance(it, 2);
    common::SourceLocationReference x1StartRef(it);
    std::advance(it, 1);
    common::SourceLocationReference x1EndRef(it);
    expectedTokens.emplace_back(Token::Type::Identifier,
                                common::SourceRangeReference(x1StartRef, x1EndRef));

    // Comma
    std::advance(it, 1);
    common::SourceLocationReference commaLine1Ref(it);
    expectedTokens.emplace_back(Token::Type::Comma,
                                common::SourceRangeReference(commaLine1Ref));

    // Identifier: cd
    std::advance(it, 2);
    common::SourceLocationReference x2StartRef(it);
    std::advance(it, 1);
    common::SourceLocationReference x2EndRef(it);
    expectedTokens.emplace_back(Token::Type::Identifier,
                                common::SourceRangeReference(x2StartRef, x2EndRef));

    // Semi colon
    std::advance(it, 1);
    common::SourceLocationReference semiColonFirstLine(it);
    expectedTokens.emplace_back(Token::Type::SemiColon,
                                common::SourceRangeReference(semiColonFirstLine));

    // BEGIN
    std::advance(it, 2);
    common::SourceLocationReference beginStartRef(it);
    std::advance(it, 4);
    common::SourceLocationReference beginEndRef(it);
    expectedTokens.emplace_back(Token::Type::Begin,
                                common::SourceRangeReference(beginStartRef, beginEndRef));

    // RETURN
    std::advance(it, 7);
    common::SourceLocationReference returnStartRef(it);
    std::advance(it, 5);
    common::SourceLocationReference returnEndRef(it);
    expectedTokens.emplace_back(Token::Type::Return,
                                common::SourceRangeReference(returnStartRef, returnEndRef));

    // (
    std::advance(it, 2);
    common::SourceLocationReference leftParenStartRef(it);
    expectedTokens.emplace_back(Token::Type::LeftParenthesis,
                                common::SourceRangeReference(leftParenStartRef));

    // Identifier: ab
    std::advance(it, 1);
    common::SourceLocationReference x1UsedStartRef(it);
    std::advance(it, 1);
    common::SourceLocationReference x1UsedEndRef(it);
    expectedTokens.emplace_back(Token::Type::Identifier,
                                common::SourceRangeReference(x1UsedStartRef, x1UsedEndRef));

    // + operator
    std::advance(it, 2);
    common::SourceLocationReference plusStartRef(it);
    expectedTokens.emplace_back(Token::Type::OpPlus,
                                common::SourceRangeReference(plusStartRef));

    // cd (referenced)
    std::advance(it, 2);
    common::SourceLocationReference x2UsedStartRef(it);
    std::advance(it, 1);
    common::SourceLocationReference x2UsedEndRef(it);
    expectedTokens.emplace_back(Token::Type::Identifier,
                                common::SourceRangeReference(x2UsedStartRef, x2UsedEndRef));

    // )
    std::advance(it, 1);
    common::SourceLocationReference rightParenStartRef(it);
    expectedTokens.emplace_back(Token::Type::RightParenthesis,
                                common::SourceRangeReference(rightParenStartRef));

    // / operator
    std::advance(it, 2);
    common::SourceLocationReference divisionOperatorRef(it);
    expectedTokens.emplace_back(Token::Type::OpDiv,
                                common::SourceRangeReference(divisionOperatorRef));

    // Literal: 1234
    std::advance(it, 2);
    common::SourceLocationReference literalStartRef(it);
    std::advance(it, 3);
    common::SourceLocationReference literalEndRef(it);
    expectedTokens.emplace_back(Token::Type::Literal,
                                common::SourceRangeReference(literalStartRef, literalEndRef));

    // END
    std::advance(it, 2);
    common::SourceLocationReference endStartRef(it);
    std::advance(it, 2);
    common::SourceLocationReference endEndRef(it);
    expectedTokens.emplace_back(Token::Type::End,
                                common::SourceRangeReference(endStartRef, endEndRef));

    // . program terminator
    std::advance(it, 2);
    common::SourceLocationReference programTerminatorRef(it);
    expectedTokens.emplace_back(Token::Type::ProgramTerminator,
                                common::SourceRangeReference(programTerminatorRef));

    executeLexerTest(sourceCodeManager, expectedTokens);
}

TEST(TestLexer, SemiColonSequence) { // NOLINT
    std::string code{";;;"};
    common::SourceCodeManager sourceCodeManager(std::move(code));

    ExpectedTokens expectedTokens;
    auto it = sourceCodeManager.getCodeBegin();

    // 1. ;
    common::SourceLocationReference firstSemiColonRef(it);
    expectedTokens.emplace_back(Token::Type::SemiColon,
                                common::SourceRangeReference(firstSemiColonRef));

    // 2. ;
    std::advance(it, 1);
    common::SourceLocationReference secondSemiColonRef(it);
    expectedTokens.emplace_back(Token::Type::SemiColon,
                                common::SourceRangeReference(secondSemiColonRef));

    // 3. ;
    std::advance(it, 1);
    common::SourceLocationReference thirdSemiColonRef(it);
    expectedTokens.emplace_back(Token::Type::SemiColon,
                                common::SourceRangeReference(thirdSemiColonRef));

    executeLexerTest(sourceCodeManager, expectedTokens);
}

TEST(TestLexer, ComplexCalculation) { // NOLINT
    std::string code{"((ab -1234)+(cd/  23 ))"};
    common::SourceCodeManager sourceCodeManager(std::move(code));

    ExpectedTokens expectedTokens;
    auto it = sourceCodeManager.getCodeBegin();

    // Left parenthesis: (
    common::SourceLocationReference leftParenthesis1Ref(it);
    expectedTokens.emplace_back(Token::Type::LeftParenthesis,
                                common::SourceRangeReference(leftParenthesis1Ref));

    // Left parenthesis: (
    std::advance(it, 1);
    common::SourceLocationReference leftParenthesis2Ref(it);
    expectedTokens.emplace_back(Token::Type::LeftParenthesis,
                                common::SourceRangeReference(leftParenthesis2Ref));

    // Identifier: ab
    std::advance(it, 1);
    common::SourceLocationReference identifierAbStartRef(it);
    std::advance(it, 1);
    common::SourceLocationReference identifierAbEndRef(it);
    expectedTokens.emplace_back(Token::Type::Identifier,
                                common::SourceRangeReference(identifierAbStartRef,
                                                             identifierAbEndRef));

    // Operator: -
    std::advance(it, 2);
    common::SourceLocationReference opMinusRef(it);
    expectedTokens.emplace_back(Token::Type::OpMinus,
                                common::SourceRangeReference(opMinusRef));

    // Literal: 1234
    std::advance(it, 1);
    common::SourceLocationReference literal1StartRef(it);
    std::advance(it, 3);
    common::SourceLocationReference literal1EndRef(it);
    expectedTokens.emplace_back(Token::Type::Literal,
                                common::SourceRangeReference(literal1StartRef,
                                                             literal1EndRef));

    // Right parenthesis: )
    std::advance(it, 1);
    common::SourceLocationReference rightParenthesis1Ref(it);
    expectedTokens.emplace_back(Token::Type::RightParenthesis,
                                common::SourceRangeReference(rightParenthesis1Ref));

    // Operator: +
    std::advance(it, 1);
    common::SourceLocationReference opPlusRef(it);
    expectedTokens.emplace_back(Token::Type::OpPlus,
                                common::SourceRangeReference(opPlusRef));

    // Left Parenthesis: (
    std::advance(it, 1);
    common::SourceLocationReference leftParenthesis3Ref(it);
    expectedTokens.emplace_back(Token::Type::LeftParenthesis,
                                common::SourceRangeReference(leftParenthesis3Ref));

    // Identifier: cd
    std::advance(it, 1);
    common::SourceLocationReference identifierCdStartRef(it);
    std::advance(it, 1);
    common::SourceLocationReference identifierCdEndRef(it);
    expectedTokens.emplace_back(Token::Type::Identifier,
                                common::SourceRangeReference(identifierCdStartRef,
                                                             identifierCdEndRef));

    // Operator: /
    std::advance(it, 1);
    common::SourceLocationReference opDivRef(it);
    expectedTokens.emplace_back(Token::Type::OpDiv,
                                common::SourceRangeReference(opDivRef));

    // Literal: 23
    std::advance(it, 3);
    common::SourceLocationReference literal2StartRef(it);
    std::advance(it, 1);
    common::SourceLocationReference literal2EndRef(it);
    expectedTokens.emplace_back(Token::Type::Literal,
                                common::SourceRangeReference(literal2StartRef,
                                                             literal2EndRef));

    // Right parenthesis
    std::advance(it, 2);
    common::SourceLocationReference rightParenthesis2Ref(it);
    expectedTokens.emplace_back(Token::Type::RightParenthesis,
                                common::SourceRangeReference(rightParenthesis2Ref));

    // Right parenthesis
    std::advance(it, 1);
    common::SourceLocationReference rightParenthesis3Ref(it);
    expectedTokens.emplace_back(Token::Type::RightParenthesis,
                                common::SourceRangeReference(rightParenthesis3Ref));

    executeLexerTest(sourceCodeManager, expectedTokens);
}

TEST(TestLexer, PackedParamsWithWeirdWhiteSpacing) { // NOLINT
    std::string code{"x,y,   u,     v\n\n\t     "};
    common::SourceCodeManager sourceCodeManager(std::move(code));

    ExpectedTokens expectedTokens;
    auto it = sourceCodeManager.getCodeBegin();

    // Identifier: x
    common::SourceLocationReference identifierXStart(it);
    expectedTokens.emplace_back(Token::Type::Identifier,
                                common::SourceRangeReference(identifierXStart));

    // Comma
    std::advance(it, 1);
    common::SourceLocationReference commaRef1(it);
    expectedTokens.emplace_back(Token::Type::Comma,
                                common::SourceRangeReference(commaRef1));

    // Identifier: y
    std::advance(it, 1);
    common::SourceLocationReference identifierYStart(it);
    expectedTokens.emplace_back(Token::Type::Identifier,
                                common::SourceRangeReference(identifierYStart));

    // Comma
    std::advance(it, 1);
    common::SourceLocationReference commaRef2(it);
    expectedTokens.emplace_back(Token::Type::Comma,
                                common::SourceRangeReference(commaRef2));

    // Identifier: u
    std::advance(it, 4);
    common::SourceLocationReference identifierUStart(it);
    expectedTokens.emplace_back(Token::Type::Identifier,
                                common::SourceRangeReference(identifierUStart));

    // Comma
    std::advance(it, 1);
    common::SourceLocationReference commaRef3(it);
    expectedTokens.emplace_back(Token::Type::Comma,
                                common::SourceRangeReference(commaRef3));

    // Identifier: v
    std::advance(it, 6);
    common::SourceLocationReference identifierVStart(it);
    expectedTokens.emplace_back(Token::Type::Identifier,
                                common::SourceRangeReference(identifierVStart));

    executeLexerTest(sourceCodeManager, expectedTokens);
}

TEST(TestLexer, PackedAssignment) { // NOLINT
    std::string code{"\tr:=ab-22;\n"};
    common::SourceCodeManager sourceCodeManager(std::move(code));

    ExpectedTokens expectedTokens;
    auto it = sourceCodeManager.getCodeBegin();

    // Identifier: r
    std::advance(it, 1);
    common::SourceLocationReference identifierRRef(it);
    expectedTokens.emplace_back(Token::Type::Identifier,
                                common::SourceRangeReference(identifierRRef));

    // Assignment: :=
    std::advance(it, 1);
    common::SourceLocationReference assignmentRefStart(it);
    std::advance(it, 1);
    common::SourceLocationReference assignmentRefEnd(it);
    expectedTokens.emplace_back(Token::Type::Assignment,
                                common::SourceRangeReference(assignmentRefStart,
                                                             assignmentRefEnd));

    // ab
    std::advance(it, 1);
    common::SourceLocationReference identifierR2RefStart(it);
    std::advance(it, 1);
    common::SourceLocationReference identifierR2RefEnd(it);
    expectedTokens.emplace_back(Token::Type::Identifier,
                                common::SourceRangeReference(identifierR2RefStart,
                                                             identifierR2RefEnd));

    // Operator: -
    std::advance(it, 1);
    common::SourceLocationReference opMinusRef(it);
    expectedTokens.emplace_back(Token::Type::OpMinus,
                                common::SourceRangeReference(opMinusRef));

    // Literal: 22
    std::advance(it, 1);
    common::SourceLocationReference literalStartRef(it);
    std::advance(it, 1);
    common::SourceLocationReference literalEndRef(it);
    expectedTokens.emplace_back(Token::Type::Literal,
                                common::SourceRangeReference(literalStartRef,
                                                             literalEndRef));

    // Semi colon
    std::advance(it, 1);
    common::SourceLocationReference semiColonRef(it);
    expectedTokens.emplace_back(Token::Type::SemiColon,
                                common::SourceRangeReference(semiColonRef));

    executeLexerTest(sourceCodeManager, expectedTokens);
}

TEST(TestLexer, SignedLiteralTest) { // NOLINT
    std::string code{"-123 - +123"};
    common::SourceCodeManager sourceCodeManager(std::move(code));

    ExpectedTokens expectedTokens;
    auto it = sourceCodeManager.getCodeBegin();

    // Operator: -
    common::SourceLocationReference opMinusRef1(it);
    expectedTokens.emplace_back(Token::Type::OpMinus,
                                common::SourceRangeReference(opMinusRef1));

    // Literal: 123
    std::advance(it, 1);
    common::SourceLocationReference literal1StartRef(it);
    std::advance(it, 2);
    common::SourceLocationReference literal1EndRef(it);
    expectedTokens.emplace_back(Token::Type::Literal,
                                common::SourceRangeReference(literal1StartRef,
                                                             literal1EndRef));

    // Operator: -
    std::advance(it, 2);
    common::SourceLocationReference opMinusRef2(it);
    expectedTokens.emplace_back(Token::Type::OpMinus,
                                common::SourceRangeReference(opMinusRef2));

    // Operator: +
    std::advance(it, 2);
    common::SourceLocationReference opPlusRef(it);
    expectedTokens.emplace_back(Token::Type::OpPlus,
                                common::SourceRangeReference(opPlusRef));

    // Literal: 123
    std::advance(it, 1);
    common::SourceLocationReference literal2StartRef(it);
    std::advance(it, 2);
    common::SourceLocationReference literal2EndRef(it);
    expectedTokens.emplace_back(Token::Type::Literal,
                                common::SourceRangeReference(literal2StartRef,
                                                             literal2EndRef));

    executeLexerTest(sourceCodeManager, expectedTokens);
}

TEST(TestLexer, PlusEquals) { // NOLINT
    std::string code{"+="};
    common::SourceCodeManager sourceCodeManager(std::move(code));

    ExpectedTokens expectedTokens;
    auto it = sourceCodeManager.getCodeBegin();

    // Operator: +
    common::SourceLocationReference opPlusRef(it);
    expectedTokens.emplace_back(Token::Type::OpPlus,
                                common::SourceRangeReference(opPlusRef));

    // Init: =
    std::advance(it, 1);
    common::SourceLocationReference initRef(it);
    expectedTokens.emplace_back(Token::Type::Init,
                                common::SourceRangeReference(initRef));

    executeLexerTest(sourceCodeManager, expectedTokens);
}

TEST(TestLexer, MixNumericAndAlphaNumericWithNoSpaces) { // NOLINT
    std::string code{"abc1234RETURN5678"};
    common::SourceCodeManager sourceCodeManager(std::move(code));

    ExpectedTokens expectedTokens;
    auto it = sourceCodeManager.getCodeBegin();

    // identifier = abc
    common::SourceLocationReference identifierRefStart(it);
    std::advance(it, 2);
    common::SourceLocationReference identifierRefEnd(it);
    expectedTokens.emplace_back(Token::Type::Identifier,
                                common::SourceRangeReference(identifierRefStart,
                                                             identifierRefEnd));

    // literal = 1234
    std::advance(it, 1);
    common::SourceLocationReference literalRefStart1(it);
    std::advance(it, 3);
    common::SourceLocationReference literalRefEnd1(it);
    expectedTokens.emplace_back(Token::Type::Literal,
                                common::SourceRangeReference(literalRefStart1,
                                                             literalRefEnd1));

    // keyword = RETURN
    std::advance(it, 1);
    common::SourceLocationReference returnRefStart(it);
    std::advance(it, 5);
    common::SourceLocationReference returnRefEnd(it);
    expectedTokens.emplace_back(Token::Type::Return,
                                common::SourceRangeReference(returnRefStart,
                                                             returnRefEnd));

    // literal = 5678
    std::advance(it, 1);
    common::SourceLocationReference literalRefStart2(it);
    std::advance(it, 3);
    common::SourceLocationReference literalRefEnd2(it);
    expectedTokens.emplace_back(Token::Type::Literal,
                                common::SourceRangeReference(literalRefStart2,
                                                             literalRefEnd2));

    executeLexerTest(sourceCodeManager, expectedTokens);
}

TEST(TestLexer, IdentifierTest) { // NOLINT
    std::string code{"PARAMa"};
    common::SourceCodeManager sourceCodeManager(std::move(code));

    ExpectedTokens expectedTokens;
    auto it = sourceCodeManager.getCodeBegin();

    common::SourceLocationReference identifierStart(it);
    std::advance(it, 5);
    common::SourceLocationReference identifierEnd(it);
    expectedTokens.emplace_back(Token::Type::Identifier,
                                common::SourceRangeReference(identifierStart,
                                                             identifierEnd));

    executeLexerTest(sourceCodeManager, expectedTokens);
}

TEST(TestLexer, Peek) { // NOLINT
    std::string code{"a 1"};
    common::SourceCodeManager sourceCodeManager(std::move(code));
    Lexer lexer(sourceCodeManager);

    auto it = sourceCodeManager.getCodeBegin();

    common::SourceLocationReference identifierRef(it);
    Token expectedToken1(Token::Type::Identifier,
                         common::SourceRangeReference(identifierRef));

    ASSERT_EQ(lexer.peek(), expectedToken1);
    ASSERT_TRUE(lexer.hasNext());
    ASSERT_EQ(lexer.peek(), expectedToken1);
    ASSERT_TRUE(lexer.hasNext());
    ASSERT_EQ(lexer.next(), expectedToken1);

    std::advance(it, 2);
    common::SourceLocationReference literalRef(it);
    Token expectedToken2(Token::Type::Literal,
                         common::SourceRangeReference(literalRef));

    ASSERT_TRUE(lexer.hasNext());
    ASSERT_EQ(lexer.peek(), expectedToken2);
    ASSERT_TRUE(lexer.hasNext());
    ASSERT_EQ(lexer.peek(), expectedToken2);
    ASSERT_TRUE(lexer.hasNext());
    ASSERT_EQ(lexer.next(), expectedToken2);

    ASSERT_FALSE(lexer.hasNext());
}

TEST(TestLexerErrors, IllegalMulticharacterOperator) { // NOLINT
    std::string code{":+="};
    common::SourceCodeManager sourceCodeManager(std::move(code));

    executeLexerErrorTest(sourceCodeManager, "1:1: error: unknown multi-character token\n"
                                             ":+=\n"
                                             "^~\n");
}

TEST(TestLexerErrors, IllegalCharacter) { // NOLINT
    std::string code{"?"};
    common::SourceCodeManager sourceCodeManager(std::move(code));

    executeLexerErrorTest(sourceCodeManager, "1:1: error: illegal character\n"
                                             "?\n"
                                             "^\n");
}

TEST(TestLexerErrors, IllegalCharacter2) { // NOLINT
    std::string code{"abcd$"};
    common::SourceCodeManager sourceCodeManager(std::move(code));

    executeLexerErrorTest(sourceCodeManager, "1:5: error: illegal character\n"
                                             "abcd$\n"
                                             "    ^\n");
}

TEST(TestLexerErrors, IllegalCharacter3) { // NOLINT
    std::string code{"\n"
                     "\n"
                     "\n"
                     "       _abcd"};
    common::SourceCodeManager sourceCodeManager(std::move(code));

    executeLexerErrorTest(sourceCodeManager, "4:8: error: illegal character\n"
                                             "       _abcd\n"
                                             "       ^\n");
}

} // namespace pljit::lexer
