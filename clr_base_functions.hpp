/*
This file defines the base functions for CLR. Base functions are those which are
hard-coded into CLR (as opposed to interpreted functions which are, at their
base most level, scripts processed in a special way).

Created by Grant Giesbrecht on 7.6.2019

*/

#include <iostream>
#include <stdio.h>
#include <vector>
#include <string>
#include <complex>
#include "clr_types.hpp"

#ifndef CLR_BASE_FUNCTIONS_HPP
#define CLR_BASE_FUNCTIONS_HPP

// All base function's callback functions must have the signature: comp function_name (comp x, comp y);

comp clrbf_sin(comp x, comp y);
comp clrbf_cos(comp x, comp y);
comp clrbf_tan(comp x, comp y);
comp clrbf_asin(comp x, comp y);
comp clrbf_acos(comp x, comp y);
comp clrbf_atan(comp x, comp y);
comp clrbf_sinh(comp x, comp y);
comp clrbf_cosh(comp x, comp y);
comp clrbf_tanh(comp x, comp y);
comp clrbf_asinh(comp x, comp y);
comp clrbf_acosh(comp x, comp y);
comp clrbf_atanh(comp x, comp y);
comp clrbf_log(comp x, comp y);
comp clrbf_ln(comp x, comp y);

void load_clr_base_functions(clr_state* state); //This isn't a base function - instead it loads them into a 'clr_state' struct. It also defines their help screens.

#endif
