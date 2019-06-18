#include "clr_interpret.hpp"
#include <IEGA/string_manip.hpp>
#include <IEGA/stdutil.hpp>
#include <cstdlib>
#include <fstream>

using namespace std;

/*
Accepts a CLC command as a string ('input'),
*/
bool interpret_clr(std::string input, clr_state* state, std::string& print_out){

	print_out = "";

	//Lex input, get tokens
	bool success;
	vector<token> tks = clr_lex(input, state, success);
	if (!success){
	    print_out = "LEX ERROR: " + tks[0].valstr + "\n";
		return false;
	}

	//Print tokens (for debugging!)
	print_out = print_out + "Tokens:\n";
    for (size_t t = 0 ; t < tks.size() ; t++){
        print_out = print_out + "\t(" + dtos(t, 0, 3) + ") " + tokenstr(tks[t]) + "\n";
    }
	print_out = print_out + "\n";


	//Parse tokens, create an abstract syntax tree
	vector<ast> trees = clr_parse(tks, state, success);
	if (!success){
		print_out = "PARSE ERROR: " + trees[0].tk.valstr + "\n";
		return false;
	}

	//Print ASTs (for debugging!)
	print_out = print_out + "Trees:\n";
	for (size_t t = 0 ; t < trees.size() ; t++){
		print_out = print_out + "\t("+dtos(t, 0, 3)+")" + aststr(trees[t]) + "\n";
	}

	//Evaluates an AST (or a subsection of an AST)
	token out;
	for (size_t t = 0 ; t < trees.size() ; t++){
		out = ast_eval(trees[t], state, success);
		if (!success){
			print_out = "EVAL ERROR: Failed to evaluate tree:\n\t" + aststr(trees[t]) + "\n";
			print_out = print_out + out.valstr + "\n";
			return false;
		}
	}


	return true;
}

/*
 Accepts a string and breaks it into a vector of tokens
 */
vector<token> clr_lex(string input, clr_state* state, bool& success){

	success = true;

	//Create token vector
	vector<token> tks;

	token temp_tok;

	ensure_whitespace(input, "+-*/^;#"); //For targets, list all symbols which may be mashed up next to another token without a space. The only symbols which fit this criterion are key symbols. This will ensure 'parse' on the next line breaks them up into words correctly
	vector<string> words = parse(input, " "); //Break up input into 'words', each holding a token. NOTE: This is not the same 'parse' as clr_parse which generates an abstract syntax tree

	//Create vector of function and variable names (will be needed within for loop)
	vector<string> fn_names;
	vector<string> var_names;
	for (size_t f = 0 ; f < state->functions.size() ; f++){
		fn_names.push_back(state->functions[f].name);
	}
	for (size_t v = 0 ; v < state->variables.size() ; v++){
		var_names.push_back(state->variables[v].name);
	}

	//Convert each 'word' into a token...
	for (size_t w = 0 ; w < words.size() ; w++){

		//Classify each word as a type of token
		if (words[w] == "+" || words[w] == "-" || words[w] == "*"  || words[w] == "/" || words[w] == "^" || words[w] == ";" || words[w] == "#"){ //Key Symbol
			//Set fields
			temp_tok.type = "ksym";
			temp_tok.valstr = words[w];

			//Add to vector of tokens
			tks.push_back(temp_tok);

			//If comment, skip remainder of input
			if (words[w] == "#"){
				break;
			}
		}else if(isnum(words[w])){ //Number
			//Set fields
			temp_tok.type = "num";
			temp_tok.valnum = strtod(words[w]);

			//Add to vector of tokens
			tks.push_back(temp_tok);
		}else if(strvec_contains(state->keywords, to_uppercase(words[w])) != -1){ //keyword
			//Set fields
			temp_tok.type = "kwrd";
			temp_tok.valstr = words[w];

			//Add to vector of tokens
			tks.push_back(temp_tok);
		}else if(strvec_contains(fn_names, to_uppercase(words[w])) != -1){ //function (interpreted or base)
			//Set fields
			temp_tok.type = "func";
			temp_tok.valstr = words[w];

			//Add to vector of tokens
			tks.push_back(temp_tok);
		}else if(strvec_contains(var_names, words[w]) != -1){ //Variable
			//Set fields
			temp_tok.type = "var";
			temp_tok.valstr = words[w];

			//Add to vector of tokens
			tks.push_back(temp_tok);
		}else{ //New variable or error

			//If it's a valid variable name, call it a variable (and assume it'll be new)
			if (is_valid_name(words[w])){
				//Set fields
				temp_tok.type = "var";
				temp_tok.valstr = words[w];

				//Add to vector of tokens
				tks.push_back(temp_tok);
			}else{ //Otherwise throw an error
				success = false;
				tks.clear();
				temp_tok.valstr = "Failed to convert word '" + words[w] + "' to token.";
				tks.push_back(temp_tok);
				return tks;
			}


		}

	} //End for loop

	//Return tokens
	return tks;

}

