#include "mongo/scripting/tinyjs/ast_parser.h"
#include "mongo/scripting/tinyjs/Node.h"
#include <stdexcept>
#include <iostream>
#include <memory>

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

std::unique_ptr<Node> ASTParser::accept(TokenType t) {
    if (currentToken.type == t) {
        std::unique_ptr<Node> leaf(new LeafNode(currentToken));
        std::cout << currentToken.value << std::endl;
        if (currentPosition < (int)tokens.size()) {
            nexttoken();
        }
        return leaf;
    }
    return NULL;
}

// Overloads accept to search non-terminals
std::unique_ptr<Node> ASTParser::accept(std::function< std::unique_ptr<Node> (void) > action) {
    int resetPosition = currentPosition;
    try {
        //std::unique_ptr<Node> subTreeHead = action();
        return action();
    } catch (const std::invalid_argument& e) {
        currentPosition = resetPosition;
        currentToken = tokens[currentPosition];
        return null;
    }
}

std::unique_ptr<Node> ASTParser::expect(TokenType t) {
    std::unique_ptr<Node> leaf = this->accept(t);
    if (leaf != null) {
        return leaf;
    }
    error("expect: unexpected token");
}

std::unique_ptr<Node> ASTParser::clauseAction() {
    std::unique_ptr<Node> head(new ClauseNode());
    std::unique_ptr<Node> child;
    if (child = accept(TokenType::kFunctionKeyword)) {
        head->addChild(child);
        head->addChild(expect(TokenType::kOpenParen));
        head->addChild(expect(TokenType::kCloseParen));
        head->addChild(expect(TokenType::kOpenCurlyBrace));
        head->addChild(returnStatementAction());
        head->addChild(expect(TokenType::kCloseCurlyBrace));
    } else if (child = (accept(returnStatementAction()))) {
        head->addChild(child);
    } else {
        error("clause: sytax error");
    }
    return head;
}

std::unique_ptr<Node> ASTParser::variableAction() {
    std::unique_ptr<Node> head(new VariableNode());
    std::unique_ptr<Node> child;
    if (child = accept(TokenType::kIdentifier)) {
        head->addChild(child);
    } else if (child = accept(objectAction())){
        head->addChild(child);
    } else {
        error("variable: syntax error");
    }
    return head;
}

std::unique_ptr<Node> ASTParser::objectAction() {
    std::unique_ptr<Node> head(new ObjectNode());
    std::unique_ptr<Node> child;
    if (child = accept(TokenType::kThisIdentifier)) {
        head->addChild(child);
        head->addChild(objectAccessorAction());
    } else if (child = accept(TokenType::kIdentifier)) {
        head->addChild(child);
        head->addChild(objectAccessorAction()); // TODO: combine these two?
    } else {
        error("object: syntax error");
    }
    return head;
}

std::unique_ptr<Node> ASTParser::objectAccessorAction() {
    std::unique_ptr<Node> head(new ObjectAccessorNode());
    std::unique_ptr<Node> child;
    if (child = accept(TokenType::kPeriod)) {
        head->addChild(child);
        head->addChild(expect(TokenType::kIdentifier));
        head->addChild(objectAccessorAction());
        return head;
    } else if (child = accept(TokenType::kOpenSquareBracket)) {
        std::unique_ptr<Node> child2;
        if (child2 = accept(TokenType::kIntegerLiteral) || child2 = accept(TokenType::kStringLiteral) ||
            child2 = accept(TokenType::kIdentifier) ||
            child2 = accept(std::bind(&ASTParser::arithmeticExpressionAction, this))) {
            head->addChild(child2);
            head->addChild(expect(TokenType::kCloseSquareBracket));
            head->addChild(objectAccessorAction());
            return head;
        } else {
            error("object: syntax error");
        }
    }
    return null;
    // optional
}

std::unique_ptr<Node> ASTParser::termAction() {
    std::unique_ptr<Node> head (new TermNode());
    std::unique_ptr<Node> child;
    if (child = accept(TokenType::kIntegerLiteral) ||
        child = accept(TokenType::kFloatLiteral) ||
        child = accept(TokenType::kStringLiteral) ||
        child = accept(std::bind(&ASTParser::variableAction, this)) ||
        child = accept(TokenType::kBooleanLiteral)) {
        head->addChild(child);
    } else {
        error("term: syntax error");
    }
    return head;
}

