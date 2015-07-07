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

#include <iostream>
#include <memory>
#include <stdexcept>

#include "mongo/scripting/tinyjs/ast_parser.h"

namespace mongo {
namespace tinyjs {

ASTParser::ASTParser(std::vector<Token> tokenInput)
    : currentPosition(0), currentToken(tokenInput[0]), tokens(tokenInput) {
    parseTokens(tokens);
}

ASTParser::~ASTParser() {
    this->head.reset();
    this->tokens.clear();
}

std::string ASTParser::traverse() {
    return ((this->head).get())->getValue();
}

void ASTParser::parseTokens(std::vector<Token> tokens) {
    this->head = clauseAction();
}

void ASTParser::nexttoken(void) {
    currentPosition++;
    currentToken = tokens[currentPosition];
}

/**
 * This function takes in a TokenType which should be one of the types that does not correspond
 * to a node in the abstract syntax tree - for example, parentheses, semicolons, "return", etc.
 * If the current token matches the input, it calls nexttoken() and returns true. Otherwise, it
 * returns false.
 */
Boolean matchToken(TokenType t) {
    if (currentToken.type == t) {
        if (currentPosition < (int)tokens.size()) {
            nexttoken();
        }
        return true;
    }
    return false;
}

/**
 * This function takes in a TokenType which should be one of the types that does correspond
 * to a node in the abstract syntax tree: kNullLiteral, kUndefinedLiteral, kIntegerLiteral,
 * kFloatLiteral, kBooleanLiteral, kStringLiteral, kIdentifier
 * If the current token matches the input, it calls nexttoken() and returns a node corresponding to
 * the token. Otherwise, it
 * returns NULL.
 */
std::unique_ptr<Node> ASTParser::matchNode(TokenType t) {
    if (currentToken.type == t) {
        std::unique_ptr<Node> leaf = std::move(makeTerminalNode(currentToken));
        if (currentPosition < (int)tokens.size()) {
            nexttoken();
        }
        return leaf;
    }
    return NULL;
}

/** 
 * This function takes in a TokenType which should be one of the types that does correspond
 * to a node in the abstract syntax tree: kNullLiteral, kUndefinedLiteral, kIntegerLiteral,
 * kFloatLiteral, kBooleanLiteral, kStringLiteral, kIdentifier. It creates and returns a corresponding TerminalNode.
 */
std::unique_ptr<TerminalNode> makeTerminalNode(Token token) {
    switch (token.type) {
        case kNullLiteral:
            std_unique_ptr<TerminalNode> node(new TerminalNode(BSONNULL));
            break;
        case kUndefinedLiteral:
            std_unique_ptr<TerminalNode> node(new TerminalNode(BSONUndefined));
            break;
        case kIntegerLiteral:
            std_unique_ptr<TerminalNode> node(new TerminalNode(std::stoi(token.value)));
            break;
        case kFloatLiteral:
            std_unique_ptr<TerminalNode> node(new TerminalNode(std::stof(token.value)));
            break;
        case kBooleanLiteral:
            bool boolValue = (token.value == "true");
            std_unique_ptr<TerminalNode> node(new TerminalNode(boolValue));
            break;
        case kStringLiteral:
            std_unique_ptr<TerminalNode> node(new TerminalNode(token.value));
            break;
        break;
        case kIdentifier:
            std_unique_ptr<TerminalNode> node(new TerminalNode(token.value, true));
            break;
        default:
            return NULL;
            break;
    }
    return node;
}

// Overloads acceptIf to search non-terminals
std::unique_ptr<Node> ASTParser::tryProductionMatch(std::function<std::unique_ptr<Node>(void)> action) {
    int resetPosition = currentPosition;
    try {
        // std::unique_ptr<Node> subTreeHead = action();
        return action();
    } catch (const std::invalid_argument &e) {
        currentPosition = resetPosition;
        currentToken = tokens[currentPosition];
        return NULL;
    }
}

std::unique_ptr<Node> ASTParser::expect(TokenType t) {
    std::unique_ptr<Node> leaf = this->acceptIf(t);
    if (leaf == NULL) {
        throw ParseException("expected different token", currentToken); //TODO better error message
    }
    return leaf;
}

/** 
 * Each of the following functions represents a production rule, described in its header comment.
 * TODO: more explanation of the functions 
 */

/**
 * clause: 
 *        'function() {' returnStatement '}'
 *      | returnStatement
 */
std::unique_ptr<Node> ASTParser::clauseAction() {
    std::unique_ptr<Node> head;
    if (matchToken(TokenType::kFunctionKeyword)) {
        expect(TokenType::kOpenParen);
        expect(TokenType::kCloseParen);
        expect(TokenType::kOpenCurlyBrace);
        head = std::move(returnStatementAction());
        expect(TokenType::kCloseCurlyBrace);
    } else if ((head = tryProductionMatch(std::bind(&ASTParser::returnStatementAction, this)))) {
        ;
    } else {
        throw ParseException("clause", currentToken);
    }
    return head;
}

/**
 * variable: 
 *        identifier
 *      | objectAccessor
 */
std::unique_ptr<Node> ASTParser::variableAction() {
    std::unique_ptr<Node> leftChild;
    leftChild = std::move(matchNode(TokenType::kIdentifier);
    return std::move(objectAccessorAction(std::move(leftChild)));
}

/**
 * objectAccessor: 
 *        '.' identifier objectAccessor
 *      | '[' arithmeticExpression ']' objectAccessor
 *      | OPTIONAL
 */
std::unique_ptr<Node> ASTParser::objectAccessorAction() {
    std::unique_ptr<Node> head;

    if ((matchToken(TokenType::kPeriod))) {
        head = make_unique<ObjectAccessorNode>(kPeriod); 

        head->setLeftChild(std::move(leftChild));
        head->setRightChild(std::move(matchNode(kIdentifier)));
        

    } else if ((tokenMatch(TokenType::kOpenSquareBracket))) {
        head = make_unique<ArithmeticNode>(kOpenSquareBracket));
        head->setLeftChild(std::move(leftChild));
        head->setRightChild(std::move(std::bind(&ASTParser::arithmeticExpressionAction, this)));
        expect(TokenType::kCloseSquareBracket);
    } else {
        // multiplicativeOperation is optional, so if it doesn't match, just return leftChild
        return leftChild;
    }

    return std::move(objectAccessorAction(head));
}

/**
 * termAction: 
 *        integer
 *      | float
 *      | string
 *      | variable
 *      | boolean
 */
std::unique_ptr<Node> ASTParser::termAction() {
    std::unique_ptr<Node> head;
    if ((head = matchNode(TokenType::kIntegerLiteral)) ||
        (head = matchNode(TokenType::kFloatLiteral)) ||
        (head = matchNode(TokenType::kStringLiteral)) ||
        (head = tryProductionMatch(std::bind(&ASTParser::variableAction, this))) ||
        (head = matchNode(TokenType::kBooleanLiteral))) {
        ;
    } else {
        throw ParseException("term", currentToken);
    }
    return head;
}

/**
 * arrayLiteral: 
 *        []
 *      | [arrayElements]
 */
std::unique_ptr<Node> ASTParser::arrayLiteralAction() {
    std::unique_ptr<Node> head(new ArrayLiteralNode());
    std::unique_ptr<Node> child;
    if (matchToken(TokenType::kOpenSquareBracket)) {
        if (matchToken(TokenType::kCloseSquareBracket)) {
            ; // Case where array is empty
        } else {
            // At this point we can assume there is at least one element in the array
            std::vector<std::unique_ptr<Node> > elements = arrayElements();
            head.setChildren(elements);
        }
    } else {
        throw ParseException("array literal", currentToken);
    }
    return head;
}

/**
 * arrayElements: 
 *        booleanExpression (',' booleanExpression)*
 */
std::vector<std::unique_ptr<Node> > arrayElements() {
    std::vector<std::unique_ptr<Node> > elements;
    elements.push_back(std::move(booleanExpressionAction()));
    while (!(matchToken(TokenType::kCloseSquareBracket))) {
        expect(TokenType::kComma);
        elements.push_back(std::move(booleanExpressionAction()));
    }
    return elements;
}

/**
 * arrayAccessor: 
 *        identifier '[' (integer | identifier | arithmeticExpression) ']'
 */
std::unique_ptr<Node> ASTParser::arrayAccessorAction() {
    std::unique_ptr<Node> head(new ArrayAccessorNode());
    std::unique_ptr<Node> leftChild;
    if ((leftChild = matchNode(TokenType::kIdentifier))) {
        head->setLeftChild(std::move(child));
        expect(TokenType::kOpenSquareBracket);
        std::unique_ptr<Node> rightChild;
        if ((rightChild = matchNode(TokenType::kIntegerLiteral)) ||
            (rightChild = matchNode(TokenType::kIdentifier)) ||
            (rightChild = tryProductionMatch(std::bind(&ASTParser::arithmeticExpressionAction, this)))) {
            head->setRightChild(std::move(rightChild));
        } else {
            throw ParseException("array accessor", currentToken);
        }
        expect(TokenType::kCloseSquareBracket);
    } else {
        throw ParseException("array accessor", currentToken);
    }
    return head;
}

/**
 * factor: 
 *        term
 *      | '(' arithmeticExpression ')'
 */
std::unique_ptr<Node> ASTParser::factorAction() {
    std::unique_ptr<Node> head;
    if ((head = tryProductionMatch(std::bind(&ASTParser::termAction, this)))) {
        ;
    } else if (matchToken(TokenType::kOpenParen)) {
        head = std::move(arithmeticExpressionAction());
        expect(TokenType::kCloseParen);
    } else {
        throw ParseException("factor", currentToken);
    }
    return head;
}

/**
 * multiplicativeExpression: 
 *        factor multiplicativeOperation
 */
std::unique_ptr<Node> ASTParser::multiplicativeExpressionAction() {
    std::unique_ptr<Node> leftChild;
    leftChild = std::move(factorAction());
    return std::move(multiplicativeOperationAction(std::move(leftChild)));
}

/**
 * multiplicativeOperation: 
 *        ('*' | '/') factor multiplicativeOperation
 *      | OPTIONAL
 */
std::unique_ptr<Node> ASTParser::multiplicativeOperationAction(std::unique_ptr<Node> leftChild) {
    std::unique_ptr<Node> head;

    if ((tokenMatch(TokenType::kMultiply))) {
        head = make_unique<ArithmeticNode>(kMultiply); //TODO make sure that this won't mess up precedence by having all the math ops Arithmetic
    } else if ((tokenMatch(TokenType::kDivide))) {
        head = make_unique<ArithmeticNode>(kDivide));
    } else {
        // multiplicativeOperation is optional, so if it doesn't match, just return leftChild
        return leftChild;
    }

