/**
 * Copyright (C) 2015 MongoDB Inc.
 *
 * This program is free software: you can redistribute it and/or  modify
 * it under the terms of the GNU Affero General Public License, version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, the copyright holders give permission to link the
 * code of portions of this program with the OpenSSL library under certain
 * conditions as described in each individual source file and distribute
 * linked combinations including the program with the OpenSSL library. You
 * must comply with the GNU Affero General Public License in all respects
 * for all of the code used other than as permitted herein. If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so. If you do not
 * wish to do so, delete this exception statement from your version. If you
 * delete this exception statement from all source files in the program,
 * then also delete it in the license file.
 */

#include "mongo/platform/basic.h"
#include "lexer.h"
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
#include "mongo/base/string_data.h"
#include "mongo/unittest/unittest.h"

namespace mongo {

    using std::string;

    // This function creates a char * from a string
    char *charArray(string s) {
        string input(s);
        char *writable = new char[input.size() + 1];
        std::copy(input.begin(), input.end(), writable);
        writable[input.size()] = '\0';  // terminating 0
        return writable;
    }

    // Simple tests for each type of token, where input contains just one token

    // This function takes in a line written in a subset of Javascript, runs the lexer on it,
    // checks that the lexer output has expectedLength tokens, 
    // and compares the types and lexemes in the lexer output to the input arrays "types" and "lexemes"
    void testLine(string inputString, tokenType *types, string *lexemes, size_t expectedLength) {
        char *input = charArray(inputString);
        std::vector<token> tokenData = lex(input);

        ASSERT(tokenData.size() == expectedLength);

        for (int i = 0; i < static_cast<int>(expectedLength); i++) {
            ASSERT(tokenData[i].type == types[i]);
            ASSERT(tokenData[i].value == StringData(lexemes[i]));
        }

        delete[] input;
    }

    // This function takes a string containing one lexeme and its token type
    // and tests the lexer's output on the input string.
    void testSingleToken(string inputString, tokenType t) {
        tokenType types[] = {t};
        string lexemes[] = {inputString};
        testLine(inputString, types, lexemes, 1);
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

        string lexemes[] = {"function()", "{", "return", "1", "==", "true", ";", "}"};

        testLine(input, types, lexemes, 8);
    }

    TEST(LexerTest, functionNestedObjArrayComparison) {
        string input = "function() {return (this.a.b.c.d >= thisArray[0]);}";

        tokenType types[] = {functionDec,   openCurly,      returnToken,    openParen,    thisToken,
                             period,        identifier,     period,         identifier,   period,
                             identifier,    period,         identifier,     comparisonOp, identifier,
                             openSqBracket, integerLiteral, closeSqBracket, closeParen,   semicolon,
                             closeCurly};

        string lexemes[] = {"function()", "{", "return", "(", "this", ".", "a",
                            ".",          "b", ".",      "c", ".",    "d", ">=",
                            "thisArray",  "[", "0",      "]", ")",    ";", "}"};

        testLine(input, types, lexemes, 21);
    }

    TEST(LexerTest, functionTernaryOp) {
        string input = "function() {return this.a['foo'] == 3 ? (this.b > 1) : (this.d == 2)}";

        tokenType types[] = {
            functionDec,   openCurly,      returnToken,    thisToken,    period,         identifier,
            openSqBracket, stringLiteral,  closeSqBracket, comparisonOp, integerLiteral, questionMark,
            openParen,     thisToken,      period,         identifier,   comparisonOp,   integerLiteral,
            closeParen,    colon,          openParen,      thisToken,    period,         identifier,
            comparisonOp,  integerLiteral, closeParen,     closeCurly};

        string lexemes[] = {"function()", "{",    "return", "this", ".",  "a", "[", "'foo'", "]", "==",
                            "3",          "?",    "(",      "this", ".",  "b", ">", "1",     ")", ":",
                            "(",          "this", ".",      "d",    "==", "2", ")", "}"};

        testLine(input, types, lexemes, 28);
    }

}  // namespace mongo