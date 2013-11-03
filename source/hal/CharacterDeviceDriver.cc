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

#include "CharacterDeviceDriver.hh"
#include <kernel/Kernel.hh>

extern Kernel* theOS;

CharacterDeviceDriver::CharacterDeviceDriver( bool sync_res, const char* p_name ) :
    GenericDeviceDriver( cStreamDevice, sync_res, p_name ) {

	this->position = 0;
	// register myself at the filesystem manager
	SimpleFileManager* fm = theOS->getFileManager();

	// be sure we have a filesystem. if not we can not continue since every driver needs to register!!
	ASSERT(fm);

	if (fm != 0) fm->registerResource( this );

}

CharacterDeviceDriver::CharacterDeviceDriver( ResourceType rt, bool sync_res, const char* p_name ) :
    GenericDeviceDriver( rt, sync_res, p_name ) {

	this->position = 0;

	// we are not registering for Directories ..
	// directories need to register themself at the apropriate directory
	if (rt != cDirectory && rt != cFile)
	{
		// register myself at the filesystem manager
		SimpleFileManager* fm = theOS->getFileManager();

		// be sure we have a filesystem. if not we can not continue since every driver needs to register!!
		ASSERT(fm);

		if (fm != 0) fm->registerResource( this );
	}

}

CharacterDeviceDriver::~CharacterDeviceDriver() {

}
