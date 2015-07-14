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
#include "mongo/scripting/tinyjs/scope.h"

namespace mongo {
namespace tinyjs {

using std::string;

/**
 * This function takes in an input string, lexes, parses, and evaluates it, 
 * and compares the result of evaluation to the expected output. 
 * It assumes that the lexing will not return with an error.
 */
void testEvaluation(string input, Value expected) {
    std::vector<Token> tokenData = lex(input).getValue();
    ASTParser a(std::move(tokenData));
    Scope* s = new Scope();
    Value res = a.evaluate(s);
    ASSERT_EQ(res,expected);
    delete s;
}

/**
 * This function takes in an input string, lexes, and parses it, 
 * and asserts that evaluating it will throw an exception.
 */
void testEvaluationError(string input) {
    std::vector<Token> tokenData = lex(input).getValue();
    Scope* s = new Scope();
    ASTParser a(std::move(tokenData));
    ASSERT_THROWS(a.evaluate(s), std::exception);
}



TEST(EvaluationTest, simple) {
    string input = "return 1;";
    testEvaluation(input, Value(1));
}

TEST(EvaluationTest, addition1) {
    string input = "return 1 + 1;";
    testEvaluation(input, Value(2));
}

TEST(EvaluationTest, addition2) {
    string input = "return 1 + \"cat\";";
    testEvaluation(input, Value("1cat"));
}

TEST(EvaluationTest, addition3) {
    string input = "return \"dog\" + 1;";
    testEvaluation(input, Value("dog1"));
}

TEST(EvaluationTest, addition4) {
    string input = "return \"dog\" + \"cat\";";
    testEvaluation(input, Value("dogcat"));
}

TEST(EvaluationTest, addition5) {
    string input = "return true + 1;";
    testEvaluation(input, Value(2));
}

TEST(EvaluationTest, addition6) {
    string input = "return 3 + false;";
    testEvaluation(input, Value(3));
}

TEST(EvaluationTest, addition7) {
    string input = "return false + null;";
    testEvaluation(input, Value(0));
}

TEST(EvaluationTest, addition8) {
    string input = "return null + true;";
    testEvaluation(input, Value(1));
}

TEST(EvaluationTest, addition9) {
    string input = "return 1.5 + 1.5;";
    testEvaluation(input, Value(3));
}

TEST(EvaluationTest, addition10) {
    string input = "return 1 + 1.5;";
    testEvaluation(input, Value(2.5));
}

TEST(EvaluationTest, addition11) {
    string input = "return 1.5 + 1;";
    testEvaluation(input, Value(2.5));
}

TEST(EvaluationTest, addition12) {
    string input = "return 0 + 0;";
    testEvaluation(input, Value(0));
}

TEST(EvaluationTest, addition13) {
    string input = "return 0 + undefined;";
    testEvaluationError(input);
}

TEST(EvaluationTest, addition14) {
    string input = "return undefined + 5;";
    testEvaluationError(input);
}

TEST(EvaluationTest, addition15) {
    string input = "return NaN + 5;";
    testEvaluationError(input);
}

TEST(EvaluationTest, addition16) {
    string input = "return 5 + NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, addition17) {
    string input = "return 3 + Infinity;";
    testEvaluation(input, Value("Infinity"));
}

TEST(EvaluationTest, addition18) {
    string input = "return Infinity + 10;";
    testEvaluation(input, Value("Infinity"));
}

TEST(EvaluationTest, addition19) {
    string input = "return [1, 2, 3] + 1;";
    testEvaluation(input, Value("1,2,31"));
}

TEST(EvaluationTest, addition20) {
    string input = "return 5 + [1, 2, 3];";
    testEvaluation(input, Value("51,2,3"));
}

TEST(EvaluationTest, addition21) {
    string input = "return [1, 2, 3] + true;";
    testEvaluation(input, Value("1,2,3true"));
}

TEST(EvaluationTest, addition22) {
    string input = "return false + [1, 2, 3];";
    testEvaluation(input, Value("false1,2,3"));
}

TEST(EvaluationTest, addition23) {
    string input = "return [1, 2, 3] + undefined;";
    testEvaluation(input, Value("1,2,3undefined"));
}

TEST(EvaluationTest, addition24) {
    string input = "return undefined + [1, 2, 3];";
    testEvaluation(input, Value("undefined1,2,3"));
}

TEST(EvaluationTest, addition25) {
    string input = "return [1, 2, 3] + NaN;";
    testEvaluation(input, Value("1,2,3NaN"));
}

TEST(EvaluationTest, addition26) {
    string input = "return NaN + [1, 2, 3];";
    testEvaluation(input, Value("NaN1,2,3"));
}

TEST(EvaluationTest, addition27) {
    string input = "return [1, 2, 3] + Infinity;";
    testEvaluation(input, Value("1,2,3Infinity"));
}

TEST(EvaluationTest, addition28) {
    string input = "return Infinity + [1, 2, 3];";
    testEvaluation(input, Value("Infinity1,2,3"));
}

TEST(EvaluationTest, addition29) {
    string input = "return [\"cat\", \"dog\"] + null;";
    testEvaluation(input, Value("cat,dognull"));
}


TEST(EvaluationTest, addition30) {
    string input = "return null + [\"cat\", \"dog\"];";
    testEvaluation(input, Value("cat,dog"));
}

TEST(EvaluationTest, addition31) {
    string input = "return [1, 2, 3] + \"cat\";";
    testEvaluation(input, Value("1,2,3cat"));
}

TEST(EvaluationTest, addition32) {
    string input = "return \"cat\" + [1, 2, 3];";
    testEvaluation(input, Value("cat1,2,3"));
}


TEST(EvaluationTest, addition33) {
    string input = "return undefined + \"cat\";";
    testEvaluation(input, Value("undefinedcat"));
}

TEST(EvaluationTest, addition34) {
    string input = "return \"cat\" + undefined;";
    testEvaluation(input, Value("catundefined"));
}

TEST(EvaluationTest, addition35) {
    string input = "return NaN + \"cat\";";
    testEvaluation(input, Value("NaNcat"));
}

TEST(EvaluationTest, addition36) {
    string input = "return \"cat\" + NaN;";
    testEvaluation(input, Value("catNaN"));
}

TEST(EvaluationTest, addition37) {
    string input = "return Infinity + \"cat\";";
    testEvaluation(input, Value("Infinitycat"));
}

TEST(EvaluationTest, addition38) {
    string input = "return \"cat\" + Infinity;";
    testEvaluation(input, Value("catInfinity"));
}

TEST(EvaluationTest, addition39) {
    string input = "return true + \"cat\";";
    testEvaluation(input, Value("truecat"));
}


TEST(EvaluationTest, addition40) {
    string input = "return \"cat\" + \"false\";";
    testEvaluation(input, Value("catfalse"));
}

TEST(EvaluationTest, addition41) {
    string input = "return null + \"cat\";";
    testEvaluation(input, Value("nullcat"));
}

TEST(EvaluationTest, addition42) {
    string input = "return \"cat\" + null;";
    testEvaluation(input, Value("catnull"));
}

TEST(EvaluationTest, addition43) {
    string input = "return NaN + NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, addition44) {
    string input = "return undefined + NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, addition45) {
    string input = "return NaN + undefined;";
    testEvaluationError(input);
}

TEST(EvaluationTest, addition46) {
    string input = "return true + NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, addition47) {
    string input = "return NaN + true;";
    testEvaluationError(input);
}

TEST(EvaluationTest, addition48) {
    string input = "return null + NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, addition49) {
    string input = "return NaN + null;";
    testEvaluationError(input);
}

TEST(EvaluationTest, addition50) {
    string input = "return Infinity + NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, addition51) {
    string input = "return NaN + Infinity;";
    testEvaluationError(input);
}

TEST(EvaluationTest, addition52) {
    string input = "return undefined + undefined;";
    testEvaluationError(input);
}

TEST(EvaluationTest, addition53) {
    string input = "return undefined + Infinity;";
    testEvaluationError(input);
}

TEST(EvaluationTest, addition54) {
    string input = "return Infinity + undefined;";
    testEvaluationError(input);
}

TEST(EvaluationTest, addition55) {
    string input = "return undefined + true;";
    testEvaluationError(input);
}

TEST(EvaluationTest, addition56) {
    string input = "return false + undefined;";
    testEvaluationError(input);
}

TEST(EvaluationTest, addition57) {
    string input = "return undefined + null;";
    testEvaluationError(input);
}

TEST(EvaluationTest, addition58) {
    string input = "return null + undefined;";
    testEvaluationError(input);
}

TEST(EvaluationTest, addition59) {
    string input = "return Infinity + true;";
    testEvaluation(input, Value("Infinity"));
}

TEST(EvaluationTest, addition60) {
    string input = "return false + Infinity;";
    testEvaluation(input, Value("Infinity"));
}

TEST(EvaluationTest, addition61) {
    string input = "return Infinity + Infinity;";
    testEvaluation(input, Value("Infinity"));
}

TEST(EvaluationTest, addition62) {
    string input = "return Infinity + null;";
    testEvaluation(input, Value("Infinity"));
}

TEST(EvaluationTest, addition63) {
    string input = "return null + Infinity;";
    testEvaluation(input, Value("Infinity"));
}

TEST(EvaluationTest, addition64) {
    string input = "return true + null;";
    testEvaluation(input, Value(1));
}

TEST(EvaluationTest, addition65) {
    string input = "return null + false;";
    testEvaluation(input, Value(0));
}

} // namespace tinyjs
} // namespace mongo
