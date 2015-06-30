#include "mongo/scripting/tinyjs/node.h"
#include "mongo/scripting/tinyjs/ast_parser.h"
#include "mongo/scripting/tinyjs/assignment_operator.h"
#include "mongo/scripting/tinyjs/arithmetic_operator.h"
#include "mongo/scripting/tinyjs/comparison_operator.h"
#include "mongo/scripting/tinyjs/lexer.h"
#include "mongo/scripting/tinyjs/logical_operator.h"
#include "mongo/scripting/tinyjs/unary_operator.h"
#include "mongo/scripting/tinyjs/operand.h"
#include "mongo/db/pipeline/value.h"
#include <stack>

namespace mongo {
namespace tinyjs {

Token currentToken;

ASTParser::ASTParser(std::vector<Token> tokens) {
    Node* head = parseTokens(tokens.begin(), tokens.end());
}

void nexttoken(void);
void error(const char msg[]);

int accept(Token t) {
    if (currentToken == t) {
        nexttoken();
        return 1;
    }
    return 0;
}

int expect(Token t) {
    if (accept(t))
        return 1;
    error("expect: unexpected token");
    return 0;
}

void clauseAction() {
    
}

}
}
