
#line 1 "lexer.rl.cpp"
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
#include <vector>

#include "mongo/base/status_with.h"
#include "mongo/base/string_data.h"
#include "mongo/util/mongoutils/str.h"
#include "mongo/scripting/tinyjs/lexer.h"

namespace mongo {
namespace tinyjs {

namespace {

/*
 * This Ragel machine lexes a line written in a subset of Javascript.
*/

#line 53 "lexer.cpp"
static const char _lexer_actions[] = {
	0, 1, 0, 1, 1, 1, 2, 1, 
	14, 1, 15, 1, 16, 1, 17, 1, 
	18, 1, 19, 1, 20, 1, 21, 1, 
	22, 1, 23, 1, 24, 1, 25, 1, 
	26, 1, 27, 1, 28, 1, 29, 1, 
	30, 1, 31, 1, 32, 1, 33, 1, 
	34, 1, 35, 1, 36, 1, 37, 1, 
	38, 1, 39, 1, 40, 1, 41, 1, 
	42, 1, 43, 1, 44, 1, 45, 1, 
	46, 1, 47, 1, 48, 2, 2, 3, 
	2, 2, 4, 2, 2, 5, 2, 2, 
	6, 2, 2, 7, 2, 2, 8, 2, 
	2, 9, 2, 2, 10, 2, 2, 11, 
	2, 2, 12, 2, 2, 13
};

static const short _lexer_key_offsets[] = {
	0, 1, 2, 4, 7, 50, 51, 52, 
	53, 54, 55, 57, 60, 63, 64, 65, 
	66, 67, 74, 82, 90, 98, 106, 114, 
	122, 130, 138, 146, 156, 164, 172, 180, 
	188, 196, 204, 212, 220, 228, 236, 244, 
	252, 260, 268, 276, 284, 292, 300, 308, 
	316, 324, 332, 340, 348, 356, 364, 372, 
	380, 388, 396, 404, 412, 420, 428
};

static const char _lexer_trans_keys[] = {
	34, 39, 48, 57, 46, 48, 57, 32, 
	33, 34, 38, 39, 40, 41, 42, 43, 
	44, 45, 46, 47, 48, 58, 59, 60, 
	61, 62, 63, 73, 78, 91, 93, 95, 
	102, 110, 114, 116, 117, 118, 119, 123, 
	124, 125, 9, 13, 49, 57, 65, 90, 
	97, 122, 61, 61, 34, 38, 39, 48, 
	57, 46, 48, 57, 46, 48, 57, 61, 
	61, 61, 61, 95, 48, 57, 65, 90, 
	97, 122, 95, 110, 48, 57, 65, 90, 
	97, 122, 95, 102, 48, 57, 65, 90, 
	97, 122, 95, 105, 48, 57, 65, 90, 
	97, 122, 95, 110, 48, 57, 65, 90, 
	97, 122, 95, 105, 48, 57, 65, 90, 
	97, 122, 95, 116, 48, 57, 65, 90, 
	97, 122, 95, 121, 48, 57, 65, 90, 
	97, 122, 95, 97, 48, 57, 65, 90, 
	98, 122, 78, 95, 48, 57, 65, 90, 
	97, 122, 95, 97, 111, 117, 48, 57, 
	65, 90, 98, 122, 95, 108, 48, 57, 
	65, 90, 97, 122, 95, 115, 48, 57, 
	65, 90, 97, 122, 95, 101, 48, 57, 
	65, 90, 97, 122, 95, 114, 48, 57, 
	65, 90, 97, 122, 95, 110, 48, 57, 
	65, 90, 97, 122, 95, 99, 48, 57, 
	65, 90, 97, 122, 95, 116, 48, 57, 
	65, 90, 97, 122, 95, 105, 48, 57, 
	65, 90, 97, 122, 95, 111, 48, 57, 
	65, 90, 97, 122, 95, 110, 48, 57, 
	65, 90, 97, 122, 95, 117, 48, 57, 
	65, 90, 97, 122, 95, 108, 48, 57, 
	65, 90, 97, 122, 95, 108, 48, 57, 
	65, 90, 97, 122, 95, 101, 48, 57, 
	65, 90, 97, 122, 95, 116, 48, 57, 
	65, 90, 97, 122, 95, 117, 48, 57, 
	65, 90, 97, 122, 95, 114, 48, 57, 
	65, 90, 97, 122, 95, 110, 48, 57, 
	65, 90, 97, 122, 95, 114, 48, 57, 
	65, 90, 97, 122, 95, 117, 48, 57, 
	65, 90, 97, 122, 95, 110, 48, 57, 
	65, 90, 97, 122, 95, 100, 48, 57, 
	65, 90, 97, 122, 95, 101, 48, 57, 
	65, 90, 97, 122, 95, 102, 48, 57, 
	65, 90, 97, 122, 95, 105, 48, 57, 
	65, 90, 97, 122, 95, 110, 48, 57, 
	65, 90, 97, 122, 95, 101, 48, 57, 
	65, 90, 97, 122, 95, 100, 48, 57, 
	65, 90, 97, 122, 95, 97, 48, 57, 
	65, 90, 98, 122, 95, 114, 48, 57, 
	65, 90, 97, 122, 95, 104, 48, 57, 
	65, 90, 97, 122, 95, 105, 48, 57, 
	65, 90, 97, 122, 95, 108, 48, 57, 
	65, 90, 97, 122, 95, 101, 48, 57, 
	65, 90, 97, 122, 124, 0
};

static const char _lexer_single_lengths[] = {
	1, 1, 0, 1, 35, 1, 1, 1, 
	1, 1, 0, 1, 1, 1, 1, 1, 
	1, 1, 2, 2, 2, 2, 2, 2, 
	2, 2, 2, 4, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 1
};

static const char _lexer_range_lengths[] = {
	0, 0, 1, 1, 4, 0, 0, 0, 
	0, 0, 1, 1, 1, 0, 0, 0, 
	0, 3, 3, 3, 3, 3, 3, 3, 
	3, 3, 3, 3, 3, 3, 3, 3, 
	3, 3, 3, 3, 3, 3, 3, 3, 
	3, 3, 3, 3, 3, 3, 3, 3, 
	3, 3, 3, 3, 3, 3, 3, 3, 
	3, 3, 3, 3, 3, 3, 0
};

static const short _lexer_index_offsets[] = {
	0, 2, 4, 6, 9, 49, 51, 53, 
	55, 57, 59, 61, 64, 67, 69, 71, 
	73, 75, 80, 86, 92, 98, 104, 110, 
	116, 122, 128, 134, 142, 148, 154, 160, 
	166, 172, 178, 184, 190, 196, 202, 208, 
	214, 220, 226, 232, 238, 244, 250, 256, 
	262, 268, 274, 280, 286, 292, 298, 304, 
	310, 316, 322, 328, 334, 340, 346
};

static const char _lexer_indicies[] = {
	2, 1, 2, 3, 5, 4, 6, 7, 
	4, 9, 10, 11, 12, 13, 14, 15, 
	16, 17, 18, 19, 20, 21, 22, 24, 
	25, 26, 27, 28, 29, 31, 32, 33, 
	34, 30, 35, 36, 37, 38, 39, 40, 
	41, 42, 43, 44, 9, 23, 30, 30, 
	8, 46, 45, 48, 47, 2, 1, 50, 
	49, 2, 3, 5, 51, 6, 7, 52, 
	6, 23, 52, 54, 53, 56, 55, 58, 
	57, 60, 59, 30, 30, 30, 30, 51, 
	30, 62, 30, 30, 30, 61, 30, 63, 
	30, 30, 30, 61, 30, 64, 30, 30, 
	30, 61, 30, 65, 30, 30, 30, 61, 
	30, 66, 30, 30, 30, 61, 30, 67, 
	30, 30, 30, 61, 30, 68, 30, 30, 
	30, 61, 30, 69, 30, 30, 30, 61, 
	68, 30, 30, 30, 30, 61, 30, 70, 
	71, 72, 30, 30, 30, 61, 30, 73, 
	30, 30, 30, 61, 30, 74, 30, 30, 
	30, 61, 30, 75, 30, 30, 30, 61, 
	30, 76, 30, 30, 30, 61, 30, 77, 
	30, 30, 30, 61, 30, 78, 30, 30, 
	30, 61, 30, 79, 30, 30, 30, 61, 
	30, 80, 30, 30, 30, 61, 30, 81, 
	30, 30, 30, 61, 30, 82, 30, 30, 
	30, 61, 30, 83, 30, 30, 30, 61, 
	30, 84, 30, 30, 30, 61, 30, 85, 
	30, 30, 30, 61, 30, 86, 30, 30, 
	30, 61, 30, 87, 30, 30, 30, 61, 
	30, 88, 30, 30, 30, 61, 30, 89, 
	30, 30, 30, 61, 30, 90, 30, 30, 
	30, 61, 30, 91, 30, 30, 30, 61, 
	30, 74, 30, 30, 30, 61, 30, 92, 
	30, 30, 30, 61, 30, 93, 30, 30, 
	30, 61, 30, 94, 30, 30, 30, 61, 
	30, 95, 30, 30, 30, 61, 30, 96, 
	30, 30, 30, 61, 30, 97, 30, 30, 
	30, 61, 30, 98, 30, 30, 30, 61, 
	30, 99, 30, 30, 30, 61, 30, 100, 
	30, 30, 30, 61, 30, 101, 30, 30, 
	30, 61, 30, 102, 30, 30, 30, 61, 
	30, 103, 30, 30, 30, 61, 30, 104, 
	30, 30, 30, 61, 30, 105, 30, 30, 
	30, 61, 106, 49, 0
};

static const char _lexer_trans_targs[] = {
	4, 0, 4, 1, 4, 10, 2, 3, 
	4, 4, 5, 7, 8, 9, 4, 4, 
	4, 4, 4, 4, 10, 4, 11, 12, 
	4, 4, 13, 14, 16, 4, 17, 18, 
	25, 4, 4, 27, 38, 41, 46, 48, 
	56, 58, 4, 62, 4, 4, 6, 4, 
	4, 4, 4, 4, 4, 4, 4, 4, 
	15, 4, 4, 4, 4, 4, 19, 20, 
	21, 22, 23, 24, 17, 26, 28, 31, 
	32, 29, 30, 17, 17, 33, 34, 35, 
	36, 37, 17, 39, 40, 17, 42, 43, 
	44, 45, 17, 47, 49, 50, 51, 52, 
	53, 54, 55, 17, 57, 17, 59, 60, 
	61, 17, 4
};

static const char _lexer_trans_actions[] = {
	73, 0, 7, 0, 71, 98, 0, 0, 
	51, 49, 0, 5, 0, 5, 31, 33, 
	13, 9, 39, 11, 107, 15, 5, 5, 
	37, 29, 0, 0, 0, 35, 104, 0, 
	0, 41, 43, 0, 0, 0, 0, 0, 
	0, 0, 45, 0, 47, 65, 0, 63, 
	23, 69, 25, 75, 53, 59, 19, 67, 
	0, 57, 17, 61, 21, 55, 0, 0, 
	0, 0, 0, 0, 98, 0, 0, 0, 
	0, 0, 0, 101, 95, 0, 0, 0, 
	0, 0, 86, 0, 0, 80, 0, 0, 
	0, 0, 77, 0, 0, 0, 0, 0, 
	0, 0, 0, 83, 0, 89, 0, 0, 
	0, 92, 27
};

static const char _lexer_to_state_actions[] = {
	0, 0, 0, 0, 1, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0
};

static const char _lexer_from_state_actions[] = {
	0, 0, 0, 0, 3, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0
};

static const short _lexer_eof_trans[] = {
	1, 1, 5, 5, 0, 46, 48, 50, 
	50, 50, 52, 53, 53, 54, 56, 58, 
	60, 52, 62, 62, 62, 62, 62, 62, 
	62, 62, 62, 62, 62, 62, 62, 62, 
	62, 62, 62, 62, 62, 62, 62, 62, 
	62, 62, 62, 62, 62, 62, 62, 62, 
	62, 62, 62, 62, 62, 62, 62, 62, 
	62, 62, 62, 62, 62, 62, 50
};

static const int lexer_start = 4;
static const int lexer_first_final = 4;
static const int lexer_error = -1;

static const int lexer_en_main = 4;


#line 262 "lexer.rl.cpp"


/*
 * This function adds a token containing type and value to the tokenData array.
 */
void emit(TokenType t, std::vector<Token> *tokenData, const char *ts, const char *te) {
    if (te < ts) {
         tokenData->emplace_back(t, StringData(""), ts, te);
    } else {
        tokenData->emplace_back(t, StringData(ts, (te - ts)), ts, te);
    }
}

/*
 * This function inserts the location of an error, relative to the end of the input 
 * string, to the errorLocations vector.
 */
void emitError(std::vector<std::size_t> *errorLocations, const char *ts, const char *eof) {
    int indexFromEnd = eof - ts;
    errorLocations->push_back(indexFromEnd);
}

}  // namespace

/*
 * This is the constructor for a Token. It takes in and stores the type, lexeme, start pointer, and
 * end pointer of  the token.
 */
Token::Token(TokenType t, StringData v, const char* ts, const char* te) {
    type = t;
    value = v;
    tokenStart = ts;
    tokenEnd = te;
};

/*
 * This function uses the Ragel machine above to lex a line 
 * written in the tinyjs subset of Javascript.
 */
StatusWith<std::vector<Token>> lex(StringData input) {
    
    // Ragel requires the initialization of the variables cs, p, pe, act, ts, te, and eof.
    // cs represents the current state.
    int cs;
    // p is a pointer to the data.
    const char *p = input.rawData();
    // pe is a pointer to the end of the data.
    const char *pe = p + input.size();
    // act is used by the scanner to keep track of the most recent successful match.
    int act;
    // ts is a pointer to the start of the current token.
    const char *ts;
    // te is a pointed to the end of the current token.
    const char *te;
    // eof is a pointer to the end of the file.
    const char *eof = pe;

    std::vector<Token> tokenData;
    std::vector<std::size_t> errorLocations;

    
#line 355 "lexer.cpp"
	{
	cs = lexer_start;
	ts = 0;
	te = 0;
	act = 0;
	}

#line 363 "lexer.cpp"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( p == pe )
		goto _test_eof;
_resume:
	_acts = _lexer_actions + _lexer_from_state_actions[cs];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 ) {
		switch ( *_acts++ ) {
	case 1:
#line 1 "NONE"
	{ts = p;}
	break;
#line 382 "lexer.cpp"
		}
	}

