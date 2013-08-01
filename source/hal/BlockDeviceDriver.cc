/*
 * BlockDeviceDriver.cc
 *
 *  Created on: 14.06.2013
 *      Author: dbaldin
 */

#include "BlockDeviceDriver.hh"
#include <kernel/Kernel.hh>

extern Kernel* theOS;

BlockDeviceDriver::BlockDeviceDriver(char* name) :  GenericDeviceDriver(cBlockDevice, true, name) {

	SimpleFileManager* fm = theOS->getFileManager();

	// be sure we have a filesystem. if not we can not continue since every driver needs to register!!
	ASSERT(fm);

	if (fm != 0) fm->registerResource( this );

	// conservative default
	this->sector_size = 512;
}

BlockDeviceDriver::~BlockDeviceDriver() {
	// TODO Auto-generated destructor stub
}

