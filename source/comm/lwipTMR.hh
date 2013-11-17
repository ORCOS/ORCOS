/*
 * lwipTMR.hh
 *
 *  Created on: 14.07.2011
 *      Author: dbaldin
 */

#ifndef LWIPTMR_HH_
#define LWIPTMR_HH_

#include "hal/CallableObject.hh"

class lwipTMR : public CallableObject {
public:
	lwipTMR();
	virtual ~lwipTMR();

	void callbackFunc(void* param);
};

#endif /* LWIPTMR_HH_ */
