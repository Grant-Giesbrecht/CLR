/*
This file contains the code required for the CLR interpreter. As such, it
includes a lexer and parser along with many support functions.

Created by Grant Giesbrecht on 6.6.2019

*/

#include <iostream>
#include <stdio.h>
#include <vector>
#include <string>
#include <complex>
#include "clr_base_functions.hpp"
#include "clr_types.hpp"

#ifndef CLR_INTERPRET_HPP
#define CLR_INTERPRET_HPP

bool interpret_clr(std::string input, clr_state* state, std::string& print_out);

//CLR's Lexer
std::vector<token> clr_lex(std::string input, clr_state* state, bool& success);

//CLR's Parser
std::vector<ast> clr_parse(std::vector<token> tks, clr_state* state, bool& success);

//Evaluates an AST (or a subsection of an AST)
token ast_eval(ast tree, clr_state* state, bool& success);

//Fills the 'state' argument's keyword vector with all CLR keywords
void fill_keywords(clr_state* state);

//Fills the 'state' argument's variables vector with all critical CLR variables
void fill_critical_variables(clr_state* state);

//Create a string form a token
std::string tokenstr(token t);

//Create a string form an AST
std::string aststr(ast t);

//Create a comp from two reals (cartesian input)
comp cart(double r, double i);

//Determines if the input is a valid variable name
bool is_valid_name(std::string x);

//Loads a list (stored in a text file) of functions (stored in .clrf files) into state.
bool load_functions(std::string path, std::string default_dir, clr_state* state);

#endif
