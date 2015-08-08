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

#include "mongo/db/pipeline/value.h"
#include "mongo/db/matcher/expression.h"
#include "mongo/db/matcher/expression_leaf.h"
#include "mongo/db/matcher/expression_tree.h"
#include "mongo/scripting/tinyjs/lexer.h"
#include "mongo/scripting/tinyjs/scope.h"

#include <iostream>


namespace mongo {
namespace tinyjs {

class Node {
public:
    virtual ~Node() {};
    virtual const Value evaluate(Scope* scope, Value& returnValue) const = 0;
    virtual std::vector<Node* > getChildren() const = 0;
    virtual StringData getName() const = 0;
    TokenType getType() const { return _type; };
    /* 
     * returns a bool that determines if the subtree contains at least 1 object access
     * input: if optimize is true, when it hits a comparison operator, it will attempt to optimize 
     */ 
    virtual bool optimizable(bool optimize=false, AndMatchExpression* root = nullptr) { std::cout<<"noop"<<std::endl;return false; };
protected:
     Node(TokenType t) : _type(t) {};
private:
    TokenType _type;
};

} // namespace tinyjs
} // namespace mongo
