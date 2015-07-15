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

#include <iostream>
#include <cstdio>
#include <string>
#include <sstream>
#include <vector>

#include "mongo/base/string_data.h"
#include "mongo/unittest/unittest.h"
#include "mongo/scripting/tinyjs/lexer.h"

namespace mongo {
namespace tinyjs {

using std::string;

/* This function takes in a line written in a subset of Javascript, runs the lexer on it,
 * checks that the lexer output has expectedLength tokens, and compares
 * the types and lexemes in the lexer output to the input arrays "types" and "lexemes"
 */
void testValidLine(string input, TokenType *types, string *lexemes, std::size_t expectedLength) {
    std::vector<Token> tokenData = lex(input).getValue();

    ASSERT(tokenData.size() == expectedLength);

    for (std::size_t i = 0; i < expectedLength; i++) {
        ASSERT(tokenData[i].type == types[i]);
        ASSERT(tokenData[i].value == StringData(lexemes[i]));
    }
}

/* This function takes in a line written in a subset of Javascript, runs the lexer on it,
 * checks that the lexer output has expectedLength tokens, and compares
 * the types and lexemes in the lexer output to the input arrays "types" and "lexemes"
 */
void testInvalidLine(string input, int errorIndex, char badCharacter) {
    StatusWith<std::vector<Token>> result = lex(input);
    ASSERT(!result.isOK());

    std::stringstream errorMessage;
    errorMessage << "Could not parse input starting with character: " << badCharacter;
    errorMessage << std::endl;
    errorMessage << input;
    errorMessage << std::endl;
    errorMessage << std::string(errorIndex, ' ');
    errorMessage << "^" << std::endl;

    ASSERT(result.getStatus().reason() == errorMessage.str());
}

/* This function takes a string containing one lexeme and its token type
 * and tests the lexer's output on the input string.
 */
void testSingleToken(string inputString, TokenType t) {
    TokenType types[] = {t};
    string lexemes[] = {inputString};
    testValidLine(inputString, types, lexemes, 1);
}

// Simple tests for each type of token, where input contains just one token

TEST(LexerTest, thisAsIdentifier) {
    testSingleToken("this", TokenType::kIdentifier);
}

TEST(LexerTest, kReturnKeyword) {
    testSingleToken("return", TokenType::kReturnKeyword);
}

TEST(LexerTest, kNullLiteral) {
    testSingleToken("null", TokenType::kNullLiteral);
}

TEST(LexerTest, kUndefinedLiteral) {
    testSingleToken("undefined", TokenType::kUndefinedLiteral);
}

TEST(LexerTest, kIntegerLiteral) {
    testSingleToken("12345", TokenType::kIntegerLiteral);
}

TEST(LexerTest, kIntegerLiteral0) {
    testSingleToken("0", TokenType::kIntegerLiteral);
}

TEST(LexerTest, invalidIntegerLeadingZero) {
    testInvalidLine("012345", 0, '0');
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
    TokenType types[] = {TokenType::kStringLiteral};
    string lexemes[] = {"hello world"};
    testValidLine("'hello world'", types, lexemes, 1);
}

TEST(LexerTest, kIdentifier) {
    testSingleToken("name", TokenType::kIdentifier);
}

TEST(LexerTest, kMultiply) {
    testSingleToken("*", TokenType::kMultiply);
}

TEST(LexerTest, kDivide) {
    testSingleToken("/", TokenType::kDivide);
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

TEST(LexerTest, kComma) {
    testSingleToken(",", TokenType::kComma);
}

TEST(LexerTest, kOpenSquareBracket) {
    testSingleToken("[", TokenType::kOpenSquareBracket);
}

TEST(LexerTest, kCloseSquareBracket) {
    testSingleToken("]", TokenType::kCloseSquareBracket);
}

TEST(LexerTest, kFunctionKeyword) {
    testSingleToken("function", TokenType::kFunctionKeyword);
}

TEST(LexerTest, kOpenCurlyBrace) {
    testSingleToken("{", TokenType::kOpenCurlyBrace);
}

TEST(LexerTest, kCloseCurlyBrace) {
    testSingleToken("}", TokenType::kCloseCurlyBrace);
}

TEST(LexerTest, functionOnly) {
    string input = "function hello";

    TokenType types[] = {TokenType::kFunctionKeyword, TokenType::kIdentifier};

    string lexemes[] = {"function", "hello"};

    testValidLine(input, types, lexemes, 2);
}

TEST(LexerTest, functionSimpleComparison) {
    string input = "function() {return 1 == true;}";

    TokenType types[] = {TokenType::kFunctionKeyword,
                         TokenType::kOpenParen,
                         TokenType::kCloseParen,
                         TokenType::kOpenCurlyBrace,
                         TokenType::kReturnKeyword,
                         TokenType::kIntegerLiteral,
                         TokenType::kDoubleEquals,
                         TokenType::kBooleanLiteral,
                         TokenType::kSemiColon,
                         TokenType::kCloseCurlyBrace};

    string lexemes[] = {"function", "(", ")", "{", "return", "1", "==", "true", ";", "}"};

    testValidLine(input, types, lexemes, 10);
}

TEST(LexerTest, functionNestedObjArrayComparison) {
    string input = "function() {return (this.a.b.c.d >= thisArray";  //[0]);}";

    TokenType types[] = {TokenType::kFunctionKeyword,
                         TokenType::kOpenParen,
                         TokenType::kCloseParen,
                         TokenType::kOpenCurlyBrace,
                         TokenType::kReturnKeyword,
                         TokenType::kOpenParen,
                         TokenType::kIdentifier,
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
                         TokenType::kCloseCurlyBrace};

    string lexemes[] = {"function",  "(", ")", "{", "return", "(", "this", ".",
                        "a",         ".", "b", ".", "c",      ".", "d",    ">=",
                        "thisArray", "[", "0", "]", ")",      ";", "}"};

    testValidLine(input, types, lexemes, 17);
}

TEST(LexerTest, functionTernaryOp) {
    string input = "function() {return this.a['foo'] == 3 ? (this.b > 1) : (this.d == 2)}";

    TokenType types[] = {TokenType::kFunctionKeyword,
                         TokenType::kOpenParen,
                         TokenType::kCloseParen,
                         TokenType::kOpenCurlyBrace,
                         TokenType::kReturnKeyword,
                         TokenType::kIdentifier,
                         TokenType::kPeriod,
                         TokenType::kIdentifier,
                         TokenType::kOpenSquareBracket,
                         TokenType::kStringLiteral,
                         TokenType::kCloseSquareBracket,
                         TokenType::kDoubleEquals,
                         TokenType::kIntegerLiteral,
                         TokenType::kQuestionMark,
                         TokenType::kOpenParen,
                         TokenType::kIdentifier,
                         TokenType::kPeriod,
                         TokenType::kIdentifier,
                         TokenType::kGreaterThan,
                         TokenType::kIntegerLiteral,
                         TokenType::kCloseParen,
                         TokenType::kColon,
                         TokenType::kOpenParen,
                         TokenType::kIdentifier,
                         TokenType::kPeriod,
                         TokenType::kIdentifier,
                         TokenType::kDoubleEquals,
                         TokenType::kIntegerLiteral,
                         TokenType::kCloseParen,
                         TokenType::kCloseCurlyBrace};

    string lexemes[] = {"function", "(",  ")", "{",    "return", "this", ".",  "a", "[", "foo",
                        "]",        "==", "3", "?",    "(",      "this", ".",  "b", ">", "1",
                        ")",        ":",  "(", "this", ".",      "d",    "==", "2", ")", "}"};

    testValidLine(input, types, lexemes, 30);
}

TEST(LexerTest, nestedNonsense) {
    string input = "return ((((3 + a) - 'string') * NaN) / x)+ -(bar['a'].this.null[1])";

    TokenType types[] = {TokenType::kReturnKeyword,
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
                         TokenType::kIdentifier,
                         TokenType::kPeriod,
                         TokenType::kNullLiteral,
                         TokenType::kOpenSquareBracket,
                         TokenType::kIntegerLiteral,
                         TokenType::kCloseSquareBracket,
                         TokenType::kCloseParen};

    string lexemes[] = {"return", "(",   "(",   "(", "(",    "3", "+",    "a", ")", "-", "string",
                        ")",      "*",   "NaN", ")", "/",   "x", ")",    "+", "-", "(", "bar",
                        "[",      "a", "]",   ".", "this", ".", "null", "[", "1", "]", ")"};

    testValidLine(input, types, lexemes, 33);
}

TEST(LexerTest, integerAndkIdentifier) {
    string input = "1variableName";

    TokenType types[] = {TokenType::kIntegerLiteral, TokenType::kIdentifier};

    string lexemes[] = {"1", "variableName"};

    testValidLine(input, types, lexemes, 2);
}

TEST(LexerTest, weirdSpacing) {
    string input = ".-)(     [>null:]";

    TokenType types[] = {TokenType::kPeriod,
                         TokenType::kSubtract,
                         TokenType::kCloseParen,
                         TokenType::kOpenParen,
                         TokenType::kOpenSquareBracket,
                         TokenType::kGreaterThan,
                         TokenType::kNullLiteral,
                         TokenType::kColon,
                         TokenType::kCloseSquareBracket};

    string lexemes[] = {".", "-", ")", "(", "[", ">", "null", ":", "]"};

    testValidLine(input, types, lexemes, 9);
}

TEST(LexerTest, specialWordkIdentifier) {
    string input = "thisisnotafunction()";

    TokenType types[] = {TokenType::kIdentifier, TokenType::kOpenParen, TokenType::kCloseParen};

    string lexemes[] = {"thisisnotafunction", "(", ")"};

    testValidLine(input, types, lexemes, 3);
}

TEST(LexerTest, empty) {
    string input = "";

    TokenType types[] = {};

    string lexemes[] = {};

    testValidLine(input, types, lexemes, 0);
}

TEST(LexerTest, unrecognizedChar) {
    string input = "^^";

    testInvalidLine(input, 0, '^');
}

TEST(LexerTest, malformedAtStart) {
    string input = "^ hello";

    testInvalidLine(input, 0, '^');
}

TEST(LexerTest, malformedAtEnd) {
    string input = "hello ^";

    testInvalidLine(input, 6, '^');
}

TEST(LexerTest, malformedInMiddle) {
    string input = "hello ~0~ world";

    testInvalidLine(input, 6, '~');
}

TEST(LexerTest, invalidVariableName) {
    string input = "var%name";

    testInvalidLine(input, 3, '%');
}

}  // namespace tinyjs
}  // namespace mongo