std::unique_ptr<Node> ASTParser::arrayElementAction() {
    std::unique_ptr<Node> head (new ArrayElementNode());
    std::unique_ptr<Node> child;
    if (child = accept(std::bind(&ASTParser::termAction, this))) {
        head->addChild(child);
    } else if (child = accept(std::bind(&ASTParser::arithmeticExpressionAction, this))) {
        head->addChild(child);
    } else if (child = accept(std::bind(&ASTParser::booleanExpressionAction, this))) {
        head->addChild(child);
    } else {
        error("arrayElement: syntax error");
    }
    return head;
}

std::unique_ptr<Node> ASTParser::arrayLiteralAction() {
    std::unique_ptr<Node> head (new ArrayLiteralNode());
    std::unique_ptr<Node> child;
    if (child = accept(TokenType::kOpenSquareBracket)) {
        head->addChild(child);
        std::unique_ptr<Node> child2;
        if (child2 = accept(TokenType::kCloseSquareBracket)) {
            head->addChild(child2);
        } else {
            head->addChild(arrayElementAction());
            head->addChild(arrayTailAction());
            head->addChild(expect(TokenType::kCloseSquareBracket));
        }
    } else {
        error("arrayLiteralAction: syntax error");
    }
    return head;
}

std::unique_ptr<Node> ASTParser::arrayTailAction() {
    std::unique_ptr<Node> head (new ArrayTailNode());
    std::unique_ptr<Node> child;
    if (child = accept(TokenType::kComma)) {
        head->addChild(child);
        head->addChild(arrayElementAction());
        head->addChild(arrayTailAction());
        return head;
    }
    return null;
    // arrayTail is optional, so if it doesn't match, it's ok
}

std::unique_ptr<Node> ASTParser::arrayIndexedAction() {
    std::unique_ptr<Node> head (new ArrayIndexedNode());
    std::unique_ptr<Node> child;
    if (child = accept(TokenType::kIdentifier)) {
        head->addChild(child);
        head->addChild(expect(TokenType::kOpenSquareBracket));
        std::unique_ptr<Node> child2;
        if (child2 = accept(TokenType::kIntegerLiteral) || child2 = accept(TokenType::kIdentifier) ||
            child2 = accept(std::bind(&ASTParser::arithmeticExpressionAction, this))) {
            head->addChild(child2);
        } else {
            error("arrayIndexed: syntax error");
        }
        head->addChild(expect(TokenType::kCloseSquareBracket));
    } else {
        error("arrayIndexed: syntax error");
    }
    return head;
}

std::unique_ptr<Node> ASTParser::factorAction() {
    std::unique_ptr<Node> head (new FactorNode());
    std::unique_ptr<Node> child;
    if (child = accept(std::bind(&ASTParser::termAction, this))) {
        head->addChild(child);
    } else if (child = accept(TokenType::kOpenParen)) {
        head->addChild(child);
        head->addChild(arithmeticExpressionAction());
        head->addChild(expect(TokenType::kCloseParen));
    } else {
        error("factor: syntax error");
    }
    return head;
}

std::unique_ptr<Node> ASTParser::multiplicativeExpressionAction() {
    std::unique_ptr<Node> head (new MultiplicativeExpressionNode());
    head->addChild(factorAction());
    head->addChild(multiplicativeOperationAction());
    return head;
}

std::unique_ptr<Node> ASTParser::multiplicativeOperationAction() {
    std::unique_ptr<Node> head (new MultiplicativeOperationNode());
    std::unique_ptr<Node> child;
    if (child = accept(TokenType::kMultiply)) {
        head->addChild(child);
        head->addChild(factorAction());
        head->addChild(multiplicativeOperationAction());
        return head;
    } else if (child = accept(TokenType::kDivide)) {
        head->addChild(child);
        head->addChild(factorAction());
        head->addChild(multiplicativeOperationAction());
        return head;
    }
    return null;
    // multiplicativeOperation is optional, so if it doesn't match, it's ok
}

std::unique_ptr<Node> ASTParser::arithmeticExpressionAction() {
    std::unique_ptr<Node> head (new ArithmeticExpressionNode());
    head->addChild(multiplicativeExpressionAction());
    head->addChild(arithmeticOperationAction());
    return head;
}

