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
#include "mongo/base/checked_cast.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <queue>

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
    std::stringstream output;
    output << "TEST" << std::endl;
    std::queue<Node *> nodes;
    nodes.push(head.get());
    nodes.push(NULL); //marker that the level is finished; indicates an endl should be added
    while (!nodes.empty()) {
        Node* node = nodes.front();
        //case where level has ended, endl added
        if (node == NULL) {
            output << std::endl;
            continue;
        }
        //otherwise, add the name of this node and add its children
        output << node->getName() << " ";
        std::vector<Node *> children = node->getChildren();
        for (size_t i = 0; i<children.size(); i++) {
            nodes.push(children[i]);
        }
        nodes.pop();
        nodes.push(NULL); //marker that the level is finished; indicates an endl should be added
    }
    return output.str(); //TODO
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
bool ASTParser::matchImplicitTerminal(TokenType t) {
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
std::unique_ptr<Node> ASTParser::matchNodeTerminal(TokenType t) {
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
std::unique_ptr<TerminalNode> ASTParser::makeTerminalNode(Token token) {
    std::unique_ptr<TerminalNode> node;
    switch (token.type) {
        case TokenType::kNullLiteral:
            node.reset((new TerminalNode(BSONNULL)));
            break;
        case TokenType::kUndefinedLiteral:
            node.reset((new TerminalNode(BSONUndefined)));
            break;
        case TokenType::kIntegerLiteral:
            node.reset((new TerminalNode(std::stoi(token.value.rawData()))));
            break;
        case TokenType::kFloatLiteral:
            node.reset((new TerminalNode(std::stod(token.value.rawData()))));
            break;
        case TokenType::kBooleanLiteral: {
            bool boolValue = (token.value == "true");
            node.reset((new TerminalNode(boolValue))); 
        }
            break;
        case TokenType::kStringLiteral:
            node.reset((new TerminalNode(token.value.rawData())));
            break;
        break;
        case TokenType::kIdentifier:
            node.reset((new TerminalNode(token.value.rawData(), true)));
            break;
        default:
            return NULL;
            break;
    }
    return node;
}

// Overloads acceptIf to search non-terminals
std::unique_ptr<Node> ASTParser::tryProductionMatch(std::function<std::unique_ptr<Node> ()> action) {
    int resetPosition = currentPosition;
    try {
        // std::unique_ptr<Node> subTreeHead = action();
        return action();
    } catch (ParseException &e) {
        currentPosition = resetPosition;
        currentToken = tokens[currentPosition];
        return NULL;
    }
}

void ASTParser::expectImplicitTerminal(TokenType t) {
    if (!(matchImplicitTerminal(t))) {
        throw ParseException("expected different token", currentToken); //TODO better error message
    }
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
    if (matchImplicitTerminal(TokenType::kFunctionKeyword)) {
        expectImplicitTerminal(TokenType::kOpenParen);
        expectImplicitTerminal(TokenType::kCloseParen);
        expectImplicitTerminal(TokenType::kOpenCurlyBrace);
        head = std::move(returnStatementAction());
        expectImplicitTerminal(TokenType::kCloseCurlyBrace);
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
    leftChild = std::move(matchNodeTerminal(TokenType::kIdentifier));
    std::cout << "just calculated variable left child" << (leftChild ? "not null" : "null") << std::endl;
    std::cout << "in variable after left child; about to look at  in objAc " << currentToken.value << std::endl;
    std::unique_ptr<Node> res =  objectAccessorAction(std::move(leftChild));
    std::cout << "returning from variable res at " << (res ? "not null" : "null") << std::endl;
    return res;
}

/**
 * objectAccessor: 
 *        '.' identifier objectAccessor
 *      | '[' arithmeticExpression ']' objectAccessor
 *      | OPTIONAL
 */
std::unique_ptr<Node> ASTParser::objectAccessorAction(std::unique_ptr<Node> leftChild) {
    if ((matchImplicitTerminal(TokenType::kPeriod))) {
        std::unique_ptr<BinaryOperator> head(new BinaryOperator(TokenType::kPeriod));
        head->setLeftChild(std::move(leftChild));
        std::cout << "objectAccessor right child " << currentToken.value << std::endl;
        head->setRightChild(std::move(matchNodeTerminal(TokenType::kIdentifier)));

        std::cout << "recursive call to objAc head at " << (head ? "not null" : "null")
                  << std::endl;
        std::unique_ptr<Node> res = objectAccessorAction(std::move(head));
        std::cout << "returning from objAc res at " << (res ? "not null" : "null") << std::endl;
        return res;

    } else if ((matchImplicitTerminal(TokenType::kOpenSquareBracket))) {
        std::unique_ptr<BinaryOperator> head(new BinaryOperator(TokenType::kOpenSquareBracket));
        head->setLeftChild(std::move(leftChild));
        head->setRightChild(std::move(arithmeticExpressionAction()));
        expectImplicitTerminal(TokenType::kCloseSquareBracket);

        std::cout << "recursive call to objAc head at " << (head ? "not null" : "null")
                  << std::endl;
        std::unique_ptr<Node> res = objectAccessorAction(std::move(head));
        std::cout << "returning from objAc res at " << (res ? "not null" : "null") << std::endl;
        return res;
    } else {
        // multiplicativeOperation is optional, so if it doesn't match, just return leftChild
        std::cout << "returning left child; about to look at" << currentToken.value << std::endl;
        return leftChild;
    }
}

/**
 * termAction: 
 *        integer
 *      | float
 *      | string
 *      | variable
 *      | boolean
 *      | array
 */
std::unique_ptr<Node> ASTParser::termAction() {
    std::cout << "in term, looking at" << currentToken.value << std::endl;
    std::unique_ptr<Node> head;
    if ((head = matchNodeTerminal(TokenType::kIntegerLiteral)) ||
        (head = matchNodeTerminal(TokenType::kFloatLiteral)) ||
        (head = matchNodeTerminal(TokenType::kStringLiteral)) ||
        (head = tryProductionMatch(std::bind(&ASTParser::variableAction, this))) ||
        (head = matchNodeTerminal(TokenType::kBooleanLiteral))) { //|| 
        //(head = tryProductionMatch(std::bind(&ASTParser::arrayLiteralAction, this)))) {
        std::cout << "in the if block of term; about to look at" << currentToken.value << std::endl;
    } else {
        throw ParseException(currentToken.value.rawData(), currentToken);
    }
    std::cout << "in term after left child; about to look at" << currentToken.value << std::endl;
    return head;
}

/**
 * arrayLiteral: 
 *        []
 *      | [arrayElements]
 */
std::unique_ptr<Node> ASTParser::arrayLiteralAction() {
    std::unique_ptr<Node> head(new ArrayLiteral(TokenType::kOpenSquareBracket));
    std::unique_ptr<Node> child;
    if (matchImplicitTerminal(TokenType::kOpenSquareBracket)) {
        if (matchImplicitTerminal(TokenType::kCloseSquareBracket)) {
            ; // Case where array is empty
        } else {
            // At this point we can assume there is at least one element in the array
            //std::vector<std::unique_ptr<Node> > elements = arrayElements();
            //(checked_cast<ArrayLiteral*>(head.get()))->setChildren(std::move(arrayElements()));
            (checked_cast<ArrayLiteral*>(head.get()))->setChild(booleanExpressionAction());
            while (!(matchImplicitTerminal(TokenType::kCloseSquareBracket))) {
                expectImplicitTerminal(TokenType::kComma);
                (checked_cast<ArrayLiteral*>(head.get()))->setChild(booleanExpressionAction());
            }
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
/*std::vector<std::unique_ptr<Node> > ASTParser::arrayElements() {
    std::vector<std::unique_ptr<Node> > elements;
    elements.push_back(std::move(booleanExpressionAction()));
    while (!(matchImplicitTerminal(TokenType::kCloseSquareBracket))) {
        expectImplicitTerminal(TokenType::kComma);
        elements.push_back(std::move(booleanExpressionAction()));
    }
    return elements;
}*/

/**
 * factor: 
 *        term
 *      | '(' arithmeticExpression ')'
 */
std::unique_ptr<Node> ASTParser::factorAction() {
    std::unique_ptr<Node> head;
    if ((head = tryProductionMatch(std::bind(&ASTParser::termAction, this)))) {
        std::cout << "in factor after termAction; about to look at" << currentToken.value << std::endl;;
    } else if (matchImplicitTerminal(TokenType::kOpenParen)) {
        head = std::move(arithmeticExpressionAction());
        expectImplicitTerminal(TokenType::kCloseParen);
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
    std::cout << "in mult expression about to look at" << currentToken.value << std::endl;
    std::unique_ptr<Node> leftChild;
    leftChild = std::move(factorAction());
    std::cout << "in multiplicativeExpression after left child; about to look at" << currentToken.value << std::endl;
    return multiplicativeOperationAction(std::move(leftChild));
}

/**
 * multiplicativeOperation: 
 *        ('*' | '/') factor multiplicativeOperation
 *      | OPTIONAL
 */
std::unique_ptr<Node> ASTParser::multiplicativeOperationAction(std::unique_ptr<Node> leftChild) {
    std::unique_ptr<BinaryOperator> head;

    if ((matchImplicitTerminal(TokenType::kMultiply))) {
        head.reset(new BinaryOperator(TokenType::kMultiply)); //TODO make sure that this won't mess up precedence by having all the math ops Arithmetic
    } else if ((matchImplicitTerminal(TokenType::kDivide))) {
        head.reset(new BinaryOperator(TokenType::kDivide));
    } else {
        // multiplicativeOperation is optional, so if it doesn't match, just return leftChild
        return leftChild;
    }

    head->setLeftChild(std::move(leftChild));
    head->setRightChild(std::move(factorAction()));
    return multiplicativeOperationAction(std::move(head));

}

/**
 * arithmeticExpression: 
 *        multiplicativeExpression arithmeticOperation
 */
std::unique_ptr<Node> ASTParser::arithmeticExpressionAction() {
    std::unique_ptr<Node> leftChild;
    leftChild = std::move(multiplicativeExpressionAction());
    std::cout << "in arithmeticExpression after left child; about to look at" << currentToken.value << std::endl;
    std::cout << "calling arithmeticOpAction leftChild at " << (leftChild ? "not null" : "null") << std::endl;
    std::unique_ptr<Node> res = arithmeticOperationAction(std::move(leftChild));
    std::cout << "returning from arithmeticExpressionAction res at " << (res ? "not null" : "null") << std::endl;
    return res;
}

/**
 * arithmeticOperation: 
 *        ('+' | '-') multiplicativeExpression arithmeticOperation
 *      | OPTIONAL
 */
std::unique_ptr<Node> ASTParser::arithmeticOperationAction(std::unique_ptr<Node> leftChild) {
    std::unique_ptr<BinaryOperator> head;

    if ((matchImplicitTerminal(TokenType::kAdd))) {
        head.reset(new BinaryOperator(TokenType::kAdd)); //TODO make sure that this won't mess up precedence by having all the math ops Arithmetic
    } else if ((matchImplicitTerminal(TokenType::kSubtract))) {
       head.reset(new BinaryOperator(TokenType::kSubtract));
    } else {
        // arithmeticOperation is optional, so if it doesn't match, just return leftChild
        return leftChild;
    }

    head->setLeftChild(std::move(leftChild));
    head->setRightChild(std::move(multiplicativeExpressionAction()));
    return arithmeticOperationAction(std::move(head));
}

/**
 * booleanFactor: 
 *        arithmeticExpression
 *      | '(' booleanExpression ')'
 */
std::unique_ptr<Node> ASTParser::booleanFactorAction() {
    std::unique_ptr<Node> head;
    if ((head = tryProductionMatch(std::bind(&ASTParser::arithmeticExpressionAction, this)))) {
        std::cout << "boolean factor match to arithmeticExpression; about to look at " << currentToken.value << std::endl;;
    } else {
        expectImplicitTerminal(TokenType::kOpenParen);
        head = std::move(booleanExpressionAction());
        expectImplicitTerminal(TokenType::kCloseParen);
    }
    std::cout << "returning from booleanFactor head at " << (head ? "not null" : "null") << std::endl;
    return head;
}

/**
 * relationalExpression: 
 *        booleanFactor relationalOperation
 */
std::unique_ptr<Node> ASTParser::relationalExpressionAction() {
    std::cout << "in relationalExpression; about to look at" << currentToken.value << std::endl;
    std::unique_ptr<Node> leftChild;
    leftChild = std::move(booleanFactorAction());
    std::cout << "in relationalExpression after left child; left child is " << (leftChild ? "not null" : "null") << std::endl;
    std::unique_ptr<Node> res = relationalOperationAction(std::move(leftChild));
    std::cout << "in relationalExpression about to return; res is " << (res ? "not null" : "null") << std::endl;
    return res;
}

/**
 * relationalOperation: 
 *        ('===' | '==' | '>' | '>=' | '<' | '>=') booleanFactor relationalOperation
 *      | OPTIONAL
 */
std::unique_ptr<Node> ASTParser::relationalOperationAction(std::unique_ptr<Node> leftChild) {
    std::unique_ptr<Node> head;

    if ((matchImplicitTerminal(TokenType::kTripleEquals))) {
        head.reset(new BinaryOperator(TokenType::kTripleEquals)); 
    } else if ((matchImplicitTerminal(TokenType::kDoubleEquals))) {
        head.reset(new BinaryOperator(TokenType::kDoubleEquals));
    } else if ((matchImplicitTerminal(TokenType::kGreaterThan))) {
        head.reset(new BinaryOperator(TokenType::kGreaterThan));
    } else if ((matchImplicitTerminal(TokenType::kGreaterThanEquals))) {
        head.reset(new BinaryOperator(TokenType::kGreaterThanEquals));
    } else if ((matchImplicitTerminal(TokenType::kLessThan))) {
        head.reset(new BinaryOperator(TokenType::kLessThan));
    } else if ((matchImplicitTerminal(TokenType::kLessThanEquals))) {
        head.reset(new BinaryOperator(TokenType::kLessThanEquals));
    } else {
        // relationalOperation is optional, so if it doesn't match, just return leftChild
        return leftChild;
    }

    checked_cast<BinaryOperator*>(head.get())->setLeftChild(std::move(leftChild));
    checked_cast<BinaryOperator*>(head.get())->setRightChild(std::move(booleanFactorAction()));
    return relationalOperationAction(std::move(head));
}

/**
 * booleanExpression: 
 *        relationalExpression booleanOperation
 */
std::unique_ptr<Node> ASTParser::booleanExpressionAction() {
    std::unique_ptr<Node> leftChild;
    leftChild = std::move(relationalExpressionAction());
    std::cout << "in booleanExpression, relationalExpression has returned " << (leftChild ? "not null" : "null") << std::endl;
    std::unique_ptr<Node> res = booleanOperationAction(std::move(leftChild));
    std::cout << "returning from booleanExpression; res is " << (res ? "not null" : "null") << std::endl;
    return res;
}

/**
 * booleanOperation: 
 *        ('&&' | '||') booleanExpression booleanOperation
        | ternaryOperation booleanOperation
 *      | OPTIONAL
 */
std::unique_ptr<Node> ASTParser::booleanOperationAction(std::unique_ptr<Node> leftChild) {
    std::cout << "received left child in booleanOperation, left child is " << (leftChild ? "not null" : "null") << std::endl;
    std::unique_ptr<Node> head;
    std::function<std::unique_ptr<Node>()> fn =
        [this, &leftChild]() { return ternaryOperationAction(std::move(leftChild));  };
    if ((head = tryProductionMatch(fn))) {
        checked_cast<TernaryOperator*>(head.get())->setLeftChild(std::move(leftChild));
    } /*else if ((matchImplicitTerminal(TokenType::kLogicalAnd))) {
        head.reset(new BinaryOperator(TokenType::kLogicalAnd));
        checked_cast<BinaryOperator*>(head.get())->setLeftChild(std::move(leftChild));
        checked_cast<BinaryOperator*>(head.get())->setRightChild(std::move(booleanExpressionAction()));
    } else if ((matchImplicitTerminal(TokenType::kLogicalOr))) {
        head.reset(new BinaryOperator(TokenType::kLogicalOr));
        checked_cast<BinaryOperator*>(head.get())->setLeftChild(std::move(leftChild));
        checked_cast<BinaryOperator*>(head.get())->setRightChild(std::move(booleanExpressionAction())); 
    }*/ else {
        // booleanOperation is optional, so if it doesn't match, just return leftChild
        std::cout << "returning left child from booleanOperation, left child is " << (leftChild ? "not null" : "null") << std::endl;
        return leftChild;
    }

    return booleanOperationAction(std::move(head));
}

/**
 * ternaryOperation: 
 *        '?' booleanExpression ':' booleanExpression
 */
std::unique_ptr<Node> ASTParser::ternaryOperationAction(std::unique_ptr<Node> leftChild) { //TODO: should this be structured more like returnStatementAction?
    std::cout << "received left child in ternaryOperationAction, left child is " << (leftChild ? "not null" : "null") << std::endl;
    std::unique_ptr<Node> head(new TernaryOperator(TokenType::kQuestionMark));
    expectImplicitTerminal(TokenType::kQuestionMark);
    checked_cast<TernaryOperator*>(head.get())->setMiddleChild(std::move(booleanExpressionAction()));
    expectImplicitTerminal(TokenType::kColon);
    checked_cast<TernaryOperator*>(head.get())->setRightChild(std::move(booleanExpressionAction()));
    std::cout << "returning in ter', left child is " << (leftChild ? "not null" : "null") << std::endl;
    return head;
}

/**
 * returnStatement: 
 *        'return' booleanExpression ';'
 */
std::unique_ptr<Node> ASTParser::returnStatementAction() {
    std::unique_ptr<Node> head(new UnaryOperator(TokenType::kReturnKeyword));
    if (matchImplicitTerminal(TokenType::kReturnKeyword)) {
        checked_cast<UnaryOperator*>(head.get())->setChild(std::move(booleanExpressionAction()));
        expectImplicitTerminal(TokenType::kSemiColon);
    } else {
        throw ParseException("return statement", currentToken);
    }
    return head;
}

}
}
