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

#include "Train.hh"
#include <orcos.hh>

/*---------------------------------------------------------------------------*/
Train::Train(TrainControlUnit* tcu, unint1 address){
/*---------------------------------------------------------------------------*/
	this->speed = 0;
	this->tcu = tcu;
	this->address = address;
	this->direction = 1;
}

/*---------------------------------------------------------------------------*/
void Train::setSpeed(unint1 speed){
/*---------------------------------------------------------------------------*/
	this->speed = speed;
	tcu->setTrainSpeed(this->address, this->speed, this->direction);
}

/*---------------------------------------------------------------------------*/
void Train::stop(){
/*---------------------------------------------------------------------------*/
	this->speed = 0;
	printf("Stopping Train!");
	tcu->stopTrain(this->address);
}

/*---------------------------------------------------------------------------*/
unint1 Train::getSpeed(){
/*---------------------------------------------------------------------------*/
	return this->speed;
}

/*---------------------------------------------------------------------------*/
void Train::enableFunction(unint4 function){
/*---------------------------------------------------------------------------*/
}

/*---------------------------------------------------------------------------*/
void Train::disableFunction(unint4 function){
/*---------------------------------------------------------------------------*/
}

/*---------------------------------------------------------------------------*/
unint1 Train::getPosition(){
/*---------------------------------------------------------------------------*/
}

unint1 Train::getNextPosition() {

}

void Train::setSwitchesForNextBlock() {

}

void Train::setRouteArray(int* route,  Switch switchRouteArray[][5], int length, int isPeriodic) {
	this->routeArray = route;
	this->routeLength = length;
	this->isRoutePeriodic = isPeriodic;
	this->myArrayPosition = 0;

	for (int i = 0; i< length; i++) {
		for (int j = 0; j< 5; j++) {
			this->switchRouteArray[i][j] = switchRouteArray[i][j];
			if (this->switchRouteArray[i][j].switchid != -1) {
			}
		}
	}
}

void Train::setDirection(unint1 direction) {
	this->direction = direction;
	tcu->setTrainSpeed(this->address, this->speed, this->direction);
}

unint1 Train::getDirection() {
	return this->direction;
}

void Train::setPosition(unint1 new_position) {

	if (this->getNextPosition() == new_position) {
		this->myArrayPosition = (this->myArrayPosition+1) % this->routeLength;
	}

	this->position = new_position;
}
