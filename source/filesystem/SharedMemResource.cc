/*
 * SharedMemResource.cc
 *
 *  Created on: 27.07.2013
 *      Author: dbaldin
 */

#include "SharedMemResource.hh"
#include "kernel/Kernel.hh"

extern Kernel* theOS;

SharedMemResource::SharedMemResource(unint4 u_size, const char* p_name, Task* p_owner) :
        Resource(cSharedMem, false, p_name) {

    this->owner = p_owner;
    TaskIdT id  = 0;
    /* Check for task is this mapping is not for the kernel (p_owner == 0)*/
    if (p_owner != 0)
        id = p_owner->getId();

    /* get a free physical area */
    this->physical_start_address = (unint4) theOS->getRamManager()->alloc(u_size, id);
    if (this->physical_start_address != 0)
    {
        this->size = u_size;

        /* register this shared memory area */
        theOS->getFileManager()->registerResource(this);

        if (p_owner != 0)
        {
            p_owner->aquiredResources.addTail(this);
        }
    }
    else
    {
        this->size = 0;
    }

}

SharedMemResource::~SharedMemResource() {
    /* check if this resource was valid!*/
    if (this->physical_start_address != 0)
    {
        theOS->getRamManager()->free(this->physical_start_address,
                                     this->physical_start_address + size);
        theOS->getFileManager()->unregisterResource(this);

#ifdef HAS_Board_HatLayerCfd
        LinkedListItem* curItem = mapping.getHead();
        while (curItem != 0)
        {
            Mapping* map = (Mapping*) curItem->getData();
            theOS->getHatLayer()->unmap(map->virtual_address, map->task->getId());
            curItem = curItem->getSucc();
            mapping.remove((ListItem*) map);
            /* delete the mapping */
            delete map;
        }
#endif
    }

}

ErrorT SharedMemResource::mapIntoTask(Task* t, unint4& virtual_address) {

    if (t == 0)
        return (cInvalidArgument );
    // we must be sure the physical start address is virtual page aligned
    // otherwise we need some offset into the virtual page
#ifdef HAS_Board_HatLayerCfd
    // TODO: protect map access from concurrent access?
    virtual_address = (unint4) theOS->getHatLayer()->map((void*) this->physical_start_address, size, 7, 3, t->getId(), false);

    if (virtual_address == 0)
        return (cError );

    Mapping* map = new Mapping(); /* free in line 60*/
    map->virtual_address = (void*) virtual_address;
    map->task = t;
    this->mapping.addTail((ListItem*) map);
#endif

    return (cOk );
}

ErrorT SharedMemResource::unmapFromTask(Task* t) {
#ifdef HAS_Board_HatLayerCfd
    LinkedListItem* curItem = mapping.getHead();
    while (curItem != 0)
    {
        Mapping* map = (Mapping*) curItem->getData();
        if (map->task == t)
        {
            /* must be the currentRunning Task unmapping! */
            theOS->getHatLayer()->unmap(map->virtual_address, t->getId());
            mapping.remove((ListItem*) map);
            /* delete the mapping */
            delete map;
            return (cOk );
        }
        curItem = curItem->getSucc();
    }
#endif
    return (cError );
}

