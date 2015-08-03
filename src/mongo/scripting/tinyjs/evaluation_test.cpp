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
    //std::cout << "about to lex " << input << std::endl;
    std::vector<Token> tokenData = lex(input).getValue();
    std::cout << "lexed successfully" << std::endl;
    ASTParser a(std::move(tokenData));
    std::cout << "made ASTParser" << std::endl;
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
    ASSERT_EQ(res,Value(std::nan("")));
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

    s->setObject("this", result);
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

TEST(EvaluationTest, superNestedObjectAccessorAndArray) {
    string input = "return this.y[\"d\"].j[1];";
    Scope* s = generateScope();
    testEvaluation(input, Value(2), s);
}

TEST(EvaluationTest, superNestedObjectAccessorAndArrayComparison) {
    string input = "return this.y[\"d\"].j[1] == 2;";
    Scope* s = generateScope();
    testEvaluation(input, Value(true), s);
}

TEST(EvaluationTest, superNestedObjectAccessorAndArrayComparison2) {
    string input = "return this.y[\"d\"].j[1] > this.y[\"d\"].j[0];";
    Scope* s = generateScope();
    testEvaluation(input, Value(true), s);
}

TEST(EvaluationTest, simple) {
    string input = "return 1;";
    testEvaluation(input, Value(1));
}

TEST(EvaluationTest, multiline) {
    string input = "3; return true;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, simpleNegation) {
    string input = "return -1;";
    testEvaluation(input, Value(-1));
}

TEST(EvaluationTest, assignment1) {
    string input = "var x = 1; \n return (x == 1);";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, assignment2) {
    string input = "var x = \"cat\"; \n return (x == \"dog\");";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, assignment3) {
    string input = "var x = \"cat\"; \n return (x == \"cat\");";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, assignment4) {
    string input = "x = 1; \n return (x == 1);";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, assignment5) {
    string input = "var x = 1;\nx = 2;\nreturn (x == 2);";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, test) {
    string input = "function() {var z = 42; z = 41; return this.x == 42;}";
    Scope* s = generateScope();
    testEvaluation(input, Value(true), s);
}

TEST(EvaluationTest, assignment6) {
    string input = "function() {var z = 42; z = 41; return this.x == z;}";
    Scope* s = generateScope();
    testEvaluation(input, Value(false), s);
}

TEST(EvaluationTest, assignment7) {
    string input = "function() {var z = 41; z = 42; return this.x == z;}";
    Scope* s = generateScope();
    testEvaluation(input, Value(true), s);
}


TEST(EvaluationTest, assignment8) {
    string input = "function() {var z = 41; z = z + 1; return this.x == z;}";
    Scope* s = generateScope();
    testEvaluation(input, Value(true), s);
}

TEST(EvaluationTest, whileLoop1) {
    string input = "function() {\n"
        "i = 0; \n"
        "x = 32;\n"
        "while (i < 10) {\n"
            "x = x + 1;\n"
            "i = i + 1;\n"
        "}\n"
        "return ((x == 42) && (this.x == x));\n"
    "}";
    Scope* s = generateScope();
    testEvaluation(input, Value(true), s);
}

TEST(EvaluationTest, whileLoop2) {
    string input = "function() {\n"
        "i = 0; \n"
        "x = 42;\n"
        "while (false) {\n"
            "x = x + 1;\n"
            "i = i + 1;\n"
        "}\n"
        "return ((x == 42) && (this.x == x));\n"
    "}";
    Scope* s = generateScope();
    testEvaluation(input, Value(true), s);
}

