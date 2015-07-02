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
#include "mongo/scripting/tinyjs/clause.h"
#include "mongo/scripting/tinyjs/node.h"

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

std::string traverseSubtree(Node* node) {
    std::string res = node->getName();
    for (std::vector<std::unique_ptr<Node>>::const_iterator it = node->getChildren().begin();
         it != node->getChildren().end(); it++) {
        res += " ";
        res += traverseSubtree((*it).get());
    }
    return res;
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

void ASTParser::error(const char msg[]) {
    throw std::invalid_argument(msg);
}

std::unique_ptr<Node> ASTParser::acceptIf(TokenType t) {
    if (currentToken.type == t) {
        std::unique_ptr<Node> leaf(new TerminalNode(currentToken));
        if (currentPosition < (int)tokens.size()) {
            nexttoken();
        }
        return leaf;
    }
    return NULL;
}

// Overloads acceptIf to search non-terminals
std::unique_ptr<Node> ASTParser::acceptIf(std::function<std::unique_ptr<Node>(void)> action) {
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
        error("expect: unexpected token");
    }
    return leaf;
}

std::unique_ptr<Node> ASTParser::clauseAction() {
    std::unique_ptr<Node> head(new ClauseNode());
    std::unique_ptr<Node> child;
    if ((child = acceptIf(TokenType::kFunctionKeyword))) {
        head->addChild(std::move(child));
        head->addChild(std::move(expect(TokenType::kOpenParen)));
        head->addChild(std::move(expect(TokenType::kCloseParen)));
        head->addChild(std::move(expect(TokenType::kOpenCurlyBrace)));
        head->addChild(std::move(returnStatementAction()));
        head->addChild(std::move(expect(TokenType::kCloseCurlyBrace)));
    } else if ((child = acceptIf(std::bind(&ASTParser::returnStatementAction, this)))) {
        head->addChild(std::move(child));
    } else {
        error("clause: syntax error");
    }
    return head;
}

std::unique_ptr<Node> ASTParser::variableAction() {
    std::unique_ptr<Node> head(new VariableNode());
    std::unique_ptr<Node> child;
    if ((child = acceptIf(TokenType::kIdentifier))) {
        head->addChild(std::move(child));
    } else if ((child = acceptIf(std::bind(&ASTParser::objectAction, this)))) {
        head->addChild(std::move(child));
    } else {
        error("variable: syntax error");
    }
    return head;
}

std::unique_ptr<Node> ASTParser::objectAction() {
    std::unique_ptr<Node> head(new ObjectNode());
    std::unique_ptr<Node> child;
    if ((child = acceptIf(TokenType::kThisIdentifier))) {
        head->addChild(std::move(child));
        head->addChild(std::move(objectAccessorAction()));
    } else if ((child = acceptIf(TokenType::kIdentifier))) {
        head->addChild(std::move(child));
        head->addChild(std::move(objectAccessorAction()));  // TODO: combine these two?
    } else {
        error("object: syntax error");
    }
    return head;
}

std::unique_ptr<Node> ASTParser::objectAccessorAction() {
    std::unique_ptr<Node> head(new ObjectAccessorNode());
    std::unique_ptr<Node> child;
    if ((child = acceptIf(TokenType::kPeriod))) {
        head->addChild(std::move(child));
        head->addChild(std::move(expect(TokenType::kIdentifier)));
        head->addChild(std::move(objectAccessorAction()));
        return head;
    } else if ((child = acceptIf(TokenType::kOpenSquareBracket))) {
        head->addChild(std::move(child));
        std::unique_ptr<Node> child2;
        if ((child2 = acceptIf(std::bind(&ASTParser::arithmeticExpressionAction, this)))) {
            head->addChild(std::move(child2));
            head->addChild(std::move(expect(TokenType::kCloseSquareBracket)));
            head->addChild(std::move(objectAccessorAction()));
            return head;
        } else {
            error("object: syntax error");
        }
    }
    return NULL;
    // optional
}

std::unique_ptr<Node> ASTParser::termAction() {
    std::unique_ptr<Node> head(new TermNode());
    std::unique_ptr<Node> child;
    if ((child = acceptIf(TokenType::kIntegerLiteral)) ||
        (child = acceptIf(TokenType::kFloatLiteral)) ||
        (child = acceptIf(TokenType::kStringLiteral)) ||
        (child = acceptIf(std::bind(&ASTParser::variableAction, this))) ||
        (child = acceptIf(TokenType::kBooleanLiteral))) {
        head->addChild(std::move(child));
    } else {
        error("term: syntax error");
    }
    return head;
}

std::unique_ptr<Node> ASTParser::arrayElementAction() {
    std::unique_ptr<Node> head(new ArrayElementNode());
    std::unique_ptr<Node> child;
    if ((child = acceptIf(std::bind(&ASTParser::termAction, this)))) {
        head->addChild(std::move(child));
    } else if ((child = acceptIf(std::bind(&ASTParser::arithmeticExpressionAction, this)))) {
        head->addChild(std::move(child));
    } else if ((child = acceptIf(std::bind(&ASTParser::booleanExpressionAction, this)))) {
        head->addChild(std::move(child));
    } else {
        error("arrayElement: syntax error");
    }
    return head;
}

