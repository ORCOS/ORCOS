/*
 * FileSystemBase.cc
 *
 *  Created on: 19.06.2013
 *      Author: dbaldin
 */

#include "FileSystemBase.hh"

FileSystemBase::FileSystemBase(Partition* myPartition)
{
	this->myPartition = myPartition;
	// we are invalid by default
	// only if superclass knows for sure this is a valid file system
	// this may be set to true
	this->isValid = false;

}

FileSystemBase::~FileSystemBase() {

}

