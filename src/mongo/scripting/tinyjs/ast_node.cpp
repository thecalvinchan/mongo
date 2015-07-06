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

#include "mongo/scripting/tinyjs/ast_node.h"

namespace mongo {
namespace tinyjs {

Node::Node(std::string name) {
    _name = name;
}

Node::~Node() {
    for (std::vector<std::unique_ptr<Node>>::iterator it = _children.begin();
         it != _children.end(); it++) {
        (*it).reset();
    }
    _children.clear();
}

void Node::addChild(std::unique_ptr<Node> child) {
    if (child) {
        _children.push_back(std::move(child));
    }
}

std::string Node::getValue() {
    std::string res = this->getName();
    for (std::vector<std::unique_ptr<Node>>::const_iterator it = _children.begin();
         it != _children.end(); it++) {
        res += " ";
        res += ((*it).get())->getValue();
    }
    return res;
}

std::vector<std::unique_ptr<Node> >* Node::getChildren() {
    return &_children;
}

}
}
