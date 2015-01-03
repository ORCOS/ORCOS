/*
 * assemblerFunctions.cc
 *
 *  Created on: 30.12.2013
 *     Copyright & Author: dbaldin
 */


#include "SCLConfig.hh"
#include <types.hh>
#include <process/Thread.hh>

unint4 sc_return_value;
unint4 sc_number;
unint4 sc_param1;
unint4 sc_param2;
unint4 sc_param3;
unint4 sc_param4;
unint4 sc_param5;

extern "C" void restoreContext(Thread* t) {
    while(1) {};
}
