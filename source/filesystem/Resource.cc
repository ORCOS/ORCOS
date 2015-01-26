/*
 ORCOS - an Organic Reconfigurable Operating System
 Copyright (C) 2008 University of Paderborn

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Resource.hh"
#include Kernel_Thread_hh
#include <assemblerFunctions.hh>
#include "process/Task.hh"
#include "filesystem/File.hh"
#include "kernel/Kernel.hh"

extern Kernel* theOS;

/* static non-const member variable initialization
 will be executed in ctor*/
ResourceIdT Resource::globalResourceIdCounter;

Resource::Resource(ResourceType rt, bool sync_res, const char* p_name) {
    this->restype    = rt;
    this->name       = p_name;
    this->refCounter = 0;

    /* TODO: take care of integer overflows here..
     * need to guarantee the same id is not reused*/
    this->myResourceId = globalResourceIdCounter++;
    if (globalResourceIdCounter == 0) {
        ERROR("Out of Resource IDs!");
    }
    /* if this is a resource that needs to be synchronized create Mutex */
    if (sync_res) {
        this->accessControl = new Mutex();
    } else {
        this->accessControl = 0;
    }
}

/*****************************************************************************
 * Method: Resource::acquire(Thread* pThread, bool blocking)
 *
 * @description
 *  Tries to acquire the resource for the given thread. If blocking
 *  and the resource is currently owned by another task and the resource
 *  is protected the thread is blocked.
 *
 * @params
 *  pThread     Thread that wants to acquire the resource
 * blocking     Shall be block the thread if resource is owned and protected?
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
int Resource::acquire(Thread* pThread, bool blocking) {
   if (pThread == 0) {
       pThread = pCurrentRunningThread;
   }

    int retval = myResourceId;

    DISABLE_IRQS(status);
    refCounter++;
    RESTORE_IRQS(status);

    if (this->accessControl != 0) {
        /* blocking call */
        if (accessControl->acquire(this, blocking) != cOk) {
            return (cError);
        }
    }

    /* myResourceId may be 0 if it has been deleted
     * during acquisition */
    DISABLE_IRQS(status2);
    if (myResourceId != 0) {
        if (pThread != 0 ) {
            /* Add resource to the set of acquired resources */
            int result = pThread->getOwner()->aquiredResources.addTail(this);
            /* forward status back to user */
            if (result < 0)
                retval = result;

            /* for files we also reset the position */
            if (this->getType() & (cFile | cDirectory)) {
                CharacterDevice* cdev = static_cast<CharacterDevice*>(this);
                cdev->resetPosition();
            }
        }
    } else {
        refCounter--;
        retval = cResourceRemoved;
    }
    RESTORE_IRQS(status2);

    return (retval);
}

Resource::~Resource() {
    DISABLE_IRQS(status);
    this->myResourceId  = 0;
    RESTORE_IRQS(status);

    LOG(FILESYSTEM, TRACE, "Deleting Resource %s.", this->name);

    /* Remove the resource from all tasks that might hold a reference
     * to this resource so that they will not access the resource again.
     * Do this only for resources that are held by some task..  */
    if (refCounter > 0) {
        LinkedList* tasks = theOS->getTaskManager()->getTaskDatabase();
        for (LinkedListItem* litem = tasks->getHead(); litem != 0; litem = litem->getSucc()) {
            Task* t = (Task*) (litem->getData());
            DISABLE_IRQS(status);
            t->aquiredResources.removeItem(this);
            RESTORE_IRQS(status);
        }
    }

    /* name might be coming from the text or data area and can not be freed
     try to free it any way. The mem manager will recognize this. */
    if (this->name != 0)
        delete this->name;

    /* for access to this member after delete */
    this->name          = "$Removed";
    this->accessControl = 0;
}

/*****************************************************************************
 * Method: Resource::release(Thread* pThread)
 *
 * @description
 *  Releases the resource.
 *
 * @params
 *  pThread     The thread that releases the resource
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT Resource::release(Thread* pThread) {
    if (pThread == 0) {
          pThread = pCurrentRunningThread;
    }

    if (this->accessControl != 0) {
        accessControl->release(pThread);
    }

    if (pThread != 0) {
        /* remove resource from the database */
        DISABLE_IRQS(status);
        ListItem* removedItem = pThread->getOwner()->aquiredResources.removeItem(this);
        if (removedItem == 0) {
            RESTORE_IRQS(status);
            return (cElementNotInDatabase);
        } else {
            refCounter--;
        }
        RESTORE_IRQS(status);
    }
    return (cOk );
}

#if 0
/*****************************************************************************
  * Method: void Resource::operator delete(void* addr)
  *
  * @description
  *  Special delete operator for Resources taking care
  *  of delayed resource deletion of this resource
  *  if it is still accessed by threads trying to acquire it.
  *******************************************************************************/
void Resource::operator delete(void* addr) {
    Kernel_MemoryManagerCfdCl* mm;
    mm = theOS->getMemoryManager();
    Resource* res = (Resource*)(addr);
    /* Check if some thread is still waiting for it. If the reference counter is <= 0
     * then this resource is not used or waited for any where..
     * its safe to free it directly. */
    if (res->refCounter <= 0) {
        mm->free(addr);
        mm->free(res->accessControl);
    }

    /* We need to delay resource memory location freeing until all threads
     * returned. Add to memory manager delayed resource deletion list. */
    mm->scheduleDeletion(res);
}

#endif
