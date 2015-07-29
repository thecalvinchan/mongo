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

#include "mongo/scripting/mozjs/numberlong.h"

#include "mongo/scripting/mozjs/implscope.h"
#include "mongo/scripting/mozjs/objectwrapper.h"
#include "mongo/scripting/mozjs/valuereader.h"
#include "mongo/scripting/mozjs/valuewriter.h"
#include "mongo/util/mongoutils/str.h"
#include "mongo/util/text.h"

namespace mongo {
namespace mozjs {

const JSFunctionSpec NumberLongInfo::methods[4] = {
    MONGO_ATTACH_JS_FUNCTION(toNumber),
    MONGO_ATTACH_JS_FUNCTION(toString),
    MONGO_ATTACH_JS_FUNCTION(valueOf),
    JS_FS_END,
};

const char* const NumberLongInfo::className = "NumberLong";

namespace {
const char* const kTop = "top";
const char* const kBottom = "bottom";
const char* const kFloatApprox = "floatApprox";
}  // namespace

long long NumberLongInfo::ToNumberLong(JSContext* cx, JS::HandleValue thisv) {
    JS::RootedObject obj(cx, thisv.toObjectOrNull());
    return ToNumberLong(cx, obj);
}

long long NumberLongInfo::ToNumberLong(JSContext* cx, JS::HandleObject thisv) {
    ObjectWrapper o(cx, thisv);

    if (!o.hasField(kTop)) {
        if (!o.hasField(kFloatApprox))
            uasserted(ErrorCodes::InternalError, "No top and no floatApprox fields");

        return o.getNumber(kFloatApprox);
    }

    if (!o.hasField(kBottom))
        uasserted(ErrorCodes::InternalError, "top but no bottom field");

    return ((unsigned long long)((long long)o.getNumber(kTop) << 32) +
            (unsigned)(o.getNumber(kBottom)));
}

void NumberLongInfo::Functions::valueOf(JSContext* cx, JS::CallArgs args) {
    long long out = NumberLongInfo::ToNumberLong(cx, args.thisv());

    args.rval().setDouble(out);
}

void NumberLongInfo::Functions::toNumber(JSContext* cx, JS::CallArgs args) {
    valueOf(cx, args);
}

void NumberLongInfo::Functions::toString(JSContext* cx, JS::CallArgs args) {
    str::stream ss;

    long long val = NumberLongInfo::ToNumberLong(cx, args.thisv());

    const long long limit = 2LL << 30;

    if (val <= -limit || limit <= val)
        ss << "NumberLong(\"" << val << "\")";
    else
        ss << "NumberLong(" << val << ")";

    ValueReader(cx, args.rval()).fromStringData(ss.operator std::string());
}

void NumberLongInfo::construct(JSContext* cx, JS::CallArgs args) {
    uassert(ErrorCodes::BadValue,
            "NumberLong needs 0, 1 or 3 arguments",
            args.length() == 0 || args.length() == 1 || args.length() == 3);

    auto scope = getScope(cx);

    JS::RootedObject thisv(cx);

    scope->getNumberLongProto().newObject(&thisv);
    ObjectWrapper o(cx, thisv);

    JS::RootedValue floatApprox(cx);
    JS::RootedValue top(cx);
    JS::RootedValue bottom(cx);

    if (args.length() == 0) {
        o.setNumber(kFloatApprox, 0);
    } else if (args.length() == 1) {
        if (args.get(0).isNumber()) {
            o.setValue(kFloatApprox, args.get(0));
        } else {
            std::string str = ValueWriter(cx, args.get(0)).toString();

            unsigned long long val = parseLL(str.c_str());

            // values above 2^53 are not accurately represented in JS
            if ((long long)val == (long long)(double)(long long)(val) &&
                val < 9007199254740992ULL) {
                o.setNumber(kFloatApprox, val);
            } else {
                o.setNumber(kFloatApprox, val);
                o.setNumber(kTop, val >> 32);
                o.setNumber(kBottom, val & 0x00000000ffffffff);
            }
        }
    } else {
        if (!args.get(0).isNumber())
            uasserted(ErrorCodes::BadValue, "floatApprox must be a number");

        if (!args.get(1).isNumber() ||
            args.get(1).toNumber() !=
                static_cast<double>(static_cast<uint32_t>(args.get(1).toNumber())))
            uasserted(ErrorCodes::BadValue, "top must be a 32 bit unsigned number");

        if (!args.get(2).isNumber() ||
            args.get(2).toNumber() !=
                static_cast<double>(static_cast<uint32_t>(args.get(2).toNumber())))
            uasserted(ErrorCodes::BadValue, "bottom must be a 32 bit unsigned number");

        o.setValue(kFloatApprox, args.get(0));
        o.setValue(kTop, args.get(1));
        o.setValue(kBottom, args.get(2));
    }

    args.rval().setObjectOrNull(thisv);
}

}  // namespace mozjs
}  // namespace mongo
