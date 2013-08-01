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

#ifndef LOCKEDQUEUE_HH
#define LOCKEDQUEUE_HH

#include "shm.hh"




class LockedQueue {

private:

	LockedQueueControl* lqControls;
	Descriptor* descriptors;

	void lock(LockedQueueControl* lqControl);
	void unlock(LockedQueueControl* lqControl);

public:
	LockedQueue();
	LockedQueue(void* lqBaseAddress, int descAddress);
	~LockedQueue();

	void init(int nodeNr);
	void initDescriptors(int maxDescriptors, char* data);
	void add (int nodeNr, Descriptor* desc);
	Descriptor* getDescriptor (int NodeNr);
};

#endif /* LOCKEDQUEUE_HH */
