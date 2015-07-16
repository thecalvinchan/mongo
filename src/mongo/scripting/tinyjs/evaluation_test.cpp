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
#include "mongo/bson/mutable/document.h"
#include "mongo/bson/json.h"
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
void testEvaluation(string input, Value expected, Scope* s = NULL) {
    std::vector<Token> tokenData = lex(input).getValue();
    ASTParser a(std::move(tokenData));
    if (s == NULL) {
        s = new Scope();
    }
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
    Value res = a.evaluate(s);
    ASSERT_EQ(res,Value("NaN"));
    delete s;
}

Scope* generateScope() {
    Scope* s = new Scope();

    const char json[] = 
    "{"
        "x: 42,"
        "y: {"
            "a: 3.14,"
            "b: 31.4,"
            "c: 314.0,"
            "d: {"
                "h: 'webscale',"
                "i: 'cloud',"
                "j: [1,2,3],"
                "k: 'mangoDB'"
            "}"
        "},"
        "z: 'a'"
    "}";

    BSONObj result = fromjson(json);

    Value object = Value(result);
    s->put(StringData("this"),object);
    return s;
}

TEST(EvaluationTest, simpleObjectAccessor) {
    string input = "return this.x;";
    Scope* s = generateScope();
    testEvaluation(input, Value(42), s);
}

TEST(EvaluationTest, nestedObjectAccessor) {
    string input = "return this.y.a;";
    Scope* s = generateScope();
    testEvaluation(input, Value(3.14), s);
}

TEST(EvaluationTest, nestedObjectAccessor2) {
    string input = "return this.y[\"a\"];";
    Scope* s = generateScope();
    testEvaluation(input, Value(3.14), s);
}

TEST(EvaluationTest, doublyNestedObjectAccessor) {
    string input = "return this.y[this.z];";
    Scope* s = generateScope();
    testEvaluation(input, Value(3.14), s);
}

TEST(EvaluationTest, superNestedObjectAccessor) {
    string input = "return this.y[\"d\"].k;";
    Scope* s = generateScope();
    testEvaluation(input, Value("mangoDB"), s);
}

TEST(EvaluationTest, simple) {
    string input = "return 1;";
    testEvaluation(input, Value(1));
}

TEST(EvaluationTest, simpleNegation) {
    string input = "return -1;";
    testEvaluation(input, Value(-1));
}

/*
 * Addition tests
 */

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
    testEvaluation(input, Value("nullcat,dog"));
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
    string input = "return Infinity + -Infinity;";
    testEvaluationError(input);
}

TEST(EvaluationTest, addition63) {
    string input = "return -Infinity + Infinity;";
    testEvaluationError(input);
}

TEST(EvaluationTest, addition64) {
    string input = "return -Infinity + -Infinity;";
    testEvaluation(input, Value("-Infinity"));
}

TEST(EvaluationTest, addition65) {
    string input = "return Infinity + null;";
    testEvaluation(input, Value("Infinity"));
}

TEST(EvaluationTest, addition66) {
    string input = "return null + Infinity;";
    testEvaluation(input, Value("Infinity"));
}

TEST(EvaluationTest, addition67) {
    string input = "return true + null;";
    testEvaluation(input, Value(1));
}

TEST(EvaluationTest, addition68) {
    string input = "return null + false;";
    testEvaluation(input, Value(0));
}

/*
 * Multiplication tests
 */

TEST(EvaluationTest, multiplication1) {
    string input = "return 1 * 1;";
    testEvaluation(input, Value(1));
}

TEST(EvaluationTest, multiplication2) {
    string input = "return 1 * \"cat\";";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication3) {
    string input = "return \"dog\" * 1;";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication4) {
    string input = "return \"dog\" * \"cat\";";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication5) {
    string input = "return true * 1;";
    testEvaluation(input, Value(1));
}

TEST(EvaluationTest, multiplication6) {
    string input = "return 3 * false;";
    testEvaluation(input, Value(0));
}

TEST(EvaluationTest, multiplication7) {
    string input = "return false * null;";
    testEvaluation(input, Value(0));
}

TEST(EvaluationTest, multiplication8) {
    string input = "return null * true;";
    testEvaluation(input, Value(0));
}

