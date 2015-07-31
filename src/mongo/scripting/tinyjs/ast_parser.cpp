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
#include "mongo/bson/bsontypes.h"
#include "mongo/scripting/tinyjs/ast_parser.h"
#include "mongo/stdx/memory.h"
#include "mongo/util/mongoutils/str.h"

namespace mongo {
namespace tinyjs {

ASTParser::ASTParser(std::vector<Token> tokenInput)
    : _currentPosition(0), _currentToken(tokenInput[0]), _tokens(tokenInput) {
    parseTokens(_tokens);
}

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

Value ASTParser::evaluate(Scope* s) {
    Value returnValue;
    returnValue = Value(BSONUndefined);
    this->_head->evaluate(s, returnValue);
    return returnValue;
}

void ASTParser::parseTokens(std::vector<Token> tokens) {
    this->_head = clauseAction();
}

void ASTParser::nexttoken() {
    _currentPosition++;
    _currentToken = _tokens[_currentPosition];
}

bool ASTParser::matchImplicitTerminal(TokenType t) {
    if (_currentToken.type == t) {
        if (_currentPosition < (int)_tokens.size()) {
            nexttoken();
        }
        return true;
    }
    return false;
}

std::unique_ptr<Node> ASTParser::matchNodeTerminal(TokenType t) {
    if (_currentToken.type == t) {
        std::unique_ptr<Node> leaf = makeTerminalNode(_currentToken);
        if (_currentPosition < (int)_tokens.size()) {
            nexttoken();
        }
        return leaf;
    }
    return nullptr;
}

std::unique_ptr<TerminalNode> ASTParser::makeTerminalNode(Token token) {
    std::unique_ptr<TerminalNode> node;
    switch (token.type) {
        case TokenType::kNullLiteral:
            node.reset((new TerminalNode(BSONNULL)));
            break;
        case TokenType::kUndefinedLiteral: {
            node.reset((new TerminalNode(BSONUndefined)));
            break;
        }
        case TokenType::kIntegerLiteral:
            node.reset((new TerminalNode(std::stoi(token.value.rawData()))));
            break;
        case TokenType::kFloatLiteral: {
            if (token.value.toString() == "NaN") {
                node.reset((new TerminalNode(std::nan(""))));
            }
            else if (token.value.toString() == "Infinity") {
                node.reset((new TerminalNode(std::numeric_limits<double>::infinity())));
            } else {
                node.reset((new TerminalNode(std::stod(token.value.rawData()))));
            }
            break;
        }
        case TokenType::kBooleanLiteral: {
            bool boolValue = (token.value == "true");
            node.reset((new TerminalNode(boolValue)));
            break;
        } 
        case TokenType::kStringLiteral:
            node.reset((new TerminalNode(token.value)));
            break;
        case TokenType::kIdentifier:
            node.reset((new Identifier(token.value)));
            break;
        default:
            throw ParseException("making terminal node for invalid terminal ", (token.value).toString()); 
            break;
    }
    return node;
}

std::unique_ptr<Node> ASTParser::tryProductionMatch(std::function<std::unique_ptr<Node>()> action) {
    int resetPosition = _currentPosition;
    try {
        return action();
    } catch (ParseException& e) {
        _currentPosition = resetPosition;
        _currentToken = _tokens[_currentPosition];
        return nullptr;
    }
}

std::string getName(TokenType t) {
    std::string names[36] = {"kThisIdentifier",
                             "kReturnKeyword",
                             "kNullLiteral",
                             "kUndefinedLiteral",
                             "kFunctionKeyword",
                             "kIntegerLiteral",
                             "kFloatLiteral",
                             "kBooleanLiteral",
                             "kStringLiteral",
                             "kIdentifier",
                             "kAdd",
                             "kSubtract",
                             "kMultiply",
                             "kDivide",
                             "kTripleEquals",
                             "kDoubleEquals",
                             "kLessThan",
                             "kLessThanEquals",
                             "kGreaterThan",
                             "kGreaterThanEquals",
                             "kNotEquals",
                             "kDoubleNotEquals",
                             "kLogicalAnd",
                             "kLogicalOr",
                             "kLogicalNot",
                             "kSemiColon",
                             "kOpenParen",
                             "kCloseParen",
                             "kQuestionMark",
                             "kColon",
                             "kPeriod",
                             "kComma",
                             "kOpenSquareBracket",
                             "kCloseSquareBracket",
                             "kOpenCurlyBrace",
                             "kCloseCurlyBrace"};
    return names[checked_cast<int>(t)];
}

void ASTParser::expectImplicitTerminal(TokenType t) {
    if (!(matchImplicitTerminal(t))) {
        throw ParseException("expected " + getName(t), (_currentToken.value).toString());  
    }
}

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
        head = blockAction();
        expectImplicitTerminal(TokenType::kCloseCurlyBrace);
    } else {
        head = blockAction();
    } 
    std::cout << "returning from clause" << std::endl;
    return head;
}