/*
Accepts a vector of tokens and converts them into a vector of abstract syntax
trees. Each tree represents one distinct command/operation (ie. evaluating a
function, pushing a number on the stack, etc).

examples:
"5;4+"" parses to		"3 2 logbase" parses to		"STO b_field" parses to

[ksym, ;] & [ksym, +]		[ksym, sin]			[func, "STO"]
	|			|				|						|
	|			|			____|______					|
	|			|			|		  |					|
[num, 5]	[num, 4]	[num, 3]  [num, 2]		[var, b_field]
*/
vector<ast> clr_parse(std::vector<token> tks, clr_state* state, bool& success){

	vector<ast> trees;
	success = true;
	ast temp_ast;
	ast temp_ast2;
	//Check tks size
	if (tks.size() < 1) return trees;

	//Check for keyword at beginning
	if (tks[0].type == "kwrd"){ //Found keyword
		temp_ast.tk = tks[0]; //Create base for AST with keyword token
		temp_ast2.next.clear();
		for (size_t tb = 1 ; tb < tks.size() ; tb++){ //Add all other tokens as branches
			temp_ast2.tk = tks[tb]; //Put token in branch
			temp_ast.next.push_back(temp_ast2);
		}
		trees.push_back(temp_ast); //Add to trees
	}else{ //Normal parsing operation - RPN

		//For each token
		size_t branch_start = 0; //Token index at which branches for next AST starts...
		for (size_t t = 0 ; t < tks.size() ; t++){
			if (tks[t].type == "ksym" || tks[t].type == "func"){ //Key Symbol or function found - create new AST and add it to vector
				temp_ast.tk = tks[t]; //Set this token as the 'base' node of the AST
				temp_ast.next.clear(); //Remove previous trees' branches
				temp_ast2.next.clear(); //(No 'branches' have branches - this is a quirk and feature of RPN - the user handles this)
				for (size_t tb = branch_start ; tb < t ; tb++){ //For each token before this trigger, add them as branches to the base
					temp_ast2.tk = tks[tb]; //Put token in branch
					temp_ast.next.push_back(temp_ast2);
				}
				branch_start = t+1; //Update index to start next branch-adding for-loop (ie. the one right above this line)
				trees.push_back(temp_ast); //Add tree
			}else if(tks[t].type == "kwrd" && t != 0){ //If keyword is found later, that's an error!
				success = false;
				trees.clear();
				temp_ast.tk.valstr = "Keyword '" + tks[t].valstr + "' found at word index " + dtos(t, 0, 3) + ".";
			}
		}

		//If a number is entered without any operator, make it the base of a one-token long AST
		if (branch_start < tks.size()){

			//Check for multiple numeric values. This is incorrect syntax
			if (tks.size() - branch_start > 1){
				success = false;
				trees.clear();
				temp_ast.tk.valstr = "Invalid Syntax: Multiple tokens provided, however none were key symbols or functions.\n";
				temp_ast.tk.valstr = temp_ast.tk.valstr + "\tUse the ENTER symbol (;) to push values into the {y} registers.";
			}
			temp_ast.tk = tks[branch_start];
			temp_ast.next.clear();
			trees.push_back(temp_ast);
		}
	}

	return trees;
}

