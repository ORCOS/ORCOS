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

#include <hal/HatLayer.hh>
#include "synchro/Mutex.hh"

#define SECTION_SIZE 0x100000

// Define for the Paged Memory Manager
#define PAGESIZE SECTION_SIZE

/*!
 * \brief This class provides an abstraction of the virtual memory management of the ARMv7 architecture
 */
class ARMv7HatLayer /*: public HatLayer*/{
private:
    /***********************************************
     * Method: mapKernel(int pidl)
     *
     * @description:
     *
     * Maps the kernel pages into the process with given PID.
     * The mapped areas are the kernel area, the cache inhibit data area
     * if used and the architecture dependent mappings provided by the
     * arch_kernelmappings structure for, e.g., MMIO devices.
     *
     * @params
     * \param pid           The PID the mapping is created for
     *
     **********************************************/
    void mapKernel(int pid);


    /*****************************************************************************
     * Method: createPT(void* logBaseAddr,
     *                  void* physBaseAddr,
     *                  size_t size,
     *                  BitmapT protection,
     *                  byte zsel,
     *                  int pid,
     *                  bool cache_inhibit,
     *                  bool nonGlobal) {
     *
     * @description
     *  Creates the page table entry for the mapping specified by the paramters.
     *
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    void* createPT(void*    logBaseAddr,
                   void*    physBaseAddr,
                   size_t   size,
                   BitmapT  protection,
                   byte     zsel,
                   int      pid,
                   int      cacheMode,
                   bool     nonGlobal);

    Mutex myMutex;

public:
    /*!
     *  \brief Constructor
     *
     *  Implements the corresponding constructor in HATLayer.hh
     *
     */
    ARMv7HatLayer();

    /* standard destructor. Do not use as this is not supported! */
    ~ARMv7HatLayer();

    /***********************************************
     * Method: map(void* logBaseAddr,
     *             void* physBaseAddr,
     *             size_t size,
     *             BitmapT protection,
     *             byte zsel,
     *             int pid,
     *             bool cache_inhibit)
     *
     * @description
     *
     * Creates a new virtual memory mapping for the given PID with given logical and physical address.
     *
     * @params
     * \param logBaseAddr   The logical address of the mapping
     * \param phyBaseAddr   The physical address of the mapping
     * \param size          The length in Bytes of the mapped area
     * \param protection    The access rights of the mapping (RWX)
     * \param zsel          The zone the mapping belongs to (unused for ARM)
     * \param pid           The PID the mapping is created for
     * \param cache_inhibit If true inhibits caching inside this memory region. Usefully for MMIO areas.
     *
     * @returns
     * void*                The physical addreess.
     *
     **********************************************/
    void* map(void* logBaseAddr, void* physBaseAddr, size_t size, BitmapT protection, byte zsel, int pid, int cacheMode = hatCacheWriteBack);

    /***********************************************
     * Method: map(void* physBaseAddr,
     *                            size_t size,
     *                            BitmapT protection,
     *                            byte zsel,
     *                            int pid,
     *                            bool cache_inhibit)
     *
     * @description
     *
     * Creates a new virtual memory mapping for the given PID and the given physical address. The mapping will
     * use a free logical address region for the mapping and returns the logical base address of the created mapping.
     *
     * @params
     * \param logBaseAddr   The logical address of the mapping
     * \param phyBaseAddr   The physical address of the mapping
     * \param size          The length in Bytes of the mapped area
     * \param protection    The access rights of the mapping (RWX)
     * \param zsel          The zone the mapping belongs to (unused for ARM)
     * \param pid           The PID the mapping is created for
     * \param cache_inhibit If true inhibits caching inside this memory region. Usefully for MMIO areas.
     *
     * @returns
     * void*                The logical base address of the mapping.
     *
     **********************************************/
    void* map(void* phyBaseAddr, size_t, BitmapT, byte, int pid, int cacheMode = hatCacheWriteBack);

    /*****************************************************************************
     * Method: unmap(void* logBaseAddr, unint1 tid)
     *
     * @description
     *  Unmaps the given logical address from the address space tid.
     *  Removes the page table entry and invalidates the TLB containing the address
     *
     * @params
     *   addr       The address to be unmapped
     *   pid        The Process ID. If 0 the current running process will be used.
     * @returns
     *  int         Error Code
     *
     *******************************************************************************/
    ErrorT unmap(void* addr, unint1 pid = 0);


    /*****************************************************************************
     * Method: unmapAll(int pid)
     *
     * @description
     *  Removes all mappings of the address space pid including
     *  invalidation of TLB entries of the address space.
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT unmapAll(int pid);


    /*****************************************************************************
     * Method: enableHAT()
     *
     * @description
     *  Enables the hardware translation unit. The processor
     *  will now treat all addresses as logical ones. If no mapping exists
     *  fauls will be generated.
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT enableHAT();


    /*****************************************************************************
     * Method: disableHAT()
     *
     * @description
     *  Disables the hardware translation unit.
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT disableHAT();

    /*****************************************************************************
     * Method: getLogicalAddress(void* physAddr)
     *
     * @description
     *  Translates a given physical address to its logical address
     *  inside the current address space.
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    void* getLogicalAddress(void* physicalAddress);

    /*****************************************************************************
     * Method: getPhysicalAddress(void* log_addr)
     *
     * @description
     *  Translates a given logical address to its phyiscal address
     *  inside the current address space.
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    void* getPhysicalAddress(void* logicalAddress);

    /*****************************************************************************
     * Method: dumpPageTable(int pid)
     *
     * @description
     *  Dumps the page table of the given address space to stdout
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    void dumpPageTable(int pid);


    /*****************************************************************************
     * Method: initialize()
     *
     * @description
     *  Initializes the page tables.
     *  Clear page table area and make entries fault entries and invalidate TLB
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    static void initialize();
};

#endif /*ARMv7HATLAYER_HH_*/
