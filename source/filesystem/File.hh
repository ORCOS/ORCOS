/*
 * File.hh
 *
 *  Created on: 23.06.2013
 *      Author: dbaldin
 */

#ifndef FILE_HH_
#define FILE_HH_

#include "hal/CharacterDevice.hh"

class File: public CharacterDevice {
protected:

    /*File Size */
    unint4 filesize;
    /* File type specific flags */
    unint4 flags;
    /* Date Time of file */
    unint4 dateTime;

public:
    File(char* name, unint4 size, unint4 flags);

    File(char* name, unint4 size, unint4 flags, ResourceType type);

    virtual ~File();

    unint4 getFileSize() {
        return (this->filesize);
    }

    unint4 getFlags() {
        return (this->flags);
    }

    unint4 getDateTime() {
        return (this->dateTime);
    }

    /*
     *Sets the current DateTime of the file
     */
    virtual ErrorT setDateTime(unint4 dateTime) {
        this->dateTime = dateTime;
        return (cOk);
    }

    /*
     * Allows renaming a file.
     */
    virtual ErrorT rename(char* newName);
};

#endif /* FILE_HH_ */
