#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>
#include "mongo/base/string_data.h"
#include "lexer.h"
#include "mongo/unittest/unittest.h"

/** Helper function to create a char * from a string */
char *charArray(std::string s){
    std::string input (s);
    char * writable = new char[input.size() + 1];
    std::copy(input.begin(), input.end(), writable);
    writable[input.size()] = '\0'; // terminating 0
    return writable;
}

// Simple tests for each type of token, where input contains just one token

/** Helper function to perform a test for a single token */
void testSingleToken(std::string s, tokenType t) {

    char *input = charArray(s);
    std::vector<token> token_data = lex(input);

    ASSERT(token_data[0].type == t);
    ASSERT(token_data[0].value == mongo::StringData(input));

    delete[] input;
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

TEST(LexerTest, endStatement) {
    testSingleToken(";", endStatement);
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