TEST(EvaluationTest, multiplication9) {
    string input = "return 1.5 * 1.5;";
    testEvaluation(input, Value(2.25));
}

TEST(EvaluationTest, multiplication10) {
    string input = "return 1 * 1.5;";
    testEvaluation(input, Value(1.5));
}

TEST(EvaluationTest, multiplication11) {
    string input = "return 1.5 * 1;";
    testEvaluation(input, Value(1.5));
}

TEST(EvaluationTest, multiplication12) {
    string input = "return 0 * 0;";
    testEvaluation(input, Value(0));
}

TEST(EvaluationTest, multiplication13) {
    string input = "return 0 * undefined;";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication14) {
    string input = "return undefined * 5;";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication15) {
    string input = "return NaN * 5;";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication16) {
    string input = "return 5 * NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication17) {
    string input = "return 3 * Infinity;";
    testEvaluation(input, Value("Infinity"));
}

TEST(EvaluationTest, multiplication18) {
    string input = "return Infinity * 10;";
    testEvaluation(input, Value("Infinity"));
}

TEST(EvaluationTest, multiplication19) {
    string input = "return [1, 2, 3] * 1;";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication20) {
    string input = "return 5 * [1, 2, 3];";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication21) {
    string input = "return [1, 2, 3] * true;";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication22) {
    string input = "return false * [1, 2, 3];";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication23) {
    string input = "return [1, 2, 3] * undefined;";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication24) {
    string input = "return undefined * [1, 2, 3];";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication25) {
    string input = "return [1, 2, 3] * NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication26) {
    string input = "return NaN * [1, 2, 3];";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication27) {
    string input = "return [1, 2, 3] * Infinity;";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication28) {
    string input = "return Infinity * [1, 2, 3];";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication29) {
    string input = "return [\"cat\", \"dog\"] * null;";
    testEvaluationError(input);
}


TEST(EvaluationTest, multiplication30) {
    string input = "return null * [\"cat\", \"dog\"];";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication31) {
    string input = "return [1, 2, 3] * \"cat\";";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication32) {
    string input = "return \"cat\" * [1, 2, 3];";
    testEvaluationError(input);
}


TEST(EvaluationTest, multiplication33) {
    string input = "return undefined * \"cat\";";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication34) {
    string input = "return \"cat\" * undefined;";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication35) {
    string input = "return NaN * \"cat\";";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication36) {
    string input = "return \"cat\" * NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication37) {
    string input = "return Infinity * \"cat\";";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication38) {
    string input = "return \"cat\" * Infinity;";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication39) {
    string input = "return true * \"cat\";";
    testEvaluationError(input);
}


TEST(EvaluationTest, multiplication40) {
    string input = "return \"cat\" * \"false\";";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication41) {
    string input = "return null * \"cat\";";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication42) {
    string input = "return \"cat\" * null;";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication43) {
    string input = "return NaN * NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication44) {
    string input = "return undefined * NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication45) {
    string input = "return NaN * undefined;";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication46) {
    string input = "return true * NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication47) {
    string input = "return NaN * true;";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication48) {
    string input = "return null * NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication49) {
    string input = "return NaN * null;";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication50) {
    string input = "return Infinity * NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication51) {
    string input = "return NaN * Infinity;";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication52) {
    string input = "return undefined * undefined;";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication53) {
    string input = "return undefined * Infinity;";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication54) {
    string input = "return Infinity * undefined;";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication55) {
    string input = "return undefined * true;";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication56) {
    string input = "return false * undefined;";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication57) {
    string input = "return undefined * null;";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication58) {
    string input = "return null * undefined;";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication59) {
    string input = "return Infinity * true;";
    testEvaluation(input, Value("Infinity"));
}

TEST(EvaluationTest, multiplication60) {
    string input = "return false * Infinity;";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication61) {
    string input = "return Infinity * Infinity;";
    testEvaluation(input, Value("Infinity"));
}

TEST(EvaluationTest, multiplication62) {
    string input = "return Infinity * -Infinity;";
    testEvaluation(input, Value("-Infinity"));
}

