#include "mongo/scripting/tinyjs/ast_parser.h"
#include <stdexcept>
#include <iostream>

namespace mongo {
namespace tinyjs {

ASTParser::ASTParser(std::vector<Token> tokenInput) :
    currentPosition(0), currentToken(tokenInput[0]), tokens(tokenInput) {
    parseTokens(tokens);
}


void ASTParser::parseTokens(std::vector<Token> tokens) {
    clauseAction();
}


void ASTParser::nexttoken(void) {
    currentPosition++;
    currentToken = tokens[currentPosition];
}


void ASTParser::error(const char msg[]) {
    throw std::invalid_argument(msg);
}

int ASTParser::accept(TokenType t) {
    if (currentToken.type == t) {
        nexttoken();
        std::cout << currentToken.value << std::endl;
        return 1;
    }
    return 0;
}

// Overloads accept to search non-terminals
int ASTParser::accept(std::function<void(void)> action) {
    int resetPosition = currentPosition;
    try {
        action();
        return 1;
    } catch (const std::invalid_argument& e) {
        currentPosition = resetPosition;
        return 0;
    }
}

int ASTParser::expect(TokenType t) {
    if (ASTParser::accept(t))
        return 1;
    error("expect: unexpected token");
    return 0;
}

void ASTParser::clauseAction() {
    if (accept(TokenType::kFunctionKeyword)) {
        expect(TokenType::kOpenParen);
        expect(TokenType::kCloseParen);
        expect(TokenType::kOpenCurlyBrace);
        returnStatementAction();
        expect(TokenType::kCloseCurlyBrace);
    } else {
        returnStatementAction();
    }
}

void ASTParser::variableAction() {
    if (accept(TokenType::kIdentifier)) {
        ;
    } else {
        objectAction();
    }
}

void ASTParser::objectAction() {
    if (accept(TokenType::kThisIdentifier)) {
        objectAccessorAction();
    } else if (accept(TokenType::kIdentifier)) {
        objectAccessorAction(); // TODO: combine these two?
    } else {
        error("object: syntax error");
        nexttoken();
    }
}

void ASTParser::objectAccessorAction() {
    if (accept(TokenType::kPeriod)) {
        expect(TokenType::kIdentifier);
        objectAccessorAction();
    } else if (accept(TokenType::kOpenSquareBracket)) {
        if (
            accept(TokenType::kIntegerLiteral) ||
            accept(TokenType::kStringLiteral) ||
            accept(TokenType::kIdentifier) ||
            accept(std::bind(&ASTParser::arithmeticExpressionAction, this))
        ) {
            expect(TokenType::kCloseSquareBracket);
            objectAccessorAction();
        } else {
            error("object: syntax error");
            nexttoken();
        }
    } 
}

void ASTParser::termAction() {
    if (accept(TokenType::kIntegerLiteral)) { //TODO: frame this as negative? restructure to use or's?
        ;
    } else if (accept(TokenType::kFloatLiteral)) {
        ;
    } else if (accept(TokenType::kStringLiteral)) {
        ;
    } else if (accept(std::bind(&ASTParser::variableAction, this))) {
        ;
    } else if (accept(TokenType::kBooleanLiteral)) {
        ;
    } else {
        error("term: syntax error");
        nexttoken();
    }
}

void ASTParser::arrayElementAction() {
    if (accept(std::bind(&ASTParser::termAction, this))) {
        ;
    } else if (accept(std::bind(&ASTParser::arithmeticExpressionAction, this))) {
        ;
    } else if (accept(std::bind(&ASTParser::booleanExpressionAction, this))) {
        ;
    } else {
        error("arrayElement: syntax error");
        nexttoken();
    }
}

void ASTParser::arrayLiteralAction() {
    if (accept(TokenType::kOpenSquareBracket)) {
        if (accept(TokenType::kCloseSquareBracket)) {
            ;
        } else {
            arrayElementAction();
            arrayTailAction();
            expect(TokenType::kCloseSquareBracket);
        }
    } else {
        error("arrayLiteralAction: syntax error");
        nexttoken();
    }
}

void ASTParser::arrayTailAction() {
    if (accept(TokenType::kComma)) {
        arrayElementAction();
        arrayTailAction();
    }
    // arrayTail is optional, so if it doesn't match, it's ok
}

void ASTParser::arrayIndexedAction() {
    if (accept(TokenType::kIdentifier)) {
        expect(TokenType::kOpenSquareBracket);
        if (
            accept(TokenType::kIntegerLiteral) ||
            accept(TokenType::kIdentifier) ||
            accept(std::bind(&ASTParser::arithmeticExpressionAction, this))) {
            ;
        } else {
            error("arrayIndexed: syntax error");
            nexttoken();
        } 
        expect(TokenType::kCloseSquareBracket);
    } else {
        error("arrayIndexed: syntax error");
        nexttoken();
    }
}

void ASTParser::factorAction() {
    if (accept(std::bind(&ASTParser::termAction, this))) {
        ;
    } else if (accept(TokenType::kOpenParen)) {
        arithmeticExpressionAction();
        expect(TokenType::kCloseParen);
    } else {
        error("factor: syntax error");
        nexttoken();
    }
}

void ASTParser::multiplicativeExpressionAction() {
    factorAction();
    multiplicativeOperationAction();
}

void ASTParser::multiplicativeOperationAction() {
    if (accept(TokenType::kMultiply)) {
        factorAction();
        multiplicativeOperationAction();
    } else if (accept(TokenType::kDivide)) {
        factorAction();
        multiplicativeOperationAction();
    }
    // multiplicativeOperation is optional, so if it doesn't match, it's ok
}

void ASTParser::arithmeticExpressionAction() {
    multiplicativeExpressionAction();
    arithmeticOperationAction();
}

void ASTParser::arithmeticOperationAction() {
    if (accept(TokenType::kAdd)) {
        multiplicativeExpressionAction();
        arithmeticOperationAction();
    } else if (accept(TokenType::kSubtract)) {
        multiplicativeExpressionAction();
        arithmeticOperationAction();
    }
    // arithmeticOperation is optional, so if it doesn't match, it's ok
}

void ASTParser::booleanFactorAction() {
    if (accept(TokenType::kOpenParen)) {
        booleanExpressionAction();
        expect(TokenType::kCloseParen);
    } else {
        arithmeticExpressionAction();
    }
}

void ASTParser::relationalExpressionAction() {
    booleanFactorAction();
    relationalOperationAction();
}

void ASTParser::relationalOperationAction() {
    if (accept(std::bind(&ASTParser::comparisonOperationAction, this))) {
        booleanFactorAction();
        relationalOperationAction();
    }
    // relationalOperation is optional, so if it doesn't match, it's ok
}

void ASTParser::booleanExpressionAction() {
    relationalExpressionAction();
    booleanOperationAction();
}

void ASTParser::booleanOperationAction() {
    if (accept(std::bind(&ASTParser::logicalOperationAction, this))) {
        relationalExpressionAction();
        booleanOperationAction();
    }
    // booleanOperation is optional, so if it doesn't match, it's ok
}

void ASTParser::ternaryOperationAction() {
    if (accept(std::bind(&ASTParser::booleanExpressionAction, this))) {
        expect(TokenType::kQuestionMark);
        booleanExpressionAction();
        expect(TokenType::kColon);
        booleanExpressionAction();
    } else {
        error("ternaryOperationAction: syntax error");
        nexttoken();
    }
}

void ASTParser::returnStatementAction() {
    if (accept(TokenType::kReturnKeyword)) {
        booleanExpressionAction();
        expect(TokenType::kSemiColon);
        nexttoken();
    } else {
        error("returnStatement: syntax error");
        nexttoken();
    }
}

void ASTParser::logicalOperationAction() {
    if (accept(TokenType::kLogicalAnd) ||
        accept(TokenType::kLogicalOr)) {
        ;
    } else {
        error("logicalOperation: syntax error");
        nexttoken();
    }
}

void ASTParser::comparisonOperationAction() {
    if (accept(TokenType::kTripleEquals) ||
        accept(TokenType::kDoubleEquals) ||
        accept(TokenType::kGreaterThan) ||
        accept(TokenType::kGreaterThanEquals) ||
        accept(TokenType::kLessThan) ||
        accept(TokenType::kLessThanEquals) ||
        accept(TokenType::kNotEquals) ||
        accept(TokenType::kDoubleNotEquals)) {
        ;
    } else {
        error("comparisonOperation: syntax error");
        nexttoken();
    }

}

}
}
