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

#ifndef ARMv7HATLAYER_HH_
#define ARMv7HATLAYER_HH_

//#include <hal/HatLayer.hh>

#define SECTION_SIZE 0x100000

// Define for the Paged Memory Manager
#define PAGESIZE SECTION_SIZE

/*!
 * \brief This class provides an abstraction of the virtual memory management of the ARMv7 architecture
 */

class ARMv7HatLayer /*: public HatLayer*/ {
private:

	/*!
	 * \brief This method maps the kernel memory
	 */
    void mapKernel( BitmapT, int, bool );

	/*!
	 * \brief This method creates a page table
	 */
    void* createPT(void* , void* , size_t , BitmapT , byte , int,  bool, bool );

public:

    /*!
     *  \brief Constructor
     *
     *  Implements the corresponding constructor in HATLayer.hh
     *
     */
    ARMv7HatLayer();

    //! standard destructor
    ~ARMv7HatLayer();

    /*!
     *  \brief Map method to map an address space with logical addresses to physical addresses
     *
     *  In this method the page table entry is created and then written to the page table.
     *  The created entry in the page table represents then a mapping of an address space starting logically at the given logical address and
     *  physically at the given physical address.
     */
    void* map( void* logBaseAddr, void* physBaseAddr, size_t size, BitmapT protection, byte zsel, int pid,
            bool cache_inhibit );


    /*!
     * \brief Maps a physical base address into an arbitray virtual address determined by the HATLayer.
     *
     */
    void* map( void* phyBaseAddr, size_t, BitmapT, byte, int pid,  bool cache_inhibit );

    /*!
     * \brief Unmap an address space with a logical address from his given physical address
     *
     * Implements the corresponding method in HATLayer.hh
     * \param addr	The address to be unmapped
     * \param pid	The Process ID. if -1 the current running process will be used.
     */
    ErrorT unmap( void* addr, unint1 tid = 0);

    /*!
     * \brief unmap all address spaces with logical addresses which has been mapped before by this HATLayer-Instance
     *
     * Implements the corresponding method in HATLayer.hh
     *
     */
    ErrorT unmapAll(int pid);

    /*!
     * \brief change Protection of an address space of a given logical address
     *
     * Implements the corresponding method in HATLayer.hh
     *
     */
    ErrorT changeProtection( void*, BitmapT );

    /*!
     * \brief clear all protections of an address space of a given logical address
     *
     * Implements the corresponding method in HATLayer.hh
     */
    ErrorT clearProtection( void* );

    /*!
     * \brief get the protection of an address space for a given logical address
     *
     * Implements the corresponding method in HATLayer.hh
     */
    BitmapT getProtection( void* );

    /*!
     * \brief enable hardware address translation
     *
     * Implements the corresponding method in HATLayer.hh
     *
     * Set translation table base control register, set translation base register,
     * set ASID and PROCID, set domain as executable and enable MMU
     */
    ErrorT enableHAT();

    /*!
     * \brief disable hardware address translation
     *
     * Implements the corresponding method in HATLayer.hh
     *
     */
    ErrorT disableHAT();

    /*!
     * \brief returns the logical address translation for a given physical address
     *
     * Implements the corresponding method in HATLayer.hh
     */
    void* getLogicalAddress( void* );

    /*!
     * \brief returns the physical address translation for a given logical address and tid
     *
     * Implements the corresponding method in HATLayer.hh
     */
    void* getPhysicalAddress( void* , int tid );

    /*!
    * \brief returns the physical address translation for a given logical address with the current pid as tid
    *
    * Implements the corresponding method in HATLayer.hh
    */
    void* getPhysicalAddress( void* );

    /*!
     * \brief handles errors if for a given logical address no mapping can be found
     *
     * Implements the corresponding method in HATLayer.hh
     *
     * This method is called by the ARMv7Interrupthandler if a Data or Instruction TLB Miss
     * occurs. If there is a debugger configured, a debug message is printed.
     */
    void handleMappingError();


    /*!
     *
     * Prints out the mapped entries of the current process.
     */
    void dumpPageTable(int pid);

    /*!
     * \brief initialization method of the HATLayer
     *
     * Implements the corresponding method in HATLayer.hh
     *
     * Clear page table area and make entries fault entries and invalidate TLB
     */
    static void initialize();

};

#endif /*ARMv7HATLAYER_HH_*/