TEST(EvaluationTest, multiplication63) {
    string input = "return -Infinity * Infinity;";
    testEvaluation(input, Value("-Infinity"));
}

TEST(EvaluationTest, multiplication64) {
    string input = "return -Infinity * -Infinity;";
    testEvaluation(input, Value("Infinity"));
}

TEST(EvaluationTest, multiplication65) {
    string input = "return Infinity * null;";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication66) {
    string input = "return null * Infinity;";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication67) {
    string input = "return true * null;";
    testEvaluation(input, Value(0));
}

TEST(EvaluationTest, multiplication68) {
    string input = "return null * false;";
    testEvaluation(input, Value(0));
}

/* 
 * Subtraction tests
 */

TEST(EvaluationTest, subtraction1) {
    string input = "return 1 - 1;";
    testEvaluation(input, Value(0));
}

TEST(EvaluationTest, subtraction2) {
    string input = "return 1 - \"cat\";";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction3) {
    string input = "return \"dog\" - 1;";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction4) {
    string input = "return \"dog\" - \"cat\";";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction5) {
    string input = "return true - 1;";
    testEvaluation(input, Value(0));
}

TEST(EvaluationTest, subtraction6) {
    string input = "return 3 - false;";
    testEvaluation(input, Value(3));
}

TEST(EvaluationTest, subtraction7) {
    string input = "return false - null;";
    testEvaluation(input, Value(0));
}

TEST(EvaluationTest, subtraction8) {
    string input = "return null - true;";
    testEvaluation(input, Value(-1));
}

TEST(EvaluationTest, subtraction9) {
    string input = "return 1.5 - 1.5;";
    testEvaluation(input, Value(0));
}

TEST(EvaluationTest, subtraction10) {
    string input = "return 1 - 1.5;";
    testEvaluation(input, Value(-0.5));
}

TEST(EvaluationTest, subtraction11) {
    string input = "return 1.5 - 1;";
    testEvaluation(input, Value(.5));
}

TEST(EvaluationTest, subtraction12) {
    string input = "return 0 - 0;";
    testEvaluation(input, Value(0));
}

TEST(EvaluationTest, subtraction13) {
    string input = "return 0 - undefined;";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction14) {
    string input = "return undefined - 5;";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction15) {
    string input = "return NaN - 5;";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction16) {
    string input = "return 5 - NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction17) {
    string input = "return 3 - Infinity;";
    testEvaluation(input, Value("-Infinity"));
}

TEST(EvaluationTest, subtraction18) {
    string input = "return Infinity - 10;";
    testEvaluation(input, Value("Infinity"));
}

TEST(EvaluationTest, subtraction19) {
    string input = "return [1, 2, 3] - 1;";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction20) {
    string input = "return 5 - [1, 2, 3];";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction21) {
    string input = "return [1, 2, 3] - true;";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction22) {
    string input = "return false - [1, 2, 3];";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction23) {
    string input = "return [1, 2, 3] - undefined;";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction24) {
    string input = "return undefined - [1, 2, 3];";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction25) {
    string input = "return [1, 2, 3] - NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction26) {
    string input = "return NaN - [1, 2, 3];";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction27) {
    string input = "return [1, 2, 3] - Infinity;";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction28) {
    string input = "return Infinity - [1, 2, 3];";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction29) {
    string input = "return [\"cat\", \"dog\"] - null;";
    testEvaluationError(input);
}


TEST(EvaluationTest, subtraction30) {
    string input = "return null - [\"cat\", \"dog\"];";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction31) {
    string input = "return [1, 2, 3] - \"cat\";";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction32) {
    string input = "return \"cat\" - [1, 2, 3];";
    testEvaluationError(input);
}


TEST(EvaluationTest, subtraction33) {
    string input = "return undefined - \"cat\";";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction34) {
    string input = "return \"cat\" - undefined;";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction35) {
    string input = "return NaN - \"cat\";";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction36) {
    string input = "return \"cat\" - NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction37) {
    string input = "return Infinity - \"cat\";";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction38) {
    string input = "return \"cat\" - Infinity;";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction39) {
    string input = "return true - \"cat\";";
    testEvaluationError(input);
}


