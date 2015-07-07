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
#include <sstream>
#include <vector>

#include "mongo/unittest/unittest.h"
#include "mongo/scripting/tinyjs/lexer.h"
#include "mongo/scripting/tinyjs/ast_parser.h"

namespace mongo {
namespace tinyjs {

using std::string;

/**
 * This function takes in an input string, lexes and parses it, 
 * and compares the preorder traversal of the parse tree to the expected output. 
 * It assumes that the lexing will not return with an error.
 */
void testParseTree(string input, string expected) {
    std::vector<Token> tokenData = lex(input).getValue();
    ASTParser* a = new ASTParser(tokenData);
    std::string res = a->traverse();
    std::cout << res << std::endl;
    ASSERT(res == expected);
    delete a;
}

/* 
 * This function takes in an input string, lexes it, and asserts 
 * that parsing it will result in a syntax error.
 */
void testSynaxError(string input) {
    std::vector<Token> tokenData = lex(input).getValue();
    ASSERT_THROWS(new ASTParser(tokenData), std::exception);
}

TEST(ParserTest, test1) {
    string input = "return this.a == 1;";

    std::stringstream expected;
    expected << "return " << std::endl;
    expected << "== " << std::endl;
    expected << ". 1 " << std::endl;
    expected << "this a " << std::endl;

    testParseTree(input, expected.str());
}

TEST(ParserTest, test2) {
    string input = "function() {return this.a === 1;}";

    std::stringstream expected;
    expected << "return " << std::endl;
    expected << "=== " << std::endl;
    expected << ". 1 " << std::endl;
    expected << "this a " << std::endl;

    testParseTree(input, expected.str());
}

TEST(ParserTest, test3) {
    string input = "function() {return this.a.b.c.d === 1;}";

    std::stringstream expected;
    expected << "return " << std::endl;
    expected << "=== " << std::endl;
    expected << ". 1 " << std::endl;
    expected << ". d " << std::endl;
    expected << ". c " << std::endl;
    expected << ". b " << std::endl;
    expected << "this a " << std::endl;

    testParseTree(input, expected.str());
}

TEST(ParserTest, test4) {
    string input = "function() {return this['a'] === this.b;}";

    std::stringstream expected;
    expected << "return " << std::endl;
    expected << "=== " << std::endl;
    expected << "[ . " << std::endl;
    expected << "this 'a' this b " << std::endl;

    testParseTree(input, expected.str());
}

TEST(ParserTest, test5) {
    string input = "return this[3+3] == 90.1;";

    std::stringstream expected;
    expected << "return " << std::endl;
    expected << "== " << std::endl;
    expected << "[ 90.1 " << std::endl;
    expected << "this + " << std::endl;
    expected << "3 3 " << std::endl;

    testParseTree(input, expected.str());
}

TEST(ParserTest, test6) {
    string input = "return this.a[3].b[2].c == this.b[5].c.d;";

    std::stringstream expected;
    expected << "return " << std::endl;
    expected << "== " << std::endl;
    expected << ". . " << std::endl;
    expected << "[ c . d " << std::endl;
    expected << ". 2 [ c " << std::endl;
    expected << "[ b . 5 " << std::endl;
    expected << ". 3 this b " << std::endl;
    expected << "this a " << std::endl;

    testParseTree(input, expected.str());
}

TEST(ParserTest, test7) {
    string input = "return this.a == 3 ? (this.b > this.c): (this.d == this.e);";

    std::stringstream expected;
    expected << "return " << std::endl;
    expected << "? " << std::endl;
    expected << "== > == " << std::endl;
    expected << ". 3 . . . . " << std::endl;
    expected << "this a this b this c this d this c " << std::endl;

    testParseTree(input, expected.str());
}

TEST(ParserTest, test8) {
    string input = "function() {return x > (3 + 1);}";

    std::stringstream expected;
    expected << "return " << std::endl;
    expected << "> " << std::endl;
    expected << "x + " << std::endl;
    expected << "3 1 " << std::endl;

    testParseTree(input, expected.str());
}

TEST(ParserTest, test9) {
    string input = "return (3*4)/8 - y;";

    std::stringstream expected;
    expected << "return " << std::endl;
    expected << "- " << std::endl;
    expected << "/ y " << std::endl;
    expected << "* 8 " << std::endl;
    expected << "3 4 " << std::endl;

    testParseTree(input, expected.str());
}


TEST(ParserTest, ErrorBadOperator) {
    testSynaxError("return (x++1);");
}

TEST(ParserTest, ErrorOperatorBeyondSubset) {
    testSynaxError("return (x++);");
}

TEST(ParserTest, ErrorNoReturn) {
    testSynaxError("x > 1");
}

TEST(ParserTest, ErrorNoSemiColon) {
    testSynaxError("return (x == 1)");
}

TEST(ParserTest, ErrorTernaryTooShort) {
    testSynaxError("return x ? 1 : ;");
}

TEST(ParserTest, ErrorObjectBeyondSubset) {
    testSynaxError("function() {return object.a;}");
}

TEST(ParserTest, ErrorFunctionArgumentsBeyondSubset) {
    testSynaxError("function(a) {return (a == 1);}");
}

TEST(ParserTest, ErrorBadArrayIndex) {
    testSynaxError("function() {return (arr['string'] == 1);}");
}

}  // namespace tinyjs
}  // namespace mongo
