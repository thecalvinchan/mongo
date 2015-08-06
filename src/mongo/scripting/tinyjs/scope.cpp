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

#include "mongo/scripting/tinyjs/scope.h"
#include "mongo/scripting/tinyjs/lexer.h"
#include "mongo/scripting/tinyjs/ast_parser.h"

namespace mongo {
namespace tinyjs {

Scope::Scope(): _parent(nullptr) {}

Scope::Scope(Scope* parent) : _parent(parent) {}


void Scope::setObject(const char* field, const BSONObj& obj, bool readOnly) {
    _thisDocument = obj;
}

bool Scope::getBoolean(const char* field) {
    if (strcmp(field,"__returnValue") == 0) {
        return _currentResult;
    } else {
        return false;
    }
}

ScriptingFunction Scope::createFunction(const char* code, std::unique_ptr<AndMatchExpression> root) {
    ScriptingFunction func = _funcs.size();
    _createFunction(code, std::move(root), func);
    return func + 1;
}

ScriptingFunction Scope::_createFunction(const char* code, std::unique_ptr<AndMatchExpression> root, 
                                              ScriptingFunction functionNumber) {
    std::cout << "in createFunction with root" << std::endl;
    std::string input(code);
    std::vector<Token> tokenData = lex(input).getValue();
    ASTParser* func = new ASTParser(std::move(tokenData));
    if (root != nullptr) {
        std::cout << "about to optimize" << std::endl;
        func->optimize(std::move(root));
        std::cout << "finished optimize" << std::endl;
    }
    _funcs.push_back(func);
    return functionNumber;
}

ScriptingFunction Scope::_createFunction(const char* code, ScriptingFunction functionNumber) {
    return _createFunction(code, nullptr, functionNumber);
}

int Scope::invoke(ScriptingFunction func,
           const BSONObj* args,
           const BSONObj* recv,
           int timeoutMs,
           bool ignoreReturn,
           bool readOnlyArgs,
           bool readOnlyRecv) {
    init(recv);
    _currentResult = _funcs[func - 1]->evaluate(this).getBool();
    return 0;
}


void Scope::put(StringData variableName, const Value& value) {
    _variables[variableName] = value;
}

Value Scope::get(StringData variableName) const {

    std::map<StringData, Value>::const_iterator it = _variables.find(variableName);

    if (it != _variables.end()) {
        // Case where variableName is found in this scope
        return it->second;
    } else if (_parent) {
        // Recursively check in the parent scope, if a parent scope exists
        return _parent->get(variableName);
    } else {
        // variable is out of scope
        return Value();
    }
}

BSONObj Scope::getDocument() const {
    return _thisDocument;
}

Scope* Scope::getParent() const {
    return _parent;
}

}  // namespace tinyjs
}  // namespace mongo
