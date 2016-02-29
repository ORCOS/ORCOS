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

#include "MicroFunkBoard.hh"
#include "kernel/Kernel.hh"
extern Board_ClockCfdCl* theClock;
extern Kernel* theOS;

/*
 * Concatenate preprocessor tokens A and B without expanding macro definitions
 * (however, if invoked from a macro, macro arguments are expanded).
 */
#define PPCAT_NX(A, B) A ## B

/*
 * Concatenate preprocessor tokens A and B after macro-expanding them.
 */
#define PPCAT(A, B) PPCAT_NX(A, B)

/*
 * Turn A into a string literal without expanding macro definitions
 * (however, if invoked from a macro, macro arguments are expanded).
 */
#define STRINGIZE_NX(A) #A

/*
 * Turn A into a string literal after macro-expanding it.
 */
#define STRINGIZE(A) STRINGIZE_NX(A)

// cppcheck-suppress uninitMemberVar
MicroFunkBoard::MicroFunkBoard() {
}

void MicroFunkBoard::early_init() {
    CLKPR = 128; // allow clock change!
    CLKPR = 0;   // divide by 1!

    for (volatile int i = 0; i < 1000; i++);

#ifdef HAS_Board_UARTCfd

    INIT_Board_UARTCfd;
    UARTCfd = new NEW_Board_UARTCfd;

   // theOS->setStdOutputDevice( UARTCfd );

     LOG(ARCH,INFO,"Mikrofunkboard Initializing...");
     LOG(ARCH,INFO,"Board UART: [" STRINGIZE(Board_UARTCfdCl) "]" );
#endif

#ifdef HAS_Board_ClockCfd
    LOG(ARCH,INFO,"Board Clock [" STRINGIZE(Board_ClockCfdCl) "]" );
    INIT_Board_ClockCfd;
    ClockCfd = new NEW_Board_ClockCfd;
    theClock = ClockCfd; // clock is now available for other devices
#endif

#ifdef HAS_Board_ProcessorCfd
     LOG(ARCH,INFO,"Board Processor [" STRINGIZE(Board_ProcessorCfdCl) "]" );
     ProcessorCfd = new NEW_Board_ProcessorCfd;
#endif
     // Watchdog
  #ifdef HAS_Board_WatchdogCfd
      WatchdogCfd = new NEW_Board_WatchdogCfd;
      //getWatchdog()->enable();
  #endif

  // Timer
#ifdef HAS_Board_TimerCfd
  LOG(ARCH,INFO,"Board Timer [" STRINGIZE(Board_TimerCfdCl) "]" );
  INIT_Board_TimerCfd;
  TimerCfd = new NEW_Board_TimerCfd;
#endif
}

MicroFunkBoard::~MicroFunkBoard() {
}
