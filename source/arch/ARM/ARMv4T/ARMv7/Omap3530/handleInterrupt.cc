/*
 ORCOS - an Organic Reconfigurable Operating System
 Copyright (C) 2010 University of Paderborn

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

#include <kernel/Kernel.hh>
#include "syscalls/syscalls.hh"
#include "inc/error.hh"
#include "OMAP3530.h"

extern "C" void handleTimerInterrupt(void* sp_int);

extern Kernel* theOS;
extern Board_TimerCfdCl* theTimer;
extern Task* pCurrentRunningTask;
extern Board_InterruptControllerCfdCl* theInterruptController;

extern "C" void dumpContext(void* sp_context) {
    unint4* context = reinterpret_cast<unint4*>(sp_context);
    LOG(ARCH, ERROR, "Context at 0x%08x:", (unint4) sp_context);
    LOG(ARCH, ERROR, "PSR: 0x%08x  PC : 0x%08x", context[0], context[13]);
    LOG(ARCH, ERROR, "r0 : 0x%08x  r1 : 0x%08x  r2 : 0x%08x", context[1] , context[2] , context[3]);
    LOG(ARCH, ERROR, "r3 : 0x%08x  r4 : 0x%08x  r5 : 0x%08x", context[4] , context[5] , context[6]);
    LOG(ARCH, ERROR, "r6 : 0x%08x  r7 : 0x%08x  r8 : 0x%08x", context[7] , context[8] , context[9]);
    LOG(ARCH, ERROR, "r9 : 0x%08x  r10: 0x%08x  r11: 0x%08x", context[10], context[11], context[12]);
    LOG(ARCH, ERROR, "r12: 0x%08x  SP : 0x%08x  LR : 0x%08x", context[13], context[14], context[15]);

    //LOG(ARCH, ERROR,"LR: 0x%08x", ((unint4*) sp_context)[15]);
}

extern "C" void handleDataAbort(int addr, int instr, int context, int spsr) {
    LOG(ARCH, ERROR, "Data Abort at address: 0x%08x , instr: 0x%08x, SPSR: 0x%08x", addr, instr, spsr);

    dumpContext(reinterpret_cast<void*>(context));

    int sp = (reinterpret_cast<unint4*>(context))[14];

    /* print the call trace */
    backtrace_addr(reinterpret_cast<void*>(instr), sp);

    int tid = 0;
    if (pCurrentRunningTask != 0)
        tid = pCurrentRunningTask->getId();

    int dfsr;
    asm (
            "MRC p15,0,%0,c5,c0,0;"
            : "=&r" (dfsr)
            :
            :
    );

    LOG(ARCH, ERROR, "TID: %d, DFSR: %x", tid, dfsr);

#ifdef HAS_Board_HatLayerCfd

    int asid = -1;
    unint4 tbb0 = 0;
    unint4 paget = ((unint4) (&__PageTableSec_start)) + tid * 0x4000;

    asm (
            "MRC p15,0,%0,c13,c0,1;"  // ; Read CP15 Context ID Register
            "MRC p15,0,%1,c2,c0,0;"// ; Read CP15 Translation Table Base Register 0
            : "=&r" (asid), "=&r" (tbb0)
            :
            : "r0"
    );

    LOG(ARCH, ERROR, "ASID: %d, TBB0: 0x%x, Task PT: 0x%x", asid, tbb0, paget);
    theOS->getHatLayer()->dumpPageTable(tid);
#endif

    /* handle the error */
    theOS->getErrorHandler()->handleError(cDataAbortError);

    __builtin_unreachable();
    while (true) { }
}

extern "C" void handleUndefinedIRQ(int addr, int spsr, int context, int sp) {
    LOG(ARCH, ERROR, "Undefined Instruction IRQ, Addr: 0x%x, SPSR: 0x%x", addr, spsr);

    dumpContext(reinterpret_cast<void*>(context));

    memdump(addr - 16, 8);

    /* print the call trace */
    backtrace_addr(reinterpret_cast<void*>(addr), sp);

    int tid = 0;
    if (pCurrentRunningTask != 0)
        tid = pCurrentRunningTask->getId();

    LOG(ARCH, ERROR, "TID: %d", tid);

#ifdef HAS_Board_HatLayerCfd

    int asid = -1;
    unint4 tbb0 = 0;
    unint4 paget = ((unint4) (&__PageTableSec_start)) + tid * 0x4000;

    asm (
            "MRC p15,0,%0,c13,c0,1;"  // ; Read CP15 Context ID Register
            "MRC p15,0,%1,c2,c0,0;"// ; Read CP15 Translation Table Base Register 0
            : "=&r" (asid), "=&r" (tbb0)
            :
            : "r0"
    );

    LOG(ARCH, ERROR, "ASID: %d %d, TBB0: 0x%x, Task PT: 0x%x", asid >> 8, asid & 0xff, tbb0, paget);
    theOS->getHatLayer()->dumpPageTable(tid);
#endif

    /* handle the error */
    theOS->getErrorHandler()->handleError(cUndefinedInstruction);
}

