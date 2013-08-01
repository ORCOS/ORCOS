/*
 * SharedMemResource.cc
 *
 *  Created on: 27.07.2013
 *      Author: dbaldin
 */

#include "SharedMemResource.hh"
#include "kernel/Kernel.hh"

extern Kernel* theOS;


SharedMemResource::SharedMemResource(unint4 size, char* name, TaskIdT owner): Resource(cSharedMem,false,name) {

	this->owner = owner;

	// get a physical area
	this->physical_start_address = (unint4) theOS->getRamManager()->alloc(size,-1);

	this->size = size;

	// register this shared memory area
	theOS->getFileManager()->registerResource(this);

}

SharedMemResource::~SharedMemResource() {
	// TODO Auto-generated destructor stub


}

ErrorT SharedMemResource::mapIntoTask(Task* t, unint4& virtual_address) {

	// we must be sure the physical start address is virtual page aligned
	// otherwise we need some offset into the virtual page

	virtual_address = (unint4) theOS->getHatLayer()->map((void*) this->physical_start_address,size,7,3,t->getId(),false);

	// TODO: store address for unmap operation

	return cOk;
}

ErrorT SharedMemResource::unmapFromTask(Task* t) {

// TODO

}
