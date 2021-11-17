#include "pljit/common/SourceCodeManager.h"
#include "pljit/parse_tree/ParseTree.h"
#include "pljit/parser/Parser.h"
#include "test/utils/TestUtils.h"
#include <gtest/gtest.h>

namespace pljit::parser {

namespace {

void executeErrorTest(std::string_view code, std::string_view expectedCout) {
    test_utils::CaptureCout cout;

    common::SourceCodeManager sourceCodeManager(std::string{code});
    Parser parser(sourceCodeManager);
    auto parseResult = parser.parseFunctionDefinition();

    ASSERT_TRUE(parseResult == nullptr);
    ASSERT_EQ(cout.stream.str(), expectedCout);
}

} // namespace

TEST(TestParserErrors, OnlyParameterDeclaration) { // NOLINT
    std::string_view code{"PARAM a, b;"};
    std::string_view expectedCout{"1:11: error: expected afterwards either 'VAR', 'CONST', or 'BEGIN'\n"
                                  "PARAM a, b;\n"
                                  "          ^\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, OnlyVariableDeclaration) { // NOLINT
    std::string_view code{"VAR a, b;"};
    std::string_view expectedCout{"1:9: error: expected afterwards either 'CONST' or 'BEGIN'\n"
                                  "VAR a, b;\n"
                                  "        ^\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, OnlyConstantDeclaration) { // NOLINT
    std::string_view code{"CONST A = 10;"};
    std::string_view expectedCout{"1:13: error: expected afterwards 'BEGIN'\n"
                                  "CONST A = 10;\n"
                                  "            ^\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, InvalidParameterDeclaration1) { // NOLINT
    std::string_view code{"PARAM a"};
    std::string_view expectedCout{"1:7: error: expected ';' afterwards\n"
                                  "PARAM a\n"
                                  "      ^\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, InvalidParameterDeclaration2) { // NOLINT
    std::string_view code{"PARAM a b;"};
    std::string_view expectedCout{"1:9: error: expected ';'\n"
                                  "PARAM a b;\n"
                                  "        ^\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, InvalidParameterDeclaration3) { // NOLINT
    std::string_view code{"PARAM a , ;"};
    std::string_view expectedCout{"1:11: error: expected identifier\n"
                                  "PARAM a , ;\n"
                                  "          ^\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, InvalidParameterDeclaration4) { // NOLINT
    std::string_view code{"PARAM a VAR ;"};
    std::string_view expectedCout{"1:9: error: expected ';'\n"
                                  "PARAM a VAR ;\n"
                                  "        ^~~\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, InvalidParameterDeclaration5) { // NOLINT
    std::string_view code{"PARAM ;"};
    std::string_view expectedCout{"1:7: error: expected identifier\n"
                                  "PARAM ;\n"
                                  "      ^\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, InvalidParameterDeclaration6) { // NOLINT
    std::string_view code{"PARAM"};
    std::string_view expectedCout{"1:5: error: expected identifier afterwards\n"
                                  "PARAM\n"
                                  "    ^\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, InvalidConstantDeclaration1) { // NOLINT
    std::string_view code{"CONST ;"};
    std::string_view expectedCout{"1:7: error: expected identifier\n"
                                  "CONST ;\n"
                                  "      ^\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, InvalidConstantDeclaration2) { // NOLINT
    std::string_view code{"CONST A = 10"};
    std::string_view expectedCout{"1:12: error: expected ';' afterwards\n"
                                  "CONST A = 10\n"
                                  "           ^\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, InvalidConstantDeclaration3) { // NOLINT
    std::string_view code{"CONST A  10;"};
    std::string_view expectedCout{"1:10: error: expected '='\n"
                                  "CONST A  10;\n"
                                  "         ^~\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, InvalidConstantDeclaration4) { // NOLINT
    std::string_view code{"CONST A = ;"};
    std::string_view expectedCout{"1:11: error: expected literal\n"
                                  "CONST A = ;\n"
                                  "          ^\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, InvalidConstantDeclaration5) { // NOLINT
    std::string_view code{"CONST A = "};
    std::string_view expectedCout{"1:9: error: expected literal afterwards\n"
                                  "CONST A = \n"
                                  "        ^\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, InvalidConstantDeclaration6) { // NOLINT
    std::string_view code{"CONST A "};
    std::string_view expectedCout{"1:7: error: expected '=' afterwards\n"
                                  "CONST A \n"
                                  "      ^\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, InvalidConstantDeclaration7) { // NOLINT
    std::string_view code{"CONST A 10"};
    std::string_view expectedCout{"1:9: error: expected '='\n"
                                  "CONST A 10\n"
                                  "        ^~\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, InvalidConstantDeclaration8) { // NOLINT
    std::string_view code{"CONST A = 10 ,"};
    std::string_view expectedCout{"1:14: error: expected identifier afterwards\n"
                                  "CONST A = 10 ,\n"
                                  "             ^\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, InvalidCompoundStatement1) { // NOLINT
    std::string_view code{"begin\n"
                          "RETURN 1\n"
                          "END."};
    std::string_view expectedCout{"1:1: error: expected 'BEGIN'\n"
                                  "begin\n"
                                  "^~~~~\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, InvalidCompoundStatement2) { // NOLINT
    std::string_view code{"BEGIN\n"
                          "RETURN 1\n"
                          "end."};
    std::string_view expectedCout{"3:1: error: expected 'END'\n"
                                  "end.\n"
                                  "^~~\n"
                                  "1:1: note: to match this 'BEGIN'\n"
                                  "BEGIN\n"
                                  "^~~~~\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, InvalidCompoundStatement3) { // NOLINT
    std::string_view code{"BEGIN\n"
                          "RETURN 1\n"
                          "."};
    std::string_view expectedCout{"3:1: error: expected 'END'\n"
                                  ".\n"
                                  "^\n"
                                  "1:1: note: to match this 'BEGIN'\n"
                                  "BEGIN\n"
                                  "^~~~~\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, InvalidCompoundStatement4) { // NOLINT
    std::string_view code{"BEGIN\n"
                          "END."};
    std::string_view expectedCout{"2:1: error: expected statement\n"
                                  "END.\n"
                                  "^~~\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, InvalidCompoundStatement5) { // NOLINT
    std::string_view code{"BEGIN\n"};
    std::string_view expectedCout{"1:5: error: expected statement afterwards\n"
                                  "BEGIN\n"
                                  "    ^\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, InvalidCompoundStatement6) { // NOLINT
    std::string_view code{"BEGIN\n"
                          "RETURN 1;\n"
                          "END."};
    std::string_view expectedCout{"3:1: error: expected statement\n"
                                  "END.\n"
                                  "^~~\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, InvalidCompoundStatement7) { // NOLINT
    std::string_view code{"BEGIN\n"
                          "RETURN 1\n"};
    std::string_view expectedCout{"2:8: error: expected 'END' afterwards\n"
                                  "RETURN 1\n"
                                  "       ^\n"
                                  "1:1: note: to match this 'BEGIN'\n"
                                  "BEGIN\n"
                                  "^~~~~\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, MissingProgramTerminator) { // NOLINT
    std::string_view code{"BEGIN\n"
                          "RETURN 1\n"
                          "END"};
    std::string_view expectedCout{"3:3: error: expected '.' afterwards\n"
                                  "END\n"
                                  "  ^\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, TokensAfterProgramTerminator) { // NOLINT
    std::string_view code{"BEGIN\n"
                          "RETURN 1\n"
                          "END. 1234"};
    std::string_view expectedCout{"3:6: error: expected no tokens after the program terminator\n"
                                  "END. 1234\n"
                                  "     ^~~~\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, InvalidAssignmentExpression1) { // NOLINT
    std::string_view code{"BEGIN\n"
                          "a RETURN b\n"
                          "END."};
    std::string_view expectedCout{"2:3: error: expected ':='\n"
                                  "a RETURN b\n"
                                  "  ^~~~~~\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, InvalidAssignmentExpression2) { // NOLINT
    std::string_view code{"BEGIN\n"
                          "a := ;\n"
                          "RETURN a\n"
                          "END."};
    std::string_view expectedCout{"2:6: error: expected primary-expression\n"
                                  "a := ;\n"
                                  "     ^\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, InvalidAssignmentExpression3) { // NOLINT
    std::string_view code{"BEGIN\n"
                          "a := "};
    std::string_view expectedCout{"2:4: error: expected unary-expression or primary-expression afterwards\n"
                                  "a := \n"
                                  "   ^\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, NotMatchingParenthesis) { // NOLINT
    std::string_view code{"BEGIN\n"
                          "c := -a * (b + d;\n"
                          "RETURN c\n"
                          "END."};
    std::string_view expectedCout{"2:17: error: expected ')'\n"
                                  "c := -a * (b + d;\n"
                                  "                ^\n"
                                  "2:11: note: to match this '('\n"
                                  "c := -a * (b + d;\n"
                                  "          ^\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, EmptyParenthesis) { // NOLINT
    std::string_view code{"BEGIN\n"
                          "c := ();\n"
                          "RETURN c\n"
                          "END."};
    std::string_view expectedCout{"2:7: error: expected primary-expression\n"
                                  "c := ();\n"
                                  "      ^\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, ReturnWithoutAnyExpression) { // NOLINT
    std::string_view code{"BEGIN\n"
                          "RETURN\n"
                          "END."};
    std::string_view expectedCout{"3:1: error: expected primary-expression\n"
                                  "END.\n"
                                  "^~~\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, TwoLiteralsInReturnStatement) { // NOLINT
    std::string_view code{"BEGIN\n"
                          "RETURN 12 34\n"
                          "END."};
    std::string_view expectedCout{"2:11: error: expected 'END'\n"
                                  "RETURN 12 34\n"
                                  "          ^~\n"
                                  "1:1: note: to match this 'BEGIN'\n"
                                  "BEGIN\n"
                                  "^~~~~\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, TwoLiteralsInAssignmentStatement) { // NOLINT
    std::string_view code{"BEGIN\n"
                          "x := 12 34;\n"
                          "RETURN x\n"
                          "END."};
    std::string_view expectedCout{"2:9: error: expected 'END'\n"
                                  "x := 12 34;\n"
                                  "        ^~\n"
                                  "1:1: note: to match this 'BEGIN'\n"
                                  "BEGIN\n"
                                  "^~~~~\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, MissingLiteralInExpression) { // NOLINT
    std::string_view code{"BEGIN\n"
                          "x := 12 + ;\n"
                          "RETURN x\n"
                          "END."};
    std::string_view expectedCout{"2:11: error: expected primary-expression\n"
                                  "x := 12 + ;\n"
                                  "          ^\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, NegativeLiteralAsConst) { // NOLINT
    std::string_view code{"CONST A = -1;\n"
                          "BEGIN\n"
                          "RETURN 1\n"
                          "END."};
    std::string_view expectedCout{"1:11: error: expected literal\n"
                                  "CONST A = -1;\n"
                                  "          ^\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, LexerErrorHandling) { // NOLINT
    std::string_view code{"CONST A != 1;"};
    std::string_view expectedCout{"1:9: error: illegal character\n"
                                  "CONST A != 1;\n"
                                  "        ^\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, LexerErrorHandling2) { // NOLINT
    std::string_view code{"PARAM a?;"};
    std::string_view expectedCout{"1:8: error: illegal character\n"
                                  "PARAM a?;\n"
                                  "       ^\n"};
    executeErrorTest(code, expectedCout);
}

TEST(TestParserErrors, LexerErrorHandling3) { // NOLINT
    std::string_view code{"CONST A = !;\n"};
    std::string_view expectedCout{"1:11: error: illegal character\n"
                                  "CONST A = !;\n"
                                  "          ^\n"};
    executeErrorTest(code, expectedCout);
}

} // namespace pljit::parser
