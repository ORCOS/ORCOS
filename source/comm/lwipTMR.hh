/*
 * lwipTMR.hh
 *
 *  Created on: 14.07.2011
 *     Copyright & Author: dbaldin
 */

#ifndef LWIPTMR_HH_
#define LWIPTMR_HH_

#include "hal/CallableObject.hh"

class lwipTMR: public CallableObject {
public:
    lwipTMR();

    ~lwipTMR();

    /*****************************************************************************
     * Method: callbackFunc(void* param)
     *
     * @description
     *  TODO: Rename this to a generic service routine
     * @params
     *
     *******************************************************************************/
    void callbackFunc(void* param);
};

#endif /* LWIPTMR_HH_ */
