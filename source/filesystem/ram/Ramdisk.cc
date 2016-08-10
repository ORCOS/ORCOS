/*
 * Ramdisk.cc
 *
 *  Created on: 06.11.2015
 *      Author: Daniel Baldin
 *
 *  Implements a simple Ramdisk. The Ramdisk is itself just a simple BlockDevice
 *  all other users may use to read and write blocks from/to.
 *  The Ramdisk gets an arbitrary ram memory area. If this area
 *  contains e.g. a filesystem binary the filesystem
 *  may easiliy be mounted ontop of the ramdisk into
 *  the user filesystem.
 *
 *
 */

#include "Ramdisk.hh"
#include "kernel/Kernel.hh"

extern Kernel* theOS;

Ramdisk::Ramdisk(intptr_t PhysicalStartAddress,
                 intptr_t LogicalStartAddress,
                 size_t   Size,
                 unint4   Sector_Size,
                 char*    Name)
                : BlockDeviceDriver(Name)
{
    if (Sector_Size) {
        this->sector_size = Sector_Size;
    }

    this->PhysicalStartAddress = PhysicalStartAddress;
    this->Size = Size;
    this->LogicalStartAddress = theOS->getHatLayer()->map(PhysicalStartAddress,
                                                          alignCeil((char*) Size, PAGESIZE),
                                                          hatProtectionRead | hatProtectionWrite,
                                                          0,
                                                          0);

    LOG(ARCH, INFO, "Ramdisk initialized at 0x%08x physical", this->PhysicalStartAddress);
    LOG(ARCH, INFO, "Ramdisk Size is 0x%08x", this->Size);
}

Ramdisk::~Ramdisk() {
    for (intptr_t Address = this->LogicalStartAddress; Address < this->LogicalStartAddress + this->Size; Address += PAGESIZE) {
        theOS->getHatLayer()->unmap(Address, 0);
    }
    this->LogicalStartAddress = 0;
    this->Size = 0;
    this->PhysicalStartAddress = 0;
}


/*****************************************************************************
  * Method: Ramdisk::readBlock(unint4 blockNum, char* buffer, unint4 length)
  *
  * @description
  *  Tries to read "length" blocks from this device starting at block number "blockNum" into the
  *  buffer at address "buffer".
  *
  *  Returns cOk on success, Error number (<0) otherwise
  *******************************************************************************/
ErrorT Ramdisk::readBlock(unint4 blockNum, char* buffer, unint4 length) {
    if (this->LogicalStartAddress)
    {
        if ((blockNum + length) * this->sector_size > this->Size) {
            return (cBlockDeviceTooManyBlocks);
        }

        intptr_t MemoryAddr = this->LogicalStartAddress + (this->sector_size * blockNum);
        for (unint4 i = 0; i < length; i++)
        {
            memcpyl(buffer, (void*) MemoryAddr, this->sector_size);
            buffer     += this->sector_size;
            MemoryAddr += this->sector_size;
        }

        return (cOk);
    }
    return (cError);
}

/*****************************************************************************
 * Method: Ramdisk::writeBlock(unint4 blockNum, char* buffer, unint4 length)
 *
 * @description
 *  Tries to write "length" blocks to the device starting at block number "blockNum".
 *
 *  Returns cOk on success, Error number (<0) otherwise.
 *******************************************************************************/
ErrorT Ramdisk::writeBlock(unint4 blockNum, char* buffer, unint4 length) {
    if (this->LogicalStartAddress)
    {
        if ((blockNum + length) * this->sector_size > this->Size) {
            return (cBlockDeviceTooManyBlocks);
        }

        intptr_t MemoryAddr = this->LogicalStartAddress + (this->sector_size * blockNum);
        for (unint4 i = 0; i < length; i++)
        {
            memcpyl((void*) MemoryAddr, buffer, this->sector_size);
            buffer     += this->sector_size;
            MemoryAddr += this->sector_size;
        }

        return (cOk);
    }
    return (cError);
}

