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

protected:
   /*!
	 * \brief list of block device ids
	 */
	static ArrayDatabase *freeBlockDeviceIDs;

public:
	BlockDeviceDriver(char* name);

	virtual ~BlockDeviceDriver();

	// The physical size of a sector
	unint4 sector_size;

	//! initialize
	 static void initialize() {
		 freeBlockDeviceIDs = new ArrayDatabase(20);
	     for (unint4 i = 0; i < 20; i++) {
		     freeBlockDeviceIDs->addTail((DatabaseItem*) i);
	     }
	 }

	/*!
	 * Tries to read "length" blocks from this device starting at block number "blockNum" into the
	 * buffer at address "buffer"
	 *
	 * returns cOk on success, Error number (<0) otherwise
	 */
	virtual ErrorT readBlock(unint4 blockNum, unint1* buffer, unint4 length) = 0;

	/*!
	 * Tries to write "length" blocks to the device starting at block number "blockNum".
	 *
	 * returns cOk on success, Error number (<0) otherwise.
	 */
	virtual ErrorT writeBlock(unint4 blockNum, unint1* buffer, unint4 length) = 0;
};

#endif /* BLOCKDEVICEDRIVER_HH_ */
