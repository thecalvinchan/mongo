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

#pragma once

#include <vector>

#include "mongo/scripting/tinyjs/binary_operator.h"
#include "mongo/scripting/tinyjs/node.h"
#include "mongo/scripting/tinyjs/terminal_node.h"
#include "mongo/scripting/tinyjs/ternary_operator.h"
#include "mongo/scripting/tinyjs/parse_exception.h"
#include "mongo/scripting/tinyjs/unary_operator.h"

namespace mongo {
namespace tinyjs {

class ASTParser {
public:
    ASTParser(std::vector<Token> tokens);
    ~ASTParser();
    void printTree();
    std::string traverse();

private:
    void parseTokens(std::vector<Token> tokens);
    void nexttoken(void);
    void error(const char msg[]);

    bool matchImplicitTerminal(TokenType t);
    std::unique_ptr<Node> matchNodeTerminal(TokenType t);
    std::unique_ptr<TerminalNode> makeTerminalNode(Token token);
    std::unique_ptr<Node> tryProductionMatch(std::function<std::unique_ptr<Node>(void)> action);
    void expectImplicitTerminal(TokenType t);

    std::unique_ptr<Node> clauseAction();
    std::unique_ptr<Node> variableAction();
    std::unique_ptr<Node> objectAction();
    std::unique_ptr<Node> objectAccessorAction(std::unique_ptr<Node> leftChild);
    std::unique_ptr<Node> termAction();
    std::unique_ptr<Node> arrayElementAction();
    std::vector<std::unique_ptr<Node> > arrayElements();
    std::unique_ptr<Node> arrayLiteralAction();
    std::unique_ptr<Node> arrayTailAction();
    std::unique_ptr<Node> arrayIndexedAction();
    std::unique_ptr<Node> factorAction();
    std::unique_ptr<Node> multiplicativeExpressionAction();
    std::unique_ptr<Node> multiplicativeOperationAction(std::unique_ptr<Node> leftChild);
    std::unique_ptr<Node> arithmeticExpressionAction();
    std::unique_ptr<Node> arithmeticOperationAction(std::unique_ptr<Node> leftChild);
    std::unique_ptr<Node> booleanFactorAction();
    std::unique_ptr<Node> relationalExpressionAction();
    std::unique_ptr<Node> relationalOperationAction(std::unique_ptr<Node> leftChild);
    std::unique_ptr<Node> booleanExpressionAction();
    std::unique_ptr<Node> booleanOperationAction(std::unique_ptr<Node> leftChild);
    std::unique_ptr<Node> ternaryOperationAction(std::unique_ptr<Node> leftChild);
    std::unique_ptr<Node> returnStatementAction();

    int currentPosition;
    Token currentToken;
    std::vector<Token> tokens;
    std::unique_ptr<Node> head;
};
}
}
