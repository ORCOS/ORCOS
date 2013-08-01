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

#include "Directory.hh"
#include "stringtools.hh"
#include "lib/defines.h"
#include "memtools.hh"
#include "filesystem/File.hh"
#include "kernel/Kernel.hh"

extern Kernel* theOS;

Directory::Directory( const char* name ) :
    CharacterDeviceDriver( cDirectory, false, name ) {
    num_entries = 0;
}

Directory::~Directory() {

	LinkedListDatabaseItem* litem = dir_content.getHead();
	Resource* res;

	while ( litem != 0 ) {
		// not found yet and we got anther entry in our database
		res = (Resource*) litem->getData();

		// we need to move to the successor before deleting the linked list container
		litem = litem->getSucc();
		dir_content.remove(res);

		delete res;
	}

}

ErrorT Directory::add( Resource* res ) {
    dir_content.addTail( res );
    num_entries++;
    return cOk;
}

ErrorT Directory::remove(Resource *res) {
	if (dir_content.remove(res) == cOk) {
		num_entries--;
		return cOk;
	}

	return cError;
}

Resource* Directory::get( const char* name ) {
    // parse the whole dir_content db for an item with name 'name'
    LinkedListDatabaseItem* litem = dir_content.getHead();
    Resource* res;

    while ( litem != 0 ) {
        // not found yet and we got anther entry in our database
        res = (Resource*) litem->getData();
        // compare names
        if ( strcmp( name, res->getName() ) == 0 )
            return res;
        litem = litem->getSucc();
    }

    return 0;
}

/*
 * Reads the contents description into the bytes array for up to length bytes
 *
 * Layout:
 *  Directory Type ID (1 byte) | dir name length (1 Byte)
 * | dir name (<256 bytes, zero terminated)| type (1 byte) | id (1byte)
 * | filesize  (4byte)| flags (4byte)
 *
 */
ErrorT Directory::readBytes( char *bytes, unint4 &length ) {

	if (length < 1) return cOk;
	// we are a base directory .. we only have name, type and id information
	bytes[0] = cDirTypeORCOS;

	unint4 pos = 1;

	LinkedListDatabaseItem* litem = this->dir_content.getHead();

	while (litem != 0) {
		Resource* ritem = (Resource*) litem->getData();
		const char* name = ritem->getName();
		int namelen = strlen(name);

		// stop if we do not fit into the bytes array any more
		if ((pos + namelen + 11) > length) break;

		// copy name into the bytes array
		bytes[pos] = namelen;
		memcpy((void*) &bytes[pos+1],(void*)name,namelen);

		bytes[pos+namelen+1] = 0;
		bytes[pos+namelen+2] = ritem->getType();
		bytes[pos+namelen+3] = ritem->getId();
		unint4 filesize = 0;
		unint4 flags 	= 0;

		if (ritem->getType() == cFile) {
			filesize = ((File*) ritem)->getFileSize();
			flags = ((File*) ritem)->getFlags();
		}
		bytes[pos+namelen+4] = (filesize & 0xff000000) >> 24;
		bytes[pos+namelen+5] = (filesize & 0xff0000) >> 16;
		bytes[pos+namelen+6] = (filesize & 0xff00) >> 8;
		bytes[pos+namelen+7] = (filesize & 0xff);

		bytes[pos+namelen+8] = (flags & 0xff000000) >> 24;
		bytes[pos+namelen+9] = (flags & 0xff0000) >> 16;
		bytes[pos+namelen+10] = (flags & 0xff00) >> 8;
		bytes[pos+namelen+11] = (flags & 0xff);


		pos += namelen+12;

		litem = litem->getSucc();
	}

    length = pos;
    return cOk;
}