/*
Evaluates an AST (or a subsection of an AST) and returns the output token
*/
token ast_eval(ast tree, clr_state* state, bool& success){

	token tk;

	//This is needed for the STO command (and perhaps others)
	vector<string> var_names;
	for (size_t v = 0 ; v < state->variables.size() ; v++){
		var_names.push_back(state->variables[v].name);
	}

	//The base will be a ksym, kwrd, or func. Determine which (each handles differently)
	if (tree.tk.type == "ksym"){ //Key Symbol

		//Push to stack UNLESS 1.) Comment or 2.) No number preceeded the key symbol
		if (tree.tk.valstr != "#" && tree.next.size() > 0){
			//The following block of code was the subroutine for ';'. I then realized that
			// you need to run the ';' code even for addition, subtraction, etc because
			// you need the number/variable to get loaded into the 'x' register.
			if (tree.next.size() != 1){ //Ensure x register has a value ready
				success = false;
				tk.valstr = "Only one token should preceed ';'. Instead "+dtos(tree.next.size(),0,3)+" were detected.";
				return tk;
			}
			if (tree.next[0].tk.type != "num" && tree.next[0].tk.type != "var"){
				success = false;
				tk.valstr = "A numeric type or variable must preceed the ';' operator.";
				return tk;
			}
			state->t = state->z;
			state->z = state->y;
			state->y = state->x;
			state->x = tree.next[0].tk.valnum;
			//End ';' code
		}

		if (tree.tk.valstr == "+"){
			state->x = state->y + state->x;
			state->y = state->z;
			state->z = state->t;
			state->t = cart(0, 0);
		}else if (tree.tk.valstr == "-"){
			state->x = state->y - state->x;
			state->y = state->z;
			state->z = state->t;
			state->t = cart(0, 0);
		}else if (tree.tk.valstr == "*"){
			state->x = state->y * state->x;
			state->y = state->z;
			state->z = state->t;
			state->t = cart(0, 0);
		}else if (tree.tk.valstr == "/"){
			state->x = state->y / state->x;
			state->y = state->z;
			state->z = state->t;
			state->t = cart(0, 0);
		}else if (tree.tk.valstr == "^"){
			state->x = pow(state->y, state->x);
			state->y = state->z;
			state->z = state->t;
			state->t = cart(0, 0);
		}else if (tree.tk.valstr == ";"){
			//do nothing (this code runs above the if-else structure)
		}else if (tree.tk.valstr == "#"){
			//do nothing (THe above if-else structure was skipped - Comment does absolutely nothing)
		}else{
			success = false;
			tk.valstr = "Unrecognized key symbol '" + tree.tk.valstr + "'.";
			return tk;
		}
	}else if(tree.tk.type == "func"){ //Function

		//Push to stack unless no number preceeded the key symbol
		if (tree.next.size() > 0){
			//The following block of code was the subroutine for ';'. I then realized that
			// you need to run the ';' code even for addition, subtraction, etc because
			// you need the number/variable to get loaded into the 'x' register.
			if (tree.next.size() != 1){ //Ensure x register has a value ready
				success = false;
				tk.valstr = "Only one token should preceed ';'. Instead "+dtos(tree.next.size(),0,3)+" were detected.";
				return tk;
			}
			if (tree.next[0].tk.type != "num" && tree.next[0].tk.type != "var"){
				success = false;
				tk.valstr = "A numeric type or variable must preceed the ';' operator.";
				return tk;
			}
			state->t = state->z;
			state->z = state->y;
			state->y = state->x;
			state->x = tree.next[0].tk.valnum;
			//End ';' code
		}

		//Scan all functions, look for the matching function
		size_t fidx = 0; //This will hold the index
		bool found = false;
		for ( ; fidx < state->functions.size() ; fidx++){
			if (state->functions[fidx].name == to_uppercase(tree.tk.valstr)){ //If this is the function...
				found = true; //Indicate that it was found
				break; //GTFO
			}
		}

		//Ensure function was found
		if (!found){
			success = false;
			tk.valstr = "Failed to locate function '" + tree.tk.valstr + "'. This is a software bug in clr_interpret.cpp";
			return tk;
		}

		//Evaluate function
		if (state->functions[fidx].interpreted){ //Interpreted function
			for (size_t l = 0 ; l < state->functions[fidx].commands.size() ; l++){
				string print_out;
				if (!interpret_clr(state->functions[fidx].commands[l], state, print_out)){
					success = false;
					tk.valstr = "Failed to execute interpreted function '" + state->functions[fidx].name + "' on line " + dtos(l, 0, 3) + ".\n";
					tk.valstr = tk.valstr + print_out;
					return tk;
				}
			}
			// interpret_clr(std::string input, clr_state* state, std::string& print_out)
		}else{ //Base function
			cout << "function!" << endl;
			state->x = state->functions[fidx].fnptr(state->x, state->y);
		}

	}else if(tree.tk.type == "kwrd"){ //Keywords

		if (to_uppercase(tree.tk.valstr) == "FLP"){ //Flip contents of {x} and {y}
			comp temp_x = state->x;
			state->x = state->y;
			state->y = temp_x;
		}else if (to_uppercase(tree.tk.valstr) == "LSTX"){

		}else if (to_uppercase(tree.tk.valstr) == "DN"){ //Roll stack down
			comp temp_x = state->x;
			state->x = state->y;
			state->y = state->z;
			state->z = state->t;
			state->t = temp_x;
		}else if (to_uppercase(tree.tk.valstr) == "UP"){ //Roll stack up
			comp temp_x = state->x;
			state->x = state->t;
			state->t = state->z;
			state->z = state->y;
			state->y = temp_x;
		}else if (to_uppercase(tree.tk.valstr) == "STK"){ //Print stack
			cout << "\t{T}: " << state->t << endl;
			cout << "\t{Z}: " << state->z << endl;
			cout << "\t{Y}: " << state->y << endl;
			cout << "\t{X}: " << state->x << endl;
		}else if (to_uppercase(tree.tk.valstr) == "STO"){ //Save {x} into the specified variable.

			//Ensure exactly one variable name follows...
			if (tree.next.size() != 1){
				success = false;
				tk.valstr = "Too many arguments provided to STO command. Exactly one argument must be given.";
				return tk;
			}

			//See if variable already exists...
			size_t vidx = strvec_contains(var_names, tree.next[0].tk.valstr);
			if (vidx != -1){ //Variable already exists
				state->variables[vidx].valnum = state->x; //Load {x} into variable
			}else{ //Create a new variable, load {x} into it, and load it into state
				variable temp_var;
				temp_var.name = tree.next[0].tk.valstr;
				temp_var.type = "num";
				temp_var.valnum = state->x;
				state->variables.push_back(temp_var);
			}
		}else if (to_uppercase(tree.tk.valstr) == "RCL"){ //Load the variable into {x} and push up the stack

			//Ensure exactly one variable name follows...
			if (tree.next.size() != 1){
				success = false;
				tk.valstr = "Too many arguments provided to STO command. Exactly one argument must be given.";
				return tk;
			}

			//See if variable already exists...
			size_t vidx = strvec_contains(var_names, tree.next[0].tk.valstr);
			if (vidx != -1){ //Variable already exists
				//Push registers up
				state->t = state->z;
				state->z = state->y;
				state->y = state->x;
				state->x = state->variables[vidx].valnum;
			}else{ //Variable does not exist - give error
				success = false;
				tk.valstr = "Variable '" + tree.next[0].tk.valstr + "' does not exist.\n";
				return tk;
			}

		}else if (to_uppercase(tree.tk.valstr) == "CLX"){ //Clear {x}
			state->x = cart(0, 0);
		}else if (to_uppercase(tree.tk.valstr) == "CLREG"){ //Clear all registers
			state->x = cart(0, 0);
			state->y = cart(0, 0);
			state->z = cart(0, 0);
			state->t = cart(0, 0);
		}else if (to_uppercase(tree.tk.valstr) == "LSVAR"){ //List all variables
			cout << "Varibales:" << endl;
			for (size_t v = 0 ; v < state->variables.size() ; v++){
				cout << "\t" << state->variables[v].name << " = " << state->variables[v].valnum << "\t\tType: " << state->variables[v].type << endl;
			}
		}else if (to_uppercase(tree.tk.valstr) == "CLVAR"){ //Clear the variables from CLR
			state->variables.clear(); //Erase all variables
			fill_critical_variables(state); //Restore those which are critical to CLR's correct operation
		}else if (to_uppercase(tree.tk.valstr) == "CLEAR"){ //Execute 'clear' in terminal. Clears the terminal
			system("clear");
		}else if (to_uppercase(tree.tk.valstr) == "HELP"){

		}else if (to_uppercase(tree.tk.valstr) == "CD"){

		}else if (to_uppercase(tree.tk.valstr) == "PWD"){ //Execute 'pwd' in terminal. Prints full path
			system("pwd");
		}else if (to_uppercase(tree.tk.valstr) == "LS"){ //Execute 'ls' in terminal. Lists directory contents
			system("ls");
		}else if (to_uppercase(tree.tk.valstr) == "EXIT"){ //Exit the program
			state->running = false;
		}else if (to_uppercase(tree.tk.valstr) == "RUN"){

		}else if (to_uppercase(tree.tk.valstr) == "DELETE"){

		}else if (to_uppercase(tree.tk.valstr) == "ADDFN"){

		}else if (to_uppercase(tree.tk.valstr) == "LSFN"){ //List all CLR functions

			bool print_long = false;

			//Process flags if present
			if (tree.next.size() > 0){
				for (size_t n = 0 ; n < tree.next.size() ; n++){ //For each extra token
					if (tree.next[n].tk.valstr == "l" || tree.next[n].tk.valstr == "L"){ //If flag is 'print_long'
						print_long = true;
					}else{
						cout << "\t Ignoring Unrecognized flag '" + tree.next[n].tk.valstr + "'." << endl;
					}
				}
			}

			if (print_long){
				cout << "Functions:" << endl;
				for (size_t f = 0 ; f < state->functions.size() ; f++){
					cout << "\t" << state->functions[f].name << "\n\t\tInterpreted: " << bool_to_str(state->functions[f].interpreted) << "\n\t\tNo. Commands: " << state->functions[f].commands.size() << endl;
				}
			}else{
				cout << "Functions:" << endl;
				for (size_t f = 0 ; f < state->functions.size() ; f++){
					cout << "\t" << state->functions[f].name << endl;
				}
			}
		//
		// }else if (to_uppercase(tree.tk.valstr) == ""){

		}else{
			success = false;
			tk.valstr = "Failed to identify function '" + tree.tk.valstr + "'.";
			return tk;
		}

	}else if(tree.tk.type == "num"){ //Number
		//The following block of code was the subroutine for ';'.
		if (tree.next.size() != 0){ //Ensure x register has a value ready
			success = false;
			tk.valstr = "Only one token should preceed a push to the stack. Instead "+dtos(tree.next.size(),0,3)+" were detected.";
			return tk;
		}
		state->t = state->z;
		state->z = state->y;
		state->y = state->x;
		state->x = tree.tk.valnum;
		//End ';' code
	}else if(tree.tk.type == "func"){ //Functions

	}else{
		success = false;
		tk.valstr = "Invalid token type. This is a software bug in clr_interpret.cpp.\n";
		tk.valstr = tk.valstr + "\tInvalid token: " + tokenstr(tree.tk);
		return tk;
	}

	return tk;

}








