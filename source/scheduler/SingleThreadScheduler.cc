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

#include "scheduler/SingleThreadScheduler.hh"
#include "inc/const.hh"

SingleThreadScheduler::SingleThreadScheduler() {
}

SingleThreadScheduler::~SingleThreadScheduler() {
}

ErrorT SingleThreadScheduler::enter( LinkedListDatabaseItem* item ) {
    this->singleThread =  item;
    return cOk;
}

LinkedListDatabaseItem* SingleThreadScheduler::getNext() {
  LinkedListDatabaseItem* ret = this->singleThread;
  this->singleThread = 0;
  return ret;
}

LinkedListDatabaseItem* SingleThreadScheduler::remove(DatabaseItem* item) {

  if (this->singleThread->getData() == item) {
	  LinkedListDatabaseItem* ret = this->singleThread;
	  singleThread = 0;
	  return ret;
  }
  return 0;
}


int SingleThreadScheduler::getNextTimerEvent(LinkedListDatabase* sleepList,unint4 dt )
{
    // update sleeplist. Since this is a specialized class
    // we only update the first thread in the sleeplist since there should only be one!

    LinkedListDatabaseItem* pDBSleepItem = sleepList->getHead();
    if ( pDBSleepItem != 0 ) {
        Kernel_ThreadCfdCl* pSleepThread = static_cast< Kernel_ThreadCfdCl*> ( pDBSleepItem->getData() );

         pSleepThread->sleepCycles -= dt;
          if ( pSleepThread->sleepCycles <= 0 ) {
              pSleepThread->status.setBits( cReadyFlag );
              LinkedListDatabaseItem* litem2 = pDBSleepItem;
              pDBSleepItem = pDBSleepItem->getSucc();

              this->enter( litem2 );
          }
    }

   return MAX_INT4;
}