extern "C" void handleFIQ() {
    LOG(ARCH, ERROR, "FIQ..");
    while (true) { }
}

extern "C" void handlePrefetchAbort(int instr, int context, int sp) {
    LOG(ARCH, ERROR, "Prefetch Abort IRQ. instr: 0x%x", instr);

    dumpContext(reinterpret_cast<void*>(context));

    /* print the call trace */
    backtrace_addr(reinterpret_cast<void*>(instr), sp);

    int tid = 0;
    if (pCurrentRunningTask != 0)
        tid = pCurrentRunningTask->getId();

    int ifar, ifsr;
    asm (
            "MRC p15,0,%0,c6,c0,2;"
            "MRC p15,0,%1,c5,c0,1;"
            : "=&r" (ifar), "=&r" (ifsr)
            :
            :
    );

    LOG(ARCH, ERROR, "TID: %d, IFAR: %x, IFSR: %x", tid, ifar, ifsr);

#ifdef HAS_Board_HatLayerCfd

    int asid = -1;
    unint4 tbb0 = 0;
    unint4 paget = ((unint4) (&__PageTableSec_start)) + tid * 0x4000;

    asm (
            "MRC p15,0,%0,c13,c0,1;"  // ; Read CP15 Context ID Register
            "MRC p15,0,%1,c2,c0,0;"// ; Read CP15 Translation Table Base Register 0
            : "=&r" (asid), "=&r" (tbb0)
            :
            : "r0"
    );

    LOG(ARCH, ERROR, "ASID: %d %d, TBB0: 0x%x, Task PT: 0x%x", asid >> 8, asid & 0xff, tbb0, paget);
    theOS->getHatLayer()->dumpPageTable(tid);
#endif

    theOS->getErrorHandler()->handleError(cDataAbortError);
}

/*
 * This method takes care of the low level IRQ functionality:
 * - registering the stack pointer at interruption (context) and IRQ return mode
 * - getting the irq number
 * - dedicated interrupt dispatching for timer interrupts
 * - forwarding of all other irqs to the generic interrupt manager
 */
extern "C" void dispatchIRQ(void* sp_int, int mode) {
    if (pCurrentRunningThread != 0) {
        ASSERT(isOk(pCurrentRunningThread->pushStackPointer(sp_int)));
        ASSERT(isOk(pCurrentRunningThread->pushStackPointer(reinterpret_cast<void*>(mode))));
    }

    int irqSrc;
    irqSrc = theInterruptController->getIRQStatusVector();

    LOG(HAL, TRACE, "IRQ number: %d, sp_int %x, mode: %d", irqSrc, sp_int, mode);

    /* jump to interrupt handler according to active interrupt */
    switch (irqSrc) {
        /* General Purpose Timer interrupt used for scheduling */
        case GPT1_IRQ: {
            /* non returning irq ..*/
            theTimer->clearIRQ();
            /* allow new interrupts to occur */
            theInterruptController->clearIRQ(irqSrc);
            theTimer->tick();
               __builtin_unreachable();
            break;
        }
        default: {
            /* all other irqs are handled by the interrupt manager
             * and scheduled using workerthreads if activated */
            theOS->getInterruptManager()->handleIRQ(irqSrc);
        }
    }

    /* get back to running thread if any. Otherwise try to dispatch.
     * if some higher priority thread got unblocked a rescheduling irq
     * will already be pending and ensure dispatch after context
     * return*/
    if (pCurrentRunningThread != 0) {
        assembler::restoreContext(pCurrentRunningThread);
    } else {
        theOS->getDispatcher()->dispatch();
    }

    __builtin_unreachable();
}

extern "C" void dispatchSWI(void* sp_int, int mode) {
    ASSERT(isOk(pCurrentRunningThread->pushStackPointer(sp_int)));
    ASSERT(isOk(pCurrentRunningThread->pushStackPointer(reinterpret_cast<void*>(mode))));

    handleSyscall((intptr_t) sp_int);
}

