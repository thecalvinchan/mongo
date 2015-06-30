#include "mongo/scripting/tinyjs/ast_parser.h"
#include <stdexcept>
#include <iostream>

namespace mongo {
namespace tinyjs {

ASTParser::ASTParser(std::vector<Token> tokenInput)
    : currentPosition(0), currentToken(tokenInput[0]), tokens(tokenInput) {
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

Node* ASTParser::accept(TokenType t) {
    if (currentToken.type == t) {
        Node* leaf = new LeafNode(currentToken);
        std::cout << currentToken.value << std::endl;
        if (currentPosition < (int)tokens.size()) {
            nexttoken();
        }
        return leaf;
    }
    return null;
}

// Overloads accept to search non-terminals
Node* ASTParser::accept(std::function<void(void)> action) {
    int resetPosition = currentPosition;
    try {
        Node* subTreeHead = action();
        return subTreeHead;
    } catch (const std::invalid_argument& e) {
        currentPosition = resetPosition;
        currentToken = tokens[currentPosition];
        return null;
    }
}

Node* ASTParser::expect(TokenType t) {
    Node* leaf;
    if (leaf = ASTParser::accept(t)) {
        return leaf;
    }
    error("expect: unexpected token");
}

Node* ASTParser::clauseAction() {
    Node* head = new ClauseNode();
    Node *child;
    if (child = (accept(TokenType::kFunctionKeyword))) {
        head->addChild(child);
        head->addChild(expect(TokenType::kOpenParen));
        head->addChild(expect(TokenType::kCloseParen));
        head->addChild(expect(TokenType::kOpenCurlyBrace));
        returnStatementAction();
        head->addChild(expect(TokenType::kCloseCurlyBrace));
    } else if (child = (accept(returnStatementAction()))) {
        head->addChild(child);
    } else {
        error("this is not optional");
    }
    return head;
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
        objectAccessorAction();  // TODO: combine these two?
    } else {
        error("object: syntax error");
    }
}

void ASTParser::objectAccessorAction() {
    if (accept(TokenType::kPeriod)) {
        expect(TokenType::kIdentifier);
        objectAccessorAction();
    } else if (accept(TokenType::kOpenSquareBracket)) {
        if (accept(TokenType::kIntegerLiteral) || accept(TokenType::kStringLiteral) ||
            accept(TokenType::kIdentifier) ||
            accept(std::bind(&ASTParser::arithmeticExpressionAction, this))) {
            expect(TokenType::kCloseSquareBracket);
            objectAccessorAction();
        } else {
            error("object: syntax error");
        }
    }
}

void ASTParser::termAction() {
    if (accept(TokenType::kIntegerLiteral)) {  // TODO: frame this as negative? restructure to use
                                               // or's?
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
        if (accept(TokenType::kIntegerLiteral) || accept(TokenType::kIdentifier) ||
            accept(std::bind(&ASTParser::arithmeticExpressionAction, this))) {
            ;
        } else {
            error("arrayIndexed: syntax error");
        }
        expect(TokenType::kCloseSquareBracket);
    } else {
        error("arrayIndexed: syntax error");
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
    }
}

Node* ASTParser::returnStatementAction() {
    Node* head = new ReturnStatementActionNode();
    Node* child;
    if (child = accept(TokenType::kReturnKeyword)) {
        head->addChild(child);
        head->addChild(booleanExpressionAction());
        head->addChild(expect(TokenType::kSemiColon));
    } else {
        error("returnStatement: syntax error");
    }
    return head;
}

Node* ASTParser::logicalOperationAction() {
    Node* head = new LogicalOperationActionNode();
    Node* child;
    if (child = accept(TokenType::kLogicalAnd) || child = accept(TokenType::kLogicalOr)) {
        head->addChild(child);
    } else {
        error("logicalOperation: syntax error");
    }
    return head;
}

Node* ASTParser::comparisonOperationAction() {
    Node* head = new ComparisonOperationActionNode();
    Node* child;
    if (child = accept(TokenType::kTripleEquals) || child = accept(TokenType::kDoubleEquals) ||
        child = accept(TokenType::kGreaterThan) || child = accept(TokenType::kGreaterThanEquals) ||
        child = accept(TokenType::kLessThan) || child = accept(TokenType::kLessThanEquals) ||
        child = accept(TokenType::kNotEquals) || child = accept(TokenType::kDoubleNotEquals)) {
        head->addChild(child);
    } else {
        error("comparisonOperation: syntax error");
    }
    return head;
}

}
}
