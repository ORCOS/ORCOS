/*
 * File.cc
 *
 *  Created on: 23.06.2013
 *   Copyright & Author: dbaldin
 */

#include "File.hh"
#include "kernel/Kernel.hh"

extern Kernel* theOS;

File::File(char* p_name, unint4 size, unint4 u_flags) :
        CharacterDevice(cFile, false, p_name) {
    this->filesize  = size;
    this->flags     = u_flags;
    this->dateTime  = 0;
    if (theOS != 0 && theOS->getClock() != 0)
        this->dateTime  = theOS->getClock()->getDateTime();
}

File::File(char* p_name, unint4 size, unint4 u_flags, ResourceType type) :
        CharacterDevice((ResourceType) (cFile | type), false, p_name) {
    this->filesize  = size;
    this->flags     = u_flags;
    this->dateTime  = 0;
    if (theOS != 0 && theOS->getClock() != 0)
        this->dateTime  = theOS->getClock()->getDateTime();
}

File::~File() {
}

/*****************************************************************************
 * Method: File::rename(char* newName)
 *
 * @description
 *  Renames the file.
 *******************************************************************************/
ErrorT File::rename(char* newName) {
    if (newName != 0) {
        /* try freeing the old name */
        delete this->name;
        this->name = newName;
        return (cOk);
    }
    return (cNullPointerProvided);
}
