/*
 * lwipTMR.hh
 *
 *  Created on: 14.07.2011
 *     Copyright & Author: dbaldin
 */

#ifndef LWIPTMR_HH_
#define LWIPTMR_HH_

#include "hal/CallableObject.hh"

class KernelServiceThread: public CallableObject {
    /* call counter. */
    int count;

public:
    KernelServiceThread();

    ~KernelServiceThread();

    /*****************************************************************************
     * Method: callbackFunc(void* param)
     *
     * @description
     *
     * @params
     *
     *******************************************************************************/
    void callbackFunc(void* param);
};

#endif /* LWIPTMR_HH_ */
