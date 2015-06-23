#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>

%%{

  machine simple_lexer;
  
  integer     = ('+'|'-')?[0-9]+;
  float       = ('+'|'-')?[0-9]+'.'[0-9]+;
  assignment  = '=';
  identifier  = [a-zA-Z][a-zA-Z_]+; 
  
  main := |*
    
    integer => { 
      std::string name = "integer";
      emit(name, &token_data, ts, te);
    };
    
    float => { 
      std::string name = "float";
      emit(name, &token_data, ts, te);
    };
    
    assignment => { 
      std::string name = "assignment";
      emit(name, &token_data, ts, te);
    };
    
    identifier => { 
      std::string name = "identifier";
      emit(name, &token_data, ts, te);
    };
    
    space;
    
  *|;

  write data;
}%%

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

        %%{
            write init;
            write exec;
        }%%

        std::cout << token_data.size() << std::endl;
        for (int i = 0; i<token_data.size(); i++) {
            std::cout << token_data[i].name + ": " + token_data[i].value << std::endl;
        }
    }

    //printf("result = %i\n", cs >= simple_lexer_first_final );
    return 1;
}
