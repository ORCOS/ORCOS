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

#ifndef PPC405MMU_HH_
#define PPC405MMU_HH_

#include "PPC405TlbEntry.hh"
#include "powerpc.h"
#include "ppc405.h"
#include "types.hh"

/*!
 *  \brief This class is an abstraction of the memory management unit of the PowerPC405 architecture
 *
 *  Thus this class provides the method to work and access directly the TLB of the target architcture.
 *  So it is possible to read, write and search TLB Entries.
 */
class PPC405MMU {
private:

public:
    //! standard constructor
    PPC405MMU();
    //! standard destructor
    ~PPC405MMU();

    /*!
     *  \brief Write an entry to the TLB
     *
     *  This method writes the entry with the values specified in the data structure
     *  referenced by parameter tlbEntry into the TLB, at the position idEntry
     */
    static ErrorT writeEntry( int idEntry, PPC405TlbEntry* tlbEntry ) {
        // get the actual process id from PID-Register
        TaskIdT pID = ppc_mfpid();
        // set PID-Register to the PID given in tlbEntry
        ppc_mtpid(tlbEntry->getProcID());
        // write the TAG-Portion of the TLB Entry
        ppc_tlbwehi(tlbEntry->getTag(),idEntry);
        // write the DATA-Portion of the TLB Entry
        ppc_tlbwelo(tlbEntry->getData(),idEntry);
        // move back the old PID to the PID-Register
        ppc_mtpid(pID);
        // perform context synchronization
        ppc_isync();
        return cOk;
    }

    /*!
     * \brief Reads an entry from the TLB
     *
     *  This method reads the TLB entry at position idEntry and stores the
     *  corresponding values in the data structure referenced by tlbEntry
     */
    static ErrorT readEntry( int idEntry, PPC405TlbEntry* tlbEntry ) {
        register int pid = ppc_mfpid();
        tlbEntry->setTag( ppc_tlbrehi(idEntry) );
        tlbEntry->setData( ppc_tlbrelo(idEntry) );
        tlbEntry->setProcID( ppc_mfpid() );
        ppc_mtpid(pid);
        ppc_isync();
        return cOk;
    }

    /*!
     * \brief search the entry in the TLB with the given logical address
     *
     *  For a given logical address, this method returns the position of the
     *  entry for this address in the TLB
     */
    static ErrorT searchEntry( unint addr, int &idEntry ) {
        ErrorT result;
        int index;
        asm volatile(
                "tlbsx. %1, 0, %2;" // search tlb entry which corresponds to the given address
                "xor %0, %0, %0;"   // set result to zero
                "beq 8;"        	// entry found, branch to .SUCCESS and finish
                "li %0, -101;"      // result is error code (like defined in error.hh)
                : "=r"(result),"=r"(index)
                : "r"(addr)
             //   : // no clobber list
        );

        idEntry = index;
        return result;
    }

    /*!
     * \brief All existing TLB entries are set to not valid
     */
    static void invalidate() {
        ppc_tlbia();

        ppc_isync();
    }

};

#endif /*PPC405MMU_HH_*/
