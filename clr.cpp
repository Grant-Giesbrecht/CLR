#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include "clr_interpret.hpp"
#include "clr_types.hpp"
#include "clr_base_functions.hpp"
#include "IEGA/string_manip.hpp"

#define FUNCTION_LIST_FILE "/usr/local/share/clr/interpreted_functions.list"
#define FUNCTION_DEFAULT_DIR "/usr/local/share/clr/functions"
//CXCOMPILE make -f clr_makefile
//CXCOMPILE ./clr
//CXGENRUN FALSE

using namespace std;

int main(int argc, char** argv){

    //********************************************************//
    //******************* INITIALIZE STATE *******************//

    //Create and initialize 'state' object
    clr_state state;
    state.running = true;
    fill_keywords(&state); //Populate keywords
    fill_critical_variables(&state); //Populate critical variables (i+j)

    //Populate functions
    load_clr_base_functions(&state); //Populate base functions
    if (!load_functions(FUNCTION_LIST_FILE, FUNCTION_DEFAULT_DIR, &state)){
        cout << "Warning: Some interpreted functions failed to load." << endl;
    } //Populate interpreted function

    //Populate functions
    //TODO
    //NOTE: The CLR interpreter requires that a variable named 'i' or 'j' always exist;

    //Initialize registers
    state.x = 0;
    state.y = 0;
    state.z = 0;
    state.t = 0;

    string line, print_out;
    bool success;
    vector<token> tks;
    while (state.running){
        tks.clear();
        cout << "> " << std::flush;
        getline(cin, line);

        interpret_clr(line, &state, print_out);
        cout << print_out;

        cout << "\tT: " << dtos(state.t.real(), 4, 3) << endl;
        cout << "\tZ: " << dtos(state.z.real(), 4, 3) << endl;
        cout << "\tY: " << dtos(state.y.real(), 4, 3) << endl;
        cout << "\tX: " << dtos(state.x.real(), 4, 3) << endl;


        // tks = clr_lex(line, &state, success);
        // if (success){
        //     for (size_t t = 0 ; t < tks.size() ; t++){
        //         cout << "(" + dtos(t, 0, 3) + ") " + tokenstr(tks[t]) << endl;
        //     }
        // }else{
        //     cout << "LEX ERROR: " << tks[0].valstr << endl;
        // }
    }

}
