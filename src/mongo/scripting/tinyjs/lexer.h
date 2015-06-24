#include <vector>
#include "../../base/string_data.h"

/** Enum to describe different token types found in the Javascript subset */
enum tokenType{thisToken, returnToken, integerLiteral, floatLiteral, booleanLiteral, 
  stringLiteral, identifier, multiplicativeOp, additiveOp, comparisonOp, logicalOp, 
  endStatement, openParen, closeParen, questionMark, colon, period, openSqBracket, 
  closeSqBracket, functionDec, openCurly, closeCurly};

/** Struct to contain token type and lexeme */
struct token {
    tokenType type;
    mongo::StringData value;
};

/** Input: string written in subset of Javascript 
 *  Output: a vector of tokens, where each token contains a tokenType and a lexeme, 
 *  in order that the lexemes were found in the input string 
 */
std::vector<token> lex(char *input);