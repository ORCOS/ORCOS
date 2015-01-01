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

#ifndef RESOURCE_HH_
#define RESOURCE_HH_

#include "db/ListItem.hh"
#include "inc/types.hh"
#include "SCLConfig.hh"
#include "synchro/Mutex.hh"

class Kernel_ThreadCfdCl;

/*!
 * \brief Base class of all resources inside the kernel.
 *
 * \ingroup filesystem
 *
 * This class is the base class of all resources inside the kernel. Any resource
 * can be added to the filesystem of the system.
 */
class Resource: public ListItem {
protected:
    // making the name protected allows classes to change their name
    //! name identifying this resource
    const char* name;

    //! The access controlling mechanism
    Mutex* accessControl;

private:
    //!  A global Resource counter.
    static ResourceIdT globalResourceIdCounter;

    //! The type of resource
    ResourceType restype;

    //! My Id
    ResourceIdT myResourceId;

public:
    //! Constructor which takes the type of this resource and its name
    Resource(ResourceType rt, bool sync_res, const char* name = 0);

    //! Destructor. Resources may only be deleted by the idle thread.
    virtual ~Resource();

    /*****************************************************************************
     * Method: initialize()
     *
     * @description
     *  static initialization done at system boot
     *******************************************************************************/
    static void initialize() {
        globalResourceIdCounter = cFirstResource;
    }

    /*****************************************************************************
     * Method: acquire(Thread* pThread, bool blocking)
     *
     * @description
     *  Tries to acquire the resource for the given thread. If blocking
     *  and the resource is currently owned by another task and the resource
     *  is protected the thread is blocked.
     *
     * @params
     *  pThread     Thread that wants to acquire the resource
     *  blocking    Shall be block the thread if resource is owned and protected?
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    int acquire(Thread* pThread = 0, bool blocking = true);


    /*****************************************************************************
     * Method: release(Thread* pThread)
     *
     * @description
     *  Releases the resource. A resource can only be released if it has been acquired before by the same task.
     *
     * @params
     *  pThread     The thread that releases the resource
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT release(Thread* pThread = 0);


    /*****************************************************************************
     * Method: getType()
     *
     * @description
     *  Returns the type of this resource which helps to identify this resource
     *******************************************************************************/
    inline ResourceType getType() const {
        return (restype);
    }


    /*****************************************************************************
     * Method: getId()
     *
     * @description
     *  Returns global resource ID
     *******************************************************************************/
    inline ResourceIdT getId() const {
        return (this->myResourceId);
    }


    /*****************************************************************************
      * Method: getName()
      *
      * @description
      *  Returns the name of this resource
      *******************************************************************************/
    inline const char* getName() const {
        if (name) {
            return (name);
        } else {
            return ("{No Name}");
        }
    }
};

#endif /*RESOURCE_HH_*/
