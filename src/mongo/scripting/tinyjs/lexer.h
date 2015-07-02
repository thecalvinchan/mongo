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

#pragma once

#include <vector>

#include "mongo/base/string_data.h"
#include "mongo/base/status_with.h"

namespace mongo {
namespace tinyjs {

/**
 * The Token types comprising the 'tinyjs' JavaScript subset.
 */
enum class TokenType {
    kThisIdentifier,
    kReturnKeyword,
    kNullLiteral,
    kUndefinedLiteral,
    kFunctionKeyword,
    kIntegerLiteral,
    kFloatLiteral,
    kBooleanLiteral,
    kStringLiteral,
    kIdentifier,
    kAdd,
    kSubtract,
    kMultiply,
    kDivide,
    kTripleEquals,
    kDoubleEquals,
    kLessThan,
    kLessThanEquals,
    kGreaterThan,
    kGreaterThanEquals,
    kNotEquals,
    kDoubleNotEquals,
    kLogicalAnd,
    kLogicalOr,
    kLogicalNot,
    kSemiColon,
    kOpenParen,
    kCloseParen,
    kQuestionMark,
    kColon,
    kPeriod,
    kComma,
    kOpenSquareBracket,
    kCloseSquareBracket,
    kOpenCurlyBrace,
    kCloseCurlyBrace
};

/**
 * This struct contains the token type and lexeme for a given token.
 */
struct Token {
    TokenType type;
    StringData value;
    const char* tokenStart;
    const char* tokenEnd;
    Token(TokenType t, StringData v, const char* ts, const char* te) {
        type = t;
        value = v;
        tokenStart = ts;
        tokenEnd = te;
    }
};

/**
 * This function takes in a line written in a subset of Javascript, lexes it,
 * and outputs a vector of tokens, where each token contains a TokenType and a lexeme,
 * in the order that the lexemes were found in the input string.
 */
StatusWith<std::vector<Token>> lex(StringData input);

}  // namespace tinyjs
}  // namespace mongo