std::unique_ptr<Node> ASTParser::statementOrLoopAction() {
    std::unique_ptr<Node> head;
    std::cout << "looking for a statement or loop" << std::endl;
    if ((head = tryProductionMatch((std::bind(&ASTParser::statementAction, this))))) {
        std::cout << "found a statement" << std::endl;
    } else if ((head = tryProductionMatch((std::bind(&ASTParser::whileLoopAction, this))))) {
        std::cout << "found a while loop" << std::endl;
    } 
    std::cout << "returning from statement or loop action " << std::endl;
    return head;
}

/**
 * block: [booleanExpression*] returnStatement
 */

std::unique_ptr<Node> ASTParser::blockAction() {
    std::unique_ptr<Node> head = stdx::make_unique<Block>(TokenType::kCloseSquareBracket);
    std::unique_ptr<Node> statement;
    while ((_currentPosition < (int)_tokens.size()) &&
           (statement = tryProductionMatch((std::bind(&ASTParser::statementOrLoopAction, this))))) {
        std::cout << "adding a child to block" << std::endl;
        (checked_cast<Block*>(head.get()))->setChild(std::move(statement));
        std::cout << "added a child to block" << std::endl;
    }
    return head;
}

std::unique_ptr<Node> ASTParser::statementAction() {
    std::unique_ptr<Node> head;
    if ((head = tryProductionMatch((std::bind(&ASTParser::returnStatementAction, this))))) {
        std::cout << "found a return statement" << std::endl;;
    } else if ((head = tryProductionMatch((std::bind(&ASTParser::noVarAssignmentAction, this))))) {
        ;
    } else if ((head = tryProductionMatch((std::bind(&ASTParser::varAssignmentAction, this))))) {
        ;
    } else {
        head = booleanExpressionAction();
        expectImplicitTerminal(TokenType::kSemiColon);
    }
    return head;
}

std::unique_ptr<Node> ASTParser::varAssignmentAction() {
    std::unique_ptr<Node> head = stdx::make_unique<AssignmentOperator>(true);
    expectImplicitTerminal(TokenType::kVarKeyword);
    std::unique_ptr<Node> leftChild;
    if ((leftChild = matchNodeTerminal(TokenType::kIdentifier))) {
        (checked_cast<AssignmentOperator*>(head.get()))->setLeftChild(std::move(leftChild));
    } else {
        throw ParseException("expected a variable", (_currentToken.value).toString());
    }

    expectImplicitTerminal(TokenType::kAssignment);
    (checked_cast<AssignmentOperator*>(head.get()))
        ->setRightChild(std::move(booleanExpressionAction()));
    expectImplicitTerminal(TokenType::kSemiColon);

    return head;
}

std::unique_ptr<Node> ASTParser::noVarAssignmentAction() {
    std::unique_ptr<Node> head = stdx::make_unique<AssignmentOperator>(false);
    std::unique_ptr<Node> leftChild;
    if ((leftChild = matchNodeTerminal(TokenType::kIdentifier))) {
        (checked_cast<AssignmentOperator*>(head.get()))->setLeftChild(std::move(leftChild));
    } else {
        throw ParseException("expected a variable", (_currentToken.value).toString());
    }

    expectImplicitTerminal(TokenType::kAssignment);
    (checked_cast<AssignmentOperator*>(head.get()))
        ->setRightChild(std::move(booleanExpressionAction()));
    expectImplicitTerminal(TokenType::kSemiColon);

    return head;
}


std::unique_ptr<Node> ASTParser::whileLoopAction() {
    std::unique_ptr<Node> head(new WhileLoop(TokenType::kWhileKeyword));
    expectImplicitTerminal(TokenType::kWhileKeyword);
    expectImplicitTerminal(TokenType::kOpenParen);
    (checked_cast<WhileLoop*>(head.get()))->setCondition(booleanExpressionAction());
    expectImplicitTerminal(TokenType::kCloseParen);  
    expectImplicitTerminal(TokenType::kOpenSquareBracket);
    (checked_cast<WhileLoop*>(head.get()))->setBlock(blockAction());
    expectImplicitTerminal(TokenType::kCloseSquareBracket);
    return head;
}





/**
 * variable:
 *        identifier
 *      | objectAccessor
 */
std::unique_ptr<Node> ASTParser::variableAction() {
    std::unique_ptr<Node> leftChild;
    leftChild = matchNodeTerminal(TokenType::kIdentifier);
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
        std::unique_ptr<BinaryOperator> head(new ObjectAccessorOperator(TokenType::kPeriod));
        head->setLeftChild(std::move(leftChild));
        head->setRightChild(std::move(matchNodeTerminal(TokenType::kIdentifier)));
        std::string path = head->getLeftChild()->getName().toString() + "." + head->getRightChild()->getName().toString();
        (checked_cast<ObjectAccessorOperator*>(head.get()))->setPath(path);
        return objectAccessorAction(std::move(head));
    } else if ((matchImplicitTerminal(TokenType::kOpenSquareBracket))) {
        std::unique_ptr<BinaryOperator> head(new ObjectAccessorOperator(TokenType::kOpenSquareBracket));
        head->setLeftChild(std::move(leftChild));
        head->setRightChild(std::move(arithmeticExpressionAction()));
        expectImplicitTerminal(TokenType::kCloseSquareBracket);
        std::string path = "[";
        (checked_cast<ObjectAccessorOperator*>(head.get()))->setPath(path);
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
    if ((head = tryProductionMatch(std::bind(&ASTParser::arrayLiteralAction, this))) ||
        (head = tryProductionMatch(std::bind(&ASTParser::numberAction, this))) ||
        (head = matchNodeTerminal(TokenType::kStringLiteral)) ||
        (head = tryProductionMatch(std::bind(&ASTParser::variableAction, this))) ||
        (head = matchNodeTerminal(TokenType::kBooleanLiteral)) ||
        (head = matchNodeTerminal(TokenType::kNullLiteral)) ||
        (head = matchNodeTerminal(TokenType::kUndefinedLiteral))) {
    } else {
        throw ParseException("expected a term", (_currentToken.value).toString());
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
        throw ParseException("expected a number", (_currentToken.value).toString());
    }
    return head;
}

