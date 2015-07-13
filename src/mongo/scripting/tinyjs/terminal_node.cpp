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

#include "mongo/bson/bsontypes.h"
#include "mongo/scripting/tinyjs/terminal_node.h"


namespace mongo {
namespace tinyjs {

TerminalNode::TerminalNode(const NullLabeler&) : _value(Value(BSONNULL)) {}
TerminalNode::TerminalNode(const UndefinedLabeler&) : _value(Value(BSONUndefined)) {}
TerminalNode::TerminalNode(const int& value) : _value(Value(value)) {}
TerminalNode::TerminalNode(const double& value) : _value(Value(value)) {}
TerminalNode::TerminalNode(const bool& value) : _value(Value(value)) {}
TerminalNode::TerminalNode(const StringData& value) : _value(Value(value)) {}

std::vector<Node*> TerminalNode::getChildren() const {
    return std::vector<Node*>();
}

const Value TerminalNode::evaluate(Scope* scope) const {
    return _value;
}

StringData TerminalNode::getName() const {
    std::string res = _value.toString();
    if (res.front() == '"') {
        return StringData(res.substr(1, res.size() - 2));
    }
    return StringData(res);
}

}  // namespace tinyjs
}  // namespace mongo
