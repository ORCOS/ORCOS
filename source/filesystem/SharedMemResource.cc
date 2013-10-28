/*
 * SharedMemResource.cc
 *
 *  Created on: 27.07.2013
 *      Author: dbaldin
 */

#include "SharedMemResource.hh"
#include "kernel/Kernel.hh"

extern Kernel* theOS;


SharedMemResource::SharedMemResource(unint4 size, char* name, Task* owner): Resource(cSharedMem,false,name) {

	this->owner = owner;
	TaskIdT id = 0;
	if (owner != 0)
		id = owner->getId();

	// get a physical area
	this->physical_start_address = (unint4) theOS->getRamManager()->alloc(size,id);

	this->size = size;

	// register this shared memory area
	theOS->getFileManager()->registerResource(this);

	if (owner != 0) {
		owner->aquiredResources.addTail(this);
	}

}

SharedMemResource::~SharedMemResource() {
	theOS->getRamManager()->free(this->physical_start_address,this->physical_start_address+size);
	theOS->getFileManager()->unregisterResource(this);

	LinkedListDatabaseItem* curItem = mapping.getHead();
	while ( curItem != 0 ) {
		Mapping* map = (Mapping*) curItem->getData();
		theOS->getHatLayer()->unmap(map->virtual_address,map->task->getId());
		curItem = curItem->getSucc();
		mapping.remove((DatabaseItem*) map);
	}

}

ErrorT SharedMemResource::mapIntoTask(Task* t, unint4& virtual_address) {

	if (t == 0) return (cInvalidArgument);
	// we must be sure the physical start address is virtual page aligned
	// otherwise we need some offset into the virtual page

	virtual_address = (unint4) theOS->getHatLayer()->map((void*) this->physical_start_address,size,7,3,t->getId(),false);

	Mapping* map = new Mapping();
	map->virtual_address = (void*) virtual_address;
	map->task = t;
	this->mapping.addTail((DatabaseItem*) map);

	return (cOk);
}

ErrorT SharedMemResource::unmapFromTask(Task* t) {

	LinkedListDatabaseItem* curItem = mapping.getHead();
	while ( curItem != 0 ) {
		Mapping* map = (Mapping*) curItem->getData();
		if (map->task == t) {
			// must be the currentRunning Task unmapping!
			theOS->getHatLayer()->unmap(map->virtual_address,t->getId());
			mapping.remove((DatabaseItem*) map);
			return (cOk);
		}
	    curItem = curItem->getSucc();
	}

	return (cError);
}