/**
 * arrayLiteral:
 *        []
 *      | [arrayElements]
 */
std::unique_ptr<Node> ASTParser::arrayLiteralAction() {
    std::unique_ptr<Node> head(new ArrayLiteral(TokenType::kCloseSquareBracket));
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
        throw ParseException("expected an array literal", (_currentToken.value).toString());
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
        head = arithmeticExpressionAction();
        expectImplicitTerminal(TokenType::kCloseParen);
    } else {
        throw ParseException("expected a factor", (_currentToken.value).toString());
    }
    return head;
}

/**
 * multiplicativeExpression:
 *        factor multiplicativeOperation
 */
std::unique_ptr<Node> ASTParser::multiplicativeExpressionAction() {
    std::unique_ptr<Node> leftChild;
    leftChild = factorAction();
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
        head.reset(new MultiplicationOperator());
    } else if ((matchImplicitTerminal(TokenType::kDivide))) {
        head.reset(new DivisionOperator());
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
    leftChild = multiplicativeExpressionAction();
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
        head.reset(new AdditionOperator());
    } else if ((matchImplicitTerminal(TokenType::kSubtract))) {
        head.reset(new SubtractionOperator());
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
        head = booleanExpressionAction();
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
    leftChild = booleanFactorAction();
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
        head.reset(new TripleEqualsOperator());
    } else if ((matchImplicitTerminal(TokenType::kDoubleEquals))) {
        head.reset(new DoubleEqualsOperator());
    } else if ((matchImplicitTerminal(TokenType::kGreaterThan))) {
        head.reset(new GreaterThanOperator());
    } else if ((matchImplicitTerminal(TokenType::kGreaterThanEquals))) {
        head.reset(new GreaterThanEqualsOperator());
    } else if ((matchImplicitTerminal(TokenType::kLessThan))) {
        head.reset(new LessThanOperator());
    } else if ((matchImplicitTerminal(TokenType::kLessThanEquals))) {
        head.reset(new LessThanEqualsOperator());
    } else if ((matchImplicitTerminal(TokenType::kNotEquals))) {
        head.reset(new NotEqualsOperator());
    } else if ((matchImplicitTerminal(TokenType::kDoubleNotEquals))) {
        head.reset(new DoubleNotEqualsOperator());
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
    leftChild = relationalExpressionAction();
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
    // TODO: can we do this without checked_cast?
    std::unique_ptr<Node> head;
    std::function<std::unique_ptr<Node>()> fn = [this]() { return ternaryOperationAction(); };
    if ((head = tryProductionMatch(fn))) {
        checked_cast<TernaryOperator*>(head.get())->setLeftChild(std::move(leftChild));
    } else if ((matchImplicitTerminal(TokenType::kLogicalAnd))) {
        head.reset(new LogicalAndOperator());
        checked_cast<BinaryOperator*>(head.get())->setLeftChild(std::move(leftChild));
        checked_cast<BinaryOperator*>(head.get())
            ->setRightChild(std::move(booleanExpressionAction()));
    } else if ((matchImplicitTerminal(TokenType::kLogicalOr))) {
        head.reset(new LogicalOrOperator());
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

    auto head = stdx::make_unique<TernaryOperator>(TokenType::kQuestionMark);
    expectImplicitTerminal(TokenType::kQuestionMark);
    head->setMiddleChild(std::move(booleanExpressionAction()));
    expectImplicitTerminal(TokenType::kColon);
    head->setRightChild(std::move(booleanExpressionAction()));
    return std::unique_ptr<Node>(head.release());

}

/**
 * returnStatement:
 *        'return' booleanExpression ';'
 */
std::unique_ptr<Node> ASTParser::returnStatementAction() {
    auto head = stdx::make_unique<UnaryOperator>(TokenType::kReturnKeyword);
    if (matchImplicitTerminal(TokenType::kReturnKeyword)) {
        head->setChild(std::move(booleanExpressionAction()));
        expectImplicitTerminal(TokenType::kSemiColon); // semicolon is optional
    } else {
        throw ParseException("expected 'return'", (_currentToken.value).toString());
    }
    return std::unique_ptr<Node>(head.release());
}
} // namespace tinyjs
} // namespace mongo