    head->setLeftChild(std::move(leftChild));
    head->setRightChild(std::move(factorAction()));
    return std::move(multiplicativeOperationAction(head));

}

/**
 * arithmeticExpression: 
 *        multiplicativeExpression arithmeticOperation
 */
std::unique_ptr<Node> ASTParser::arithmeticExpressionAction() {
    std::unique_ptr<Node> leftChild;
    leftChild = std::move(multiplicativeExpressionAction());
    return std::move(arithmeticOperationAction(std::move(leftChild)));
}

/**
 * arithmeticOperation: 
 *        ('+' | '-') multiplicativeExpression arithmeticOperation
 *      | OPTIONAL
 */
std::unique_ptr<Node> ASTParser::arithmeticOperationAction(std::unique_ptr<Node> leftChild) {
    std::unique_ptr<Node> head;

    if ((tokenMatch(TokenType::kAdd))) {
        head = make_unique<ArithmeticNode>(kAdd); //TODO make sure that this won't mess up precedence by having all the math ops Arithmetic
    } else if ((tokenMatch(TokenType::kSubtract))) {
        head = make_unique<ArithmeticNode>(kSubtract));
    } else {
        // arithmeticOperation is optional, so if it doesn't match, just return leftChild
        return leftChild;
    }

    head->setLeftChild(std::move(leftChild));
    head->setRightChild(std::move(multiplicativeExpressionAction()));
    return std::move(arithmeticOperationAction(head));
}

