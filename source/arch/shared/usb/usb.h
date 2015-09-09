/*
 * usb.h
 *
 *  Created on: 27.08.2015
 *      Author: Daniel
 */

#ifndef SOURCE_ARCH_SHARED_USB_USB_H_
#define SOURCE_ARCH_SHARED_USB_USB_H_

#include "inc/types.hh"


#define USB_DIR_IN                  1 /* BULK transfer IN direction */
#define USB_DIR_OUT                 0 /* BULK transfer OUT direction */
#define USB_DIR_NONE                2
#define USB_IGNORE_ERROR            4

#define DIRECTION_OUT                   0
#define DIRECTION_IN                    1

#define PID_TOKEN_OUT                   0
#define PID_TOKEN_IN                    1
#define PID_TOKEN_SETUP                 2

#define USB_TYPE_STANDARD           (0x00 << 5)
#define USB_TYPE_CLASS              (0x01 << 5)
#define USB_TYPE_VENDOR             (0x02 << 5)
#define USB_TYPE_RESERVED           (0x03 << 5)

#define USB_RECIP_DEVICE            0x00
#define USB_RECIP_INTERFACE         0x01
#define USB_RECIP_ENDPOINT          0x02
#define USB_RECIP_OTHER             0x03

#define PORT_CONNECTION     0
#define PORT_ENABLE         1
#define PORT_SUSPEND        2
#define PORT_OVER_CURRENT   3
#define PORT_RESET          4
#define PORT_POWER          0x8
#define PORT_LOW_SPEED      9
#define C_PORT_CONNECTION   16
#define C_PORT_ENABLE       17
#define C_PORT_SUSPEND      18
#define C_PORT_OVER_CURRENT 19
#define C_PORT_RESET        20
#define PORT_TEST           21
#define PORT_INDICATOR      22


//value.low, value.high, index.low, index.high, len.low, len.high
#define USB_GETHUB_DESCRIPTOR()            {0xa0, 0x06, 0x0, 0x0, 0x0, 0x0, 0x40, 0x0 }
#define USB_GETHUB_STATUS()                {0xa0, 0x00, 0x0, 0x0, 0x0, 0x0, 0x4, 0x0 }
#define USB_GETPORT_STATUS()               {0xa3, 0x00, 0x0, 0x0, 0x0, 0x0, 0x4, 0x0 }
#define USB_SETPORT_FEATURE(feature)       {0x23, 0x03, feature, 0x0, 0x0, 0x0, 0x0, 0x0 }
#define USB_CLEARPORT_FEATURE(feature)     {0x23, 0x01, feature, 0x0, 0x0, 0x0, 0x0, 0x0 }

#define DEVICE_DESCRIPTOR 0x1
#define CONFIGURATION_DESCRIPTOR 0x2
#define STRING_DESCRIPTOR 0x3
#define INTERFACE_DESCRIPTOR 0x4
#define ENDPOINT_DESCRIPTOR 0x5

#define USB_DESCRIPTOR_REQUEST(type)           {0x80, 0x06, 0x0, type, 0x0, 0x0, 0x40, 0x0 }
#define USB_SETADDRESS_REQUEST(addr)           {0x00, 0x05, addr, 0x0, 0x0, 0x0, 0x0 , 0x0 }
#define USB_SETCONFIGURATION_REQUEST(id)       {0x00, 0x09, id, 0x0 , 0x0, 0x0, 0x0, 0x0 }
#define USB_SETINTERFACE_REQUEST(id)           {0x01, 0x0b, 0x0, 0x0 , 0x0, id, 0x0, 0x0 }
#define USB_GETDEVICE_STATUS()                 {0x80, 0x00, 0x0, 0x0 , 0x0, 0x0, 0x2, 0x0 }
#define USB_CLEAR_FEATURE(feature, endpoint)   {0x02, 0x01, feature, 0x0, endpoint, 0x0, 0x0, 0x0 }

#define USB_SET_INDEX_FIELD(req, index) *(reinterpret_cast<unint2*>(&req[4])) = htons(index)
#define USB_SET_VALUE_FIELD(req, value) *(reinterpret_cast<unint2*>(&req[2])) = htons(value)



// TODO: only works if network = host endianess!
typedef struct {
    unint1 bLength;
    unint1 btype;
    unint1 v_usblow;
    unint1 v_usbhigh;

    unint1 bDeviceClass;
    unint1 bDeviceSubClass;
    unint1 bDeviceProtocol;
    unint1 bMaxPacketSize;

    unint2 idVendor;
    unint2 idProduct;

    unint2 bcdDevice;
    unint1 iManufacturer;
    unint1 iProduct;
    unint1 iSerialNumber;
    unint1 bNumConfigurations;
}__attribute__((packed)) DeviceDescriptor;

typedef struct {
    unint1 bLength;         // == 0x7
    unint1 btype;           // == 0x5
    unint1 bEndpointAddress;

    unint1 bmAttributes;
    unint2 wMaxPacketSize;
    unint1 bInterval;
}__attribute__((packed)) EndpointDescriptor;

typedef struct {
    unint1 bLength;         // == 0x9
    unint1 btype;           // == 0x4
    unint1 bInterfaceNumber;

    unint1 bAlternateSetting;
    unint1 bNumEndpoints;
    unint1 bInterfaceClass;
    unint1 bInterfaceSubClass;
    unint1 bInterfaceProtocol;
    unint1 iInterface;
}__attribute__((packed)) InterfaceDescriptor;

typedef struct {
    unint1 bLength;
    unint1 btype;
    unint2 wtotalLength;

    unint1 bNumInterfaces;
    unint1 bConfigurationValue;
    unint1 iConfiguration;
    unint1 bmAttributes;

    unint1 bMaxPower;
}__attribute__((packed)) ConfigurationDescriptor;

typedef struct {
    unint1 bLength;
    unint1 btype;
    unint1 bnrPorts;

    unint2 wHubCharacteristics;
    unint1 bPwrOn2PwrGood;
    unint1 bHubContrCurrent;
    unint1 DeviceRemovable;
}__attribute__((packed)) HubDescriptor;

enum TransferType {
    Control, Isochronous, Bulk, Interrupt, UnknownTT
};

enum TransferDirection {
    Out, In, Both, UnknownDir
};

typedef struct {
    // the endpoint descriptor
    EndpointDescriptor descriptor __attribute__((aligned(4)));

    void* device_priv;
    void* device_priv2;
    // each endpoint gets its own queue head
    //QH*     queue_head;

    // The qtd used for transer to this device. Currently only one
    //qTD*    qtd;

    // receive buffer for interrupt transfers
    char *recv_buffer;

    // The transfer type of this endpoint
    TransferType type;

    // The direction of this endpoint
    TransferDirection direction;

    // the address of this endpoint at the device
    unint1 address;

    // for interrupt endpoints we need a polling frequency
    unint1 poll_frequency;

    // expected maximum receive size for this transfers issuing an interrupt
    unint2 interrupt_receive_size;

    // data toggle used for bulk out transfers
    unint1 data_toggle;
} DeviceEndpoint;


#endif /* SOURCE_ARCH_SHARED_USB_USB_H_ */
