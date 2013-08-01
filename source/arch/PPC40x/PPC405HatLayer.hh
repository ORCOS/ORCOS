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

#ifndef PPC405HATLAYER_HH_
#define PPC405HATLAYER_HH_

#include <hal/HatLayer.hh>
#include "PPC405MMU.hh"

///  Page size 1 KB
#define PPC405PageSize1Kb	0
///  Page size 4 KB
#define PPC405PageSize4Kb	1
///  Page size 16 KB
#define PPC405PageSize16Kb	2
///  Page size 64 KB
#define PPC405PageSize64Kb	3
///  Page size 256 KB
#define PPC405PageSize256Kb	4
///  Page size 1 MB
#define PPC405PageSize1Mb	5
///  Page size 4 MB
#define PPC405PageSize4Mb	6
///  Page size 16 MB
#define PPC405PageSize16Mb	7

#define MAX_PAGE_SIZE 0x1000000

#define isSet(Bitmap,Bit) (Bitmap & ( 1 << Bit ))

#define clearBit(Bitmap,Bit) Bitmap &= ~( 1 << Bit)

#define setBit(Bitmap,Bit) Bitmap |= ( 1 << Bit )

/*!
 * \brief This class provides an abstraction of the virtual memory management of the PPC405 architecture
 *
 * For the Hardware Address Translation the PPC405 provides a TLB with 64 entries. Every TLB entry can contain
 * a mapping of a logical address space to a physical address space of different size (1K, 4K, 16K, 64K, 256K, 1M, 4M, or 16M) with
 * the possibility to set protection rights (more information in the PPC405 UserManual).
 * This implementation uses a Superpaging approach:
 * - one TLB entry for the Kernel code and its static global variables.
 * - one (or more TLB entries, depending on size) for the Kernel heap
 * - one TLB entry for Memory Mapped I/O
 * - one TLB entry for every Task (including its code and heap)
 * For the purpose of dealing with Real Time, only 64 TLB entries are allowed to be created.
 * After that, no TLB entry is written anymore. Thus no swapping will take place.
 * Protection is set in the following way:
 * (Note: in the PPC405 architecture read access is be given by the V flag which also indicates if an TLB entry can be used,
 *  so the only possibility to deny read access in a valid TLB entry (only in problem state)is to set the ZSEL to a ZPR region with value 00)
 * - for every task:
 *   every task has in its TLB entry its TaskID. All rights are set (Read, Write, Execute).
 *   The part of the ZPR register to which the ZSEL(Zone selection)field of a task's TLB entry
 *   references has the value 01 (meaning the access is only controlled by the protection rights).
 *   Every task has only access to the address space of the TLB entry with its TaskID
 *
 * - for the Kernel:
 *   The Kernel has ID 0 and therefore all of his TLB entries use TaskID 0. In the PPC405 architecture TLB entries
 *   with ID 0 can be used without consideration of the Task ID (that means if Task 1 is running with the PID register set to 1,
 *   then this Task could also use the addresses of the entries with ID 0).
 *   Advantage: if an interrupt occurs, the Interrupt Routine and called Handler methods of the Kernel can be directly executed
 *   The following Protection rights are applied:
 *   for the TLB entry with the mapping of the Kernel code read and execute rights are set, write rights are refused.
 *   The ZSEL field is set to a ZPR Region with the value 10 (meaning full rights in supervisor state, in problem state only rights with the flags set).
 *   For the TLB entries for the Kernel heap and the memory-mapped I/O read and write rights are set, execute rights are refused.
 *   The ZSEL field is set to a ZPR Region with the value 00 (meaning no (even no read) access in problem state, access corresponding to the given rights in supervisor state)
 */
class PPC405HatLayer /*: public HatLayer*/ {
private:
    static unint8 TLBEntries;

    /*!
     * \brief returns the next free TLB entry
     *
     * The entries are searched from 0 up to 63. At the address of the given int* parameter, the
     * index is written. If there is no free TLB entry any more, an error code is returned.
     */
    ErrorT getFreeEntry( int* );

