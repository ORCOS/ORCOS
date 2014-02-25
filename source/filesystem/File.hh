/*
 * File.hh
 *
 *  Created on: 23.06.2013
 *      Author: dbaldin
 */

#ifndef FILE_HH_
#define FILE_HH_

#include "hal/CharacterDeviceDriver.hh"

class File: public CharacterDeviceDriver {
protected:
	unint4 filesize;
	unint4 flags;

public:
	File(char* name, unint4 size, unint4 flags);

	virtual ~File();

	unint4 getFileSize() { return (this->filesize);}

	unint4 getFlags() { return (this->flags); }
};

#endif /* FILE_HH_ */
