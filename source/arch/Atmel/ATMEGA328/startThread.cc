/*
 * startThread.c
 *
 *  Created on: 30.12.2013
 *      Author: dbaldin
 */

#include "SCLConfig.hh"
#include "assemblerFunctions.hh"
#include <process/Task.hh>
#include "kernel/Kernel.hh"
#include "memtools.hh"

extern Kernel* theOS;
void startThread(Thread* thread)  __attribute__((noreturn));

extern void* __RAM_END;
extern void* __stack;
/*!
 *  This method will jump to the addr given by the effective addr while
 *  setting the correct PID for the MMU and the correct stack pointer
 *
 *  thread - the thread we want to start for the first time
 *
 */
void startThread( register Thread* thread ) {
    //void* entry = thread->getStartRoutinePointer();
    asm volatile(
              // "ldi r16, hi8(__stack); "
               //"sts SPH,r16;"
               //"ldi r16, lo8(__stack);"
               //"sts SPL,r16;"
               "call task_main;"
                : // no output variables
                :
                :
            );


while(1){};
}