std::unique_ptr<Node> ASTParser::arrayLiteralAction() {
    std::unique_ptr<Node> head(new ArrayLiteralNode());
    std::unique_ptr<Node> child;
    if ((child = acceptIf(TokenType::kOpenSquareBracket))) {
        head->addChild(std::move(child));
        std::unique_ptr<Node> child2;
        if ((child2 = acceptIf(TokenType::kCloseSquareBracket))) {
            head->addChild(std::move(child2));
        } else {
            head->addChild(std::move(arrayElementAction()));
            head->addChild(std::move(arrayTailAction()));
            head->addChild(std::move(expect(TokenType::kCloseSquareBracket)));
        }
    } else {
        error("arrayLiteralAction: syntax error");
    }
    return head;
}

std::unique_ptr<Node> ASTParser::arrayTailAction() {
    std::unique_ptr<Node> head(new ArrayTailNode());
    std::unique_ptr<Node> child;
    if ((child = acceptIf(TokenType::kComma))) {
        head->addChild(std::move(child));
        head->addChild(std::move(arrayElementAction()));
        head->addChild(std::move(arrayTailAction()));
        return head;
    }
    return NULL;
    // arrayTail is optional, so if it doesn't match, it's ok
}

std::unique_ptr<Node> ASTParser::arrayIndexedAction() {
    std::unique_ptr<Node> head(new ArrayIndexedNode());
    std::unique_ptr<Node> child;
    if ((child = acceptIf(TokenType::kIdentifier))) {
        head->addChild(std::move(child));
        head->addChild(std::move(expect(TokenType::kOpenSquareBracket)));
        std::unique_ptr<Node> child2;
        if ((child2 = acceptIf(TokenType::kIntegerLiteral)) ||
            (child2 = acceptIf(TokenType::kIdentifier)) ||
            (child2 = acceptIf(std::bind(&ASTParser::arithmeticExpressionAction, this)))) {
            head->addChild(std::move(child2));
        } else {
            error("arrayIndexed: syntax error");
        }
        head->addChild(std::move(expect(TokenType::kCloseSquareBracket)));
    } else {
        error("arrayIndexed: syntax error");
    }
    return head;
}

std::unique_ptr<Node> ASTParser::factorAction() {
    std::unique_ptr<Node> head(new FactorNode());
    std::unique_ptr<Node> child;
    if ((child = acceptIf(std::bind(&ASTParser::termAction, this)))) {
        head->addChild(std::move(child));
    } else if ((child = acceptIf(TokenType::kOpenParen))) {
        head->addChild(std::move(child));
        head->addChild(std::move(arithmeticExpressionAction()));
        head->addChild(std::move(expect(TokenType::kCloseParen)));
    } else {
        error("factor: syntax error");
    }
    return head;
}

std::unique_ptr<Node> ASTParser::multiplicativeExpressionAction() {
    std::unique_ptr<Node> head(new MultiplicativeExpressionNode());
    head->addChild(std::move(factorAction()));
    head->addChild(std::move(multiplicativeOperationAction()));
    return head;
}

std::unique_ptr<Node> ASTParser::multiplicativeOperationAction() {
    std::unique_ptr<Node> head(new MultiplicativeOperationNode());
    std::unique_ptr<Node> child;
    if ((child = acceptIf(TokenType::kMultiply))) {
        head->addChild(std::move(child));
        head->addChild(std::move(factorAction()));
        head->addChild(std::move(multiplicativeOperationAction()));
        return head;
    } else if ((child = acceptIf(TokenType::kDivide))) {
        head->addChild(std::move(child));
        head->addChild(std::move(factorAction()));
        head->addChild(std::move(multiplicativeOperationAction()));
        return head;
    }
    return NULL;
    // multiplicativeOperation is optional, so if it doesn't match, it's ok
}

std::unique_ptr<Node> ASTParser::arithmeticExpressionAction() {
    std::unique_ptr<Node> head(new ArithmeticExpressionNode());
    head->addChild(std::move(multiplicativeExpressionAction()));
    head->addChild(std::move(arithmeticOperationAction()));
    return head;
}

std::unique_ptr<Node> ASTParser::arithmeticOperationAction() {
    std::unique_ptr<Node> head(new ArithmeticOperationNode());
    std::unique_ptr<Node> child;
    if ((child = acceptIf(TokenType::kAdd))) {
        head->addChild(std::move(child));
        head->addChild(std::move(multiplicativeExpressionAction()));
        head->addChild(std::move(arithmeticOperationAction()));
        return head;
    } else if ((child = acceptIf(TokenType::kSubtract))) {
        head->addChild(std::move(child));
        head->addChild(std::move(multiplicativeExpressionAction()));
        head->addChild(std::move(arithmeticOperationAction()));
        return head;
    }
    return NULL;
    // arithmeticOperation is optional, so if it doesn't match, it's ok
}

