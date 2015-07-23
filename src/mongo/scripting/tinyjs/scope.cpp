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

#include "mongo/scripting/tinyjs/scope.h"
#include "mongo/scripting/tinyjs/lexer.h"
#include "mongo/scripting/tinyjs/ast_parser.h"

namespace mongo {
namespace tinyjs {

Scope::Scope() {}

Scope::Scope(Scope* parent) : _parent(parent) {}


void Scope::init(const BSONObj* data) {
    if (!data)
        return;

    Value object = Value(*data);
    put(StringData("this"), object);
}

bool Scope::getBoolean(const char* field) {
    if (strcmp(field,"__returnValue") == 0) {
        return _currentResult;
    } else {
        return false;
    }
}

ScriptingFunction Scope::createFunction(const char* code) {
    ScriptingFunction func = _funcs.size();
    _createFunction(code, func);
    return func;
}

ScriptingFunction Scope::_createFunction(const char* code,
                                              ScriptingFunction functionNumber) {
    std::string input(code);
    std::vector<Token> tokenData = lex(input).getValue();
    ASTParser* func = new ASTParser(std::move(tokenData));
    _funcs.push_back(func);
    return functionNumber;
}

int Scope::invoke(ScriptingFunction func,
           const BSONObj* args,
           const BSONObj* recv,
           int timeoutMs,
           bool ignoreReturn,
           bool readOnlyArgs,
           bool readOnlyRecv) {
    _currentResult = _funcs[func]->evaluate(this).getBool();
    return 0;
}


void Scope::put(StringData variableName, Value value) {
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
        return Value(BSONUndefined);
    }
}

const Scope* Scope::getParent() const {
    return _parent;
}

}  // namespace tinyjs
}  // namespace mongo
