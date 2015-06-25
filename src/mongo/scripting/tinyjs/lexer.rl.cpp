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

// This Ragel machine lexes a line written in a subset of Javascript.
%%{

    machine lexer;
  
    thisToken        = 'this';
    returnToken      = 'return';
    integerLiteral   = [0-9]+;
    floatLiteral     = ('+'|'-')?[0-9]*'.'[0-9]+;
    booleanLiteral   = 'true' | 'false';
    stringLiteral    = "'"[^\']*"'" 
                      | '"'[^\"]*'"'; 
                      #" (comment to fix syntax highlighting)
    identifier       = [a-zA-Z_][a-zA-Z_0-9]*;
    additiveOp       = '+' | '-';
    multiplicativeOp = '*' | '\\';
    comparisonOp     = '===' | '==' | '<' | '<=' | '>' | '>=' | '!=' | '!==';
    logicalOp        = '&&' | '||' | '!';
    semicolon        = ';';
    openParen        = '\(';
    closeParen       = '\)';  
    questionMark     = '?';
    colon            = ':';
    period           = '.';
    openSqBracket    = '[';
    closeSqBracket   = ']';
    functionDec      = 'function()';
    openCurly        = '{';
    closeCurly       = '}';
  
    main := |*
      
        thisToken => {
            emit(thisToken, &tokenData, ts, te);
        };

        returnToken => {
            emit(returnToken, &tokenData, ts, te);
        };

        integerLiteral => {
            emit(integerLiteral, &tokenData, ts, te);
        };

        floatLiteral => {
            emit(floatLiteral, &tokenData, ts, te);
        };
        
        booleanLiteral => {
            emit(booleanLiteral, &tokenData, ts, te);
        };

        stringLiteral => {
            emit(stringLiteral, &tokenData, ts, te);
        };
        
        identifier => {
            emit(identifier, &tokenData, ts, te);
        };

        additiveOp => {
            emit(additiveOp, &tokenData, ts, te);
        };

        multiplicativeOp => {
            emit(multiplicativeOp, &tokenData, ts, te);
        };

        comparisonOp => {
            emit(comparisonOp, &tokenData, ts, te);
        };

        logicalOp => {
            emit(logicalOp, &tokenData, ts, te);
        };

        semicolon => {
            emit(semicolon, &tokenData, ts, te);
        };

        openParen => {
            emit(openParen, &tokenData, ts, te);
        };

        closeParen => {
            emit(closeParen, &tokenData, ts, te);
        };

        questionMark => {
            emit(questionMark, &tokenData, ts, te);
        };

        colon => {
            emit(colon, &tokenData, ts, te);
        };

        period => {
            emit(period, &tokenData, ts, te);
        };

        openSqBracket => {
            emit(openSqBracket, &tokenData, ts, te);
        };

        closeSqBracket => {
            emit(closeSqBracket, &tokenData, ts, te);
        };

        functionDec => {
            emit(functionDec, &tokenData, ts, te);
        };

        openCurly => {
            emit(openCurly, &tokenData, ts, te);
        };

        closeCurly => {
            emit(closeCurly, &tokenData, ts, te);
        };
        
        space;
      
    *|;

    write data;
}%%

// This function adds a token containing type and value to the tokenData array.
void emit(tokenType t, std::vector<token> *tokenData, char *ts, char *te) {
    token tk;
    tk.type = t;
    tk.value = mongo::StringData(ts, (te - ts));
    tokenData->push_back(tk);
}

// This function uses the Ragel machine above to lex a line written in a subset of Javascript.
std::vector<token> lex(char *input) {
    int cs;
    char *p = input;

    char *pe = p + strlen(p);
    int act;
    char *ts;
    char *te;
    char *eof = pe;
    std::vector<token> tokenData;

    %%{
        write init;
        write exec;
    }%%

    return tokenData;
}