    /*!
     * \brief Set the corresponding flag in the global Bitmap representing the TLB entries
     *
     * If the parameter is > 32, HighEntries is set to 1 at the corresponding position in the Bitmap,
     * otherwise LowEntries
     */
    ErrorT setEntry( int );

    /*!
     * \brief Deletes the corresponding flag in the global Bitmap representing the TLB entries
     *
     * If the parameter is > 32, HighEntries is set to 0 at the corresponding position in the Bitmap,
     * otherwise LowEntries
     */
    ErrorT releaseEntry( int );

    /*!
     * \brief Gets for the given size_t the next higher possible size for a proper TLB entry
     *
     * The given size is enlarged to the next higher size: 1K, 4K, 16K, 64K, 256K, 1M, 4M, 16M
     * If the given size is bigger than 16M, then 16M is returned
     */
    byte getCorrectSizeEntry( size_t );

public:

    /*!
     *  \brief Constructor
     *
     *  Implements the corresponding constructor in HATLayer.hh
     *
     */
    PPC405HatLayer();

    //! standard destructor
    ~PPC405HatLayer();

    /*!
     *  \brief private map method to map an address space with logical addresses to physical addresses
     *
     *  In this method the TLB entry is created and then written to the TLB.
     *  Additional parameter of type byte is the ZSEL ( = which value should have the region in the ZPR for this value)
     *
     *  The created entry in the TLB represents than a mapping of a address space starting logically at the given logical address and
     *  physically at the given physical address with the size of the address space being given by the getCorrectSizeEntry. If the requested size
     *  is bigger than 16 M then so many entries are created until the whole memory space of the given size is mapped (e.g. if the size is 32 M there
     *  will be created 2 entries of size 16 M). The V,EX and WR flag are set corresponding to the given protetion Bitmap, ZSEL corresponding to the last
     *  parameter. The I field is also set.
     *  In the global Bitmaps representing the TLB entries and in the Bitmaps representing the TLB entries created by this HATLayer instance,
     *  the flag corresponding to the index of the TLB entry is set.
     *  If there exists no more free entry in the TLB the method aborts and returns an error value.
     */
    void* map( void*, void*, size_t, BitmapT, byte, int pid,  bool cache_inhibit );

    /*!
     * \brief unmap an address space with a logical address from his given physical address
     *
     * Implements the corresponding method in HATLayer.hh
     *
     * For the TLB entry with the given logical address the V flag is cleared.
     * An error value is returned if there is no entry written by this HATLayer instance with
     * the given logical address
     */
    ErrorT unmap( void* );

    /*!
     * \brief unmap all address spaces with logical addresses which has been mapped before by this HATLayer-Instance
     *
     * Implements the corresponding method in HATLayer.hh
     *
     * For all TLB entries with the index corresponding to the entries in the myLowEntries and myHighEntries-Bitmaps
     * the V flag is cleared.
     */
    ErrorT unmapAll();

    /*!
     * \brief change Protection of an address space of a given logical address
     *
     * Implements the corresponding method in HATLayer.hh
     *
     * The TLB entry with the given address is searched, saved in a data structure, changed and
     * written back to the TLB.
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
     * The stack pointer value in Register r1 and the old stack pointer value (before
     * calling this method) are manually translated to the corresponding logical addresses.
     * Then the IR and the DR flag of the MSR register are set
     */
    ErrorT enableHAT();

    /*!
     * \brief disable hardware address translation
     *
     * Implements the corresponding method in HATLayer.hh
     *
     * The stack pointer value in Register r1 and the old stack pointer value (before
     * calling this method) are manually translated to the corresponding physical addresses.
     * Then the IR and the DR flag of the MSR register are cleared
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
     * This method is called by the PPC405Interrupthandler if a Data or Instruction TLB Miss
     * occurs. If there is a debugger configured, a debug message is printed.
     */
    void handleMappingError();

    /*!
     * \brief initialization method of the HATLayer
     *
     * Implements the corresponding method in HATLayer.hh
     *
     * All TLB entries are invalidated (all V flags are cleared)
     */
    static void initialize();

};

#endif /*PPC405HATLAYER_HH_*/
