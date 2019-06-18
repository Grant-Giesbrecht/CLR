/*
This file defines structs for tokens, ASTs, a global state, functions, and
variables.

Created by Grant Giesbrecht on 6.6.2019

*/

#include <iostream>
#include <stdio.h>
#include <vector>
#include <string>
#include <complex>

#ifndef CLR_TYPES_HPP
#define CLR_TYPES_HPP

typedef std::complex<double> comp;

/*
 Represents a token (a vector of which is to be returned by the lexer)

 type:
 	ksym = key symbol
    num = numeric constant
 	var = variable
 	kwrd = key word
 	func = function
 */
typedef struct{
	std::string type;
	std::string valstr;
	comp valnum;
}token;



/*
 Represents a cell of an abstract syntax tree (the complete tree is returned by the parser)
 */
typedef struct ast ast;
struct ast{
    token tk;
    std::vector<ast> next;
};

/*
Represents a CLR function.

name = Function name (ie. how it's called)
interpreted = Bool representing if the function is interpreted (ie. script-based) or a base-function (hard-coded)
commands = vector of strings containing all commands for the function (only if interpreted)
fnptr = Function pointer pointing to the C++ funtion which executes the CLR function (Only for base-functions)
helpstr = String containing the help page information
*/
typedef struct{
    std::string name;
    bool interpreted;
    std::vector<std::string> commands;
    comp (*fnptr) (comp, comp);
    std::string helpstr;
}clr_function; //Would be named function, but that's ambiguous.

/*
Represents a CLR variable. In the current version, only complex<double>s are supported,
but I'm setting up the infrastructure to expand this if that ever comes up...

name = Variable name
type = Variable type. Presently only option is 'num'
valnum = Value (if a num)
*/
typedef struct{
    std::string name;
    std::string type;
    comp valnum;
}variable;

/*
 Contains all data for an instance of CLR.
 */
typedef struct{
	comp x; //x register
	comp y; //y register
	comp z; //z register
	comp t; //t register
    std::vector<std::string> keywords; //Vector of all CLR keywords
    std::vector<clr_function> functions; //Vector of all CLR functions (interpreted & base)
    std::vector<variable> variables; //Vector of all CLR variables
    bool running; //Specifies if main loop should still run
	std::string help_dir; //Directory in which to search for help files.
}clr_state;

#endif
