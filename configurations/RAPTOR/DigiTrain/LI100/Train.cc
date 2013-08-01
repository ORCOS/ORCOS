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

/*---------------------------------------------------------------------------*/
Train::Train(TrainControlUnit* tcu, unint1 address){
/*---------------------------------------------------------------------------*/
	this->speed = 0;
	this->tcu = tcu;
	this->address = address;
	this->direction = 1;
	this->position = NOBLOCK;
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
	this->setSpeed(0);
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
	return this->position;
}

inline void Train::setDirection(unint1 direction) {
	this->direction = direction;
	tcu->setTrainSpeed(this->address, this->speed, this->direction);
}

inline unint1 Train::getDirection() {
	return this->direction;
}

inline void Train::setPosition(unint1 new_position) {
	this->position = new_position;
}
