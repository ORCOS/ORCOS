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

#ifndef TRAIN_HH_
#define TRAIN_HH_

#include "ETHLI100.hh"

class Train {
private:
	unint1 address;

	unint1 speed;

	unint1 direction;
	// The block the train is currently standing on
	// needs to be updated by the user
	unint1 position;

	int* routeArray;

	int routeLength;

	Switch switchRouteArray[20][5];

	int isRoutePeriodic;

	int myArrayPosition;

	TrainControlUnit* tcu;


public:
	Train(TrainControlUnit* tcu, unint1 address);

	unint1 getAddress() { return address; };

	void setSpeed(unint1 speed);

	unint1 getSpeed();

	void setDirection(unint1 direction);

	unint1 getDirection();

	void stop();

	void enableFunction(unint4 function);

	void disableFunction(unint4 function);

	unint1 getPosition();

	unint1 getNextPosition();

	void setPosition(unint1 new_position);

	void setRouteArray(int* route, Switch switchRouteArray[][5], int length,int isPeriodic );

	void setSwitchesForNextBlock();

};



#endif