std::unique_ptr<Node> ASTParser::booleanFactorAction() {
    std::unique_ptr<Node> head(new BooleanFactorNode());
    std::unique_ptr<Node> child;
    if ((child = acceptIf(TokenType::kOpenParen))) {
        head->addChild(std::move(child));
        head->addChild(std::move(booleanExpressionAction()));
        head->addChild(std::move(expect(TokenType::kCloseParen)));
    } else {
        head->addChild(std::move(arithmeticExpressionAction()));
    }
    return head;
}

std::unique_ptr<Node> ASTParser::relationalExpressionAction() {
    std::unique_ptr<Node> head(new RelationalExpressionNode());
    head->addChild(std::move(booleanFactorAction()));
    head->addChild(std::move(relationalOperationAction()));
    return head;
}

std::unique_ptr<Node> ASTParser::relationalOperationAction() {
    std::unique_ptr<Node> head(new RelationalOperationNode());
    std::unique_ptr<Node> child;
    if ((child = acceptIf(std::bind(&ASTParser::comparisonOperationAction, this)))) {
        head->addChild(std::move(child));
        head->addChild(std::move(booleanFactorAction()));
        head->addChild(std::move(relationalOperationAction()));
        return head;
    }
    return NULL;
    // relationalOperation is optional, so if it doesn't match, it's ok
}

std::unique_ptr<Node> ASTParser::booleanExpressionAction() {
    std::unique_ptr<Node> head (new BooleanExpressionNode());
    std::unique_ptr<Node> child;
    if ((child = acceptIf(std::bind(&ASTParser::relationalExpressionAction, this)))) {
        head->addChild(std::move(child));
        head->addChild(std::move(booleanOperationAction()));
    } else {
        error("booleanExpressionAction: syntax error");
    }
    return head;
}

std::unique_ptr<Node> ASTParser::booleanOperationAction() {
    std::unique_ptr<Node> head(new BooleanOperationNode());
    std::unique_ptr<Node> child;
    if ((child = acceptIf(std::bind(&ASTParser::logicalOperationAction, this)))) {
        head->addChild(std::move(child));
        head->addChild(std::move(relationalExpressionAction()));
        head->addChild(std::move(booleanOperationAction()));
    } else if ((child = acceptIf(std::bind(&ASTParser::ternaryOperationAction, this)))) {
        head->addChild(std::move(child));
    } else {
        return NULL;
        // booleanOperation is optional, so if it doesn't match, it's ok
    }
    return head;
}

std::unique_ptr<Node> ASTParser::ternaryOperationAction() {
    std::unique_ptr<Node> head(new TernaryOperationNode());
    std::unique_ptr<Node> child;
    if ((child = acceptIf(TokenType::kQuestionMark))) {
        head->addChild(std::move(child));
        head->addChild(std::move(booleanExpressionAction()));
        head->addChild(std::move(expect(TokenType::kColon)));
        head->addChild(std::move(booleanExpressionAction()));
    } else {
        error("ternaryOperationAction: syntax error");
    }
    return head;
}

std::unique_ptr<Node> ASTParser::returnStatementAction() {
    std::unique_ptr<Node> head(new ReturnStatementNode());
    std::unique_ptr<Node> child;
    if ((child = acceptIf(TokenType::kReturnKeyword))) {
        head->addChild(std::move(child));
        head->addChild(std::move(booleanExpressionAction()));
        head->addChild(std::move(expect(TokenType::kSemiColon)));
    } else {
        error("returnStatement: syntax error");
    }
    return head;
}

std::unique_ptr<Node> ASTParser::logicalOperationAction() {
    std::unique_ptr<Node> head(new LogicalOperationNode());
    std::unique_ptr<Node> child;
    if ((child = acceptIf(TokenType::kLogicalAnd)) || (child = acceptIf(TokenType::kLogicalOr))) {
        head->addChild(std::move(child));
    } else {
        error("logicalOperation: syntax error");
    }
    return head;
}

std::unique_ptr<Node> ASTParser::comparisonOperationAction() {
    std::unique_ptr<Node> head(new ComparisonOperationNode());
    std::unique_ptr<Node> child;
    if ((child = acceptIf(TokenType::kTripleEquals)) ||
        (child = acceptIf(TokenType::kDoubleEquals)) ||
        (child = acceptIf(TokenType::kGreaterThan)) ||
        (child = acceptIf(TokenType::kGreaterThanEquals)) ||
        (child = acceptIf(TokenType::kLessThan)) ||
        (child = acceptIf(TokenType::kLessThanEquals)) ||
        (child = acceptIf(TokenType::kNotEquals)) ||
        (child = acceptIf(TokenType::kDoubleNotEquals))) {
        head->addChild(std::move(child));
    } else {
        error("comparisonOperation: syntax error");
    }
    return head;
}
}
}
