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

#include "mongo/base/string_data.h"
#include "mongo/db/pipeline/value.h"
#include "mongo/scripting/engine.h"

namespace mongo {
namespace tinyjs {

class ASTParser;

class Scope : public mongo::Scope {
public:
    Scope();
    Scope(Scope* parent);

    void reset() override {}
    void init(const BSONObj* data) override {}
    void registerOperation(OperationContext* txn) override {}
    void unregisterOperation() override {}
    void localConnectForDbEval(OperationContext* txn, const char* dbName) override {}
    void externalSetup() override {}
    BSONObj getObject(const char* field) override {
        return BSONObj();
    }
    std::string getString(const char* field) override {
        return NULL;
    }
    bool getBoolean(const char* field) override;  // IMPLEMENT
    double getNumber(const char* field) override {
        return 0.0;
    }
    void setElement(const char* field, const BSONElement& e) override {}
    void setNumber(const char* field, double val) override {}
    void setString(const char* field, StringData val) override {}
    void setObject(const char* field, const BSONObj& obj, bool readOnly = true) override;
    void setBoolean(const char* field, bool val) override {}
    void setFunction(const char* field, const char* code) override {}
    int type(const char* field) override {
        return 0;
    }
    void rename(const char* from, const char* to) override {}
    std::string getError() override {
        return "error";
    }
    bool hasOutOfMemoryException() override {
        return false;
    }
    bool isKillPending() const override {
        return false;
    }
    void gc() override {}
    ScriptingFunction createFunction(const char* code) override;  // IMPLEMENT
    /**
    * @return 0 on success
    */
    int invoke(ScriptingFunction func,
               const BSONObj* args,
               const BSONObj* recv,
               int timeoutMs = 0,
               bool ignoreReturn = false,
               bool readOnlyArgs = false,
               bool readOnlyRecv = false) override;  // IMPLEMENT
    void injectNative(const char* field, NativeFunction func, void* data = 0) override {}
    bool exec(StringData code,
              const std::string& name,
              bool printResult,
              bool reportError,
              bool assertOnError,
              int timeoutMs = 0) override {
        return false;
    }

    /*
     * This function inserts a variableName and value pair into the scope, regardless of whether
     * that variableName was already in scope.
     */
    void put(StringData variableName, const Value& value);
    Value get(StringData variableName) const;
    BSONObj getDocument() const;
    Scope* getParent() const;

private:
    Scope* _parent;
    BSONObj _thisDocument;
    std::map<StringData, Value> _variables;
    bool _currentResult;
    ScriptingFunction _createFunction(const char* code,
                                      ScriptingFunction functionNumber = 0) override;
    std::vector<ASTParser*> _funcs;
};

}  // namespace tinyjs
}  // namespace mongo
