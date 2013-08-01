/*
 * File.cc
 *
 *  Created on: 23.06.2013
 *      Author: dbaldin
 */

#include "File.hh"

File::File(char* name, unint4 size, unint4 flags) : CharacterDeviceDriver(cFile,true,name) {
	this->filesize = size;
	this->flags = flags;

}

File::~File() {

}

