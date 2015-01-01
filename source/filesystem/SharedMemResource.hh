/*
 * SharedMemResource.hh
 *
 *  Created on: 27.07.2013
 *      Copyright & Author: dbaldin
 */

#ifndef SHAREDMEMRESOURCE_HH_
#define SHAREDMEMRESOURCE_HH_

#include "Resource.hh"

typedef struct {
    intptr_t virtual_address;
    Task* task;
}__attribute__((aligned(4))) Mapping;

/*
 * Shared Memory Area Resource. Allows the creation of shared memory areas at
 * arbitrary physical memory locations. This may even be used to create
 * overlays for tasks and IO which on the one hand might be a security risk.
 */
class SharedMemResource: public Resource {
private:
    /* The physical start address of this memory area */
    unint4 physical_start_address;

    /* The size of this memory area */
    unint4 size;

    /* The owner (creator) of this resource */
    Task* owner;

    /* List of mappings */
    LinkedList mapping;

public:
    /*****************************************************************************
     * Method: SharedMemResource(unint4 size, const char* name, Task* owner = 0)
     *
     * @description
     *  Constructor which does not take an explicit phyiscal start address. It
     *  tries to allocate a memory area of the given size to be used as a new
     *  shared memory area.
     *
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    SharedMemResource(unint4 size, const char* name, Task* owner = 0);

    virtual ~SharedMemResource();

    /*****************************************************************************
     * Method: mapIntoTask(Task* t, unint4& virtual_address)
     *
     * @description
     * Tries to map this shared resource into the task t.
     * Maps the shared memory area into the address space of the given task
     *
     * @params
     *  t         The task the shared memory region shall be mapped into
     * @returns
     *  int                 Error Code
     *  virtual_address     The virtual address the region was mapped to
     *******************************************************************************/
    ErrorT mapIntoTask(Task* t, unint4& virtual_address);

    /*****************************************************************************
     * Method: unmapFromTask(Task* t)
     *
     * @description
     *  Removes the mapped shared memory area from the address space of the given task
     *
     * @params
     *  t         The task the shared memory region shall be unmapped from
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT unmapFromTask(Task* t);

    /*****************************************************************************
     * Method: getOwner()
     *
     * @description
     *   Returns the Owner Task ID that initially creatd the shared memory region
     * @returns
     *  Task*         Owning task
     *******************************************************************************/
    inline Task* getOwner() {
        return (owner);
    }

    /*****************************************************************************
     * Method: getSize()
     *
     * @description
     *  Returns the size of the shared memory area.
     * @returns
     *  unint4         Size of the region
     *******************************************************************************/
    inline unint4 getSize() {
        return (size);
    }

    /*****************************************************************************
     * Method: getMappedCount()
     *
     * @description
     *  Returns the number of tasks this shared memory area is mapped to.
     *******************************************************************************/
    inline unint4 getMappedCount() {
        return (mapping.getSize());
    }

    /*****************************************************************************
     * Method: getPhysicalStartAddress()
     *
     * @description
     *  Returns the physical start address of the shared memory area.
     *******************************************************************************/

    inline unint4 getPhysicalStartAddress() {
        return (physical_start_address);
    }
};

#endif /* SHAREDMEMRESOURCE_HH_ */