TEST(EvaluationTest, whileLoop3) {
    string input = "function() {\n"
        "i = 0; \n"
        "x = 42;\n"
        "while (i < 10) {\n"
            "return ((x == 42) && (this.x == x));\n"
            "x = x + 1;\n"
            "i = i + 1;\n"
        "}\n"
        "return ((x == 42) && (this.x == x));\n"
    "}";
    Scope* s = generateScope();
    testEvaluation(input, Value(true), s);
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
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, addition18) {
    string input = "return Infinity + 10;";
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
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
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, addition60) {
    string input = "return false + Infinity;";
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, addition61) {
    string input = "return Infinity + Infinity;";
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
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
    testEvaluation(input, Value(-std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, addition65) {
    string input = "return Infinity + null;";
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, addition66) {
    string input = "return null + Infinity;";
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
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
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, multiplication18) {
    string input = "return Infinity * 10;";
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
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
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, multiplication60) {
    string input = "return false * Infinity;";
    testEvaluationError(input);
}

TEST(EvaluationTest, multiplication61) {
    string input = "return Infinity * Infinity;";
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, multiplication62) {
    string input = "return Infinity * -Infinity;";
    testEvaluation(input, Value(-std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, multiplication63) {
    string input = "return -Infinity * Infinity;";
    testEvaluation(input, Value(-std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, multiplication64) {
    string input = "return -Infinity * -Infinity;";
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
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
    testEvaluation(input, Value(-std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, subtraction18) {
    string input = "return Infinity - 10;";
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
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
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, subtraction60) {
    string input = "return false - Infinity;";
    testEvaluation(input, Value(-std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, subtraction61) {
    string input = "return Infinity - Infinity;";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction62) {
    string input = "return Infinity - -Infinity;";
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, subtraction63) {
    string input = "return -Infinity - Infinity;";
    testEvaluation(input, Value(-std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, subtraction64) {
    string input = "return -Infinity - -Infinity;";
    testEvaluationError(input);
}

TEST(EvaluationTest, subtraction65) {
    string input = "return Infinity - null;";
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, subtraction66) {
    string input = "return null - Infinity;";
    testEvaluation(input, Value(-std::numeric_limits<double>::infinity()));
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
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
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
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
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
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
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
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, division66) {
    string input = "return null / Infinity;";
    testEvaluation(input, Value(0));
}

TEST(EvaluationTest, division67) {
    string input = "return true / null;";
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, division68) {
    string input = "return null / false;";
    testEvaluationError(input);
}

TEST(EvaluationTest, division69) {
    string input = "return 1 / 0;";
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, division70) {
    string input = "return -1 / 0;";
    testEvaluation(input, Value(-std::numeric_limits<double>::infinity()));
}

/*
 * LogicalAnd tests
 */

TEST(EvaluationTest, logicalAnd1) {
    string input = "return 1 && 1;";
    testEvaluation(input, Value(1));
}

TEST(EvaluationTest, logicalAnd2) {
    string input = "return 1 && \"cat\";";
    testEvaluation(input, Value("cat"));
}

TEST(EvaluationTest, logicalAnd3) {
    string input = "return \"dog\" && 0;";
    testEvaluation(input, Value(0));
}

TEST(EvaluationTest, logicalAnd4) {
    string input = "return \"dog\" && \"cat\";";
    testEvaluation(input, Value("cat"));
}

TEST(EvaluationTest, logicalAnd5) {
    string input = "return true && 1;";
    testEvaluation(input, Value(1));
}

TEST(EvaluationTest, logicalAnd6) {
    string input = "return 3 && false;";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, logicalAnd7) {
    string input = "return false && null;";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, logicalAnd8) {
    string input = "return null && true;";
    testEvaluation(input, Value(BSONNULL));
}

TEST(EvaluationTest, logicalAnd9) {
    string input = "return 1.5 && 2.5;";
    testEvaluation(input, Value(2.5));
}

TEST(EvaluationTest, logicalAnd10) {
    string input = "return 1 && 1.5;";
    testEvaluation(input, Value(1.5));
}

TEST(EvaluationTest, logicalAnd11) {
    string input = "return 1.5 && 1;";
    testEvaluation(input, Value(1));
}

TEST(EvaluationTest, logicalAnd12) {
    string input = "return 0 && 0;";
    testEvaluation(input, Value(0));
}

TEST(EvaluationTest, logicalAnd13) {
    string input = "return 0 && undefined;";
    testEvaluation(input, Value(0));
}

TEST(EvaluationTest, logicalAnd14) {
    string input = "return undefined && 5;";
    testEvaluation(input, Value(BSONUndefined));
}

TEST(EvaluationTest, logicalAnd15) {
    string input = "return NaN && 5;";
    testEvaluationError(input);
}

TEST(EvaluationTest, logicalAnd16) {
    string input = "return 5 && NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, logicalAnd17) {
    string input = "return 3 && Infinity;";
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, logicalAnd18) {
    string input = "return Infinity && 10;";
    testEvaluation(input, Value(10));
}

TEST(EvaluationTest, logicalAnd19) {
    string input = "return [1, 2, 3] && 1;";
    testEvaluation(input, Value(1));
}

TEST(EvaluationTest, logicalAnd20) {
    string input = "return 5 && [1, 2, 3];";
    testEvaluation(input, Value({Value(1), Value(2), Value(3)}));
}

TEST(EvaluationTest, logicalAnd21) {
    string input = "return [1, 2, 3] && true;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, logicalAnd22) {
    string input = "return false && [1, 2, 3];";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, logicalAnd23) {
    string input = "return [1, 2, 3] && undefined;";
    testEvaluation(input, Value(BSONUndefined));
}

TEST(EvaluationTest, logicalAnd24) {
    string input = "return undefined && [1, 2, 3];";
    testEvaluation(input, Value(BSONUndefined));
}

TEST(EvaluationTest, logicalAnd25) {
    string input = "return [1, 2, 3] && NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, logicalAnd26) {
    string input = "return NaN && [1, 2, 3];";
    testEvaluationError(input);
}

TEST(EvaluationTest, logicalAnd27) {
    string input = "return [1, 2, 3] && Infinity;";
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, logicalAnd28) {
    string input = "return Infinity && [1, 2, 3];";
    testEvaluation(input, Value({Value(1), Value(2), Value(3)}));
}

TEST(EvaluationTest, logicalAnd29) {
    string input = "return [\"cat\", \"dog\"] && null;";
    testEvaluation(input, Value(BSONNULL));
}


TEST(EvaluationTest, logicalAnd30) {
    string input = "return null && [\"cat\", \"dog\"];";
    testEvaluation(input, Value(BSONNULL));
}

TEST(EvaluationTest, logicalAnd31) {
    string input = "return [1, 2, 3] && \"cat\";";
    testEvaluation(input, Value("cat"));
}

TEST(EvaluationTest, logicalAnd32) {
    string input = "return \"cat\" && [1, 2, 3];";
    testEvaluation(input, Value({Value(1), Value(2), Value(3)}));
}


TEST(EvaluationTest, logicalAnd33) {
    string input = "return undefined && \"cat\";";
    testEvaluation(input, Value(BSONUndefined));
}

TEST(EvaluationTest, logicalAnd34) {
    string input = "return \"cat\" && undefined;";
    testEvaluation(input, Value(BSONUndefined));
}

TEST(EvaluationTest, logicalAnd35) {
    string input = "return NaN && \"cat\";";
    testEvaluationError(input);
}

TEST(EvaluationTest, logicalAnd36) {
    string input = "return \"cat\" && NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, logicalAnd37) {
    string input = "return Infinity && \"cat\";";
    testEvaluation(input, Value("cat"));
}

TEST(EvaluationTest, logicalAnd38) {
    string input = "return \"cat\" && Infinity;";
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, logicalAnd39) {
    string input = "return true && \"cat\";";
    testEvaluation(input, Value("cat"));
}


TEST(EvaluationTest, logicalAnd40) {
    string input = "return \"cat\" && false;";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, logicalAnd41) {
    string input = "return null && \"cat\";";
    testEvaluation(input, Value(BSONNULL));
}

TEST(EvaluationTest, logicalAnd42) {
    string input = "return \"cat\" && null;";
    testEvaluation(input, Value(BSONNULL));
}

TEST(EvaluationTest, logicalAnd43) {
    string input = "return NaN && NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, logicalAnd44) {
    string input = "return undefined && NaN;";
    testEvaluation(input, Value(BSONUndefined));
}

TEST(EvaluationTest, logicalAnd45) {
    string input = "return NaN && undefined;";
    testEvaluationError(input);
}

TEST(EvaluationTest, logicalAnd46) {
    string input = "return true && NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, logicalAnd47) {
    string input = "return NaN && true;";
    testEvaluationError(input);
}

TEST(EvaluationTest, logicalAnd48) {
    string input = "return null && NaN;";
    testEvaluation(input, Value(BSONNULL));
}

TEST(EvaluationTest, logicalAnd49) {
    string input = "return NaN && null;";
    testEvaluationError(input);
}

TEST(EvaluationTest, logicalAnd50) {
    string input = "return Infinity && NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, logicalAnd51) {
    string input = "return NaN && Infinity;";
    testEvaluationError(input);
}

TEST(EvaluationTest, logicalAnd52) {
    string input = "return undefined && undefined;";
    testEvaluation(input, Value(BSONUndefined));
}

TEST(EvaluationTest, logicalAnd53) {
    string input = "return undefined && Infinity;";
    testEvaluation(input, Value(BSONUndefined));
}

TEST(EvaluationTest, logicalAnd54) {
    string input = "return Infinity && undefined;";
    testEvaluation(input, Value(BSONUndefined));
}

TEST(EvaluationTest, logicalAnd55) {
    string input = "return undefined && true;";
    testEvaluation(input, Value(BSONUndefined));
}

TEST(EvaluationTest, logicalAnd56) {
    string input = "return false && undefined;";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, logicalAnd57) {
    string input = "return undefined && null;";
    testEvaluation(input, Value(BSONUndefined));
}

TEST(EvaluationTest, logicalAnd58) {
    string input = "return null && undefined;";
    testEvaluation(input, Value(BSONNULL));
}

TEST(EvaluationTest, logicalAnd59) {
    string input = "return Infinity && true;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, logicalAnd60) {
    string input = "return false && Infinity;";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, logicalAnd61) {
    string input = "return Infinity && Infinity;";
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, logicalAnd62) {
    string input = "return Infinity && -Infinity;";
    testEvaluation(input, Value(-std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, logicalAnd63) {
    string input = "return -Infinity && Infinity;";
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, logicalAnd64) {
    string input = "return -Infinity && -Infinity;";
    testEvaluation(input, Value(-std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, logicalAnd65) {
    string input = "return Infinity && null;";
    testEvaluation(input, Value(BSONNULL));
}

TEST(EvaluationTest, logicalAnd66) {
    string input = "return null && Infinity;";
    testEvaluation(input, Value(BSONNULL));
}

TEST(EvaluationTest, logicalAnd67) {
    string input = "return true && null;";
    testEvaluation(input, Value(BSONNULL));
}

TEST(EvaluationTest, logicalAnd68) {
    string input = "return null && false;";
    testEvaluation(input, Value(BSONNULL));
}

TEST(EvaluationTest, logicalAnd69) {
    string input = "return 1 && -1;";
    testEvaluation(input, Value(-1));
}

TEST(EvaluationTest, logicalAnd70) {
    string input = "return -1 && 1;";
    testEvaluation(input, Value(1));
}


TEST(EvaluationTest, logicalAnd71) {
    string input = "return \"dog\" && 5;";
    testEvaluation(input, Value(5));
}

TEST(EvaluationTest, logicalAnd72) {
    string input = "return 0 && \"cat\";";
    testEvaluation(input, Value(0));
}

/*
 * LogicalOr tests
 */

TEST(EvaluationTest, logicalOr1) {
    string input = "return 1 || 1;";
    testEvaluation(input, Value(1));
}

TEST(EvaluationTest, logicalOr2) {
    string input = "return 1 || \"cat\";";
    testEvaluation(input, Value(1));
}

TEST(EvaluationTest, logicalOr3) {
    string input = "return \"dog\" || 0;";
    testEvaluation(input, Value("dog"));
}

TEST(EvaluationTest, logicalOr4) {
    string input = "return \"dog\" || \"cat\";";
    testEvaluation(input, Value("dog"));
}

TEST(EvaluationTest, logicalOr5) {
    string input = "return true || 1;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, logicalOr6) {
    string input = "return 3 || false;";
    testEvaluation(input, Value(3));
}

TEST(EvaluationTest, logicalOr7) {
    string input = "return false || null;";
    testEvaluation(input, Value(BSONNULL));
}

TEST(EvaluationTest, logicalOr8) {
    string input = "return null || true;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, logicalOr9) {
    string input = "return 1.5 || 2.5;";
    testEvaluation(input, Value(1.5));
}

TEST(EvaluationTest, logicalOr10) {
    string input = "return 1 || 1.5;";
    testEvaluation(input, Value(1));
}

TEST(EvaluationTest, logicalOr11) {
    string input = "return 1.5 || 1;";
    testEvaluation(input, Value(1.5));
}

TEST(EvaluationTest, logicalOr12) {
    string input = "return 0 || 0;";
    testEvaluation(input, Value(0));
}

TEST(EvaluationTest, logicalOr13) {
    string input = "return 0 || undefined;";
    testEvaluation(input, Value(BSONUndefined));
}

TEST(EvaluationTest, logicalOr14) {
    string input = "return undefined || 5;";
    testEvaluation(input, Value(5));
}

TEST(EvaluationTest, logicalOr15) {
    string input = "return NaN || 5;";
    testEvaluation(input, Value(5));
}

TEST(EvaluationTest, logicalOr16) {
    string input = "return 5 || NaN;";
    testEvaluation(input, Value(5));
}

TEST(EvaluationTest, logicalOr17) {
    string input = "return 3 || Infinity;";
    testEvaluation(input, Value(3));
}

TEST(EvaluationTest, logicalOr18) {
    string input = "return Infinity || 10;";
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, logicalOr19) {
    string input = "return [1, 2, 3] || 1;";
    testEvaluation(input, Value({Value(1), Value(2), Value(3)}));
}

TEST(EvaluationTest, logicalOr20) {
    string input = "return 5 || [1, 2, 3];";
    testEvaluation(input, Value(5));
}

TEST(EvaluationTest, logicalOr21) {
    string input = "return [1, 2, 3] || true;";
    testEvaluation(input, Value({Value(1), Value(2), Value(3)}));
}

TEST(EvaluationTest, logicalOr22) {
    string input = "return false || [1, 2, 3];";
    testEvaluation(input, Value({Value(1), Value(2), Value(3)}));
}

TEST(EvaluationTest, logicalOr23) {
    string input = "return [1, 2, 3] || undefined;";
    testEvaluation(input, Value({Value(1), Value(2), Value(3)}));
}

TEST(EvaluationTest, logicalOr24) {
    string input = "return undefined || [1, 2, 3];";
    testEvaluation(input, Value({Value(1), Value(2), Value(3)}));
}

TEST(EvaluationTest, logicalOr25) {
    string input = "return [1, 2, 3] || NaN;";
    testEvaluation(input, Value({Value(1), Value(2), Value(3)}));
}

TEST(EvaluationTest, logicalOr26) {
    string input = "return NaN || [1, 2, 3];";
    testEvaluation(input, Value({Value(1), Value(2), Value(3)}));
}

TEST(EvaluationTest, logicalOr27) {
    string input = "return [1, 2, 3] || Infinity;";
    testEvaluation(input, Value({Value(1), Value(2), Value(3)}));
}

TEST(EvaluationTest, logicalOr28) {
    string input = "return Infinity || [1, 2, 3];";
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, logicalOr29) {
    string input = "return [\"cat\", \"dog\"] || null;";
    std::vector<Value> vec; 
    vec.push_back(Value("cat"));
    vec.push_back(Value("dog"));
    testEvaluation(input, Value(vec));
}


TEST(EvaluationTest, logicalOr30) {
    string input = "return null || [\"cat\", \"dog\"];";
    std::vector<Value> vec; 
    vec.push_back(Value("cat"));
    vec.push_back(Value("dog"));
    testEvaluation(input, Value(vec));
}

TEST(EvaluationTest, logicalOr31) {
    string input = "return [1, 2, 3] || \"cat\";";
    testEvaluation(input, Value({Value(1), Value(2), Value(3)}));
}

TEST(EvaluationTest, logicalOr32) {
    string input = "return \"cat\" || [1, 2, 3];";
    testEvaluation(input, Value("cat"));
}


TEST(EvaluationTest, logicalOr33) {
    string input = "return undefined || \"cat\";";
    testEvaluation(input, Value("cat"));
}

TEST(EvaluationTest, logicalOr34) {
    string input = "return \"cat\" || undefined;";
    testEvaluation(input, Value("cat"));
}

TEST(EvaluationTest, logicalOr35) {
    string input = "return NaN || \"cat\";";
    testEvaluation(input, Value("cat"));
}

TEST(EvaluationTest, logicalOr36) {
    string input = "return \"cat\" || NaN;";
    testEvaluation(input, Value("cat"));
}

TEST(EvaluationTest, logicalOr37) {
    string input = "return Infinity || \"cat\";";
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, logicalOr38) {
    string input = "return \"cat\" || Infinity;";
    testEvaluation(input, Value("cat"));
}

TEST(EvaluationTest, logicalOr39) {
    string input = "return true || \"cat\";";
    testEvaluation(input, Value(true));
}


TEST(EvaluationTest, logicalOr40) {
    string input = "return \"cat\" || false;";
    testEvaluation(input, Value("cat"));
}

TEST(EvaluationTest, logicalOr41) {
    string input = "return null || \"cat\";";
    testEvaluation(input, Value("cat"));
}

TEST(EvaluationTest, logicalOr42) {
    string input = "return \"cat\" || null;";
    testEvaluation(input, Value("cat"));
}

TEST(EvaluationTest, logicalOr43) {
    string input = "return NaN || NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, logicalOr44) {
    string input = "return undefined || NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, logicalOr45) {
    string input = "return NaN || undefined;";
    testEvaluation(input, Value(BSONUndefined));
}

TEST(EvaluationTest, logicalOr46) {
    string input = "return true || NaN;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, logicalOr47) {
    string input = "return NaN || true;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, logicalOr48) {
    string input = "return null || NaN;";
    testEvaluationError(input);
}

TEST(EvaluationTest, logicalOr49) {
    string input = "return NaN || null;";
    testEvaluation(input, Value(BSONNULL));
}

TEST(EvaluationTest, logicalOr50) {
    string input = "return Infinity || NaN;";
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, logicalOr51) {
    string input = "return NaN || Infinity;";
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, logicalOr52) {
    string input = "return undefined || undefined;";
    testEvaluation(input, Value(BSONUndefined));
}

TEST(EvaluationTest, logicalOr53) {
    string input = "return undefined || Infinity;";
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, logicalOr54) {
    string input = "return Infinity || undefined;";
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, logicalOr55) {
    string input = "return undefined || true;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, logicalOr56) {
    string input = "return false || undefined;";
    testEvaluation(input, Value(BSONUndefined));
}

TEST(EvaluationTest, logicalOr57) {
    string input = "return undefined || null;";
    testEvaluation(input, Value(BSONNULL));
}

TEST(EvaluationTest, logicalOr58) {
    string input = "return null || undefined;";
    testEvaluation(input, Value(BSONUndefined));
}

TEST(EvaluationTest, logicalOr59) {
    string input = "return Infinity || true;";
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, logicalOr60) {
    string input = "return false || Infinity;";
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, logicalOr61) {
    string input = "return Infinity || Infinity;";
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, logicalOr62) {
    string input = "return Infinity || -Infinity;";
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, logicalOr63) {
    string input = "return -Infinity || Infinity;";
    testEvaluation(input, Value(-std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, logicalOr64) {
    string input = "return -Infinity || -Infinity;";
    testEvaluation(input, Value(-std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, logicalOr65) {
    string input = "return Infinity || null;";
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, logicalOr66) {
    string input = "return null || Infinity;";
    testEvaluation(input, Value(std::numeric_limits<double>::infinity()));
}

TEST(EvaluationTest, logicalOr67) {
    string input = "return true || null;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, logicalOr68) {
    string input = "return null || false;";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, logicalOr69) {
    string input = "return 1 || -1;";
    testEvaluation(input, Value(1));
}

TEST(EvaluationTest, logicalOr70) {
    string input = "return -1 || 1;";
    testEvaluation(input, Value(-1));
}

TEST(EvaluationTest, logicalOr71) {
    string input = "return \"dog\" || 5;";
    testEvaluation(input, Value("dog"));
}

TEST(EvaluationTest, logicalOr72) {
    string input = "return 0 || \"cat\";";
    testEvaluation(input, Value("cat"));
}

TEST(EvaluationTest, logicalOr73) {
    string input = "return [] || 0;";
    std::vector<Value> vec;
    testEvaluation(input, Value(vec));
}

TEST(EvaluationTest, logicalOr74) {
    string input = "return 0 || [];";
    std::vector<Value> vec;
    testEvaluation(input, Value(vec));
}

/* 
 * Loose equality tests
 */

TEST(EvaluationTest, looseEquality1) {
    string input = "return true == true;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality2) {
    string input = "return true == 1;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality3) {
    string input = "return true == \"1\";";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality4) {
    string input = "return true == [1];";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality5) {
    string input = "return false == false;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality6) {
    string input = "return false == 0;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality7) {
    string input = "return false == \"0\";";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality8) {
    string input = "return false == \"\";";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality9) {
    string input = "return false == [];";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality10) {
    string input = "return false == [[]];";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality11) {
    string input = "return false == [0];";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality12) {
    string input = "return 1 == true;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality13) {
    string input = "return 1 == 1;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality14) {
    string input = "return 1 == \"1\";";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality15) {
    string input = "return 1 == [1];";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality16) {
    string input = "return 0 == false;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality17) {
    string input = "return 0 == 0;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality18) {
    string input = "return 0 == \"0\";";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality19) {
    string input = "return 0 == \"\";";
    testEvaluation(input, Value(true));
}


TEST(EvaluationTest, looseEquality20) {
    string input = "return 0 == [];";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality21) {
    string input = "return 0 == [[]];";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality22) {
    string input = "return 0 == [0];";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality23) {
    string input = "return -1 == -1;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality24) {
    string input = "return -1 == \"-1\";";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality25) {
    string input = "return \"true\" == \"true\";";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality26) {
    string input = "return \"false\" == \"false\";";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality27) {
    string input = "return \"1\" == true;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality28) {
    string input = "return \"1\" == 1;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality29) {
    string input = "return \"1\" == \"1\";";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality30) {
    string input = "return \"1\" == [1];";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality31) {
    string input = "return \"0\" == false;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality32) {
    string input = "return \"0\" == 0;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality33) {
    string input = "return \"0\" == \"0\";";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality34) {
    string input = "return \"0\" == [0];";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality35) {
    string input = "return \"\" == false;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality36) {
    string input = "return \"\" == 0;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality37) {
    string input = "return \"\" == \"\";";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality38) {
    string input = "return \"\" == [];";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality39) {
    string input = "return \"\" == [[]];";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality40) {
    string input = "return null == null;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality41) {
    string input = "return null == undefined;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality42) {
    string input = "return undefined == null;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality43) {
    string input = "return undefined == undefined;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality44) {
    string input = "return Infinity == Infinity;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality45) {
    string input = "return -Infinity == -Infinity;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality46) {
    string input = "return [] == false;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality47) {
    string input = "return [] == 0;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality48) {
    string input = "return [] == \"\";";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, looseEquality49) {
    string input = "return [[]] == false;";
    testEvaluation(input, Value(true));
}

// Some negative tests, not extensive

TEST(EvaluationTest, looseEquality50) {
    string input = "return false == true;";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, looseEquality51) {
    string input = "return false == \"false\";";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, looseEquality52) {
    string input = "return 1 == null;";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, looseEquality53) {
    string input = "return 2 == [1];";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, looseEquality54) {
    string input = "return undefined == -1;";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, looseEquality55) {
    string input = "return [] == [];";
    testEvaluation(input, Value(false));
}


/* 
 * Strict equality tests
 */

TEST(EvaluationTest, strictEquality1) {
    string input = "return true === true;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, strictEquality2) {
    string input = "return true === 1;";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality3) {
    string input = "return true === \"1\";";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality4) {
    string input = "return true === [1];";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality5) {
    string input = "return false === false;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, strictEquality6) {
    string input = "return false === 0;";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality7) {
    string input = "return false === \"0\";";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality8) {
    string input = "return false === \"\";";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality9) {
    string input = "return false === [];";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality10) {
    string input = "return false === [[]];";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality11) {
    string input = "return false === [0];";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality12) {
    string input = "return 1 === false;";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality13) {
    string input = "return 1 === 1;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, strictEquality14) {
    string input = "return 1 === \"1\";";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality15) {
    string input = "return 1 === [1];";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality16) {
    string input = "return 0 === false;";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality17) {
    string input = "return 0 === 0;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, strictEquality18) {
    string input = "return 0 === \"0\";";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality19) {
    string input = "return 0 === \"\";";
    testEvaluation(input, Value(false));
}


TEST(EvaluationTest, strictEquality20) {
    string input = "return 0 === [];";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality21) {
    string input = "return 0 === [[]];";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality22) {
    string input = "return 0 === [0];";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality23) {
    string input = "return -1 === -1;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, strictEquality24) {
    string input = "return -1 === \"-1\";";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality25) {
    string input = "return \"true\" === \"true\";";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, strictEquality26) {
    string input = "return \"false\" === \"false\";";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, strictEquality27) {
    string input = "return \"1\" === false;";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality28) {
    string input = "return \"1\" === 1;";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality29) {
    string input = "return \"1\" === \"1\";";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, strictEquality30) {
    string input = "return \"1\" === [1];";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality31) {
    string input = "return \"0\" === false;";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality32) {
    string input = "return \"0\" === 0;";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality33) {
    string input = "return \"0\" === \"0\";";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, strictEquality34) {
    string input = "return \"0\" === [0];";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality35) {
    string input = "return \"\" === false;";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality36) {
    string input = "return \"\" === 0;";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality37) {
    string input = "return \"\" === \"\";";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, strictEquality38) {
    string input = "return \"\" === [];";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality39) {
    string input = "return \"\" === [[]];";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality40) {
    string input = "return null === null;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, strictEquality41) {
    string input = "return null === undefined;";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality42) {
    string input = "return undefined === null;";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality43) {
    string input = "return undefined === undefined;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, strictEquality44) {
    string input = "return Infinity === Infinity;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, strictEquality45) {
    string input = "return -Infinity === -Infinity;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, strictEquality46) {
    string input = "return [] === false;";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality47) {
    string input = "return [] === 0;";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality48) {
    string input = "return [] === \"\";";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality49) {
    string input = "return [[]] === false;";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality50) {
    string input = "return false === false;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, strictEquality51) {
    string input = "return false === \"false\";";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality52) {
    string input = "return 1 === null;";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality53) {
    string input = "return 2 === [1];";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality54) {
    string input = "return undefined === -1;";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, strictEquality55) {
    string input = "return [] === [];";
    testEvaluation(input, Value(false));
}

/*
 * Comparison tests
 */

TEST(EvaluationTest, comparison1) {
    string input = "return 1 < 2;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, comparison2) {
    string input = "return 1 <= 2;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, comparison3) {
    string input = "return 1 > 2;";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, comparison4) {
    string input = "return 1 >= 2;";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, comparison5) {
    string input = "return 1 <= 1;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, comparison6) {
    string input = "return 1 >= 1;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, comparison7) {
    string input = "return \"cat\" < \"dog\";";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, comparison81) {
    string input = "return \"cat\" <= \"dog\";";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, comparison9) {
    string input = "return \"cat\" > \"dog\";";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, comparison10) {
    string input = "return \"cat\" >= \"dog\";";
    testEvaluation(input, Value(false));
}

TEST(EvaluationTest, comparison11) {
    string input = "return [2] > 1;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, comparison12) {
    string input = "return [2] >= 1;";
    testEvaluation(input, Value(true));
}
/*
TEST(EvaluationTest, comparison13) {
    string input = "return [0] < 2;";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, comparison14) {
    string input = "return [0] <= 2;";
    testEvaluation(input, Value(true));
}*/

TEST(EvaluationTest, comparison15) {
    string input = "return [2] < [3];";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, comparison16) {
    string input = "return [2, 3] < [3];";
    testEvaluation(input, Value(true));
}

TEST(EvaluationTest, comparison17) {
    string input = "return [2, 3, 3] < [4, 1];";
    testEvaluation(input, Value(true));
}
/*
TEST(EvaluationTest, comparison18) {
    string input = "return 1 < [4, 5, 6];";
    testEvaluation(input, Value(false));
}*/

} // namespace tinyjs
} // namespace mongo