	_keys = _lexer_trans_keys + _lexer_key_offsets[cs];
	_trans = _lexer_index_offsets[cs];

	_klen = _lexer_single_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (unsigned int)(_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _lexer_range_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += (unsigned int)((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	_trans = _lexer_indicies[_trans];
_eof_trans:
	cs = _lexer_trans_targs[_trans];

	if ( _lexer_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _lexer_actions + _lexer_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 2:
#line 1 "NONE"
	{te = p+1;}
	break;
	case 3:
#line 97 "lexer.rl.cpp"
	{act = 1;}
	break;
	case 4:
#line 101 "lexer.rl.cpp"
	{act = 2;}
	break;
	case 5:
#line 105 "lexer.rl.cpp"
	{act = 3;}
	break;
	case 6:
#line 109 "lexer.rl.cpp"
	{act = 4;}
	break;
	case 7:
#line 113 "lexer.rl.cpp"
	{act = 5;}
	break;
	case 8:
#line 117 "lexer.rl.cpp"
	{act = 6;}
	break;
	case 9:
#line 121 "lexer.rl.cpp"
	{act = 7;}
	break;
	case 10:
#line 129 "lexer.rl.cpp"
	{act = 9;}
	break;
	case 11:
#line 133 "lexer.rl.cpp"
	{act = 10;}
	break;
	case 12:
#line 141 "lexer.rl.cpp"
	{act = 12;}
	break;
	case 13:
#line 225 "lexer.rl.cpp"
	{act = 33;}
	break;
	case 14:
#line 137 "lexer.rl.cpp"
	{te = p+1;{
        emit(TokenType::kStringLiteral, &tokenData, ts + 1, te - 1);
        }}
	break;
	case 15:
#line 145 "lexer.rl.cpp"
	{te = p+1;{
        emit(TokenType::kAdd, &tokenData, ts, te);
        }}
	break;
	case 16:
#line 149 "lexer.rl.cpp"
	{te = p+1;{
        emit(TokenType::kSubtract, &tokenData, ts, te);
        }}
	break;
	case 17:
#line 153 "lexer.rl.cpp"
	{te = p+1;{
        emit(TokenType::kMultiply, &tokenData, ts, te);
        }}
	break;
	case 18:
#line 157 "lexer.rl.cpp"
	{te = p+1;{
        emit(TokenType::kDivide, &tokenData, ts, te);
        }}
	break;
	case 19:
#line 161 "lexer.rl.cpp"
	{te = p+1;{
        emit(TokenType::kTripleEquals, &tokenData, ts, te);
        }}
	break;
	case 20:
#line 173 "lexer.rl.cpp"
	{te = p+1;{
        emit(TokenType::kLessThanEquals, &tokenData, ts, te);
        }}
	break;
	case 21:
#line 181 "lexer.rl.cpp"
	{te = p+1;{
        emit(TokenType::kGreaterThanEquals, &tokenData, ts, te);
        }}
	break;
	case 22:
#line 189 "lexer.rl.cpp"
	{te = p+1;{
        emit(TokenType::kDoubleNotEquals, &tokenData, ts, te);
        }}
	break;
	case 23:
#line 193 "lexer.rl.cpp"
	{te = p+1;{
        emit(TokenType::kLogicalAnd, &tokenData, ts, te);
        }}
	break;
	case 24:
#line 197 "lexer.rl.cpp"
	{te = p+1;{
        emit(TokenType::kLogicalOr, &tokenData, ts, te);
        }}
	break;
	case 25:
#line 205 "lexer.rl.cpp"
	{te = p+1;{
        emit(TokenType::kSemiColon, &tokenData, ts, te);
        }}
	break;
	case 26:
#line 209 "lexer.rl.cpp"
	{te = p+1;{
        emit(TokenType::kOpenParen, &tokenData, ts, te);
        }}
	break;
	case 27:
#line 213 "lexer.rl.cpp"
	{te = p+1;{
        emit(TokenType::kCloseParen, &tokenData, ts, te);
        }}
	break;
	case 28:
#line 217 "lexer.rl.cpp"
	{te = p+1;{
        emit(TokenType::kQuestionMark, &tokenData, ts, te);
        }}
	break;
	case 29:
#line 221 "lexer.rl.cpp"
	{te = p+1;{
        emit(TokenType::kColon, &tokenData, ts, te);
        }}
	break;
	case 30:
#line 229 "lexer.rl.cpp"
	{te = p+1;{
        emit(TokenType::kComma, &tokenData, ts, te);
        }}
	break;
	case 31:
#line 233 "lexer.rl.cpp"
	{te = p+1;{
        emit(TokenType::kOpenSquareBracket, &tokenData, ts, te);
        }}
	break;
	case 32:
#line 237 "lexer.rl.cpp"
	{te = p+1;{
        emit(TokenType::kCloseSquareBracket, &tokenData, ts, te);
        }}
	break;
	case 33:
#line 241 "lexer.rl.cpp"
	{te = p+1;{
        emit(TokenType::kOpenCurlyBrace, &tokenData, ts, te);
        }}
	break;
	case 34:
#line 245 "lexer.rl.cpp"
	{te = p+1;{
        emit(TokenType::kCloseCurlyBrace, &tokenData, ts, te);
        }}
	break;
	case 35:
#line 253 "lexer.rl.cpp"
	{te = p+1;}
	break;
	case 36:
#line 255 "lexer.rl.cpp"
	{te = p+1;{
        emitError(&errorLocations, ts, eof);
        }}
	break;
	case 37:
#line 125 "lexer.rl.cpp"
	{te = p;p--;{
        emit(TokenType::kIntegerLiteral, &tokenData, ts, te);
        }}
	break;
	case 38:
#line 141 "lexer.rl.cpp"
	{te = p;p--;{
        emit(TokenType::kIdentifier, &tokenData, ts, te);
        }}
	break;
	case 39:
#line 165 "lexer.rl.cpp"
	{te = p;p--;{
        emit(TokenType::kDoubleEquals, &tokenData, ts, te);
        }}
	break;
	case 40:
#line 169 "lexer.rl.cpp"
	{te = p;p--;{
        emit(TokenType::kLessThan, &tokenData, ts, te);
        }}
	break;
	case 41:
#line 177 "lexer.rl.cpp"
	{te = p;p--;{
        emit(TokenType::kGreaterThan, &tokenData, ts, te);
        }}
	break;
	case 42:
#line 185 "lexer.rl.cpp"
	{te = p;p--;{
        emit(TokenType::kNotEquals, &tokenData, ts, te);
        }}
	break;
	case 43:
#line 201 "lexer.rl.cpp"
	{te = p;p--;{
        emit(TokenType::kLogicalNot, &tokenData, ts, te);
        }}
	break;
	case 44:
#line 249 "lexer.rl.cpp"
	{te = p;p--;{
        emit(TokenType::kAssignment, &tokenData, ts, te);
        }}
	break;
	case 45:
#line 255 "lexer.rl.cpp"
	{te = p;p--;{
        emitError(&errorLocations, ts, eof);
        }}
	break;
	case 46:
#line 125 "lexer.rl.cpp"
	{{p = ((te))-1;}{
        emit(TokenType::kIntegerLiteral, &tokenData, ts, te);
        }}
	break;
	case 47:
#line 255 "lexer.rl.cpp"
	{{p = ((te))-1;}{
        emitError(&errorLocations, ts, eof);
        }}
	break;
	case 48:
#line 1 "NONE"
	{	switch( act ) {
	case 1:
	{{p = ((te))-1;}
        emit(TokenType::kReturnKeyword, &tokenData, ts, te);
        }
	break;
	case 2:
	{{p = ((te))-1;}
        emit(TokenType::kNullLiteral, &tokenData, ts, te);
        }
	break;
	case 3:
	{{p = ((te))-1;}
        emit(TokenType::kUndefinedLiteral, &tokenData, ts, te);
        }
	break;
	case 4:
	{{p = ((te))-1;}
        emit(TokenType::kFunctionKeyword, &tokenData, ts, te);
        }
	break;
	case 5:
	{{p = ((te))-1;}
        emit(TokenType::kVarKeyword, &tokenData, ts, te);
        }
	break;
	case 6:
	{{p = ((te))-1;}
        emit(TokenType::kWhileKeyword, &tokenData, ts, te);
        }
	break;
	case 7:
	{{p = ((te))-1;}
        emit(TokenType::kForKeyword, &tokenData, ts, te);
        }
	break;
	case 9:
	{{p = ((te))-1;}
        emit(TokenType::kFloatLiteral, &tokenData, ts, te);
        }
	break;
	case 10:
	{{p = ((te))-1;}
        emit(TokenType::kBooleanLiteral, &tokenData, ts, te);
        }
	break;
	case 12:
	{{p = ((te))-1;}
        emit(TokenType::kIdentifier, &tokenData, ts, te);
        }
	break;
	case 33:
	{{p = ((te))-1;}
        emit(TokenType::kPeriod, &tokenData, ts, te);
        }
	break;
	}
	}
	break;
#line 759 "lexer.cpp"
		}
	}

_again:
	_acts = _lexer_actions + _lexer_to_state_actions[cs];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 ) {
		switch ( *_acts++ ) {
	case 0:
#line 1 "NONE"
	{ts = 0;}
	break;
#line 772 "lexer.cpp"
		}
	}

	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	if ( p == eof )
	{
	if ( _lexer_eof_trans[cs] > 0 ) {
		_trans = _lexer_eof_trans[cs] - 1;
		goto _eof_trans;
	}
	}

	}

#line 325 "lexer.rl.cpp"


    if (!errorLocations.empty()) {
        int lastIndex = input.size();
        int errorIndex = lastIndex - errorLocations[0];
        return StatusWith<std::vector<Token>>(
            ErrorCodes::FailedToParse,
            str::stream() << "Could not parse input starting with character: " << input[errorIndex]
                          << "\n" << input << "\n" << std::string(errorIndex, ' ') << "^"
                          << "\n");
    }

    return tokenData;
}

} // namespace tinyjs
} // namespace mongo