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

namespace mongo {
namespace tinyjs {

class Scope {
public:
    Scope();
    Scope(Scope* parent);
    /* 
     * This function inserts a variableName and value pair into the scope, regardless of whether that variableName was already in scope.
     */
    void put(StringData variableName, Value* value);
    Value* get(StringData variableName);
    bool isInScope(StringData variableName);
    void setParent(Scope* parent);
    Scope* getParent();
private:
    Scope* _parent;
    std::map<StringData, Value*> _variables;
};

} // namespace tinyjs
} // namespace mongo