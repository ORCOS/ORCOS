/*
 * BlockDeviceDriver.hh
 *
 *  Created on: 14.06.2013
 *     Copyright & Author: dbaldin
 */

#ifndef BLOCKDEVICEDRIVER_HH_
#define BLOCKDEVICEDRIVER_HH_

#include "GenericDeviceDriver.hh"
#include "inc/Bitmap.hh"

class BlockDeviceDriver:  public GenericDeviceDriver {
protected:
    /*!
     * \brief list of block device ids
     */
    static IDMap<20> freeBlockDeviceIDs;
public:
    /*****************************************************************************
     * Method: BlockDeviceDriver(char* name)
     *
     * @description
     *
     *******************************************************************************/
    explicit BlockDeviceDriver(char* name);

    virtual ~BlockDeviceDriver();

    // The physical size of a sector
    unint4 sector_size;


    /*****************************************************************************
     * Method: initialize()
     *
     * @description
     *  initialize
     *******************************************************************************/
    static void initialize() {
    }

    /*****************************************************************************
     * Method: readBlock(unint4 blockNum, char* buffer, unint4 length)
     *
     * @description
     *  Tries to read "length" blocks from this device starting at block number "blockNum" into the
     *  buffer at address "buffer"
     *
     *  Returns cOk on success, Error number (<0) otherwise
     *******************************************************************************/
    virtual ErrorT readBlock(unint4 blockNum, char* buffer, unint4 length) = 0;

    /*****************************************************************************
     * Method: writeBlock(unint4 blockNum, char* buffer, unint4 length)
     *
     * @description
     *  Tries to write "length" blocks to the device starting at block number "blockNum".
     *
     *  Returns cOk on success, Error number (<0) otherwise.
     *******************************************************************************/
    virtual ErrorT writeBlock(unint4 blockNum, char* buffer, unint4 length) = 0;
};

#endif /* BLOCKDEVICEDRIVER_HH_ */
