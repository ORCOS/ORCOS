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
    Partition* myPartition;

    bool isValid;

    unint4 numBlocks;

    unint4 blockSize;

    unint4 freeBlocks;

    Directory* sysFsDir;

public:
    /*****************************************************************************
     * Method: FileSystemBase(Partition* myPartition)
     *
     * @description
     *******************************************************************************/
    explicit FileSystemBase(Partition* myPartition);

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
