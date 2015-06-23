
#line 1 "tinyjs.rl.cpp"
#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>


#line 10 "tinyjs.cpp"
static const int simple_lexer_start = 4;
static const int simple_lexer_first_final = 4;
static const int simple_lexer_error = 0;

static const int simple_lexer_en_main = 4;


#line 42 "tinyjs.rl.cpp"


struct token {
    std::string name;
    std::string value;
};

void emit(std::string identifier, std::vector<token> *token_array, char *ts, char *te) {
    token tk;
    tk.name = identifier;
    while (ts != te) {
        tk.value = tk.value + *ts;
        ts++;
    }
    token_array->push_back(tk);
}

int main( int argc, char **argv )
{
    int cs;
    if ( argc > 1 ) {
        char *p = argv[1];
        char *pe = p + strlen( p );
        int act;
        char *ts;
        char *te;
        char *eof = pe;
        std::vector<token> token_data;

        
#line 49 "tinyjs.cpp"
	{
	cs = simple_lexer_start;
	ts = 0;
	te = 0;
	act = 0;
	}

#line 57 "tinyjs.cpp"
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
tr2:
#line 17 "tinyjs.rl.cpp"
	{{p = ((te))-1;}{ 
      std::string name = "integer";
      emit(name, &token_data, ts, te);
    }}
	goto st4;
tr5:
#line 37 "tinyjs.rl.cpp"
	{te = p+1;}
	goto st4;
tr7:
#line 27 "tinyjs.rl.cpp"
	{te = p+1;{ 
      std::string name = "assignment";
      emit(name, &token_data, ts, te);
    }}
	goto st4;
tr9:
#line 17 "tinyjs.rl.cpp"
	{te = p;p--;{ 
      std::string name = "integer";
      emit(name, &token_data, ts, te);
    }}
	goto st4;
tr11:
#line 22 "tinyjs.rl.cpp"
	{te = p;p--;{ 
      std::string name = "float";
      emit(name, &token_data, ts, te);
    }}
	goto st4;
tr12:
#line 32 "tinyjs.rl.cpp"
	{te = p;p--;{ 
      std::string name = "identifier";
      emit(name, &token_data, ts, te);
    }}
	goto st4;
st4:
#line 1 "NONE"
	{ts = 0;}
	if ( ++p == pe )
		goto _test_eof4;
case 4:
#line 1 "NONE"
	{ts = p;}
#line 110 "tinyjs.cpp"
	switch( (*p) ) {
		case 32: goto tr5;
		case 43: goto st1;
		case 45: goto st1;
		case 61: goto tr7;
	}
	if ( (*p) < 48 ) {
		if ( 9 <= (*p) && (*p) <= 13 )
			goto tr5;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st3;
		} else if ( (*p) >= 65 )
			goto st3;
	} else
		goto tr0;
	goto st0;
st0:
cs = 0;
	goto _out;
st1:
	if ( ++p == pe )
		goto _test_eof1;
case 1:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr0;
	goto st0;
tr0:
#line 1 "NONE"
	{te = p+1;}
	goto st5;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
#line 147 "tinyjs.cpp"
	if ( (*p) == 46 )
		goto st2;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr0;
	goto tr9;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st6;
	goto tr2;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st6;
	goto tr11;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
	if ( (*p) == 95 )
		goto st7;
	if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st7;
	} else if ( (*p) >= 65 )
		goto st7;
	goto st0;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
	if ( (*p) == 95 )
		goto st7;
	if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st7;
	} else if ( (*p) >= 65 )
		goto st7;
	goto tr12;
	}
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof1: cs = 1; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 5: goto tr9;
	case 2: goto tr2;
	case 6: goto tr11;
	case 7: goto tr12;
	}
	}

	_out: {}
	}

#line 74 "tinyjs.rl.cpp"


        std::cout << token_data.size() << std::endl;
        for (int i = 0; i<token_data.size(); i++) {
            std::cout << token_data[i].name + ": " + token_data[i].value << std::endl;
        }
    }

    //printf("result = %i\n", cs >= simple_lexer_first_final );
    return 1;
}
