/*
    ORCOS - an Organic Reconfigurable Operating System
    Copyright (C) 2008 University of Paderborn

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "assemblerFunctions.hh"
#include "kernel/Kernel.hh"

#define STACK_CONTENT(sp,offset) *( (long*) ( ( (long) sp) + offset)  )

extern Kernel* theOS;

extern "C" long testandset(
           long testValue, // -> Previous value (r3).
           long setValue,  // -> New value (r4).
           void* addr )    // -> Location to update (r5).
{

    asm(
            "ldstub [%2], %%g1;"
            "xorcc %0, %%g1, %%g0;"
            "bne exit;"    //lock
            "mov 0, %%i0;"

            "stb %1, [%2];"
            "mov 1, %%i0;"

            "exit:"
            "stb %%g1, [%2];"
            "ret;"
            "restore;"

            : // no output variables
            : "r" (testValue) , "r" (setValue) , "r" (addr) // input variables
            : "memory" , "%g1"
    );

    // This return can never be reached
    return 0;
}


/*
 * Read time register for clock
 */
extern "C" void readTimeRegister(unint4* regValuePtr)
{
    asm volatile (
                "set 0x80000310, %%g1;"
                "ld [%%g1], %%g1;"
                "st %%g1, [%0+4];"
                :
                : "r" (regValuePtr)
                : "g1" // these registers get altered during calc
        );
}

extern "C" void restoreContext(Thread*  t) {
    void* sp = 0;

    if (!isOk(t->popStackPointer(sp))){
        ERROR("Restore Context failed while popping the stack!");
    }

    ASSERT(sp);

    TaskIdT pid = t->getOwner()->getId();

    LOG(PROCESS,TRACE,(PROCESS,TRACE,"Restore Context: sp @  0x%x." ,sp));

    #if USE_SAFE_KERNEL_STACKS
        // we use safe kernel stacks so we must free the stack slot
        // if we are returning to user space!
           int2 myBucketIndex =  t->getKernelStackBucketIndex();
           FREE_KERNEL_STACK_SLOT(myBucketIndex);
    #endif

   // print the content of the registers!
  //LOG(PROCESS,TRACE,(PROCESS,TRACE,"l0 : 0x%x, l1: 0x%x, l2: 0x%x, i3:0x%x" , STACK_CONTENT(sp, 0),STACK_CONTENT(sp,4),STACK_CONTENT(sp,8),STACK_CONTENT(sp,12)));
  //LOG(PROCESS,TRACE,(PROCESS,TRACE,"l4 : 0x%x, l5: 0x%x, l6: 0x%x, i7:0x%x" , STACK_CONTENT(sp,16),STACK_CONTENT(sp,20),STACK_CONTENT(sp,24),STACK_CONTENT(sp,28)));
  //LOG(PROCESS,TRACE,(PROCESS,TRACE,"i4 : 0x%x, i5: 0x%x, i6: 0x%x, l7:0x%x" , STACK_CONTENT(sp, 48),STACK_CONTENT(sp,52),STACK_CONTENT(sp,56),STACK_CONTENT(sp,60)));
  //LOG(PROCESS,TRACE,(PROCESS,TRACE,"g0 : 0x%x, g1: 0x%x, g2: 0x%x, g3:0x%x" , STACK_CONTENT(sp, 64),STACK_CONTENT(sp,68),STACK_CONTENT(sp,72),STACK_CONTENT(sp,76)));

    asm volatile (
        //set stack pointer and branch to leaveHandler
        "mov %0, %%g5;"
        "mov %1, %%g6;"
        "ba leavehandler;"
        "nop"
        :
        :"r" (sp), "r" (pid)
        :
    );
}

