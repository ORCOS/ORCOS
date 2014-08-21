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

#ifndef SERVICEDISCOVERY_HH_
#define SERVICEDISCOVERY_HH_

#include "inc/types.hh"
#include "inc/const.hh"
#include "filesystem/Directory.hh"

/*!
 * \brief The ServiceDiscovery Base Class.
 * \ingroup comm
 *
 * Base class of all Service Discovery Classes.
 *
 */
class ServiceDiscovery {
protected:

    int2 id;
    Directory* commdevsdir;

public:
    ServiceDiscovery(int2 i_id) {
        this->id = i_id;
    }
    ;

    ~ServiceDiscovery() {
    }
    ;

    //! Returns the Id of this protocol
    int2 getId() {
        return (id);
    }
    ;

    /*!
     * \brief Lookup method which will try to lookup n services.
     *
     * \param name the name of the service
     * \param return_socks array of n sockaddr structures to be filled
     * \param n amount of replicas to be found
     *
     * \returns amount of replicas found
     */
    /*virtual unint1
     nlookup(const service_name name, servicedescriptor* return_socks, unint1 n) = 0;

     virtual bool addService(servicedescriptor* addr) = 0;*/

};

#endif /*SERVICEDISCOVERY_HH_*/
