/*
 * Ramdisk.cc
 *
 *  Created on: 22.02.2014
 *      Author: dbaldin
 */

#include "Ramdisk.hh"
#include "inc/types.hh"
#include "kernel/Kernel.hh"
#include "inc/Alignment.hh"

extern Kernel* theOS;


#define BLOCK_SIZE 4096



Ramdisk::Ramdisk(T_Ramdisk_Init* init) {

	unint4 start 	= init->StartAddress;
	unint4 end 		= init->StartAddress + init->Size-1;



	blockChain 				= (unint4*) start;

	unint4 numblocks  		= ((end - start) / BLOCK_SIZE);
	unint4 blockChainSize 	= (sizeof(unint4*)) * numblocks;
	numblocks 				-= (blockChainSize / BLOCK_SIZE);
	blockChainSize 			= (sizeof(unint4*)) * numblocks;

	blockChainEntries 		= numblocks;

	firstBlock 	  			= start + (numblocks * sizeof(unint4*));
	firstBlock 				= (unint4) alignCeil((char*) firstBlock,BLOCK_SIZE);

	if (theOS->getRamManager() != 0) {
		/* Mark the ram disk area as used so it can not be used to allocate tasks a.s.o*/
		theOS->getRamManager()->markAsUsed(start,end,0);
	}

	for (int i = 0; i < blockChainEntries; i++)
		blockChain[i] = -1; // mark as free

	// create the ramdisk mount directory
	RamdiskDirectory* dir = new RamdiskDirectory(this,"ramdisk");
	theOS->getFileManager()->getDirectory("/mnt")->add(dir);

}

unint4 Ramdisk::allocateBlock(unint4 prev) {
	for (int i = 0; i < blockChainEntries; i++)
		if (blockChain[i] == -1) {  // -1 == free
			blockChain[i] = 0;  	// 0 == End of Chain
			if (prev != -1)
				blockChain[prev] = i;

			return (i);
		}
	return (-1);
}

int Ramdisk::freeBlock(unint4 blockNum) {

	if (blockNum != -1) {
		unint4 nextBlock = blockChain[blockNum];
		blockChain[blockNum] = -1;

		while (nextBlock != 0 && nextBlock != -1) {
			blockNum = blockChain[nextBlock];
			blockChain[nextBlock] = -1;
			nextBlock = blockNum;
		}

	}

	return (cOk);
}

unint4 Ramdisk::getNextBlock(unint4 currentBlock, bool allocate) {

	if (currentBlock != -1) {
		unint4 nextBlock = blockChain[currentBlock];

		if ((nextBlock == 0 || nextBlock == -1) && allocate) {
				return (allocateBlock(currentBlock));
		} else	return (nextBlock);

	}
	else return (-1);

}

Ramdisk::~Ramdisk() {

}

ErrorT RamdiskDirectory::remove(Resource* res) {
	int error = Directory::remove(res);
	if (isError(error)) return (error);

	RamdiskFile* f = (RamdiskFile*) res;
	myRamDisk->freeBlock(f->myBlockNumber);

	return (cOk);
}

File* RamdiskDirectory::createFile(char* name, unint4 flags) {

	// search a free entry
	unint4 block = this->myRamDisk->allocateBlock(-1);
	if (block == -1) return (0);

	RamdiskFile* f = new RamdiskFile(this->myRamDisk, block,name,flags);
	this->add(f);
	return (f);
}

ErrorT RamdiskFile::readBytes(char* bytes, unint4& length) {
	LOG(ARCH,DEBUG,(ARCH,DEBUG,"FATFile::readBytes length: %d",length));
		unint4 sector_pos;
		unint4 sector_read_len;
		unint4 pos = 0;

		unint4 readlength 	= length;
		// be sure we are not reading over the end of the file
		if ((this->filesize - this->position) < readlength) readlength =  (this->filesize - this->position);

		char* buffer;
		bool sector_changed = false;

		while (readlength > 0) {

			// position inside our current sector
			//sector_pos 		= this->position % sector_size;
			// sector size must be a multiple of 2
			sector_pos 		= this->position & ( BLOCK_SIZE - 1);

			// set readlength to remaining bytes in this sector
			sector_read_len = BLOCK_SIZE - sector_pos;

			// check if we read less than the remaining bytes in this sector
			if (readlength < sector_read_len) sector_read_len = readlength;
			else {
				sector_changed = true;
			}

			buffer = (char*) ((currentBlock * BLOCK_SIZE)+ myRamDisk->getFirstBlockAddress());
			// copy the desired bytes in this sector into the buffer
			memcpy(&bytes[pos],&buffer[sector_pos],sector_read_len);

			// increase position by bytes read
			this->position += sector_read_len;
			pos += sector_read_len;

			// check if we reached the sector boundary
			if (sector_changed) {
				currentBlock = myRamDisk->getNextBlock(currentBlock,false);
				if (currentBlock == -1) {
					// end of file reached
					length = pos;
					return (cOk);
				}
			}

			sector_changed = false;
			// decrease total amount to read by read amount of bytes
			readlength-= sector_read_len;

		}


		length = pos;
		return (cOk);

}

ErrorT RamdiskFile::writeBytes(const char* bytes, unint4 length) {


	unint4 sector_pos;
	unint4 sector_write_len;
	bool 	sector_changed = false;
	unint4 pos = 0;	// position inside the bytes array
	bool new_sector = false;
	int error;

	char* buffer;

	// keep writing while we have bytes to write
	while (length > 0) {
		// check if the next write operation is overwriting something or appending
		sector_pos 			= this->position & ( BLOCK_SIZE - 1);  // position inside the sector
		sector_write_len 	= BLOCK_SIZE - sector_pos;				// the length we are writing inside this sector

		// check if we read less than the remaining bytes in this sector
		if (length < sector_write_len) sector_write_len = length;
		else {
			sector_changed = true;
		}

		buffer = (char*) ((currentBlock * BLOCK_SIZE)+ myRamDisk->getFirstBlockAddress());

		// copy the desired bytes in this sector into the buffer
		memcpy(&buffer[sector_pos],&bytes[pos],sector_write_len);

		// increase position by bytes read
		this->position += sector_write_len;
		pos += sector_write_len;

		// check if we reached the sector boundary
		if (sector_changed) {
			// append if neccessary
			currentBlock = myRamDisk->getNextBlock(currentBlock,true);
			if (currentBlock == -1) {
				// no more memory..
				return (cDeviceMemoryExhausted);
			}
			// if we reach a new sector and the position > filesize
			// we can omit reading the contents of that sector
			new_sector = position > filesize;
		}

		sector_changed = false;
		// decrease total amount to read by read amount of bytes
		length-= sector_write_len;

	}

	if (this->position > this->filesize) {
		this->filesize = this->position;
	}

	return (cOk);
}

RamdiskFile::RamdiskFile(Ramdisk* myRamdisk, unint4 blockNum, char* name, int flags) : File(name,0,flags) {
	 this->myRamDisk = myRamdisk;
	 this->myBlockNumber = blockNum;
	 currentBlock=myBlockNumber;
}

ErrorT RamdiskFile::resetPosition() {
	File::resetPosition();
	currentBlock = myBlockNumber;
	return (cOk);
}
