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
        this->accessControl = new Mutex(p_name);
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

    int retval = cError;

    if (this->accessControl != 0) {
        /* blocking call */
        if (accessControl->acquire(this, blocking) != cOk) {
            return (cError);
        }
    }

    ATOMIC_ADD(&refCounter, 1);

    /* myResourceId may be 0 if it has been deleted
     * during acquisition */
    if (myResourceId != 0) {
        if (pThread != 0 ) {
            /* Add resource to the set of acquired resources */
            retval = pThread->getOwner()->addResource(this);
            /* forward status back to user */
            if (isError(retval)) {
                /* error adding resource to task.. */
                ATOMIC_ADD(&refCounter, -1);
                return (retval);
            }

            /* success adding resource to task */
            /* for files we also reset the position */
            if (this->getType() & (cFile | cDirectory | cStreamDevice)) {
                CharacterDevice* cdev = static_cast<CharacterDevice*>(this);
                cdev->resetPosition();
            }
        }
    } else {
        ATOMIC_ADD(&refCounter, -1);
        retval = cResourceRemoved;
    }

    return (retval);
}

Resource::~Resource() {
    this->myResourceId  = 0;

    LOG(FILESYSTEM, TRACE, "Deleting Resource %s.", this->name);

    /* Remove the resource from all tasks that might hold a reference
     * to this resource so that they will not access the resource again.
     * Do this only for resources that are held by some task..  */
    if (refCounter > 0) {
        LinkedList* tasks = theOS->getTaskManager()->getTaskDatabase();
        for (LinkedListItem* litem = tasks->getHead(); litem != 0; litem = litem->getSucc()) {
            Task* t = (Task*) (litem->getData());
            t->removeResource(this);
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
    ErrorT ret = cOk;
    if (pThread == 0) {
          pThread = pCurrentRunningThread;
    }

    if (pThread != 0) {
        /* remove resource from the database */
        ret = pThread->getOwner()->removeResource(this);
        if (isError(ret)) {
            return (ret);
        } else {
            ATOMIC_ADD(&refCounter, -1);
        }
    }

    if (this->accessControl != 0) {
        accessControl->release((Kernel_ThreadCfdCl*)pThread);
    }

    return (ret);
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
