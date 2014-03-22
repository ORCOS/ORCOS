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
#include "inc/newlib/newlib_helper.hh"
#include "kernel/Kernel.hh"

extern Kernel* theOS;
// static non-const member variable initialization
// will be executed in ctor
ResourceIdT Resource::globalResourceIdCounter;

// Global Variables to speed up access to these objects
extern Kernel_ThreadCfdCl* pCurrentRunningThread;
extern Task* pCurrentRunningTask;

Resource::Resource( ResourceType rt, bool sync_res, const char* p_name ) {
    this->restype = rt;
    this->name = p_name;
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

    // blocking call!
    if ( this->accessControl != 0 ) {
        if ( accessControl->acquire( this, blocking ) != cOk)
        	retval = cError;
    }
    else {

		// this resource is not synchronized. so just add it to the set of aquired resources
		int result = pThread->getOwner()->aquiredResources.addTail( this );
		// forward status back to user
		if (result < 0) retval = result;

       // for files we also reset the position
       if (this->getType() & cFile) ((File*) this)->resetPosition();
    }

    // TODO: evaluate oif removing this "directy" return is ok
    // so we can reuse this code for aquire calls not coming from syscalls
    void* sp_int;
    GET_RETURN_CONTEXT(pCurrentRunningThread,sp_int);
    SET_RETURN_VALUE(sp_int,retval);

    DISABLE_IRQS(status);

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
        return (accessControl->release());
    else {
        // remove myself from the database
        pThread->getOwner()->aquiredResources.removeItem( this );
        return (cOk);
    }

}