/**
 * booleanFactor: 
 *        arithmeticExpression
 *      | '(' booleanExpression ')'
 */
std::unique_ptr<Node> ASTParser::booleanFactorAction() {
    std::unique_ptr<Node> head;
    if ((head = tryProductionMatch(std::bind(&ASTParser::arithmeticExpressionAction, this)))) {
        ;
    } else {
        expect(TokenType::kOpenParen);
        head = std::move(booleanExpressionAction());
        expect(TokenType::kCloseParen);
    }
    return head;
}

/**
 * relationalExpression: 
 *        booleanFactor relationalOperation
 */
std::unique_ptr<Node> ASTParser::relationalExpressionAction() {
    std::unique_ptr<Node> leftChild;
    leftChild = std::move(booleanFactorAction());
    return std::move(relationalOperationAction(std::move(leftChild)));
}

/**
 * relationalOperation: 
 *        ('===' | '==' | '>' | '>=' | '<' | '>=') booleanFactor relationalOperation
 *      | OPTIONAL
 */
std::unique_ptr<Node> ASTParser::relationalOperationAction(std::unique_ptr<Node> leftChild) {
    std::unique_ptr<Node> head;

    if ((tokenMatch(TokenType::kTripleEquals))) {
        head = make_unique<ComparisonNode>(kTripleEquals); 
    } else if ((tokenMatch(TokenType::kDoubleEquals))) {
        head = make_unique<ComparisonNode>(kDoubleEquals));
    } else if ((tokenMatch(TokenType::kGreaterThan))) {
        head = make_unique<ComparisonNode>(kGreaterThan));
    } else if ((tokenMatch(TokenType::kGreaterThanEquals))) {
        head = make_unique<ComparisonNode>(kGreaterThanEquals));
    } else if ((tokenMatch(TokenType::kLessThan))) {
        head = make_unique<ComparisonNode>(kLessThan));
    } else if ((tokenMatch(TokenType::kLessThanEquals))) {
        head = make_unique<ComparisonNode>(kLessThanEquals));
    } else {
        // relationalOperation is optional, so if it doesn't match, just return leftChild
        return leftChild;
    }

    head->setLeftChild(std::move(leftChild));
    head->setRightChild(std::move(booleanFactorAction()));
    return std::move(relationalOperationAction(head));
}

