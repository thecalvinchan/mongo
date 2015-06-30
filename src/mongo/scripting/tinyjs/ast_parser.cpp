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

int accept(void (*action)(void)) {
    try {
    } catch {
    }
}

int expect(Token t) {
    if (accept(t))
        return 1;
    error("expect: unexpected token");
    return 0;
}

void clauseAction() {
    if (accept(kFunctionKeyword)) {
        expect(kOpenParen);
        expect(kCloseParen);
        expect(kOpenCurlyBrace);
        returnStatementAction();
        expect(kCloseCurlyBrace);
    } else {
        returnStatementAction();
    }
}

void variableAction() {
    if (accept(kIdentifier)) {
        ;
    } else {
        objectAction();
    }
}

void objectAction() {
    if (accept(kThisIdentifier)) {
        objectAccessorAction();
    } else if (accept(kIdentifier)) {
        objectAccessorAction(); // TODO: combine these two?
    } else {
        error("object: syntax error");
        nexttoken();
    }
}

void objectAccessorAction() {
    if (accept(kPeriod)) {
        expect(kIdentifier);
        objectAccessorAction();
    } else if (accept(kOpenSquareBracket)) {
        if (token == kIntegerLiteral ||
            token == kStringLiteral ||
            token == kIdentifier ||
            token == kArithmeticExpression) {
            
            expect(kCloseSquareBracket);
            objectAccessorAction();
        } else {
            error("object: syntax error");
            nexttoken();
        }
    } 
}

void termAction() {
    if (accept(kIntegerLiteral)) {
        ;
    } else if (accept(kFloatLiteral)) {
        ;
    } else if (accept(kStringLiteral)) {
        ;
    } else if (accept(kVariable)) {
        ;
    } else if (accept(kBooleanLiteral)) {
        ;
    } else {
        error("term: syntax error");
        nexttoken();
    }
}

void arrayElementAction() {

}

void comparisonOpAction() {
    if (currentToken == kTripleEquals ||
        currentToken == kDoubleEquals ||
        currentToken == kGreaterThan ||
        currentToken == kGreaterThanEquals ||
        currentToken == kLessThan ||
        currentToken == kLessThanEquals ||
        currentToken == kNotEquals ||
        currentToken == kDoubleNotEquals) {
        nextsym();
    }
}

void logicalOpAction() {
    if (currentToken == kLogicalAnd ||
        currentToken == kLogicalOr) {
        nextsym();
    }
}

void returnStatementAction() {
    if (accept()) {
    } else {
        error()
    }
}

}
}
