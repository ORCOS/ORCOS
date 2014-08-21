/*
 * SharedMemResource.hh
 *
 *  Created on: 27.07.2013
 *      Author: dbaldin
 */

#ifndef SHAREDMEMRESOURCE_HH_
#define SHAREDMEMRESOURCE_HH_

#include "Resource.hh"

typedef struct {
    void* virtual_address;
    Task* task;
}__attribute__((aligned(4))) Mapping;

/*
 * Shared Memory Area Resource. Allows the creation of shared memory areas at
 * arbitrary phyiscal memory locations. This may even be used to create
 * overlays for tasks, which on the one hand might be a security risk.
 */
class SharedMemResource: public Resource {

private:
    // The physical start address of this memory area
    unint4 physical_start_address;
    // The size of this memory area
    unint4 size;
    // The owner (creator) of this resource
    Task* owner;

    LinkedList mapping;

public:
    SharedMemResource(unint4 size, const char* name, Task* owner = 0);

    virtual ~SharedMemResource();

    /*!
     * Tries to map this shared resource into the task t.
     * On success virtual address contains the virtual address this area is mapped to.
     */
    ErrorT mapIntoTask(Task* t, unint4& virtual_address);

    /*!
     * Removes the shared memory mapping from a task.
     */
    ErrorT unmapFromTask(Task* t);

    /*!
     * Returns the Owner Task ID.
     */
    inline Task* getOwner() {
        return (owner);
    }

    /*!
     * Returns the size of the shared memory area.
     */
    inline unint4 getSize() {
        return (size);
    }

    /*!
     * Returns the number of tasks this shared memory area is mapped to.
     */
    inline unint4 getMappedCount() {
        return (mapping.getSize());
    }

    /*!
     * Returns the physical start address of the shared memory area.
     */
    inline unint4 getPhysicalStartAddress() {
        return (physical_start_address);
    }
};

#endif /* SHAREDMEMRESOURCE_HH_ */
