/*
 * FileSystemBase.hh
 *
 *  Created on: 19.06.2013
 *     Copyright & Author: dbaldin
 */

#ifndef FILESYSTEMBASE_HH_
#define FILESYSTEMBASE_HH_

#include "filesystem/Partition.hh"

class Partition;

/*!
 * Base Class of all FileSystem Handler. This class handles mounting into the
 * ORCOS filesystem and defines methods to be implemented by the superclass to
 * ensure filesystem traversal works.
 */
class FileSystemBase {
protected:
    /* Reference to the partition this filesystem is contained in.
     * Maybe 0 if no such physical partition exists as e.g. inside ramdisk.*/
    Partition*   myPartition;

    /* Valid flag of this filesystem. */
    bool        isValid;

    /* Number of blocks inside this filesystem */
    unint4      numBlocks;

    /* Blocksize of this filesystem */
    unint4      blockSize;

    /* Free blocks inside this filesystem */
    unint4      freeBlocks;

#if SYSFS_SUPPORT
    /* The sysfs directory */
    Directory*  sysFsDir;
#endif

public:
    /*****************************************************************************
     * Method: FileSystemBase(Partition* myPartition)
     *
     * @description
     *******************************************************************************/
    explicit FileSystemBase(Partition* myPartition);

    /*****************************************************************************
     * Method: FileSystemBase(const char* name)
     *
      * @description
      * Constructor for anonymous partitions as e.g. ramdisks.
    *******************************************************************************/
    explicit FileSystemBase(const char* name);

    virtual ~FileSystemBase();

    /*****************************************************************************
     * Method: initialize()
     *
     * @description
     *  Initialization method of FileSystem which is called upon flag isValid == true
     *******************************************************************************/
    virtual ErrorT initialize() = 0;

    /*****************************************************************************
     * Method: isValidFileSystem()
     *
     * @description
     *   Returns the state of the filesystem on the partition.
     *  If the superclass determines the filesystem to be corrupt
     *  this flag will be set to false, true otherwise.
     *******************************************************************************/
    bool isValidFileSystem() {
        return (this->isValid);
    }
};

#endif /* FILESYSTEMBASE_HH_ */
