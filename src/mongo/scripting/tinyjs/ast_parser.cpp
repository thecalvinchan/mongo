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
#include <stdexcept>

namespace mongo {
namespace tinyjs {

Token currentToken;
int currentPosition;

ASTParser::ASTParser(std::vector<Token> tokens) {
    Node* head = parseTokens(tokens);
}


Node* parseTokens(std::vector<Token> tokens) {
    currentPosition = 0;
    currentToken = tokens[currentPosition];
    clauseAction();
    return NULL;
}


void nexttoken(void) {
    currentPosition++;
    currentToken = tokens[currentPosition];
}


void error(const char msg[]) {
    throw std::invalid_argument(msg);
}

int accept(Token t) {
    if (currentToken.type == t) {
        nexttoken();
        return 1;
    }
    return 0;
}

// Overloads accept to search non-terminals
int accept(void (*action)(void)) {
    int resetPosition = currentPosition;
    try {
        action();
        return 1;
    } catch (const std::invalid_argument& e) {
        currentPosition = resetPosition;
        return 0;
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
    if (accept(kIntegerLiteral)) { //TODO: frame this as negative? restructure to use or's?
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
    if (accept(&termAction)) {
        ;
    } else if (accept(&arithmeticExpressionAction)) {
        ;
    } else if (accept(&booleanExpressionAction)) {
        ;
    } else {
        error("arrayElement: syntax error");
        nexttoken();
    }
}

void arrayLiteralAction() {
    if (accept(kOpenSquareBracket)) {
        if (accept(kCloseSquareBracket)) {
            ;
        } else {
            arrayElementAction();
            arrayTailAction();
            expect(kCloseSquareBracket);
        }
    } else {
        error("arrayLiteralAction: syntax error");
        nexttoken();
    }
}

void arrayTailAction() {
    if (accept(kComma)) {
        arrayElementAction();
        arrayTailAction();
    }
    // arrayTail is optional, so if it doesn't match, it's ok
}

void arrayIndexedAction() {
    if (accept(kIdentifier)) {
        expect(kOpenSquareBracket);
        if (token == kIntegerLiteral ||
            token == kIdentifier ||
            token == kArithmeticExpression) {
            ;
        } else {
            error("arrayIndexed: syntax error");
            nexttoken();
        } 
        expect(kCloseSquareBracket);
    } else {
        error("arrayIndexed: syntax error");
        nexttoken();
    }
}

void factorAction() {
    if (accept(&termAction)) {
        ;
    } else if accept(kOpenParen) {
        arithmeticExpressionAction();
        expect(kCloseParen);
    } else {
        error("factor: syntax error");
        nexttoken();
    }
}

void multiplicativeExpressionAction() {
    factorAction();
    multiplicativeOperationAction();
}

void multiplicativeOperationAction() {
    if (accept(kMultiply)) {
        factorAction();
        multiplicativeOperationAction();
    } else if (accept(kDivide)) {
        factorAction();
        multiplicativeOperationAction();
    }
    // multiplicativeOperation is optional, so if it doesn't match, it's ok
}

void arithmeticExpressionAction() {
    multiplicativeExpressionAction();
    arithmeticOperationAction();
}

void arithmeticOperationAction() {
    if (accept(kAdd)) {
        multiplicativeExpressionAction();
        arithmeticOperationAction();
    } else if (accept(kSubtract)) {
        multiplicativeExpressionAction();
        arithmeticOperationAction();
    }
    // arithmeticOperation is optional, so if it doesn't match, it's ok
}

void booleanFactorAction() {
    if (accept(kOpenParen)) {
        booleanExpressionAction();
        expect(kCloseParen);
    } else {
        arithmeticExpressionAction();
    }
}

void relationalExpressionAction() {
    booleanFactorAction();
    relationalOperationAction();
}

void relationalOperationAction() {
    if (accept(&comparisonOperationAction)) {
        booleanFactorAction();
        relationalOperationAction();
    }
    // relationalOperation is optional, so if it doesn't match, it's ok
}

void booleanExpressionAction() {
    relationalExpressionAction();
    booleanOperationAction();
}

void booleanOperationAction() {
    if (accept(&logicalOperationAction)) {
        relationalExpressionAction();
        booleanOperationAction();
    }
    // booleanOperation is optional, so if it doesn't match, it's ok
}

void ternaryOperationAction() {
    booleanExpressionAction();
    expect(kQuestionMark);
    booleanExpressionAction();
    expect(kColon);
    booleanExpressionAction();
}

void ternaryOpAction() {
    if (accept(&booleanExpressionAction)) {
        expect(kQuestionMark);
        expect
    }
}

void returnStatementAction() {
    if (accept(kReturnKeyword)) {
        booleanExpressionAction();
        expect(kSemiColon);
        nexttoken();
    } else {
        error("returnStatement: syntax error");
        nexttoken();
    }
}

void logicalOperationAction() {
    if (currentToken.type == kLogicalAnd ||
        currentToken.type == kLogicalOr) {
        nexttoken();
    }
}

void comparisonOperationAction() {
    if (currentToken.type == kTripleEquals ||
        currentToken.type == kDoubleEquals ||
        currentToken.type == kGreaterThan ||
        currentToken.type == kGreaterThanEquals ||
        currentToken.type == kLessThan ||
        currentToken.type == kLessThanEquals ||
        currentToken.type == kNotEquals ||
        currentToken.type == kDoubleNotEquals) {
        nexttoken();
    }
}

}
}
