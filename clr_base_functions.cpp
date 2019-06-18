#include "clr_base_functions.hpp"

using namespace std;

//****************************************************************************
// TRIG FUNCTIONS

/*
Defines the base function 'sin'. Computes the sine of 'x'. 'y' unused.
*/
comp clrbf_sin(comp x, comp y){
    return sin(x);
}

/*
Defines the base function 'cos'. Computes the cosine of 'x'. 'y' unused.
*/
comp clrbf_cos(comp x, comp y){
    return cos(x);
}

/*
Defines the base function 'tan'. Computes the tangent of 'x'. 'y' unused.
*/
comp clrbf_tan(comp x, comp y){
    return tan(x);
}

//(INVERSE)

/*
Defines the base function 'asin'. Computes the arcsine of 'x'. 'y' unused.
*/
comp clrbf_asin(comp x, comp y){
    return asin(x);
}

/*
Defines the base function 'acos'. Computes the arccosine of 'x'. 'y' unused.
*/
comp clrbf_acos(comp x, comp y){
    return acos(x);
}

/*
Defines the base function 'atan'. Computes the arctangent of 'x'. 'y' unused.
*/
comp clrbf_atan(comp x, comp y){
    return atan(x);
}

//****************************************************************************
//HYPERBOLIC TRIG FUNCTIONS

/*
Defines the base function 'sinh'. Computes the hyp sine of 'x'. 'y' unused.
*/
comp clrbf_sinh(comp x, comp y){
    return sinh(x);
}

/*
Defines the base function 'cosh'. Computes the hyp cosine of 'x'. 'y' unused.
*/
comp clrbf_cosh(comp x, comp y){
    return cosh(x);
}

/*
Defines the base function 'tanh'. Computes the hyp tangent of 'x'. 'y' unused.
*/
comp clrbf_tanh(comp x, comp y){
    return tanh(x);
}

//(INVERSE)

/*
Defines the base function 'asinh'. Computes the hyp arcsine of 'x'. 'y' unused.
*/
comp clrbf_asinh(comp x, comp y){
    return asinh(x);
}

/*
Defines the base function 'acosh'. Computes the hyp arccosine of 'x'. 'y' unused.
*/
comp clrbf_acosh(comp x, comp y){
    return acosh(x);
}

/*
Defines the base function 'atanh'. Computes the hyp arctangent of 'x'. 'y' unused.
*/
comp clrbf_atanh(comp x, comp y){
    return atanh(x);
}

//****************************************************************************
//LOGARITHMS

/*
Defines the base function 'log'. Computes the logarithm (base 10) of 'x'. 'y' unused.
*/
comp clrbf_log(comp x, comp y){
    return log(x);
}

/*
Defines the base function 'ln'. Computes the natural logarithm of 'x'. 'y' unused.
*/
comp clrbf_ln(comp x, comp y){
    return log(x);
}

 /*
 This isn't a base function - instead it loads them into a 'clr_state' struct.
 */
void load_clr_base_functions(clr_state* state){

    clr_function temp_func;
    temp_func.interpreted = false;

    //sin
    temp_func.name = "SIN";
    temp_func.fnptr = clrbf_sin;
    temp_func.helpstr = "************** SIN Help ****************\n\nComputes the sine of {x}.\n\nsin({x}) -> {x}\n\nType: Base Function\n";
    state->functions.push_back(temp_func);

    //cos
    temp_func.name = "COS";
    temp_func.fnptr = clrbf_cos;
    temp_func.helpstr = "************** COS Help ****************\n\nComputes the cosine of {x}.\n\ncos({x}) -> {x}\n\nType: Base Function\n";
    state->functions.push_back(temp_func);

    //tan
    temp_func.name = "TAN";
    temp_func.fnptr = clrbf_tan;
    temp_func.helpstr = "************** TAN Help ****************\n\nComputes the tangent of {x}.\n\ntan({x}) -> {x}\n\nType: Base Function\n";
    state->functions.push_back(temp_func);

    //asin
    temp_func.name = "ASIN";
    temp_func.fnptr = clrbf_asin;
    temp_func.helpstr = "************** ASIN Help ***************\n\nComputes the arc sine of {x}.\n\nasin({x}) -> {x}\n\nType: Base Function\n";
    state->functions.push_back(temp_func);

    //acos
    temp_func.name = "ACOS";
    temp_func.fnptr = clrbf_acos;
    temp_func.helpstr = "************** ACOS Help ***************\n\nComputes the arc cosine of {x}.\n\nacos({x}) -> {x}\n\nType: Base Function\n";
    state->functions.push_back(temp_func);

    //atan
    temp_func.name = "ATAN";
    temp_func.fnptr = clrbf_atan;
    temp_func.helpstr = "************** ATAN Help ***************\n\nComputes the arc tangent of {x}.\n\natan({x}) -> {x}\n\nType: Base Function\n";
    state->functions.push_back(temp_func);

    //sinh
    temp_func.name = "SINH";
    temp_func.fnptr = clrbf_sinh;
    temp_func.helpstr = "************** SINH Help ***************\n\nComputes the hyperbolic sine of {x}.\n\nsin({x}) -> {x}\n\nType: Base Function\n";
    state->functions.push_back(temp_func);

    //cosh
    temp_func.name = "COSH";
    temp_func.fnptr = clrbf_cosh;
    temp_func.helpstr = "************** COSH Help ***************\n\nComputes the hyperbolic cosine of {x}.\n\ncos({x}) -> {x}\n\nType: Base Function\n";
    state->functions.push_back(temp_func);

    //tanh
    temp_func.name = "TANH";
    temp_func.fnptr = clrbf_tanh;
    temp_func.helpstr = "************** TANH Help ***************\n\nComputes the hyperbolic tangent of {x}.\n\ntan({x}) -> {x}\n\nType: Base Function\n";
    state->functions.push_back(temp_func);

    //asinh
    temp_func.name = "ASINH";
    temp_func.fnptr = clrbf_asinh;
    temp_func.helpstr = "************* ASINH Help ***************\n\nComputes the hyperbolic arc sine of {x}.\n\nasin({x}) -> {x}\n\nType: Base Function\n";
    state->functions.push_back(temp_func);

    //acosh
    temp_func.name = "ACOSH";
    temp_func.fnptr = clrbf_acosh;
    temp_func.helpstr = "************* ACOSH Help ***************\n\nComputes the hyperbolic arc cosine of {x}.\n\nacos({x}) -> {x}\n\nType: Base Function\n";
    state->functions.push_back(temp_func);

    //atanh
    temp_func.name = "ATANH";
    temp_func.fnptr = clrbf_atanh;
    temp_func.helpstr = "************* ATANH Help ***************\n\nComputes the hyperbolic arc tangent of {x}.\n\natan({x}) -> {x}\n\nType: Base Function\n";
    state->functions.push_back(temp_func);

    //log
    temp_func.name = "LOG";
    temp_func.fnptr = clrbf_log;
    temp_func.helpstr = "************** LOG Help ****************\n\nComputes the logarithm of {x}.\n\nsin({x}) -> {x}\n\nType: Base Function\n";
    state->functions.push_back(temp_func);

    //ln
    temp_func.name = "LN";
    temp_func.fnptr = clrbf_ln;
    temp_func.helpstr = "*************** LN Help ****************\n\nComputes the natural logarithm of {x}.\n\ncos({x}) -> {x}\n\nType: Base Function\n";
    state->functions.push_back(temp_func);


}
