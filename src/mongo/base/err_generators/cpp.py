import base

class Generator(base.Generator):
    def generate(self):
        if len(self.args) != 2:
            usage("Wrong number of arguments.")
        self.generate_header()
        self.generate_source()

    def generate_header(self):

        enum_declarations = ',\n            '.join('%s = %s' % ec for ec in self.error_codes)
        predicate_declarations = ';\n        '.join(
            'static bool is%s(Error err)' % ec[0] for ec in self.error_classes)

        open(self.args[0], 'wb').write(self.header_template % dict(
                error_code_enum_declarations=enum_declarations,
                error_code_class_predicate_declarations=predicate_declarations))

    def generate_source(self):
        symbol_to_string_cases = ';\n        '.join(
            'case %s: return "%s"' % (ec[0], ec[0]) for ec in self.error_codes)
        string_to_symbol_cases = ';\n        '.join(
            'if (name == "%s") return %s' % (ec[0], ec[0])
            for ec in self.error_codes)
        int_to_symbol_cases = ';\n        '.join(
            'case %s: return %s' % (ec[0], ec[0]) for ec in self.error_codes)
        predicate_definitions = '\n    '.join(
            self.generate_error_class_predicate_definition(*ec) for ec in self.error_classes)
        open(self.args[1], 'wb').write(self.source_template % dict(
                symbol_to_string_cases=symbol_to_string_cases,
                string_to_symbol_cases=string_to_symbol_cases,
                int_to_symbol_cases=int_to_symbol_cases,
                error_code_class_predicate_definitions=predicate_definitions))

    def generate_error_class_predicate_definition(self, class_name, code_names):
        cases = '\n        '.join('case %s:' % c for c in code_names)
        return self.error_class_predicate_template % dict(class_name=class_name, cases=cases)

    header_template = '''// AUTO-GENERATED FILE DO NOT EDIT
    // See src/mongo/base/generate_error_codes.py
    /*    Copyright 2014 MongoDB, Inc.
     *
     *    This program is free software: you can redistribute it and/or  modify
     *    it under the terms of the GNU Affero General Public License, version 3,
     *    as published by the Free Software Foundation.
     *
     *    This program is distributed in the hope that it will be useful,
     *    but WITHOUT ANY WARRANTY; without even the implied warranty of
     *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     *    GNU Affero General Public License for more details.
     *
     *    You should have received a copy of the GNU Affero General Public License
     *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
     *
     *    As a special exception, the copyright holders give permission to link the
     *    code of portions of this program with the OpenSSL library under certain
     *    conditions as described in each individual source file and distribute
     *    linked combinations including the program with the OpenSSL library. You
     *    must comply with the GNU Affero General Public License in all respects
     *    for all of the code used other than as permitted herein. If you modify
     *    file(s) with this exception, you may extend this exception to your
     *    version of the file(s), but you are not obligated to do so. If you do not
     *    wish to do so, delete this exception statement from your version. If you
     *    delete this exception statement from all source files in the program,
     *    then also delete it in the license file.
     */

#pragma once

#include <string>

#include "mongo/base/string_data.h"

    namespace mongo {

        /**
         * This is a generated class containing a table of error codes and their corresponding error
         * strings. The class is derived from the definitions in src/mongo/base/error_codes.err file.
         *
         * Do not update this file directly. Update src/mongo/base/error_codes.err instead.
         */

        class ErrorCodes {
        public:
            enum Error {
                %(error_code_enum_declarations)s,
                MaxError
            };

            static std::string errorString(Error err);

            /**
             * Parses an Error from its "name".  Returns UnknownError if "name" is unrecognized.
             *
             * NOTE: Also returns UnknownError for the string "UnknownError".
             */
            static Error fromString(StringData name);

            /**
             * Casts an integer "code" to an Error.  Unrecognized codes are preserved, meaning
             * that the result of a call to fromInt() may not be one of the values in the
             * Error enumeration.
             */
            static Error fromInt(int code);

            %(error_code_class_predicate_declarations)s;
        };

    }  // namespace mongo
    '''

    source_template = '''// AUTO-GENERATED FILE DO NOT EDIT
    // See src/mongo/base/generate_error_codes.py
    /*    Copyright 2014 MongoDB, Inc.
     *
     *    This program is free software: you can redistribute it and/or  modify
     *    it under the terms of the GNU Affero General Public License, version 3,
     *    as published by the Free Software Foundation.
     *
     *    This program is distributed in the hope that it will be useful,
     *    but WITHOUT ANY WARRANTY; without even the implied warranty of
     *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     *    GNU Affero General Public License for more details.
     *
     *    You should have received a copy of the GNU Affero General Public License
     *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
     *
     *    As a special exception, the copyright holders give permission to link the
     *    code of portions of this program with the OpenSSL library under certain
     *    conditions as described in each individual source file and distribute
     *    linked combinations including the program with the OpenSSL library. You
     *    must comply with the GNU Affero General Public License in all respects
     *    for all of the code used other than as permitted herein. If you modify
     *    file(s) with this exception, you may extend this exception to your
     *    version of the file(s), but you are not obligated to do so. If you do not
     *    wish to do so, delete this exception statement from your version. If you
     *    delete this exception statement from all source files in the program,
     *    then also delete it in the license file.
     */

#include "mongo/base/error_codes.h"


#include "mongo/util/mongoutils/str.h"

    namespace mongo {

        std::string ErrorCodes::errorString(Error err) {
            switch (err) {
            %(symbol_to_string_cases)s;
            default: return mongoutils::str::stream() << "Location" << err;
            }
        }

        ErrorCodes::Error ErrorCodes::fromString(StringData name) {
            %(string_to_symbol_cases)s;
            return UnknownError;
        }

        ErrorCodes::Error ErrorCodes::fromInt(int code) {
            return static_cast<Error>(code);
        }

        %(error_code_class_predicate_definitions)s

    namespace {
        static_assert(sizeof(ErrorCodes::Error) == sizeof(int), "sizeof(ErrorCodes::Error) == sizeof(int)");
    }  // namespace
    }  // namespace mongo
    '''

    error_class_predicate_template = '''bool ErrorCodes::is%(class_name)s(Error err) {
            switch (err) {
            %(cases)s
                return true;
            default:
                return false;
            }
        }
    '''
