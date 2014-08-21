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

#ifndef HATLAYER_HH_
#define HATLAYER_HH_

//#include <OS_Configuration.hh>
#include "SCLConfig.hh"
#include <types.hh>
#include <error.hh>
#include <Bitmap.hh>

typedef struct t_mapping {
    unint4 vir_addr;
    unint4 phy_addr;
    size_t size;
    BitmapT protection;
    BitmapT flags;
} t_mapping;

typedef struct t_archmappings {
    unint1 count;
    t_mapping* mappings;
} t_archmappings;

/* Protection Flags */
#define hatProtectionRead         (BitmapT)1
#define hatProtectionWrite        (BitmapT)2
#define hatProtectionExecute      (BitmapT)4

/* Mapping Flags */
#define hatCacheInhibit           (BitmapT)1

/*!
 * \ingroup memmanager
 * \brief Superclass for HAT(Hardware address translation) layers, provides an interface for them.
 *
 * HATLayer is an interface which should be implemented depending on the target hardware architecture.
 * It is an abstraction of the given hardware resources to work with virtual memory. By using this interface
 * it is ensured that the ORCOS-OS can deal with virtual memory for every hardware architecture supporting
 * virtual memory without changing the Kernel itself. The concrete HATLayer is a member of the Memory Manager
 * and must be configured using SCL.
 *
 *
 */
class HatLayer {
public:
    /*!
     *  \brief Constructor - only for the Kernel MM in kernelmain.cc
     *
     * The member taskID is set to 0 (as this constructor should only be called by the Kernel's MM).
     * The necessary mappings for the Kernel should be executed here (Kernelcode, Kernelheap, Memory-Mapped I/O)
     */
    HatLayer() {
    }

    /*!
     *  \brief map an address space with a logical address to a physical address
     *
     *  logBaseAddr  - the logical start address of the mapped address space
     *  physBaseAddr - the physical start address of the mapped address space
     *  size         - the size of the mapped address space
     *  protection   - which protection should be set for this address space (read,write, execute)
     *
     *  After calling the map method, the OS should be able to work with the logical addresses
     *  from logBaseAddr up to (logBaseAddr + size) and find the corresponding physical addresses
     *  by the hardware dependent translation mechanisms
     *
     *  Returns the real page (physical address) that has been assigned to enclose the segment.
     */
    void* map(void*, void*, int, int, int, TaskIdT, bool) {
        return (void*) -1;
    }

    /*!
     * \brief unmap an address space with a logical address from his given physical address
     *
     * For the given logical start address: if there exists a mapping for this address (which has been performed
     * by this instance of the HATLayer) then the corresponding mapping is reverted and cannot longer be used
     * by the system for address translation
     *
     */
    ErrorT unmap(void* logBaseAddr) {
        return cNotImplemented ;
    }

    /*!
     * \brief unmap all address spaces with logical addresses which has been mapped before by this HATLayer-Instance
     */
    ErrorT unmapAll();

    /*!
     * \brief change Protection of an address space of a given logical address
     */
    ErrorT changeProtection(void* logBaseAddr, BitmapT newProtection) {
        return cNotImplemented ;
    }

    /*!
     * \brief clear all protections of an address space of a given logical address
     */
    ErrorT clearProtection(void* logBaseAddr) {
        return (changeProtection(logBaseAddr, (BitmapT) 0));
    }

    /*!
     * \brief get the protection of an address space for a given logical address
     */
    BitmapT getProtection(void* logBaseAddr) {
        return (BitmapT) 0;
    }

    /*!
     * \brief enable hardware address translation
     *
     * By executing this method the hardware address translation mechanism of the target architecture
     * is enabled. So after that the system will only work correctly with logical addresses. It must be ensured,
     * that the system is able to do this: there must be correct mappings for the used physical address space,
     * physical addresses which has been stored in pointers and should be reused must be translated to the corresponding
     * logical addresses, the stack pointer should also be set to its logical equivalent.
     * This method is called once in the ORCOS-OS in the kernelmain Method
     *
     */
    ErrorT enableHAT() {
        return cNotImplemented ;
    }

    /*!
     * \brief disable hardware address translation
     *
     * By executing this method the hardware address translation mechanism of the target architecture is
     * disabled. The system will only work correctly with physical addresses. Therefore every address in any
     * pointer which should be reused must be translated back to its physical address manually before calling this method!!!!
     *
     */
    ErrorT disableHAT() {
        return cNotImplemented ;
    }

    /*!
     * \brief returns the logical address translation for a given physical address
     *
     * For the given physical address this method returns the corresponding logical address
     * if a mapping for this physical address exists. If there is no virtual memory configured,
     * the given address itself will only be returned
     *
     */
    void* getLogicalAddress(void* physAddr) {
        return physAddr;
    }

    /*!
     *  \brief returns the physical address translation for a given logical address
     *
     * For the given logical address this method returns the corresponding physical address
     * if a mapping for this logical address exists. If there is no virtual memory configured,
     * the given address itself will only be returned
     */
    void* getPhysicalAddress(void* logAddr) {
        return logAddr;
    }

    /*!
     * \brief returns if an address space for a given logical address is readable
     */
    bool isReadable(void* logBaseAddr) {
        return ((Bitmap) getProtection(logBaseAddr)).isSet( hatProtectionRead);
    }

    /*!
     * \brief returns if an address space for a given logical address is readable
     */
    bool isWritable(void* logBaseAddr) {
        return ((Bitmap) getProtection(logBaseAddr)).isSet( hatProtectionWrite);
    }

    /*!
     * \brief returns if an address space for a given logical address is readable
     */
    bool isExecutable(void* logBaseAddr) {
        return ((Bitmap) getProtection(logBaseAddr)).isSet( hatProtectionExecute);
    }

    /*!
     * \brief handles errors if for a given logical address no mapping can be found
     *
     * This method should be called by the Interrupt Handler of the OS.
     */
    void handleMappingError() {
    }

    /*!
     * \brief initialization method of the HATLayer
     *
     * Any initialization which should be performed before working with virtual memory
     * is done here. This method is called at the start of the kernelmain-Method
     */
    static void initialize() {
    }
};

#endif /*HATLAYER_HH_*/