/*
Clear's the keyword vector of 'state', then repopulates it with all keywords in
CLR.

Void return
*/
void fill_keywords(clr_state* state){

	state->keywords.clear();
	state->keywords.push_back("FLP");
	state->keywords.push_back("LSTX");
	state->keywords.push_back("DN");
	state->keywords.push_back("UP");
	state->keywords.push_back("STK");
	state->keywords.push_back("STO");
	state->keywords.push_back("RCL");
	state->keywords.push_back("CLX");
	state->keywords.push_back("CLREG");
	state->keywords.push_back("LSVAR");
	state->keywords.push_back("CLVAR");
	state->keywords.push_back("CLEAR");
	state->keywords.push_back("HELP");
	state->keywords.push_back("CD");
	state->keywords.push_back("PWD");
	state->keywords.push_back("LS");
	state->keywords.push_back("EXIT");
	state->keywords.push_back("RUN");
	state->keywords.push_back("DELETE");
	state->keywords.push_back("ADDFN");
	state->keywords.push_back("LSFN");

}

/*
Fills the 'state' argument's variables vector with all critical CLR variables
after clearing state.variables.

Void return
*/
void fill_critical_variables(clr_state* state){

	std::string name;
	std::string type;
	comp valnum;

	state->variables.clear();
	variable v;
	v.name = "i";
	v.type = "num";
	v.valnum = cart(0, 1);
	state->variables.push_back(v);
	v.name = "j";
	state->variables.push_back(v);
}

