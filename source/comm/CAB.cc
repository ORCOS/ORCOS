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

#include "CAB.hh"
#include <memtools.hh>
#include <assemblerFunctions.hh>
#include "kernel/Kernel.hh"

extern Task* pCurrentRunningTask;
extern Kernel* theOS;

CAB::CAB( void* p_bufferstart, unint4 u_length, Task* p_ownerTask ) {

#ifdef HAS_Board_HatLayerCfd
    // well be working on the physical address when storing only
    this->bufferstart_physical = (char*) theOS->getHatLayer()->getPhysicalAddress(bufferstart);
    ASSERT(bufferstart_physical == 0);
#endif

    ASSERT(bufferstart);
    this->bufferstart = p_bufferstart;
    this->length = u_length;

    this->dim_buf = (unint2) (u_length / MAX_BUF);
    this->free = 0;
    this->actb = -1;
    this->ownerTask = p_ownerTask;

    LOG( COMM, DEBUG,(COMM, DEBUG, "CAB::CAB(): created CAB with %d buffer of size %d", MAX_BUF, dim_buf) );
}

CAB::~CAB() {

}

/*!
 * pmsg can point to either:
 * 	- address in tasks heap (if the communication is local)
 *  - address in kernel heap (if message was received from a remot peer)
 *
 */
ErrorT CAB::store( char* pmsg, int msglen ) {
   ASSERT(pmsg);

   if( msglen > this->dim_buf ) {
       if (pCurrentRunningTask){
           LOG( COMM, ERROR,(COMM, ERROR, "%d: CAB::store(): msglen to big for CAB! %d > %d", pCurrentRunningTask->getId(), msglen, dim_buf) );
       }else{
           LOG( COMM, ERROR,(COMM, ERROR, "CAB::store(): msglen to big for CAB! %d > %d", msglen, dim_buf) );
       }
           return (cError);
       }

   if (pCurrentRunningTask){
       LOG( COMM, DEBUG,(COMM, DEBUG, "Task %d: CAB::store(): actb=%d, free=%d", pCurrentRunningTask->getId(), actb, free) );
   }else {
       LOG( COMM, DEBUG,(COMM, DEBUG, "CAB::store(): actb=%d, free=%d", actb, free) );
   }

   if ( free == actb ) {
           // increment actb so it points to the second oldest and now new oldest msg
	   	   actb++;
	   	   if ( actb == MAX_BUF) actb = 0;
      }


   unint4 dest;
	 bool int_enabled;
	 GET_INTERRUPT_ENABLE_BIT(int_enabled);

	 // TODO: disable interrupts for the complete memcpy may take too long
	 _disableInterrupts();


#ifdef HAS_Board_HatLayerCfd

    unint2 pid = 0;
    if (pCurrentRunningTask != 0)
    	pid = pCurrentRunningTask->getId();

    // map this buffer into the logical address space of the calling task
    // TODO: size calculation may be wrong! mapping may need more than 1 page ..
    // ARM does need bufferstart_physical to be a page address!

    unint4 phypage = (((unint4)bufferstart_physical) >> 20) << 20;
    unint4 offset = (unint4)bufferstart_physical - (unint4)phypage;

    void* realPage = theOS->getHatLayer()->map((void*) 0x2000000,(void*) phypage, 0x200000-1 ,7,3,pid, !ICACHE_ENABLE);

    dest = ((int4) 0x2000000 + offset + this->free * this->dim_buf);

    void* dest_phy = theOS->getHatLayer()->getPhysicalAddress( (void*) dest);

    LOG( COMM, DEBUG,(COMM, DEBUG,"CAB::store(): buf_phy: 0x%x realPage: 0x%x dest_log: 0x%x, dest_phy; 0x%x", bufferstart_physical, realPage,dest,dest_phy) );

#else
    dest = (unint4) bufferstart + this->free * this->dim_buf;
#endif

    ( (int2*) dest )[ 0 ] = 0;
    // write data into buffer
    ( (int2*) (dest + 2 ) )[ 0 ] = (int2) msglen;
   // copy from source to dest
    memcpy( (void*) ( dest +  4 ), (void*) pmsg, msglen );

#ifdef HAS_Board_HatLayerCfd
    theOS->getHatLayer()->unmap((void*) 0x2000000);
    theOS->getHatLayer()->unmap((void*) 0x2100000);
#endif
    if ( int_enabled ) {
               _enableInterrupts();
           }
    // set actual buffer to this buffer if it was unset before
    if ( actb == -1 )
        actb = free;

    int ret = free;

    free++;
    if (free == MAX_BUF) free = 0;

    return (ret);
}

int2 CAB::get( char** addressof_ret_ptrtomsg, int2 &buffer ) {
    ASSERT(addressof_ret_ptrtomsg);
    if (pCurrentRunningTask){
    	 LOG( COMM, DEBUG,(COMM, DEBUG, "%d: CAB::get(): actb=%d, free=%d", pCurrentRunningTask->getId(), actb, free) );
    }else {
    	 LOG( COMM, DEBUG,(COMM, DEBUG, "CAB::get(): actb=%d, free=%d", actb, free) );
    }

    buffer = actb;
#ifdef HAS_Board_HatLayerCfd
    unint4 addr = ((unint4) bufferstart) + this->actb * this->dim_buf;
    void* phy_addr =  theOS->getHatLayer()->getPhysicalAddress( (void*) addr );
    LOG( COMM, DEBUG,(COMM, DEBUG,"CAB::get(): addr: 0x%x phy_addr: 0x%x", addr,phy_addr) );
#endif

    int2 retlen = 0;
    // check if we have a message at all
    if ( this->actb != -1 ) {

        *addressof_ret_ptrtomsg = (char*) ( (int) bufferstart + this->actb * this->dim_buf + 4 );

        // return length of data as pointer
        retlen = ( (int2*) ( (int) bufferstart + this->actb * this->dim_buf + 2 ) )[ 0 ];

        actb++;
        if (actb == MAX_BUF) actb = 0;

        // check if next buffer to read has message
        if ( actb == free )
            actb = -1;
    }
    else {
        *addressof_ret_ptrtomsg = (char*) 0;
    }



    return retlen;
}