/**
 * booleanExpression: 
 *        relationalExpression booleanOperation
 */
std::unique_ptr<Node> ASTParser::booleanExpressionAction() {
    std::unique_ptr<Node> leftChild;
    leftChild = std::move(relationalExpressionAction());
    return std::move(booleanOperationAction(std::move(leftChild)));
}

/**
 * booleanOperation: 
 *        ('&&' | '||') booleanExpression booleanOperation
        | ternaryOperation booleanOperation
 *      | OPTIONAL
 */
std::unique_ptr<Node> ASTParser::booleanOperationAction(std::unique_ptr<Node> leftChild) {
    std::unique_ptr<Node> head;

    if (head = tryProductionMatch(std::bind(&ASTParser::ternaryOperationAction, this))) {
        head->setChild1(leftChild);
    } else if ((tokenMatch(TokenType::kLogicalAnd))) {
        head = make_unique<ComparisonNode>(kLogicalAnd);
        head->setLeftChild(std::move(leftChild));
        head->setRightChild(std::move(booleanExpressionAction())); 
    } else if ((tokenMatch(TokenType::kLogicalOr))) {
        head = make_unique<ComparisonNode>(kLogicalOr));
        head->setLeftChild(std::move(leftChild));
        head->setRightChild(std::move(booleanExpressionAction())); 
    } else {
        // booleanOperation is optional, so if it doesn't match, just return leftChild
        return leftChild;
    }

    return std::move(booleanOperationAction(head));
}

/**
 * ternaryOperation: 
 *        '?' booleanExpression ':' booleanExpression
 */
std::unique_ptr<Node> ASTParser::ternaryOperationAction() { //TODO: should this be structured more like returnStatementAction?
    std::unique_ptr<Node> head(new TernaryOperationNode());
    expect(TokenType::kQuestionMark);
    head->setChild2(std::move(booleanExpressionAction()));
    expect(TokenType::kColon);
    head->setChild3(std::move(booleanExpressionAction()));
    return head;
}

/**
 * returnStatement: 
 *        'return' booleanExpression ';'
 */
std::unique_ptr<Node> ASTParser::returnStatementAction() {
    std::unique_ptr<Node> head(new ReturnStatementNode());
    if (matchToken(TokenType::kReturnKeyword)) {
        head->setChild(std::move(booleanExpressionAction()));
        expect(TokenType::kSemiColon);
    } else {
        throw ParseException("return statement", currentToken);
    }
    return head;
}

}
}
