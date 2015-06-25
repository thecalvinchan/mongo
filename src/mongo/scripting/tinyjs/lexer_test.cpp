#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>
#include "mongo/base/string_data.h"
#include "lexer.h"
#include "mongo/unittest/unittest.h"

namespace mongo {

using std::string;

/** Helper function to create a char * from a string */
char *charArray(string s) {
    string input(s);
    char *writable = new char[input.size() + 1];
    std::copy(input.begin(), input.end(), writable);
    writable[input.size()] = '\0';  // terminating 0
    return writable;
}

// Simple tests for each type of token, where input contains just one token

/** Helper function to perform a test for a single line */
void testLine(string inputString, tokenType *types, string *strings, int expectedLength) {
    char *input = charArray(inputString);
    std::vector<token> token_data = lex(input);

    for (int i = 0; i < expectedLength; i++) {
        ASSERT(token_data[i].type == types[i]);
        ASSERT(token_data[i].value == StringData(strings[i]));
    }

    delete[] input;
}

void testSingleToken(string inputString, tokenType t) {
    tokenType types[] = {t};
    string strings[] = {inputString};
    testLine(inputString, types, strings, 1);
}

TEST(LexerTest, thisToken) {
    testSingleToken("this", thisToken);
}

TEST(LexerTest, returnToken) {
    testSingleToken("return", returnToken);
}

TEST(LexerTest, integerLiteral) {
    testSingleToken("012345", integerLiteral);
}

TEST(LexerTest, floatLiteralGt1) {
    testSingleToken("123.45", floatLiteral);
}

TEST(LexerTest, floatLiteralLt1) {
    testSingleToken(".45", floatLiteral);
}

TEST(LexerTest, booleanTrue) {
    testSingleToken("true", booleanLiteral);
}

TEST(LexerTest, booleanFalse) {
    testSingleToken("false", booleanLiteral);
}

TEST(LexerTest, stringLiteral) {
    testSingleToken("'hello world'", stringLiteral);
}

TEST(LexerTest, identifier) {
    testSingleToken("name", identifier);
}

TEST(LexerTest, multiplicativeOpTimes) {
    testSingleToken("*", multiplicativeOp);
}

TEST(LexerTest, multiplicativeOpDivide) {
    testSingleToken("\\", multiplicativeOp);
}

TEST(LexerTest, additiveOpPlus) {
    testSingleToken("+", additiveOp);
}

TEST(LexerTest, additiveOpMinus) {
    testSingleToken("-", additiveOp);
}

TEST(LexerTest, comparisonOpTripleEquals) {
    testSingleToken("===", comparisonOp);
}

TEST(LexerTest, comparisonOpDoubleEquals) {
    testSingleToken("==", comparisonOp);
}

TEST(LexerTest, comparisonOpLt) {
    testSingleToken("<", comparisonOp);
}

TEST(LexerTest, comparisonOpLte) {
    testSingleToken("<=", comparisonOp);
}

TEST(LexerTest, comparisonOpGt) {
    testSingleToken(">", comparisonOp);
}

TEST(LexerTest, comparisonOpGte) {
    testSingleToken(">=", comparisonOp);
}

TEST(LexerTest, comparisonOpNeq) {
    testSingleToken("!=", comparisonOp);
}

TEST(LexerTest, comparisonOpDoubleNeq) {
    testSingleToken("!==", comparisonOp);
}

TEST(LexerTest, logicalOpAnd) {
    testSingleToken("&&", logicalOp);
}

TEST(LexerTest, logicalOpOr) {
    testSingleToken("||", logicalOp);
}

TEST(LexerTest, logicalOpNot) {
    testSingleToken("!", logicalOp);
}

TEST(LexerTest, semicolon) {
    testSingleToken(";", semicolon);
}

TEST(LexerTest, openParen) {
    testSingleToken("(", openParen);
}

TEST(LexerTest, closeParen) {
    testSingleToken(")", closeParen);
}

TEST(LexerTest, questionMark) {
    testSingleToken("?", questionMark);
}

TEST(LexerTest, colon) {
    testSingleToken(":", colon);
}

TEST(LexerTest, period) {
    testSingleToken(".", period);
}

TEST(LexerTest, openSqBracket) {
    testSingleToken("[", openSqBracket);
}

TEST(LexerTest, closeSqBracket) {
    testSingleToken("]", closeSqBracket);
}

TEST(LexerTest, functionDec) {
    testSingleToken("function()", functionDec);
}

TEST(LexerTest, openCurly) {
    testSingleToken("{", openCurly);
}

TEST(LexerTest, closeCurly) {
    testSingleToken("}", closeCurly);
}

TEST(LexerTest, functionSimpleComparison) {
    string input = "function() {return 1 == true;}";

    tokenType types[] = {functionDec,
                         openCurly,
                         returnToken,
                         integerLiteral,
                         comparisonOp,
                         booleanLiteral,
                         semicolon,
                         closeCurly};

    string strings[] = {"function()", "{", "return", "1", "==", "true", ";", "}"};

    testLine(input, types, strings, 7);
}

TEST(LexerTest, functionNestedObjArrayComparison) {
    string input = "function() {return (this.a.b.c.d >= thisArray[0]);}";

    tokenType types[] = {functionDec,   openCurly,      returnToken,    openParen,    thisToken,
                         period,        identifier,     period,         identifier,   period,
                         identifier,    period,         identifier,     comparisonOp, identifier,
                         openSqBracket, integerLiteral, closeSqBracket, closeParen,   semicolon,
                         closeCurly};

    string strings[] = {"function()", "{", "return", "(", "this", ".", "a",
                        ".",          "b", ".",      "c", ".",    "d", ">=",
                        "thisArray",  "[", "0",      "]", ")",    ";", "}"};

    testLine(input, types, strings, 21);
}

TEST(LexerTest, functionTernaryOp) {
    string input = "function() {return this.a['foo'] == 3 ? (this.b > 1) : (this.d == 2)}";

    tokenType types[] = {
        functionDec,   openCurly,      returnToken,    thisToken,    period,         identifier,
        openSqBracket, stringLiteral,  closeSqBracket, comparisonOp, integerLiteral, questionMark,
        openParen,     thisToken,      period,         identifier,   comparisonOp,   integerLiteral,
        closeParen,    colon,          openParen,      thisToken,    period,         identifier,
        comparisonOp,  integerLiteral, closeParen,     closeCurly};

    string strings[] = {"function()", "{",    "return", "this", ".",  "a", "[", "'foo'", "]", "==",
                        "3",          "?",    "(",      "this", ".",  "b", ">", "1",     ")", ":",
                        "(",          "this", ".",      "d",    "==", "2", ")", "}"};

    testLine(input, types, strings, 28);
}

}  // end namespace