/*
Creates a printable string from the token 't'.
*/
std::string tokenstr(token t){
	std::string s;
	s = "[" + t.type + ",";
	if (t.type == "ksym" || t.type == "var" || t.type == "kwrd" || t.type == "func"){
		s = s + t.valstr + "]";
	}else if(t.type == "num"){
		s = s + dtos(t.valnum.real(), 3, 3)+ "+" + dtos(t.valnum.imag(), 3, 3) + "i]";
	}else{
		s = s + "?]";
	}

	return s;
}

/*
Creates a printable string from the AST 't'.
*/
std::string aststr(ast t){
	std::string s;
	s = "{" + tokenstr(t.tk) + "}\t\t";
	for (size_t n = 0 ; n < t.next.size() ; n++){
		s = s + tokenstr(t.next[n].tk) + "\t";
	}
	return s;
}

//Create a comp from two reals (cartesian input)
comp cart(double r, double i){
	comp x(r, i);
	return x;
}

//Ensures 'x' is a valid variable name for CLR
bool is_valid_name(string x){
	if (x.length() < 1) return false;
	if (x.find("!") != string::npos || x.find("@") != string::npos || x.find("$") != string::npos || x.find("%") != string::npos || x.find("&") != string::npos) return false;
	if (x.find("*") != string::npos || x.find("(") != string::npos || x.find(")") != string::npos || x.find("\"") != string::npos || x.find("'") != string::npos) return false;
	if (x.find("=") != string::npos || x.find("+") != string::npos || x.find(":") != string::npos || x.find("/") != string::npos || x.find("?") != string::npos) return false;
	if (x.find("<") != string::npos || x.find(",") != string::npos || x.find(".") != string::npos || x.find(">") != string::npos || x.find("`") != string::npos) return false;
	if (x.find("~") != string::npos || x.find("|") != string::npos) return false;
	if (x[0] == '0' || x[0] == '1' || x[0] == '2' || x[0] == '3' || x[0] == '4' || x[0] == '5' || x[0] == '6' || x[0] == '7' || x[0] == '8' || x[0] == '9') return false;
	return true;
}

