/*
 * PartitionManager.hh
 *
 *  Created on: 19.06.2013
 *    Copyright &  Author: dbaldin
 */

#ifndef PARTITIONMANAGER_HH_
#define PARTITIONMANAGER_HH_

#include "PartitionManager.hh"
#include "hal/BlockDeviceDriver.hh"
#include "filesystem/Partition.hh"
#include "filesystem/Directory.hh"

typedef enum  {
    GENERIC_MBR,
    STANDARD_MBR,
    AAP_MBR,
    NEWLDR_MBR,
    ASTNEC_MBR,
    DISKMANAGER_MBR
} MBRType ;

class PartitionManager: Directory {
private:
    /*****************************************************************************
     * Method: tryDOSMBR(BlockDeviceDriver *bdev)
     *
     * @description
     *  Tries to read a DOS MBR from the block device.
     *
     *  returns: cOk on success
     *******************************************************************************/
    ErrorT tryMBR(BlockDeviceDriver *bdev);

    /*****************************************************************************
     * Method: handleEFIPartitionTable(BlockDeviceDriver* bdev)
     *
     * @description
     *
     *******************************************************************************/
    ErrorT handleEFIPartitionTable(BlockDeviceDriver* bdev);

public:
    PartitionManager();

    /*****************************************************************************
     * Method: registerBlockDevice(BlockDeviceDriver *bdev)
     *
     * @description
     *
     *******************************************************************************/
    void registerBlockDevice(BlockDeviceDriver *bdev);

    /*****************************************************************************
     * Method: unregisterBlockDevice(BlockDeviceDriver *bdev)
     *
     * @description
     *  Try and find the correct partition type of the device
     *******************************************************************************/
    void unregisterBlockDevice(BlockDeviceDriver *bdev);


    virtual ~PartitionManager();
};

#endif /* PARTITIONMANAGER_HH_ */
