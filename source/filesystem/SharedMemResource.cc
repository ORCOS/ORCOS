/*
 * SharedMemResource.cc
 *
 *  Created on: 27.07.2013
 *     Copyright & Author: dbaldin
 */

#include "SharedMemResource.hh"
#include "kernel/Kernel.hh"

extern Kernel* theOS;

SharedMemResource::SharedMemResource(const char* p_name, unint4 physical_address, unint4 size) : Resource(cSharedMem, false, p_name){
    this->owner = 0;
    this->size  = size;
    this->physical_start_address = physical_address;

    /* register this shared memory area */
    theOS->getFileManager()->registerResource(this);
}

SharedMemResource::SharedMemResource(unint4 u_size, const char* p_name, Task* p_owner) :
        Resource(cSharedMem, false, p_name) {
    this->owner = p_owner;
    TaskIdT id = 0;
    /* Check for task if this mapping is not for the kernel (p_owner == 0)*/
    if (p_owner != 0)
        id = p_owner->getId();

    if (u_size > 0x010000) {
        /* bigger than 64 KB.. cannot map this using one entry .. use a 1 MB page */
        u_size = (unint4) alignCeil((char*) u_size, 0x100000);
    } else if (size > 0x001000) {
        /* bigger than 4 KB.. cannot map this using one entry .. use a 64 KB page */
        u_size = (unint4) alignCeil((char*) u_size, 0x010000);
    } else {
        /* use a single 4 KB page */
        u_size = 0x001000;
    }

    /* get a free physical area */
    this->physical_start_address = (unint4) theOS->getRamManager()->alloc_physical(u_size, id);
    if (this->physical_start_address != 0) {
        this->size = u_size;

        /* register this shared memory area */
        theOS->getFileManager()->registerResource(this);

        if (p_owner != 0) {
            p_owner->aquiredResources.addTail(this);
        }
    } else {
        this->physical_start_address = -1;
        this->size = 0;
        LOG(MEM, ERROR, "SharedMemResource() could not create shared memory area. Error allocating memory!");
    }
}

SharedMemResource::~SharedMemResource() {
    /* check if this resource was valid!*/
    if (this->physical_start_address != (unint4) -1) {
        theOS->getRamManager()->free_physical(this->physical_start_address, this->physical_start_address + size);
        theOS->getFileManager()->unregisterResource(this);

#ifdef HAS_Board_HatLayerCfd
        LinkedListItem* curItem = mapping.getHead();
        while (curItem != 0) {
            Mapping* map = reinterpret_cast<Mapping*>(curItem->getData());
            theOS->getHatLayer()->unmap(reinterpret_cast<void*>(map->virtual_address), map->task->getId());
            curItem = curItem->getSucc();
            mapping.remove(reinterpret_cast<ListItem*>(map));
            /* delete the mapping */
            delete map;
        }
#endif
    }
}

/*****************************************************************************
 * Method: SharedMemResource::mapIntoTask(Task* t, unint4& virtual_address)
 *
 * @description
 * Maps the shared memory area into the address space of the given task
 *
 * @params
 *  t         The task the shared memory region shall be mapped into
 * @returns
 *  int                 Error Code
 *  virtual_address     The virtual address the region was mapped to.
 *                      If != 0 this is the address this memory region shall be mapped
 *                      to.
 *******************************************************************************/
ErrorT SharedMemResource::mapIntoTask(Task* t, unint4& virtual_address, unint4 offset, unint4& map_size, unint4 flags) {
    if (t == 0)
        return (cInvalidArgument);

    if (this->physical_start_address == (unint4) -1) {
        return (cError);
    }


    if ((offset & 0xfff) || (offset >= size) || (size-offset) < map_size) {
        return (cInvalidArgument);
    }

    /* we must be sure the physical start address is virtual page aligned
     otherwise we need some offset into the virtual page */
#ifdef HAS_Board_HatLayerCfd
    int cachemode = hatCacheWriteBack;
    if (flags & cCacheInhibit) {
        cachemode = hatCacheInhibit;
    }

    virtual_address = (unint4) theOS->getHatLayer()->map(reinterpret_cast<void*>(virtual_address),
                                                         reinterpret_cast<void*>(this->physical_start_address + offset),
                                                         map_size,
                                                         7,
                                                         3,
                                                         t->getId(),
                                                         cachemode);

    if (virtual_address == 0)
        return (cError);

    Mapping* map            = new Mapping(); /* free in line 64 and 136*/
    map->physical_address   = this->physical_start_address + offset;
    map->virtual_address    = virtual_address;
    map->task               = t;
    map->flags              = flags;
    map->size               = map_size;
    this->mapping.addTail(reinterpret_cast<ListItem*>(map));
#endif

    return (cOk);
}

/*****************************************************************************
 * Method: SharedMemResource::unmapFromTask(Task* t)
 *
 * @description
 *  Removes the mapped shared memory area from the address space of the given task
 *
 * @params
 *  t         The task the shared memory region shall be unmapped from
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT SharedMemResource::unmapFromTask(Task* t) {
#ifdef HAS_Board_HatLayerCfd
    LOG(MEM, INFO, "SharedMemResource::unmapFromTask() unmapping %s from task %u", this->getName(), t->getId());
    LinkedListItem* curItem = mapping.getHead();
    while (curItem != 0) {
        Mapping* map = reinterpret_cast<Mapping*>(curItem->getData());
        // get next item before link is destroyed in mapping.remove
        curItem = curItem->getSucc();
        if (map->task == t) {
            /* remove mapping */
            theOS->getHatLayer()->unmap(reinterpret_cast<void*>(map->virtual_address), t->getId());
            if (map->flags & cAllocate) {
                /* mapping has been allocated by a superclass .. free it*/
                theOS->getRamManager()->free_physical(map->physical_address, map->physical_address + map->size);
            }
            mapping.remove(reinterpret_cast<ListItem*>(map));
            /* delete the mapping */
            delete map;
        }
    }
#endif
    return (cOk);
}