/*
Loads a list (stored in a text file) of functions (stored in .clrf files) into state.
'path' points to the file containing the list. Each line of that file must hold
 the full path to a .clrf file, although $(DEFAULT_DIR) will be replaced with
 whatever is passed to the function as 'default_dir'. Loads the interpreted functions
 into 'state'. Comments can be put in the list file by putting a '#' as the first
 character in the line (no whitespace before it as well).

path - path to list file
default_dir - a directory which can be abbreviated as $(DEFAULT_DIR) for installation
	convenience.
state - clr_state in which to load function

********************************************************************************
NOTE:
 * .clrf functions are essentially CLR scripts except they must contain a few
 	extra lines.
   * They must have a line starting with '@' which specifies the funciton name.
   * They must have 1+ lines starting with '~' which specify the contents of
	  the help file.
   * Although not required, it is suggested for consistancy & readability that
   	  the '@' directive is the first line and that the '~' directives are
	  consecutive and the last lines.

********************************************************************************

EXAMPLE:
 ___________________________
|clr_interpreted_funcs.list |
+--------------------------------------------
|$(DEFAULT_DIR)/abs.clrf					 |
|$(DEFAULT_DIR)/sqr.clrf					 |
|$(DEFAULT_DIR)/sqrt.clrf					 |
|# This is a user added function:			 |
|/Users/ExampleUser/Desktop/my_function.clrf |
 --------------------------------------------

 ________
|sqr.clrf|
+-------------------------------------------
|@SQR										|
|#Preserve {T}								|
|UP											|
|STO temp									|
|DN											|
|# Compute square							|
|2 ^										|
|# Restore {T}								|
|UP											|
|RCL temp									|
|DN											|
|~************** SQR Help ****************	|
|~											|
|~Computes {x}^2.							|
|~											|
|~{x}^2 -> {x}								|
|~											|
|~Type: Interpreted Function 				|
 -------------------------------------------

"/usr/local/share/clr/clr_interpreted_funcs.list" -----> 'path'
"/usr/local/share/clr/functions" --> 'default_dir"
*/
bool load_functions(std::string path, std::string default_dir, clr_state* state){

	clr_function temp_func;
	temp_func.interpreted = true;

	//Open list file
	ifstream list_file(path);
	if (!list_file.is_open()){
		cout << "Failed to open list file." << endl;
		return false;
	}

	//For each line
	string line;
	bool ret_val = true;
	while (getline(list_file, line)){

		//Skip blank lines
		if (line.length() < 1) continue;

		//Ignore comments
		if (line[0] == '#'){
			continue;
		}

		//Replace $(DEFAULT_DIR) with 'default_dir'
		size_t index = 0;
		while (true){
		     // Locate $(DEFAULT_DIR)
		     index = line.find("$(DEFAULT_DIR)", index);
		     if (index == string::npos) break;

		     //Replace substring
		     line.replace(index, 14, default_dir);

			 //Set next begin scan position to end of the replacement substring
			 index += default_dir.length();
		}

		//Open .clrf file
		ifstream clrf(line);
		if (!clrf.is_open()){ //If can't open file....
			ret_val = false; //Report not all opened successfully
			continue; //Try next file
		}

		//Reset temp_func
		temp_func.helpstr = "";
		temp_func.name = "";
		temp_func.commands.clear();


		//Scan each life of .clrf file
		string fline;
		size_t count = 0;
		while (getline(clrf, fline)){

			//Skip blank lines
			if (line.length() < 1) continue;

			//Process line...
			if (fline[0] == '@'){ //Look for function name
				temp_func.name = fline.substr(1); //Add remainder of line as function name
			}else if (fline[0] == '~'){ //Look for the funciton description
				temp_func.helpstr = temp_func.helpstr + fline.substr(1) + "\n"; //Add remainder of line as a help file line
			}else if (fline[0] == '#'){ //Is a comment. Skip!
				//Do nothing
			}else{ //Line is a command line
				temp_func.commands.push_back(fline);
			}

		}

		clrf.close();

		//Add function to 'state' if it's valid
		if (temp_func.name == "" or temp_func.helpstr == ""){ //If name or helpstring is blank, say the read failed
			ret_val = false;
		}else{ //Otherwise add to 'state'
			state->functions.push_back(temp_func);
		}

	}

	list_file.close();


	return ret_val;
}