TEST(EvaluationTest, subtraction40) {
    string input = "return \"cat\" - \"false\";";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction41) {
    string input = "return null - \"cat\";";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction42) {
    string input = "return \"cat\" - null;";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction43) {
    string input = "return NaN - NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction44) {
    string input = "return undefined - NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction45) {
    string input = "return NaN - undefined;";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction46) {
    string input = "return true - NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction47) {
    string input = "return NaN - true;";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction48) {
    string input = "return null - NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction49) {
    string input = "return NaN - null;";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction50) {
    string input = "return Infinity - NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction51) {
    string input = "return NaN - Infinity;";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction52) {
    string input = "return undefined - undefined;";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction53) {
    string input = "return undefined - Infinity;";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction54) {
    string input = "return Infinity - undefined;";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction55) {
    string input = "return undefined - true;";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction56) {
    string input = "return false - undefined;";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction57) {
    string input = "return undefined - null;";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction58) {
    string input = "return null - undefined;";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction59) {
    string input = "return Infinity - true;";
    testEvaluation(input, Value("Infinity"));
}

TEST(EvaluationTest, subtraction60) {
    string input = "return false - Infinity;";
    testEvaluation(input, Value("-Infinity"));
}

TEST(EvaluationTest, subtraction61) {
    string input = "return Infinity - Infinity;";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction62) {
    string input = "return Infinity - -Infinity;";
    testEvaluation(input, Value("Infinity"));
}

TEST(EvaluationTest, subtraction63) {
    string input = "return -Infinity - Infinity;";
    testEvaluation(input, Value("-Infinity"));
}

TEST(EvaluationTest, subtraction64) {
    string input = "return -Infinity - -Infinity;";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction65) {
    string input = "return Infinity - null;";
    testEvaluation(input, Value("Infinity"));
}

TEST(EvaluationTest, subtraction66) {
    string input = "return null - Infinity;";
    testEvaluation(input, Value("-Infinity"));
}

TEST(EvaluationTest, subtraction67) {
    string input = "return true - null;";
    testEvaluation(input, Value(1));
}

TEST(EvaluationTest, subtraction68) {
    string input = "return null - false;";
    testEvaluation(input, Value(0));
}

/* 
 * Division tests
 */

TEST(EvaluationTest, division1) {
    string input = "return 1 / 1;";
    testEvaluation(input, Value(1));
}

TEST(EvaluationTest, division2) {
    string input = "return 1 / \"cat\";";
    testEvaluationError(input);
}

TEST(EvaluationTest, division3) {
    string input = "return \"dog\" / 1;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division4) {
    string input = "return \"dog\" / \"cat\";";
    testEvaluationError(input);
}

TEST(EvaluationTest, division5) {
    string input = "return true / 1;";
    testEvaluation(input, Value(1));
}

TEST(EvaluationTest, division6) {
    string input = "return 3 / false;";
    testEvaluation(input, Value("Infinity"));
}

TEST(EvaluationTest, division7) {
    string input = "return false / null;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division8) {
    string input = "return null / true;";
    testEvaluation(input, Value(0));
}

TEST(EvaluationTest, division9) {
    string input = "return 1.5 / 1.5;";
    testEvaluation(input, Value(1));
}

TEST(EvaluationTest, division10) {
    string input = "return 1 / 0.5;";
    testEvaluation(input, Value(2));
}

TEST(EvaluationTest, division11) {
    string input = "return 1.5 / 1;";
    testEvaluation(input, Value(1.5));
}

TEST(EvaluationTest, division12) {
    string input = "return 0 / 0;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division13) {
    string input = "return 0 / undefined;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division14) {
    string input = "return undefined / 5;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division15) {
    string input = "return NaN / 5;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division16) {
    string input = "return 5 / NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division17) {
    string input = "return 3 / Infinity;";
    testEvaluation(input, Value(0));
}

TEST(EvaluationTest, division18) {
    string input = "return Infinity / 10;";
    testEvaluation(input, Value("Infinity"));
}

TEST(EvaluationTest, division19) {
    string input = "return [1, 2, 3] / 1;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division20) {
    string input = "return 5 / [1, 2, 3];";
    testEvaluationError(input);
}

