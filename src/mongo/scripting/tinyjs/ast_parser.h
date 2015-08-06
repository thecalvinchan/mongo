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
#include "mongo/scripting/tinyjs/identifier.h"
#include "mongo/scripting/tinyjs/ternary_operator.h"
#include "mongo/scripting/tinyjs/parse_exception.h"
#include "mongo/scripting/tinyjs/unary_operator.h"
#include "mongo/scripting/tinyjs/array_literal.h"
#include "mongo/scripting/tinyjs/block.h"
#include "mongo/scripting/tinyjs/assignment_operator.h"
#include "mongo/scripting/tinyjs/object_accessor_operator.h"
#include "mongo/scripting/tinyjs/multiplication_operator.h"
#include "mongo/scripting/tinyjs/division_operator.h"
#include "mongo/scripting/tinyjs/addition_operator.h"
#include "mongo/scripting/tinyjs/subtraction_operator.h"
#include "mongo/scripting/tinyjs/triple_equals_operator.h"
#include "mongo/scripting/tinyjs/double_equals_operator.h"
#include "mongo/scripting/tinyjs/greater_than_operator.h"
#include "mongo/scripting/tinyjs/greater_than_equals_operator.h"
#include "mongo/scripting/tinyjs/less_than_operator.h"
#include "mongo/scripting/tinyjs/less_than_equals_operator.h"
#include "mongo/scripting/tinyjs/not_equals_operator.h"
#include "mongo/scripting/tinyjs/double_not_equals_operator.h"
#include "mongo/scripting/tinyjs/logical_and_operator.h"
#include "mongo/scripting/tinyjs/logical_or_operator.h"
#include "mongo/scripting/tinyjs/while_loop.h"


namespace mongo {
namespace tinyjs {

class ASTParser {
public:
    ASTParser(std::vector<Token> tokens);
    void printTree();
    /**
     * This function walks the tree and returns a string of the node names separated by spaces in
     * BFS-traversal order.
     */
    std::string traverse();
    void optimize(std::unique_ptr<AndMatchExpression> root);
    Value evaluate(Scope* s);
private:
    void parseTokens(std::vector<Token> tokens);
    void nexttoken(void);
    void error(const char msg[]);
    /**
     * This function takes in a TokenType which should be one of the types that does not correspond
     * to a node in the abstract syntax tree - for example, parentheses, semicolons, "return", etc.
     * If the current token matches the input, it calls nexttoken() and returns true. Otherwise, it
     * returns false.
     */
    bool matchImplicitTerminal(TokenType t);
    /**
     * This function takes in a TokenType which should be one of the types that does correspond
     * to a node in the abstract syntax tree: knullptrLiteral, kUndefinedLiteral, kIntegerLiteral,
     * kFloatLiteral, kBooleanLiteral, kStringLiteral, kIdentifier
     * If the current token matches the input, it calls nexttoken() and returns a node corresponding to
     * the token. Otherwise, it
     * returns nullptr.
     */
    std::unique_ptr<Node> matchNodeTerminal(TokenType t);
    /**
     * This function takes in a TokenType which should be one of the types that corresponds
     * to a node in the abstract syntax tree: knullptrLiteral, kUndefinedLiteral, kIntegerLiteral,
     * kFloatLiteral, kBooleanLiteral, kStringLiteral, kIdentifier. It creates and returns a
     * corresponding TerminalNode. If the type of the token is not valid, an error is thrown.
     */
    std::unique_ptr<TerminalNode> makeTerminalNode(Token token);
    /** 
     * This function attempts to apply the specified production rule to the input. If there is a match, 
     * it returns the resulting node in the AST. Otherwise, it returns nullptr.
     */
    std::unique_ptr<Node> tryProductionMatch(std::function<std::unique_ptr<Node>(void)> action);
    /**
     * ExpectImplcitTerminal advances to the next token if the current token matches the input
     * and throws an error otherwise. It only checks simple tokens that won't become nodes,
     * because nodes are created by matchNodeTerminal or by calling an action. TODO why exactly don't we
     * need an expect that returns a node?
     */
    void expectImplicitTerminal(TokenType t);


    /**
     * Each of the following functions represents a production rule, described in its header comment.
     * TODO: more explanation of the functions
     */
    std::unique_ptr<Node> clauseAction();
    std::unique_ptr<Node> blockAction();
    std::unique_ptr<Node> statementOrLoopAction();
    std::unique_ptr<Node> statementAction();
    std::unique_ptr<Node> varAssignmentAction();
    std::unique_ptr<Node> noVarAssignmentAction();
    std::unique_ptr<Node> whileLoopAction();
    std::unique_ptr<Node> forLoopAction();
    std::unique_ptr<Node> variableAction();
    std::unique_ptr<Node> objectAction();
    std::unique_ptr<Node> objectAccessorAction(std::unique_ptr<Node> leftChild);
    std::unique_ptr<Node> termAction();
    std::unique_ptr<Node> numberAction();
    std::unique_ptr<Node> arrayElementAction();
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
    std::unique_ptr<Node> ternaryOperationAction();
    std::unique_ptr<Node> returnStatementAction();

    int _currentPosition;
    Token _currentToken;
    std::vector<Token> _tokens;
    std::unique_ptr<Node> _head;
};
} // namespace tinyjs
} // namespace mongo
