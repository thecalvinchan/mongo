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
#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>
#include "mongo/base/string_data.h"
#include "lexer.h"

namespace mongo {
namespace tinyjs {

// This Ragel machine lexes a line written in a subset of Javascript.
%%{

    machine lexer;
  
    kThisIdentifier      = 'this';
    kReturnIdentifier    = 'return';
    kNullIdentifier      = 'null';
    kUndefinedIdentifier = 'undefined';
    kIntegerLiteral      = [0-9]+;
    kFloatLiteral        = ('+'|'-')?[0-9]*'.'[0-9]+ | 'NaN' | 'Infinity';
    kBooleanLiteral      = 'true' | 'false';
    kStringLiteral       = "'"[^\']*"'" 
                          | '"'[^\"]*'"'; 
                          #" (comment to fix syntax highlighting)
    kIdentifier          = [a-zA-Z_][a-zA-Z_0-9]*;
    kAdd                 = '+';
    kSubtract            = '-';
    kMultiply            = '*';
    kDivide              = '\\';
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
    kOpenSquareBracket   = '[';
    kCloseSquareBracket  = ']';
    kFunctionDec         = 'function()';
    kOpenCurly           = '{';
    kCloseCurly          = '}';
  
    main := |*
      
        kThisIdentifier => {
            emit(TokenType::kThisIdentifier, &tokenData, ts, te);
        };

        kReturnIdentifier => {
            emit(TokenType::kReturnIdentifier, &tokenData, ts, te);
        };

        kNullIdentifier => {
            emit(TokenType::kNullIdentifier, &tokenData, ts, te);
        };

        kUndefinedIdentifier => {
            emit(TokenType::kUndefinedIdentifier, &tokenData, ts, te);
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

        kOpenSquareBracket => {
            emit(TokenType::kOpenSquareBracket, &tokenData, ts, te);
        };

        kCloseSquareBracket => {
            emit(TokenType::kCloseSquareBracket, &tokenData, ts, te);
        };

        kFunctionDec => {
            emit(TokenType::kFunctionDec, &tokenData, ts, te);
        };

        kOpenCurly => {
            emit(TokenType::kOpenCurly, &tokenData, ts, te);
        };

        kCloseCurly => {
            emit(TokenType::kCloseCurly, &tokenData, ts, te);
        };
        
        space;
      
    *|;

    write data;
}%%

// This function adds a token containing type and value to the tokenData array.
void emit(TokenType t, std::vector<Token> *tokenData, char *ts, char *te) {
    Token tk;
    tk.type = t;
    tk.value = mongo::StringData(ts, (te - ts));
    tokenData->push_back(tk);
}

// This function uses the Ragel machine above to lex a line written in a subset of Javascript.
std::vector<Token> lex(StringData input) {
    int cs;
    const char *data = input.rawData();
    /*if (data[input.size()] != '\0') {
      return none;
    }*/

    char *p = new char[input.size()];
    strcpy(p, data);

    char *pe = p + strlen(p);
    int act;
    char *ts;
    char *te;
    char *eof = pe;
    std::vector<Token> tokenData;

    %%{
        write init;
        write exec;
    }%%

    return tokenData;
}

} // namespace tinyjs
} // namespace mongo