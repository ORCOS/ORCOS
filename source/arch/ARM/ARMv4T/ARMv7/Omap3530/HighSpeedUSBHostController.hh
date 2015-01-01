/*
 * HighSpeedUSBHostController.hh
 *
 *  Created on: 04.02.2013
 *      Copyright & Author: danielb
 */

#ifndef HIGHSPEEDUSBHOSTCONTROLLER_HH_
#define HIGHSPEEDUSBHOSTCONTROLLER_HH_

#include "arch/shared/USBEHCIHostController.hh"

class HighSpeedUSBHostController: public USB_EHCI_Host_Controller {
public:
    /*****************************************************************************
     * Method: HighSpeedUSBHostController(T_HighSpeedUSBHostController_Init *init)
     *
     * @description
     *
     *******************************************************************************/
    explicit HighSpeedUSBHostController(T_HighSpeedUSBHostController_Init *init);

    virtual ~HighSpeedUSBHostController();
};

#endif /* HIGHSPEEDUSBHOSTCONTROLLER_HH_ */
