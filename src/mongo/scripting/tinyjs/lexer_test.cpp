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
namespace tinyjs {
using std::string;

// This function takes in a line written in a subset of Javascript, runs the lexer on it,
// checks that the lexer output has expectedLength tokens,
// and compares the types and lexemes in the lexer output to the input arrays "types" and "lexemes"
void testLine(string inputString, TokenType *types, string *lexemes, size_t expectedLength) {

    std::vector<Token> TokenData = lex(inputString);

    ASSERT(TokenData.size() == expectedLength);

    for (int i = 0; i < static_cast<int>(expectedLength); i++) {
        ASSERT(TokenData[i].type == types[i]);
        ASSERT(TokenData[i].value == StringData(lexemes[i]));
    }
}

// This function takes a string containing one lexeme and its token type
// and tests the lexer's output on the input string.
void testSingleToken(string inputString, TokenType t) {
    TokenType types[] = {t};
    string lexemes[] = {inputString};
    testLine(inputString, types, lexemes, 1);
}

// Simple tests for each type of token, where input contains just one token

TEST(LexerTest, kThisIdentifier) {
    testSingleToken("this", TokenType::kThisIdentifier);
}

TEST(LexerTest, kReturnIdentifier) {
    testSingleToken("return", TokenType::kReturnIdentifier);
}

TEST(LexerTest, kNullIdentifier) {
    testSingleToken("null", TokenType::kNullIdentifier);
}

TEST(LexerTest, kUndefinedIdentifier) {
    testSingleToken("undefined", TokenType::kUndefinedIdentifier);
}

TEST(LexerTest, kIntegerLiteral) {
    testSingleToken("012345", TokenType::kIntegerLiteral);
}

TEST(LexerTest, kFloatLiteralGt1) {
    testSingleToken("123.45", TokenType::kFloatLiteral);
}

TEST(LexerTest, kFloatLiteralLt1) {
    testSingleToken(".45", TokenType::kFloatLiteral);
}

TEST(LexerTest, kFloatLiteralNaN) {
    testSingleToken("NaN", TokenType::kFloatLiteral);
}

TEST(LexerTest, booleanTrue) {
    testSingleToken("true", TokenType::kBooleanLiteral);
}

TEST(LexerTest, booleanFalse) {
    testSingleToken("false", TokenType::kBooleanLiteral);
}

TEST(LexerTest, kStringLiteral) {
    testSingleToken("'hello world'", TokenType::kStringLiteral);
}

TEST(LexerTest, kIdentifier) {
    testSingleToken("name", TokenType::kIdentifier);
}

TEST(LexerTest, kMultiply) {
    testSingleToken("*", TokenType::kMultiply);
}

TEST(LexerTest, kDivide) {
    testSingleToken("\\", TokenType::kDivide);
}

TEST(LexerTest, kAdd) {
    testSingleToken("+", TokenType::kAdd);
}

TEST(LexerTest, kSubtract) {
    testSingleToken("-", TokenType::kSubtract);
}

TEST(LexerTest, kTripleEquals) {
    testSingleToken("===", TokenType::kTripleEquals);
}

TEST(LexerTest, kDoubleEquals) {
    testSingleToken("==", TokenType::kDoubleEquals);
}

TEST(LexerTest, kLessThan) {
    testSingleToken("<", TokenType::kLessThan);
}

TEST(LexerTest, kLessThanEquals) {
    testSingleToken("<=", TokenType::kLessThanEquals);
}

TEST(LexerTest, kGreaterThan) {
    testSingleToken(">", TokenType::kGreaterThan);
}

TEST(LexerTest, kGreaterThanEquals) {
    testSingleToken(">=", TokenType::kGreaterThanEquals);
}

TEST(LexerTest, kNotEquals) {
    testSingleToken("!=", TokenType::kNotEquals);
}

TEST(LexerTest, kDoubleNotEquals) {
    testSingleToken("!==", TokenType::kDoubleNotEquals);
}

TEST(LexerTest, kLogicalAnd) {
    testSingleToken("&&", TokenType::kLogicalAnd);
}

TEST(LexerTest, kLogicalOr) {
    testSingleToken("||", TokenType::kLogicalOr);
}

TEST(LexerTest, kLogicalNot) {
    testSingleToken("!", TokenType::kLogicalNot);
}

TEST(LexerTest, kSemiColon) {
    testSingleToken(";", TokenType::kSemiColon);
}

TEST(LexerTest, kOpenParen) {
    testSingleToken("(", TokenType::kOpenParen);
}

TEST(LexerTest, kCloseParen) {
    testSingleToken(")", TokenType::kCloseParen);
}

TEST(LexerTest, kQuestionMark) {
    testSingleToken("?", TokenType::kQuestionMark);
}

TEST(LexerTest, kColon) {
    testSingleToken(":", TokenType::kColon);
}

TEST(LexerTest, kPeriod) {
    testSingleToken(".", TokenType::kPeriod);
}

TEST(LexerTest, kOpenSquareBracket) {
    testSingleToken("[", TokenType::kOpenSquareBracket);
}

TEST(LexerTest, kCloseSquareBracket) {
    testSingleToken("]", TokenType::kCloseSquareBracket);
}

TEST(LexerTest, kFunctionDec) {
    testSingleToken("function()", TokenType::kFunctionDec);
}

TEST(LexerTest, kOpenCurly) {
    testSingleToken("{", TokenType::kOpenCurly);
}

TEST(LexerTest, kCloseCurly) {
    testSingleToken("}", TokenType::kCloseCurly);
}

TEST(LexerTest, functionSimpleComparison) {
    string input = "function() {return 1 == true;}";

    TokenType types[] = {TokenType::kFunctionDec,
                         TokenType::kOpenCurly,
                         TokenType::kReturnIdentifier,
                         TokenType::kIntegerLiteral,
                         TokenType::kDoubleEquals,
                         TokenType::kBooleanLiteral,
                         TokenType::kSemiColon,
                         TokenType::kCloseCurly};

    string lexemes[] = {"function()", "{", "return", "1", "==", "true", ";", "}"};

    testLine(input, types, lexemes, 8);
}

TEST(LexerTest, functionNestedObjArrayComparison) {
    string input = "function() {return (this.a.b.c.d >= thisArray[0]);}";

    TokenType types[] = {TokenType::kFunctionDec,
                         TokenType::kOpenCurly,
                         TokenType::kReturnIdentifier,
                         TokenType::kOpenParen,
                         TokenType::kThisIdentifier,
                         TokenType::kPeriod,
                         TokenType::kIdentifier,
                         TokenType::kPeriod,
                         TokenType::kIdentifier,
                         TokenType::kPeriod,
                         TokenType::kIdentifier,
                         TokenType::kPeriod,
                         TokenType::kIdentifier,
                         TokenType::kGreaterThanEquals,
                         TokenType::kIdentifier,
                         TokenType::kOpenSquareBracket,
                         TokenType::kIntegerLiteral,
                         TokenType::kCloseSquareBracket,
                         TokenType::kCloseParen,
                         TokenType::kSemiColon,
                         TokenType::kCloseCurly};

    string lexemes[] = {"function()", "{", "return", "(", "this", ".", "a",
                        ".",          "b", ".",      "c", ".",    "d", ">=",
                        "thisArray",  "[", "0",      "]", ")",    ";", "}"};

    testLine(input, types, lexemes, 21);
}

TEST(LexerTest, functionTernaryOp) {
    string input = "function() {return this.a['foo'] == 3 ? (this.b > 1) : (this.d == 2)}";

    TokenType types[] = {TokenType::kFunctionDec,
                         TokenType::kOpenCurly,
                         TokenType::kReturnIdentifier,
                         TokenType::kThisIdentifier,
                         TokenType::kPeriod,
                         TokenType::kIdentifier,
                         TokenType::kOpenSquareBracket,
                         TokenType::kStringLiteral,
                         TokenType::kCloseSquareBracket,
                         TokenType::kDoubleEquals,
                         TokenType::kIntegerLiteral,
                         TokenType::kQuestionMark,
                         TokenType::kOpenParen,
                         TokenType::kThisIdentifier,
                         TokenType::kPeriod,
                         TokenType::kIdentifier,
                         TokenType::kGreaterThan,
                         TokenType::kIntegerLiteral,
                         TokenType::kCloseParen,
                         TokenType::kColon,
                         TokenType::kOpenParen,
                         TokenType::kThisIdentifier,
                         TokenType::kPeriod,
                         TokenType::kIdentifier,
                         TokenType::kDoubleEquals,
                         TokenType::kIntegerLiteral,
                         TokenType::kCloseParen,
                         TokenType::kCloseCurly};

    string lexemes[] = {"function()", "{",    "return", "this", ".",  "a", "[", "'foo'", "]", "==",
                        "3",          "?",    "(",      "this", ".",  "b", ">", "1",     ")", ":",
                        "(",          "this", ".",      "d",    "==", "2", ")", "}"};

    testLine(input, types, lexemes, 28);
}

TEST(LexerTest, nestedNonsense) {
    string input = "return ((((3 + a) - 'string') * NaN) \\ x)+ -(bar['a'].this.null[1])";

    TokenType types[] = {TokenType::kReturnIdentifier,
                         TokenType::kOpenParen,
                         TokenType::kOpenParen,
                         TokenType::kOpenParen,
                         TokenType::kOpenParen,
                         TokenType::kIntegerLiteral,
                         TokenType::kAdd,
                         TokenType::kIdentifier,
                         TokenType::kCloseParen,
                         TokenType::kSubtract,
                         TokenType::kStringLiteral,
                         TokenType::kCloseParen,
                         TokenType::kMultiply,
                         TokenType::kFloatLiteral,
                         TokenType::kCloseParen,
                         TokenType::kDivide,
                         TokenType::kIdentifier,
                         TokenType::kCloseParen,
                         TokenType::kAdd,
                         TokenType::kSubtract,
                         TokenType::kOpenParen,
                         TokenType::kIdentifier,
                         TokenType::kOpenSquareBracket,
                         TokenType::kStringLiteral,
                         TokenType::kCloseSquareBracket,
                         TokenType::kPeriod,
                         TokenType::kThisIdentifier,
                         TokenType::kPeriod,
                         TokenType::kNullIdentifier,
                         TokenType::kOpenSquareBracket,
                         TokenType::kIntegerLiteral,
                         TokenType::kCloseSquareBracket,
                         TokenType::kCloseParen};

    string lexemes[] = {"return", "(",   "(",   "(", "(",    "3", "+",    "a", ")", "-", "'string'",
                        ")",      "*",   "NaN", ")", "\\",   "x", ")",    "+", "-", "(", "bar",
                        "[",      "'a'", "]",   ".", "this", ".", "null", "[", "1", "]", ")"};

    testLine(input, types, lexemes, 33);
}

TEST(LexerTest, integerAndkIdentifier) {
    string input = "1variableName";

    TokenType types[] = {TokenType::kIntegerLiteral, TokenType::kIdentifier};

    string lexemes[] = {"1", "variableName"};

    testLine(input, types, lexemes, 2);
}

TEST(LexerTest, weirdSpacing) {
    string input = ".-)(     [>null:]";

    TokenType types[] = {TokenType::kPeriod,
                         TokenType::kSubtract,
                         TokenType::kCloseParen,
                         TokenType::kOpenParen,
                         TokenType::kOpenSquareBracket,
                         TokenType::kGreaterThan,
                         TokenType::kNullIdentifier,
                         TokenType::kColon,
                         TokenType::kCloseSquareBracket};

    string lexemes[] = {".", "-", ")", "(", "[", ">", "null", ":", "]"};

    testLine(input, types, lexemes, 9);
}

TEST(LexerTest, specialWordkIdentifier) {
    string input = "thisisnotafunction()";

    TokenType types[] = {TokenType::kIdentifier, TokenType::kOpenParen, TokenType::kCloseParen};

    string lexemes[] = {"thisisnotafunction", "(", ")"};

    testLine(input, types, lexemes, 3);
}
}  // namespace tinyjs
}  // namespace mongo