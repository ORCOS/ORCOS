/*
 * File.cc
 *
 *  Created on: 23.06.2013
 *      Author: dbaldin
 */

#include "File.hh"

File::File(char* p_name, unint4 size, unint4 u_flags) : CharacterDeviceDriver(cFile,true,p_name) {
	this->filesize = size;
	this->flags = u_flags;

}

File::~File() {

}

