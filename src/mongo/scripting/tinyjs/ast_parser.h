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

#include "mongo/scripting/tinyjs/node.h"
#include "mongo/scripting/tinyjs/parse_exception.h"

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

    Boolean matchImplicitTerminal(TokenType t);
    std::unique_ptr<Node> ASTParser::matchNodeTerminal(TokenType t);
    std::unique_ptr<Node> ASTParser::tryProductionMatch(std::function<std::unique_ptr<Node>(void)> action)
    void ASTParser::expectImplicitTerminal(TokenType t);

    std::unique_ptr<Node> clauseAction();
    std::unique_ptr<Node> variableAction();
    std::unique_ptr<Node> objectAction();
    std::unique_ptr<Node> objectAccessorAction();
    std::unique_ptr<Node> termAction();
    std::unique_ptr<Node> arrayElementAction();
    std::unique_ptr<Node> arrayLiteralAction();
    std::unique_ptr<Node> arrayTailAction();
    std::unique_ptr<Node> arrayIndexedAction();
    std::unique_ptr<Node> factorAction();
    std::unique_ptr<Node> multiplicativeExpressionAction();
    std::unique_ptr<Node> multiplicativeOperationAction();
    std::unique_ptr<Node> arithmeticExpressionAction();
    std::unique_ptr<Node> arithmeticOperationAction();
    std::unique_ptr<Node> booleanFactorAction();
    std::unique_ptr<Node> relationalExpressionAction();
    std::unique_ptr<Node> relationalOperationAction();
    std::unique_ptr<Node> booleanExpressionAction();
    std::unique_ptr<Node> booleanOperationAction();
    std::unique_ptr<Node> ternaryOperationAction();
    std::unique_ptr<Node> returnStatementAction();
    std::unique_ptr<Node> logicalOperationAction();
    std::unique_ptr<Node> comparisonOperationAction();

    int currentPosition;
    Token currentToken;
    std::vector<Token> tokens;
    std::unique_ptr<Node> head;
};
}
}
