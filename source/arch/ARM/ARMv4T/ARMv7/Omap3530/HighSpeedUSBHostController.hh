/*
 * HighSpeedUSBHostController.hh
 *
 *  Created on: 04.02.2013
 *      Author: danielb
 */

#ifndef HIGHSPEEDUSBHOSTCONTROLLER_HH_
#define HIGHSPEEDUSBHOSTCONTROLLER_HH_

#include "arch/shared/USBEHCIHostController.hh"

class HighSpeedUSBHostController: public USB_EHCI_Host_Controller {
public:

    HighSpeedUSBHostController(T_HighSpeedUSBHostController_Init *init);

    virtual ~HighSpeedUSBHostController();
};

#endif /* HIGHSPEEDUSBHOSTCONTROLLER_HH_ */
