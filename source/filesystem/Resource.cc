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
#include ThreadCfd_hh
#include <assemblerFunctions.hh>
#include "process/Task.hh"
#include "filesystem/File.hh"
#include "inc/newlib/newlib_helper.hh"
#include "kernel/Kernel.hh"

extern Kernel* theOS;
// static non-const member variable initialization
// will be executed in ctor
ResourceIdT Resource::globalResourceIdCounter;

// Global Variables to speed up access to these objects
extern ThreadCfdCl* pCurrentRunningThread;
extern Task* pCurrentRunningTask;

Resource::Resource( ResourceType rt, bool sync_res, const char* name ) {
    this->restype = rt;
    this->name = name;
    this->myResourceId = globalResourceIdCounter++;
    // if this is a resource that needs to be syncronized create Mutex
    if ( sync_res )
        this->accessControl = new ( DO_ALIGN ) Mutex( );
    else
    	this->accessControl = 0;
}

void Resource::aquire( Thread* pThread, bool blocking ) {
    ASSERT(pThread);

    int retval = this->myResourceId;
    void* sp_int = pThread->threadStack.stackptrs[0];

    // blocking call!
    if ( this->accessControl != 0 ) {
        if ( accessControl->acquire( this, blocking ) != cOk)
        	retval = cError;
    }
    else {
        // this resource is not synchronized. so just add it to the set of aquired resources
       int result = pThread->getOwner()->aquiredResources.addTail( this );
       // for files we also reset the position
       if (this->getType() == cFile) ((File*) this)->resetPosition();
       // forward status back to user
       if (result < 0) retval = result;
    }

    SET_RETURN_VALUE(sp_int,(void*) retval);

#if ENABLE_NESTED_INTERRUPTS
    // first to do is disable interrupts now since we are going to restore the context now
    _disableInterrupts();

    pCurrentRunningThread->executinginthandler = false;
#endif

    // if we got here we got the resource directly
    // so just return this syscall
    assembler::restoreContext(pCurrentRunningThread);

}

Resource::~Resource() {

	LOG(FILESYSTEM,TRACE,(FILESYSTEM,TRACE,"Deleting Resource %s.",this->name));
	// TODO: remove the resource from the thread aquired resources
	// check for these threads..
	// before deleteing this resource check if some threads are blocked waiting for this resource
	// if true delay deletion


	// name might be coming from the text or data area and can not be freed
	// try to free it any way. The mem manager will recognise this.
	if (this->name != 0) 		  delete this->name;
	if (this->accessControl != 0) delete this->accessControl;
}

ErrorT Resource::release( Thread* pThread ) {
    ASSERT(pThread);

    if ( this->accessControl != 0 )
        return accessControl->release();
    else {
        // remove myself from the database
        pThread->getOwner()->aquiredResources.removeItem( this );
        return cOk;
    }

}
