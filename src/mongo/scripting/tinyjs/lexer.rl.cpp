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
#include <vector>

#include "mongo/base/status_with.h"
#include "mongo/base/string_data.h"
#include "mongo/util/mongoutils/str.h"
#include "mongo/scripting/tinyjs/lexer.h"

namespace mongo {
namespace tinyjs {

namespace {

/*
 * This Ragel machine lexes a line written in a subset of Javascript.
*/
%%{
    machine lexer;

    kReturnKeyword       = 'return';
    kNullLiteral         = 'null';
    kUndefinedLiteral    = 'undefined';
    kFunctionKeyword     = 'function';
    kIntegerLiteral      = 0 | ([1-9][0-9]*);
    kFloatLiteral        = [0-9]*'.'[0-9]+ | 'NaN' | 'Infinity';
    kBooleanLiteral      = 'true' | 'false';
    kStringLiteral       = "'"[^\']*"'" 
                          | '"'[^\"]*'"'; 
                          #" (comment to fix syntax highlighting)
    kIdentifier          = [a-zA-Z_][a-zA-Z_0-9]*;
    kAdd                 = '+';
    kSubtract            = '-';
    kMultiply            = '*';
    kDivide              = '/';
    kTripleEquals        = '===';
    kDoubleEquals        = '==';
    kLessThan            = '<';
    kLessThanEquals      = '<=';
    kGreaterThan         = '>';
    kGreaterThanEquals   = '>=';
    kNotEquals           = '!=';
    kDoubleNotEquals     = '!==';
    kLogicalAnd          = '&&';
    kLogicalOr           = '||';
    kLogicalNot          = "!";
    kSemiColon           = ';';
    kOpenParen           = '\(';
    kCloseParen          = '\)';  
    kQuestionMark        = '?';
    kColon               = ':';
    kPeriod              = '.';
    kComma               = ',';
    kOpenSquareBracket   = '[';
    kCloseSquareBracket  = ']';
    kOpenCurlyBrace      = '{';
    kCloseCurlyBrace     = '}';
    kError               = any;
  
    main := |*

        kReturnKeyword => {
        emit(TokenType::kReturnKeyword, &tokenData, ts, te);
        };

        kNullLiteral => {
        emit(TokenType::kNullLiteral, &tokenData, ts, te);
        };

        kUndefinedLiteral => {
        emit(TokenType::kUndefinedLiteral, &tokenData, ts, te);
        };

        kFunctionKeyword => {
        emit(TokenType::kFunctionKeyword, &tokenData, ts, te);
        };

        kIntegerLiteral => {
        emit(TokenType::kIntegerLiteral, &tokenData, ts, te);
        };

        kFloatLiteral => {
        emit(TokenType::kFloatLiteral, &tokenData, ts, te);
        };
        
        kBooleanLiteral => {
        emit(TokenType::kBooleanLiteral, &tokenData, ts, te);
        };

        kStringLiteral => {
        emit(TokenType::kStringLiteral, &tokenData, ts, te);
        };
        
        kIdentifier => {
        emit(TokenType::kIdentifier, &tokenData, ts, te);
        };

        kAdd => {
        emit(TokenType::kAdd, &tokenData, ts, te);
        };

        kSubtract => {
        emit(TokenType::kSubtract, &tokenData, ts, te);
        };

        kMultiply => {
        emit(TokenType::kMultiply, &tokenData, ts, te);
        };

        kDivide => {
        emit(TokenType::kDivide, &tokenData, ts, te);
        };

        kTripleEquals => {
        emit(TokenType::kTripleEquals, &tokenData, ts, te);
        };

        kDoubleEquals => {
        emit(TokenType::kDoubleEquals, &tokenData, ts, te);
        };

        kLessThan => {
        emit(TokenType::kLessThan, &tokenData, ts, te);
        };

        kLessThanEquals => {
        emit(TokenType::kLessThanEquals, &tokenData, ts, te);
        };

        kGreaterThan => {
        emit(TokenType::kGreaterThan, &tokenData, ts, te);
        };

        kGreaterThanEquals => {
        emit(TokenType::kGreaterThanEquals, &tokenData, ts, te);
        };

        kNotEquals => {
        emit(TokenType::kNotEquals, &tokenData, ts, te);
        };

        kDoubleNotEquals => {
        emit(TokenType::kDoubleNotEquals, &tokenData, ts, te);
        };

        kLogicalAnd => {
        emit(TokenType::kLogicalAnd, &tokenData, ts, te);
        };

        kLogicalOr => {
        emit(TokenType::kLogicalOr, &tokenData, ts, te);
        };

        kLogicalNot => {
        emit(TokenType::kLogicalNot, &tokenData, ts, te);
        };

        kSemiColon => {
        emit(TokenType::kSemiColon, &tokenData, ts, te);
        };

        kOpenParen => {
        emit(TokenType::kOpenParen, &tokenData, ts, te);
        };

        kCloseParen => {
        emit(TokenType::kCloseParen, &tokenData, ts, te);
        };

        kQuestionMark => {
        emit(TokenType::kQuestionMark, &tokenData, ts, te);
        };

        kColon => {
        emit(TokenType::kColon, &tokenData, ts, te);
        };

        kPeriod => {
        emit(TokenType::kPeriod, &tokenData, ts, te);
        };

        kComma => {
        emit(TokenType::kComma, &tokenData, ts, te);
        };

        kOpenSquareBracket => {
        emit(TokenType::kOpenSquareBracket, &tokenData, ts, te);
        };

        kCloseSquareBracket => {
        emit(TokenType::kCloseSquareBracket, &tokenData, ts, te);
        };

        kOpenCurlyBrace => {
        emit(TokenType::kOpenCurlyBrace, &tokenData, ts, te);
        };

        kCloseCurlyBrace => {
        emit(TokenType::kCloseCurlyBrace, &tokenData, ts, te);
        };
        
        space;

        kError => {
        emitError(&errorLocations, ts, eof);
        };

    *|;

    write data;
}%%

/*
 * This function adds a token containing type and value to the tokenData array.
 */
void emit(TokenType t, std::vector<Token> *tokenData, const char *ts, const char *te) {
    tokenData->emplace_back(t, StringData(ts, (te - ts)), ts, te);
}

/*
 * This function inserts the location of an error, relative to the end of the input 
 * string, to the errorLocations vector.
 */
void emitError(std::vector<std::size_t> *errorLocations, const char *ts, const char *eof) {
    int indexFromEnd = eof - ts;
    errorLocations->push_back(indexFromEnd);
}

}  // namespace

/*
 * This is the constructor for a Token. It takes in and stores the type, lexeme, start pointer, and
 * end pointer of  the token.
 */
Token::Token(TokenType t, StringData v, const char* ts, const char* te) {
    type = t;
    value = v;
    tokenStart = ts;
    tokenEnd = te;
};

/*
 * This function uses the Ragel machine above to lex a line 
 * written in the tinyjs subset of Javascript.
 */
StatusWith<std::vector<Token>> lex(StringData input) {
    
    // Ragel requires the initialization of the variables cs, p, pe, act, ts, te, and eof.
    // cs represents the current state.
    int cs;
    // p is a pointer to the data.
    const char *p = input.rawData();
    // pe is a pointer to the end of the data.
    const char *pe = p + input.size();
    // act is used by the scanner to keep track of the most recent successful match.
    int act;
    // ts is a pointer to the start of the current token.
    const char *ts;
    // te is a pointed to the end of the current token.
    const char *te;
    // eof is a pointer to the end of the file.
    const char *eof = pe;

    std::vector<Token> tokenData;
    std::vector<std::size_t> errorLocations;

    %%{
        write init;
        write exec;
    }%%

    if (!errorLocations.empty()) {
        int lastIndex = input.size();
        int errorIndex = lastIndex - errorLocations[0];
        return StatusWith<std::vector<Token>>(
            ErrorCodes::FailedToParse,
            str::stream() << "Could not parse input starting with character: " << input[errorIndex]
                          << "\n" << input << "\n" << std::string(errorIndex, ' ') << "^"
                          << "\n");
    }

    return tokenData;
}

}  // namespace tinyjs
}  // namespace mongo