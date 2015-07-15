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
    ASTParser a(std::move(tokenData));
    std::string res = a.traverse();
    std::cout << res << std::endl;
    ASSERT_EQ(res,expected);
}

/* 
 * This function takes in an input string, lexes it, and asserts 
 * that parsing it will result in a syntax error.
 */
void testSyntaxError(string input) {
    std::vector<Token> tokenData = lex(input).getValue();
    ASSERT_THROWS(ASTParser(std::move(tokenData)), std::exception);
}

TEST(ParserTest, test1) {
    string input = "return this.a == 1;";

    std::stringstream expected;
    expected << "return ";
    expected << "== ";
    expected << ". 1 ";
    expected << "this a " << std::endl;

    testParseTree(input, expected.str());
}

TEST(ParserTest, test2) {
    string input = "function() {return this.a === 1;}";

    std::stringstream expected;
    expected << "return ";
    expected << "=== ";
    expected << ". 1 ";
    expected << "this a " << std::endl;

    testParseTree(input, expected.str());
}

TEST(ParserTest, test3) {
    string input = "function() {return this.a.b.c.d === 1;}";

    std::stringstream expected;
    expected << "return ";
    expected << "=== ";
    expected << ". 1 ";
    expected << ". d ";
    expected << ". c ";
    expected << ". b ";
    expected << "this a " << std::endl;

    testParseTree(input, expected.str());
}

TEST(ParserTest, test4) {
    string input = "function() {return this['a'] === true;}";

    std::stringstream expected;
    expected << "return ";
    expected << "=== ";
    expected << "[ true ";
    expected << "this a " << std::endl;

    testParseTree(input, expected.str());
}

TEST(ParserTest, test5) {
    string input = "return this[3+3] == 90.1;";

    std::stringstream expected;
    expected << "return ";
    expected << "== ";
    expected << "[ 90.1 ";
    expected << "this + ";
    expected << "3 3 " << std::endl;

    testParseTree(input, expected.str());
}

TEST(ParserTest, test6) {
    string input = "return this.a[3].b[2].c == this.b[5].c.d;";

    std::stringstream expected;
    expected << "return ";
    expected << "== ";
    expected << ". . ";
    expected << "[ c . d ";
    expected << ". 2 [ c ";
    expected << "[ b . 5 ";
    expected << ". 3 this b ";
    expected << "this a " << std::endl;

    testParseTree(input, expected.str());
}

TEST(ParserTest, test7) {
    string input = "return this.a == 3 ? (this.b > this.c): (this.d == this.e);";

    std::stringstream expected;
    expected << "return ";
    expected << "? ";
    expected << "== > == ";
    expected << ". 3 . . . . ";
    expected << "this a this b this c this d this e " << std::endl;

    testParseTree(input, expected.str());
}

TEST(ParserTest, test8) {
    string input = "function() {return x > (3 + 1);}";

    std::stringstream expected;
    expected << "return ";
    expected << "> ";
    expected << "x + ";
    expected << "3 1 " << std::endl;

    testParseTree(input, expected.str());
}

TEST(ParserTest, test9) {
    string input = "return (3*4)/8 - y;";

    std::stringstream expected;
    expected << "return ";
    expected << "- ";
    expected << "/ y ";
    expected << "* 8 ";
    expected << "3 4 " << std::endl;

    testParseTree(input, expected.str());
}

TEST(ParserTest, test10) {
    string input = "return this.elements == ['cat', 'dog'];";

    std::stringstream expected;
    expected << "return ";
    expected << "== ";
    expected << ". [ ";
    expected << "this elements cat dog " << std::endl;

    testParseTree(input, expected.str());
}

TEST(ParserTest, test11) {
    string input = "return true ? (1 > this.pets ? 'dog' : 'cat') : 'no pets';";

    std::stringstream expected;
    expected << "return ";
    expected << "? ";
    expected << "true ? no pets ";
    expected << "> dog cat ";
    expected << "1 . ";
    expected << "this pets " << std::endl;

    testParseTree(input, expected.str());
}

TEST(ParserTest, test12) {
    string input = "return true ? 1 > this.pets ? 'dog' : 'cat' : 'no pets';";

    std::stringstream expected;
    expected << "return ";
    expected << "? ";
    expected << "true ? no pets ";
    expected << "> dog cat ";
    expected << "1 . ";
    expected << "this pets " << std::endl;

    testParseTree(input, expected.str());
}

TEST(ParserTest, test13) {
    string input = "return this.a != 1;";

    std::stringstream expected;
    expected << "return ";
    expected << "!= ";
    expected << ". 1 ";
    expected << "this a " << std::endl;

    testParseTree(input, expected.str());
}

TEST(ParserTest, test14) {
    string input = "return this.a !== 1;";

    std::stringstream expected;
    expected << "return ";
    expected << "!== ";
    expected << ". 1 ";
    expected << "this a " << std::endl;

    testParseTree(input, expected.str());
}

TEST(ParserTest, test15) {
    string input = "return this.a[this.b] == 3;";

    std::stringstream expected;
    expected << "return ";
    expected << "== ";
    expected << "[ 3 ";
    expected << ". . ";
    expected << "this a this b " << std::endl;

    testParseTree(input, expected.str());
}

TEST(ParserTest, test16) {
    string input = "return this.a[this.b] == this.c[\"foo\"];";

    std::stringstream expected;
    expected << "return ";
    expected << "== ";
    expected << "[ [ ";
    expected << ". . . foo ";
    expected << "this a this b this c " << std::endl;

    testParseTree(input, expected.str());
}

TEST(ParserTest, test17) {
    string input = "return (-3*4)/8 - y;";

    std::stringstream expected;
    expected << "return ";
    expected << "- ";
    expected << "/ y ";
    expected << "* 8 ";
    expected << "- 4 ";
    expected << "3 " << std::endl;

    testParseTree(input, expected.str());
}

TEST(ParserTest, test18) {
    string input = "return this.a[3 + 3] == this.a[this.b[this.c[this.d[this.e]]]];";

    std::stringstream expected;
    expected << "return ";
    expected << "== ";
    expected << "[ [ ";
    expected << ". + . [ ";
    expected << "this a 3 3 this a . [ ";
    expected << "this b . [ ";
    expected << "this c . . ";
    expected << "this d this e " << std::endl;

    testParseTree(input, expected.str());
}

TEST(ParserTest, test19) {
    string input = "return 1;";

    std::stringstream expected;
    expected << "return ";
    expected << "1 " << std::endl;

    testParseTree(input, expected.str());
}

TEST(ParserTest, test20) {
    string input = "return true / 1;";

    std::stringstream expected;
    expected << "return ";
    expected << "/ ";
    expected << "true 1 " << std::endl;

    testParseTree(input, expected.str());
}

TEST(ParserTest, ErrorBadOperator) {
    testSyntaxError("return (x++1);");
}

TEST(ParserTest, ErrorOperatorBeyondSubset) {
    testSyntaxError("return (x++);");
}

TEST(ParserTest, ErrorNoReturn) {
    testSyntaxError("x > 1");
}

TEST(ParserTest, ErrorNoSemiColon) {
    testSyntaxError("return (x == 1)");
}

TEST(ParserTest, ErrorNoOperator) {
    testSyntaxError("return a b c");
}

TEST(ParserTest, ErrorTernaryTooShort) {
    testSyntaxError("return x ? 1 : ;");
}

TEST(ParserTest, ErrorFunctionArgumentsBeyondSubset) {
    testSyntaxError("function(a) {return (a == 1);}");
}

} // namespace tinyjs
} // namespace mongo
