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

    //! initialize
    static void initialize() {
        globalResourceIdCounter = cFirstResource;
    }

    /*!
     * \brief Acquire this resource
     *
     * The thread given as parameter tries to aquire this resource. If this
     * resource is not available the thread will be blocked until it becomes
     * available again.
     */
    int acquire(Thread* pThread, bool blocking = true);

    /*!
     * \brief Release the resource.
     *
     * A resource can only be released if it has been acquired before by the same task.
     */
    ErrorT release(Thread* pThread);

    //! Returns the type of this resource which helps to identify this resource
    inline ResourceType getType() {
        return (restype);
    }

    //! Returns the id
    inline ResourceIdT getId() {
        return (this->myResourceId);
    }

    //! Returns the name of this resource
    inline const char* getName() {
        if (name)
            return (name);
        else return ("{No Name}");
    }
};

#endif /*RESOURCE_HH_*/
