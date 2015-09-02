/*
 * USBHostController.hh
 *
 *  Created on: 27.08.2015
 *      Author: Daniel
 */

#ifndef SOURCE_ARCH_SHARED_USB_USBHOSTCONTROLLER_HH_
#define SOURCE_ARCH_SHARED_USB_USBHOSTCONTROLLER_HH_

#include "usb.h"
#include "USBDevice.hh"
#include "USBDriverLibrary.hh"

class USBDevice;

typedef struct {
    // status of this port
    int status;

    // the root device on this port
    USBDevice *root_device;
} PortInfo;


/*!
 * \brief Generic USB EHCI Host Controller implementation
 *
 */
class USB_Host_Controller: public GenericDeviceDriver {
protected:

    // ports
    PortInfo ports[3];

    // maximum of 10 registered devices
    USBDeviceDriver *registered_devices[10];

    /*****************************************************************************
     * Method: resetPort(unint1 port)
     *
     * @description
     *  Resets the usb port. Returns cOk on success.
     *******************************************************************************/
    virtual ErrorT resetPort(unint1 port) = 0;

public:
    /*****************************************************************************
     * Method: registerDevice(USBDeviceDriver* drv)()
     *
     * @description
     *  Registers a new USB Device Driver that has been
     *  instantiated for a new USB Device
     *******************************************************************************/
    void registerDevice(USBDeviceDriver* drv);

    /*****************************************************************************
     * Method: unregisterDevice(USBDeviceDriver* drv)()
     *
     * @description
     *  Unregisters a USB Device Driver
     *******************************************************************************/
    void unregisterDevice(USBDeviceDriver* drv);


    /*****************************************************************************
     * Method: USB_Host_Controller()
     *
     * @description
     *  Creates a new instance of the Host Controller
     *******************************************************************************/
    explicit USB_Host_Controller(char* hc_name) : GenericDeviceDriver(true, hc_name) {
        for (int i = 0; i < 10; i++) {
           registered_devices[i] = 0;
        }
    };

    /*****************************************************************************
     * Method: enumerateDevice(USBDevice *dev)
     *
     * @description
     *  Tries to enumerate a given device in order to load a driver for it
     *******************************************************************************/
    ErrorT enumerateDevice(USBDevice *dev);

    /*****************************************************************************
     * Method: sendUSBControlMsg(USBDevice *dev,
     *                           unint1     endpoint,
     *                           char      *control_msg,
     *                           unint1     direction = USB_DIR_IN,
     *                           unint1     length = 0,
     *                           char      *data = 0);
     *
     * @description
     *  Sends a USB control message (setup msg) with optional data read/write phase
     *  to a USB device
     *
     * @params
     *  dev         The device the control message shall be send to
     *  endpoint    The endpoint of the device the control message will be send to
     *  control_msg Pointer to the control msg (8 bytes) to be send
     *  direction   Direction of the data transfer following the control msg
     *              USB_DIR_IN or USB_DIR_OUT
     *  length      Length of bytes to be transferred
     *  data        Pointer to the data to be send or the buffer to be filled with
     *              the received data
     *
     * @returns
     *  int         Number of bytes transferred inside the data phase
     *******************************************************************************/
    virtual int sendUSBControlMsg(USBDevice *dev,
                          unint1 endpoint,
                          char *control_msg,
                          unint1 direction = USB_DIR_IN,
                          unint1 length = 0,
                          char *data = 0) = 0;

    /*****************************************************************************
     * Method: USBBulkMsg(USBDevice *dev,
     *                    unint1     endpoint,
     *                    unint1     direction,
     *                    unint2     data_len,
     *                    char      *data);
     *
     * @description
     *  Starts a bulk transfer to the given USBDevice and endpoint.
     *  The Bulk transfer direction can either be USB_IN or USB_OUT.
     *  The length of the bulk transfer is given by data_len. The data is send from / stored to
     *  the pointer given by data.
     *
     *  Upon success this method returns the number of bytes transfered >= 0.
     *  Upon error returns an error code < 0.
     *
     * @params
     *  dev         The device the data shall be transferred to/from
     *  endpoint    The endpoint of the device
     *  direction   Direction of the data transfer
     *              USB_DIR_IN or USB_DIR_OUT
     *  data_len    Number of bytes to transfer
     *  data        Pointer to the data to be send or the buffer to be filled with
     *              the received data
     *
     * @returns
     *  int         Number of bytes transferred inside the data phase
     *******************************************************************************/
    virtual int USBBulkMsg(USBDevice *dev,
                   unint1 endpoint,
                   unint1 direction,
                   unint2 data_len,
                   char *data) = 0;

    /*****************************************************************************
     * Method: insertPeriodicQH(QH* qh, int poll_rate)
     *
     * @description
     *  Inserts a Queue Head into the periodic queue
     *  Poll rates are given in milliseconds. No support for poll rates < 1 ms.
     *  Be aware: 1 ms poll rate will fill up the whole queue.
     *
     * @params
     *  qh          The Queue Head to be inserted
     *  poll_rate   Poll rate in ms
     *
     *******************************************************************************/
    //virtual void insertPeriodicQH(QH* qh, int poll_rate) = 0;

    /*****************************************************************************
     * Method: removefromPeriodic(QH* qh, int poll_rate)
     *
     * @description
     *  Removes a queue head from the periodic list effectively stopping
     *  the periodic transfer.
     *
     * @params
     *  qh          The Queue Head to be removed
     *
     *******************************************************************************/
    //virtual void removefromPeriodic(QH* qh, int poll_rate) = 0;

    virtual USBDevice* createDevice(USBDevice *parent, unint1 port, unint1 speed) = 0;

    // destructor
    virtual ~USB_Host_Controller() {};
};

#endif /* SOURCE_ARCH_SHARED_USB_USBHOSTCONTROLLER_HH_ */
