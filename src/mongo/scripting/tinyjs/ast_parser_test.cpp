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
#include <cstdio>
#include <string>
#include <vector>

#include "mongo/unittest/unittest.h"
#include "mongo/scripting/tinyjs/lexer.h"
#include "mongo/scripting/tinyjs/ast_parser.h"

namespace mongo {
namespace tinyjs {

using std::string;

void testParseTree(string input, string expected) {
    std::vector<Token> tokenData = lex(input).getValue();
    ASTParser *a = new ASTParser(tokenData);
    std::string res = a->traverse();
    std::cout << res << std::endl;
    ASSERT(res == expected);
}

TEST(ParserTest, test1) {
    string input = "return this.a == 1;";

    string expected =
        "ClauseNode ReturnStatementNode LeafNode BooleanExpressionNode RelationalExpressionNode "
        "BooleanFactorNode ArithmeticExpressionNode MultiplicativeExpressionNode FactorNode "
        "TermNode VariableNode ObjectNode LeafNode ObjectAccessorNode LeafNode LeafNode "
        "RelationalOperationNode ComparisonOperationNode LeafNode BooleanFactorNode "
        "ArithmeticExpressionNode MultiplicativeExpressionNode FactorNode TermNode LeafNode "
        "LeafNode";

    testParseTree(input, expected);
}

TEST(ParserTest, test2) {
    string input = "function() {return this.a === 1;}";

    string expected =
        "ClauseNode LeafNode LeafNode LeafNode LeafNode ReturnStatementNode LeafNode "
        "BooleanExpressionNode RelationalExpressionNode BooleanFactorNode ArithmeticExpressionNode "
        "MultiplicativeExpressionNode FactorNode "
        "TermNode VariableNode ObjectNode LeafNode ObjectAccessorNode LeafNode LeafNode "
        "RelationalOperationNode ComparisonOperationNode LeafNode BooleanFactorNode "
        "ArithmeticExpressionNode MultiplicativeExpressionNode FactorNode TermNode LeafNode "
        "LeafNode LeafNode";

    testParseTree(input, expected);
}

TEST(ParserTest, test3) {
    string input = "function() {return this.a.b.c.d === 1;}";

    string expected =
        "ClauseNode LeafNode LeafNode LeafNode LeafNode ReturnStatementNode LeafNode "
        "BooleanExpressionNode RelationalExpressionNode BooleanFactorNode ArithmeticExpressionNode "
        "MultiplicativeExpressionNode FactorNode "
        "TermNode VariableNode ObjectNode LeafNode ObjectAccessorNode LeafNode LeafNode "
        "ObjectAccessorNode LeafNode LeafNode ObjectAccessorNode LeafNode LeafNode "
        "ObjectAccessorNode LeafNode LeafNode "
        "RelationalOperationNode ComparisonOperationNode LeafNode BooleanFactorNode "
        "ArithmeticExpressionNode MultiplicativeExpressionNode FactorNode TermNode LeafNode "
        "LeafNode LeafNode";

    testParseTree(input, expected);
}

TEST(ParserTest, test4) {
    string input = "function() {return this['a'] === this.b;}";

    string expected =
        "ClauseNode LeafNode LeafNode LeafNode LeafNode ReturnStatementNode LeafNode "
        "BooleanExpressionNode RelationalExpressionNode BooleanFactorNode ArithmeticExpressionNode "
        "MultiplicativeExpressionNode FactorNode "
        "TermNode VariableNode ObjectNode LeafNode ObjectAccessorNode LeafNode LeafNode LeafNode "
        "RelationalOperationNode ComparisonOperationNode LeafNode BooleanFactorNode "
        "ArithmeticExpressionNode MultiplicativeExpressionNode FactorNode TermNode "
        "VariableNode ObjectNode LeafNode ObjectAccessorNode LeafNode LeafNode "
        "LeafNode LeafNode";

    testParseTree(input, expected);
}

TEST(ParserTest, test1) {
    string input = "return this[3+3] == 90.1;";

    string expected =
        "ClauseNode ReturnStatementNode LeafNode BooleanExpressionNode RelationalExpressionNode "
        "BooleanFactorNode ArithmeticExpressionNode MultiplicativeExpressionNode FactorNode "
        "TermNode VariableNode ObjectNode LeafNode ObjectAccessorNode LeafNode LeafNode "
        "RelationalOperationNode ComparisonOperationNode LeafNode BooleanFactorNode "
        "ArithmeticExpressionNode MultiplicativeExpressionNode FactorNode TermNode LeafNode "
        "LeafNode";

    //testParseTree(input, expected);
}


}  // namespace tinyjs
}  // namespace mongo
