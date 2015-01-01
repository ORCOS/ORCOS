/*
 * File.hh
 *
 *  Created on: 23.06.2013
 *     Copyright & Author: dbaldin
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

    /*****************************************************************************
     * Method: getFileSize()
     *
     * @description
     *  Returns the size of the file
     *******************************************************************************/
    unint4 getFileSize() const {
        return (this->filesize);
    }

    /*****************************************************************************
     * Method: getFlags()
     *
     * @description
     *  Returns the file flags
     *******************************************************************************/
    unint4 getFlags() const {
        return (this->flags);
    }

    /*****************************************************************************
     * Method: getDateTime()
     *
     * @description
     *  Returns the datetime of the file
     *******************************************************************************/
    unint4 getDateTime() const {
        return (this->dateTime);
    }

    /*****************************************************************************
     * Method: setDateTime(unint4 dateTime)
     *
     * @description
     *  Sets the current DateTime of the file
     *******************************************************************************/
    virtual ErrorT setDateTime(unint4 dateTime) {
        this->dateTime = dateTime;
        return (cOk);
    }

    /*****************************************************************************
    * Method: onClose()
    *
    * @description
    *  Callback event, called when the file is closed. Allows
    *  specialisations to e.g., flush internal cashed on close to permanent
    *  storage.
    *******************************************************************************/
    virtual ErrorT onClose() {
        return (cOk);
    }

    /*****************************************************************************
     * Method: rename(char* newName)
     *
     * @description
     *  Renames the file.
     *******************************************************************************/
    virtual ErrorT rename(char* newName);
};

#endif /* FILE_HH_ */
