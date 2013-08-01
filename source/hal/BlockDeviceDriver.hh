/*
 * BlockDeviceDriver.hh
 *
 *  Created on: 14.06.2013
 *      Author: dbaldin
 */

#ifndef BLOCKDEVICEDRIVER_HH_
#define BLOCKDEVICEDRIVER_HH_

#include "GenericDeviceDriver.hh"

class BlockDeviceDriver: public GenericDeviceDriver {
public:
	BlockDeviceDriver(char* name);

	virtual ~BlockDeviceDriver();

	// The physical size of a sector
	unint4 sector_size;

	/*!
	 * Tries to read "length" bytes from this device starting at block number "blockNum" into the
	 * buffer at address "buffer"
	 *
	 * returns cOk on success, Error number (<0) otherwise
	 */
	virtual ErrorT readBlock(unint4 blockNum, char* buffer, unint4 length) = 0;
};

#endif /* BLOCKDEVICEDRIVER_HH_ */