TEST(EvaluationTest, division21) {
    string input = "return [1, 2, 3] / true;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division22) {
    string input = "return false / [1, 2, 3];";
    testEvaluationError(input);
}

TEST(EvaluationTest, division23) {
    string input = "return [1, 2, 3] / undefined;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division24) {
    string input = "return undefined / [1, 2, 3];";
    testEvaluationError(input);
}

TEST(EvaluationTest, division25) {
    string input = "return [1, 2, 3] / NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division26) {
    string input = "return NaN / [1, 2, 3];";
    testEvaluationError(input);
}

TEST(EvaluationTest, division27) {
    string input = "return [1, 2, 3] / Infinity;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division28) {
    string input = "return Infinity / [1, 2, 3];";
    testEvaluationError(input);
}

TEST(EvaluationTest, division29) {
    string input = "return [\"cat\", \"dog\"] / null;";
    testEvaluationError(input);
}


TEST(EvaluationTest, division30) {
    string input = "return null / [\"cat\", \"dog\"];";
    testEvaluationError(input);
}

TEST(EvaluationTest, division31) {
    string input = "return [1, 2, 3] / \"cat\";";
    testEvaluationError(input);
}

TEST(EvaluationTest, division32) {
    string input = "return \"cat\" / [1, 2, 3];";
    testEvaluationError(input);
}


TEST(EvaluationTest, division33) {
    string input = "return undefined / \"cat\";";
    testEvaluationError(input);
}

TEST(EvaluationTest, division34) {
    string input = "return \"cat\" / undefined;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division35) {
    string input = "return NaN / \"cat\";";
    testEvaluationError(input);
}

TEST(EvaluationTest, division36) {
    string input = "return \"cat\" / NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division37) {
    string input = "return Infinity / \"cat\";";
    testEvaluationError(input);
}

TEST(EvaluationTest, division38) {
    string input = "return \"cat\" / Infinity;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division39) {
    string input = "return true / \"cat\";";
    testEvaluationError(input);
}

TEST(EvaluationTest, division40) {
    string input = "return \"cat\" / \"false\";";
    testEvaluationError(input);
}

TEST(EvaluationTest, division41) {
    string input = "return null / \"cat\";";
    testEvaluationError(input);
}

TEST(EvaluationTest, division42) {
    string input = "return \"cat\" / null;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division43) {
    string input = "return NaN / NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division44) {
    string input = "return undefined / NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division45) {
    string input = "return NaN / undefined;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division46) {
    string input = "return true / NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division47) {
    string input = "return NaN / true;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division48) {
    string input = "return null / NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division49) {
    string input = "return NaN / null;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division50) {
    string input = "return Infinity / NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division51) {
    string input = "return NaN / Infinity;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division52) {
    string input = "return undefined / undefined;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division53) {
    string input = "return undefined / Infinity;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division54) {
    string input = "return Infinity / undefined;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division55) {
    string input = "return undefined / true;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division56) {
    string input = "return false / undefined;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division57) {
    string input = "return undefined / null;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division58) {
    string input = "return null / undefined;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division59) {
    string input = "return Infinity / true;";
    testEvaluation(input, Value("Infinity"));
}

TEST(EvaluationTest, division60) {
    string input = "return false / Infinity;";
    testEvaluation(input, Value(0));
}

TEST(EvaluationTest, division61) {
    string input = "return Infinity / Infinity;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division62) {
    string input = "return Infinity / -Infinity;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division63) {
    string input = "return -Infinity / Infinity;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division64) {
    string input = "return -Infinity / -Infinity;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division65) {
    string input = "return Infinity / null;";
    testEvaluation(input, Value("Infinity"));
}

TEST(EvaluationTest, division66) {
    string input = "return null / Infinity;";
    testEvaluation(input, Value(0));
}

TEST(EvaluationTest, division67) {
    string input = "return true / null;";
    testEvaluation(input, Value("Infinity"));
}

TEST(EvaluationTest, division68) {
    string input = "return null / false;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division69) {
    string input = "return 1 / 0;";
    testEvaluation(input, Value("Infinity"));
}

TEST(EvaluationTest, division70) {
    string input = "return -1 / 0;";
    testEvaluation(input, Value("-Infinity"));
}

} // namespace tinyjs
} // namespace mongo
