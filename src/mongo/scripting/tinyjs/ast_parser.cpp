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
#include <queue>
#include <stdexcept>
#include <sstream>

#include "mongo/base/checked_cast.h"
#include "mongo/scripting/tinyjs/ast_parser.h"
#include "mongo/util/mongoutils/str.h"

namespace mongo {
namespace tinyjs {

ASTParser::ASTParser(std::vector<Token> tokenInput)
    : _currentPosition(0), _currentToken(tokenInput[0]), _tokens(tokenInput) {
    parseTokens(_tokens);
}

// TODO: linker error if we don't include destructor
ASTParser::~ASTParser() {
    this->_head.reset();
    this->_tokens.clear();
}

/**
 * This function walks the tree and returns a string of the node names separated by spaces in
 * BFS-traversal order.
 */
std::string ASTParser::traverse() {
    str::stream output;
    std::queue<Node*> nodes;
    nodes.push(_head.get());
    while (!nodes.empty()) {
        Node* node = nodes.front();
        output << node->getName() << " ";
        std::vector<Node*> children = node->getChildren();
        for (size_t i = 0; i < children.size(); i++) {
            nodes.push(children[i]);
        }
        nodes.pop();
    }
    output << "\n";
    return output;
}

void ASTParser::parseTokens(std::vector<Token> tokens) {
    this->_head = clauseAction();
}

void ASTParser::nexttoken(void) {
    _currentPosition++;
    _currentToken = _tokens[_currentPosition];
}

/**
 * This function takes in a TokenType which should be one of the types that does not correspond
 * to a node in the abstract syntax tree - for example, parentheses, semicolons, "return", etc.
 * If the current token matches the input, it calls nexttoken() and returns true. Otherwise, it
 * returns false.
 */
bool ASTParser::matchImplicitTerminal(TokenType t) {
    if (_currentToken.type == t) {
        if (_currentPosition < (int)_tokens.size()) {
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
    if (_currentToken.type == t) {
        std::unique_ptr<Node> leaf = std::move(makeTerminalNode(_currentToken));
        if (_currentPosition < (int)_tokens.size()) {
            nexttoken();
        }
        return leaf;
    }
    return NULL;
}

/**
 * This function takes in a TokenType which should be one of the types that does correspond
 * to a node in the abstract syntax tree: kNullLiteral, kUndefinedLiteral, kIntegerLiteral,
 * kFloatLiteral, kBooleanLiteral, kStringLiteral, kIdentifier. It creates and returns a
 * corresponding TerminalNode.
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
        } break;
        case TokenType::kStringLiteral:
            node.reset((new TerminalNode(token.value)));
            break;
            break;
        case TokenType::kIdentifier:
            node.reset((new TerminalNode(token.value, true)));
            break;
        default:
            return NULL;
            break;
    }
    return node;
}

std::unique_ptr<Node> ASTParser::tryProductionMatch(std::function<std::unique_ptr<Node>()> action) {
    int resetPosition = _currentPosition;
    try {
        // std::unique_ptr<Node> subTreeHead = action();
        return action();
    } catch (ParseException& e) {
        _currentPosition = resetPosition;
        _currentToken = _tokens[_currentPosition];
        return NULL;
    }
}

/**
 * ExpectImplcitTerminal advances to the next token if the current token matches the input
 * and throws an error otherwise. It only checks simple tokens that won't become nodes,
 * because nodes are created by matchNodeTerminal or by calling an action. TODO why exactly don't we
 * need an expect that returns a node?
 */
void ASTParser::expectImplicitTerminal(TokenType t) {
    if (!(matchImplicitTerminal(t))) {
        throw ParseException("expected different token", _currentToken);  // TODO better error
                                                                         // message
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
        throw ParseException("clause", _currentToken);
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
    std::unique_ptr<Node> res = objectAccessorAction(std::move(leftChild));
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
        head->setRightChild(std::move(matchNodeTerminal(TokenType::kIdentifier)));
        return objectAccessorAction(std::move(head));
    } else if ((matchImplicitTerminal(TokenType::kOpenSquareBracket))) {
        std::unique_ptr<BinaryOperator> head(new BinaryOperator(TokenType::kOpenSquareBracket));
        head->setLeftChild(std::move(leftChild));
        head->setRightChild(std::move(arithmeticExpressionAction()));
        expectImplicitTerminal(TokenType::kCloseSquareBracket);
        return objectAccessorAction(std::move(head));
    } else {
        // multiplicativeOperation is optional, so if it doesn't match, just return leftChild
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
    std::unique_ptr<Node> head;
    if ((head = tryProductionMatch(std::bind(&ASTParser::numberAction, this))) ||
        (head = matchNodeTerminal(TokenType::kStringLiteral)) ||
        (head = tryProductionMatch(std::bind(&ASTParser::variableAction, this))) ||
        (head = matchNodeTerminal(TokenType::kBooleanLiteral)) ||
        (head = tryProductionMatch(std::bind(&ASTParser::arrayLiteralAction, this)))) {
    } else {
        throw ParseException(_currentToken.value.rawData(), _currentToken);
    }
    return head;
}

/**
 * numberAction:
 *        - integer
 *      | - float
 *      | integer
 *      | float
 */
std::unique_ptr<Node> ASTParser::numberAction() {
    std::unique_ptr<Node> head;
    bool isNegative = matchImplicitTerminal(TokenType::kSubtract);
    if ((head = matchNodeTerminal(TokenType::kIntegerLiteral)) ||
        (head = matchNodeTerminal(TokenType::kFloatLiteral)))  {
        if (isNegative) {
            UnaryOperator *negative_head = new UnaryOperator(TokenType::kSubtract);
            negative_head->setChild(std::move(head));
            return std::unique_ptr<Node>(negative_head);
        }
    } else {
        throw ParseException(_currentToken.value.rawData(), _currentToken);
    }
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
            ;  // Case where array is empty
        } else {
            // At this point we can assume there is at least one element in the array
            (checked_cast<ArrayLiteral*>(head.get()))->setChild(booleanExpressionAction());
            while (!(matchImplicitTerminal(TokenType::kCloseSquareBracket))) {
                expectImplicitTerminal(TokenType::kComma);
                (checked_cast<ArrayLiteral*>(head.get()))->setChild(booleanExpressionAction());
            }
        }
    } else {
        throw ParseException("array literal", _currentToken);
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
    } else if (matchImplicitTerminal(TokenType::kOpenParen)) {
        head = std::move(arithmeticExpressionAction());
        expectImplicitTerminal(TokenType::kCloseParen);
    } else {
        throw ParseException("factor", _currentToken);
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
        head.reset(new BinaryOperator(TokenType::kMultiply));
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
    std::unique_ptr<Node> res = arithmeticOperationAction(std::move(leftChild));
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
        head.reset(new BinaryOperator(TokenType::kAdd));
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
    } else {
        expectImplicitTerminal(TokenType::kOpenParen);
        head = std::move(booleanExpressionAction());
        expectImplicitTerminal(TokenType::kCloseParen);
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
    std::unique_ptr<Node> res = relationalOperationAction(std::move(leftChild));
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
    std::unique_ptr<Node> res = booleanOperationAction(std::move(leftChild));
    return res;
}

/**
 * booleanOperation:
 *        ('&&' | '||') booleanExpression booleanOperation
        | ternaryOperation booleanOperation
 *      | OPTIONAL
 */
std::unique_ptr<Node> ASTParser::booleanOperationAction(std::unique_ptr<Node> leftChild) {
    std::unique_ptr<Node> head;
    std::function<std::unique_ptr<Node>()> fn = [this]() { return ternaryOperationAction(); };
    if ((head = tryProductionMatch(fn))) {
        checked_cast<TernaryOperator*>(head.get())->setLeftChild(std::move(leftChild));
    } else if ((matchImplicitTerminal(TokenType::kLogicalAnd))) {
        head.reset(new BinaryOperator(TokenType::kLogicalAnd));
        checked_cast<BinaryOperator*>(head.get())->setLeftChild(std::move(leftChild));
        checked_cast<BinaryOperator*>(head.get())
            ->setRightChild(std::move(booleanExpressionAction()));
    } else if ((matchImplicitTerminal(TokenType::kLogicalOr))) {
        head.reset(new BinaryOperator(TokenType::kLogicalOr));
        checked_cast<BinaryOperator*>(head.get())->setLeftChild(std::move(leftChild));
        checked_cast<BinaryOperator*>(head.get())
            ->setRightChild(std::move(booleanExpressionAction()));
    } else {
        // booleanOperation is optional, so if it doesn't match, just return leftChild
        return leftChild;
    }

    std::unique_ptr<Node> res = booleanOperationAction(std::move(head));
    return res;
}

/**
 * ternaryOperation:
 *        '?' booleanExpression ':' booleanExpression
 */
std::unique_ptr<Node> ASTParser::ternaryOperationAction() {  // TODO: should this be structured more
                                                             // like returnStatementAction?
    std::unique_ptr<Node> head(new TernaryOperator(TokenType::kQuestionMark));
    expectImplicitTerminal(TokenType::kQuestionMark);
    checked_cast<TernaryOperator*>(head.get())
        ->setMiddleChild(std::move(booleanExpressionAction()));
    expectImplicitTerminal(TokenType::kColon);
    checked_cast<TernaryOperator*>(head.get())->setRightChild(std::move(booleanExpressionAction()));
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
        throw ParseException("return statement", _currentToken);
    }
    return head;
}
} // namespace tinyjs
} // namespace mongo
