/*
 * Ramdisk.hh
 *
 *  Created on: 06.11.2015
 *      Author: Daniel
 */

#ifndef SOURCE_FILESYSTEM_RAM_RAMDISK_HH_
#define SOURCE_FILESYSTEM_RAM_RAMDISK_HH_

/*
 *
 */
class Ramdisk: public BlockDeviceDriver {
private:
    // Physical Start Address of Ramdisk
    intptr_t PhysicalStartAddress;
    // Logical Start Address of Ramdisk
    intptr_t LogicalStartAddress;
    // Size of the Ramdisk
    size_t   Size;

public:
    Ramdisk(intptr_t PhyiscalStartAddress,
            intptr_t LogicalStartAddress,
            size_t   Size,
            unint4   Sector_Size,
            char*    Name);

    virtual ~Ramdisk();

    /*****************************************************************************
      * Method: readBlock(unint4 blockNum, char* buffer, unint4 length)
      *
      * @description
      *  Tries to read "length" blocks from this device starting at block number "blockNum" into the
      *  buffer at address "buffer".
      *
      *  Returns cOk on success, Error number (<0) otherwise
      *******************************************************************************/
     ErrorT readBlock(unint4 blockNum, char* buffer, unint4 length);

     /*****************************************************************************
      * Method: writeBlock(unint4 blockNum, char* buffer, unint4 length)
      *
      * @description
      *  Tries to write "length" blocks to the device starting at block number "blockNum".
      *
      *  Returns cOk on success, Error number (<0) otherwise.
      *******************************************************************************/
     ErrorT writeBlock(unint4 blockNum, char* buffer, unint4 length);

};

#endif /* SOURCE_FILESYSTEM_RAM_RAMDISK_HH_ */