std::unique_ptr<Node> ASTParser::arithmeticOperationAction() {
    std::unique_ptr<Node> head (new ArithmeticOperationNode());
    std::unique_ptr<Node> child;
    if (child = accept(TokenType::kAdd)) {
        head->addChild(child);
        head->addChild(multiplicativeExpressionAction());
        head->addChild(arithmeticOperationAction());
        return head;
    } else if (child = accept(TokenType::kSubtract)) {
        head->addChild(child);
        head->addChild(multiplicativeExpressionAction());
        head->addChild(arithmeticOperationAction());
        return head;
    }
    return null
    // arithmeticOperation is optional, so if it doesn't match, it's ok
}

std::unique_ptr<Node> ASTParser::booleanFactorAction() {
    std::unique_ptr<Node> head (new BooleanFactorNode());
    std::unique_ptr<Node> child;
    if (child = accept(TokenType::kOpenParen)) {
        head->addChild(child);
        head->addChild(booleanExpressionAction());
        head->addChild(expect(TokenType::kCloseParen));
    } else {
        head->addChild(arithmeticExpressionAction());
    }
    return head;
}

std::unique_ptr<Node> ASTParser::relationalExpressionAction() {
    std::unique_ptr<Node> head (new RelationalExpressionNode());
    head->addChild(booleanFactorAction());
    head->addChild(relationalOperationAction());
    return head;
}

std::unique_ptr<Node> ASTParser::relationalOperationAction() {
    std::unique_ptr<Node> head (new RelationalOperationNode());
    std::unique_ptr<Node> child;
    if (child = accept(std::bind(&ASTParser::comparisonOperationAction, this))) {
        head->addChild(child);
        head->addChild(booleanFactorAction());
        head->addChild(relationalOperationAction());
        return head;
    }
    return null;
    // relationalOperation is optional, so if it doesn't match, it's ok
}

std::unique_ptr<Node> ASTParser::booleanExpressionAction() {
    std::unique_ptr<Node> head (new BooleanExpressionNode());
    head->addChild(relationalExpressionAction());
    head->addChild(booleanOperationAction());
    return head;
}

std::unique_ptr<Node> ASTParser::booleanOperationAction() {
    std::unique_ptr<Node> head (new BooleanOperationNode());
    std::unique_ptr<Node> child;
    if (child = accept(std::bind(&ASTParser::logicalOperationAction, this))) {
        Node* head = new BooleanOperationNode();
        Node* child;
        head->addChild(child);
        head->addChild(relationalExpressionAction());
        head->addChild(booleanOperationAction());
        return head;
    }
    return null;
    // booleanOperation is optional, so if it doesn't match, it's ok
}

std::unique_ptr<Node> ASTParser::ternaryOperationAction() {
    std::unique_ptr<Node> head (new TernaryOperationNode());
    std::unique_ptr<Node> child;
    if (child = accept(std::bind(&ASTParser::booleanExpressionAction, this))) {
        head->addChild(child);
        head->addChild(expect(TokenType::kQuestionMark));
        head->addChild(booleanExpressionAction());
        head->addChild(expect(TokenType::kColon));
        head->addChild(booleanExpressionAction());
    } else {
        error("ternaryOperationAction: syntax error");
    }
    return head;
}

std::unique_ptr<Node> ASTParser::returnStatementAction() {
    std::unique_ptr<Node> head (new ReturnStatementNode());
    std::unique_ptr<Node> child;
    if (child = accept(TokenType::kReturnKeyword)) {
        head->addChild(child);
        head->addChild(booleanExpressionAction());
        head->addChild(expect(TokenType::kSemiColon));
    } else {
        error("returnStatement: syntax error");
    }
    return head;
}

std::unique_ptr<Node> ASTParser::logicalOperationAction() {
    std::unique_ptr<Node> head (new LogicalOperationNode());
    std::unique_ptr<Node> child;
    if (child = accept(TokenType::kLogicalAnd) || child = accept(TokenType::kLogicalOr)) {
        head->addChild(child);
    } else {
        error("logicalOperation: syntax error");
    }
    return head;
}

std::unique_ptr<Node> ASTParser::comparisonOperationAction() {
    std::unique_ptr<Node> head (new ComparisonOperationNode());
    std::unique_ptr<Node> child;
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
