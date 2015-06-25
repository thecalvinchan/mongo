#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>
#include "mongo/base/string_data.h"
#include "lexer.h"

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
        emit(thisToken, &token_data, ts, te);
    };

    returnToken => {
        emit(returnToken, &token_data, ts, te);
    };

    integerLiteral => {
        emit(integerLiteral, &token_data, ts, te);
    };

    floatLiteral => {
        emit(floatLiteral, &token_data, ts, te);
    };
    
    booleanLiteral => {
        emit(booleanLiteral, &token_data, ts, te);
    };

    stringLiteral => {
        emit(stringLiteral, &token_data, ts, te);
    };
    
    identifier => {
        emit(identifier, &token_data, ts, te);
    };

    additiveOp => {
        emit(additiveOp, &token_data, ts, te);
    };

    multiplicativeOp => {
        emit(multiplicativeOp, &token_data, ts, te);
    };

    comparisonOp => {
        emit(comparisonOp, &token_data, ts, te);
    };

    logicalOp => {
        emit(logicalOp, &token_data, ts, te);
    };

    semicolon => {
        emit(semicolon, &token_data, ts, te);
    };

    openParen => {
        emit(openParen, &token_data, ts, te);
    };

    closeParen => {
        emit(closeParen, &token_data, ts, te);
    };

    questionMark => {
        emit(questionMark, &token_data, ts, te);
    };

    colon => {
        emit(colon, &token_data, ts, te);
    };

    period => {
        emit(period, &token_data, ts, te);
    };

    openSqBracket => {
        emit(openSqBracket, &token_data, ts, te);
    };

    closeSqBracket => {
        emit(closeSqBracket, &token_data, ts, te);
    };

    functionDec => {
        emit(functionDec, &token_data, ts, te);
    };

    openCurly => {
        emit(openCurly, &token_data, ts, te);
    };

    closeCurly => {
        emit(closeCurly, &token_data, ts, te);
    };
    
    space;
    
  *|;

  write data;
}%%

void emit(tokenType t, std::vector<token> *token_data, char *ts, char *te) {
    token tk;
    tk.type = t;
    tk.value = mongo::StringData(ts, (te - ts));
    token_data->push_back(tk);
}

std::vector<token> lex(char *input) {
    int cs;
    char *p = input;

    char *pe = p + strlen(p);
    int act;
    char *ts;
    char *te;
    char *eof = pe;
    std::vector<token> token_data;

  %%{
      write init;
      write exec;
  }%%

      return token_data;
}
