/*
 * USBEHCIHostController.hh
 *
 *  Created on: 03.02.2013
 *      Author: dbaldin
 */

#ifndef USBEHCIHOSTCONTROLLER_HH_
#define USBEHCIHOSTCONTROLLER_HH_

#include "inc/types.hh"
#include "inc/memio.h"
#include "inc/error.hh"
#include "usb/USBDeviceDriverFactory.hh"
//----------------------------
// Defines
//----------------------------

/*
00h 1 CAPLENGTH Core Capability Register Length
01h 1 Reserved Core N/A
02h 2 HCIVERSION Core Interface Version Number
04h 4 HCSPARAMS Core Structural Parameters
08h 4 HCCPARAMS Core Capability Parameters
0Ch 8 HCSP-PORTROUTE Core Companion Port Route Description
*/

#define CAPLENGTH_OFFSET	  0x0
#define HCIVERSION_OFFSET	  0x1
#define HCSPARAMS_OFFSET	  0x4
#define HCCPARAMS_OFFSET	  0x8
#define HCSP_PORTROUTE_OFFSET 0xC   // optional field

/* USB EHCI Hardware register offsets:

00h USBCMD 				USB Command Core
04h USBSTS 				USB Status Core
08h USBINTR				USB Interrupt Enable Core
0Ch FRINDEX 			USB Frame Index Core
10h CTRLDSSEGMENT 		4G Segment Selector Core
14h PERIODICLISTBASE 	Frame List Base Address Core
18h ASYNCLISTADDR 		Next Asynchronous List Address Core
1C-3Fh 					Reserved Core
40h CONFIGFLAG 			Configured Flag Register Aux
44h PORTSC(1-N_PORTS)	Port Status/Control Aux

 */

// USB EHCI Register offsets
#define USBCMD_OFFSET 			0x0
#define USBSTS_OFFSET 			0x4
#define USBINTR_OFFSET			0x8
#define FRINDEX_OFFSET			0xc
#define CTRLDSSEGMENT_OFFSET 	0x10
#define PERIODICLISTBASE_OFFSET	0x14
#define ASYNCLISTADDR_OFFSET	0x18
#define CONFIGFLAG_OFFSET		0x40
#define PORTSC1_OFFSET			0x44


/*
 * Workload Types
 *
00b Isochronous Transfer Descriptor (iTD, see Section 3.3)
01b Queue Head (QH, see Section 3.6)
10b Split Transaction Isochronous Transfer Descriptor (siTD, see Section 3.4).
11b Frame Span Traversal Node (FSTN, see Section 3.7)
 *
 */

#define EHCI_DS_TYPE_iTD 	0
#define EHCI_DS_TYPE_QH 	1
#define EHCI_DS_TYPE_siTD 	2
#define EHCI_DS_TYPE_FSTN 	3


//----------------------------
// Structures
//----------------------------

// These structures require GCC 4.4 so that no bit fields are padded

#define DIRECTION_OUT 0
#define DIRECTION_IN  1


#define PID_TOKEN_OUT 	0
#define PID_TOKEN_IN 	1
#define PID_TOKEN_SETUP 2


#define	QT_NEXT_TERMINATE			1
#define QT_TOKEN_DT(x)				(((x) & 0x1) << 31)	/* Data Toggle */
#define QT_TOKEN_GET_DT(x)			(((x) >> 31) & 0x1)
#define QT_TOKEN_TOTALBYTES(x)		(((x) & 0x7fff) << 16)	/* Total Bytes to Transfer */
#define QT_TOKEN_GET_TOTALBYTES(x)	(((x) >> 16) & 0x7fff)
#define QT_TOKEN_IOC(x)				(((x) & 0x1) << 15)	/* Interrupt On Complete */
#define QT_TOKEN_CPAGE(x)			(((x) & 0x7) << 12)	/* Current Page */
#define QT_TOKEN_CERR(x)			(((x) & 0x3) << 10)	/* Error Counter */
#define QT_TOKEN_PID(x)				(((x) & 0x3) << 8)	/* PID Code */
#define QT_TOKEN_PID_OUT			0x0
#define QT_TOKEN_PID_IN				0x1
#define QT_TOKEN_PID_SETUP			0x2
#define QT_TOKEN_STATUS(x)			(((x) & 0xff) << 0)	/* Status */
#define QT_TOKEN_GET_STATUS(x)		(((x) >> 0) & 0xff)
#define QT_TOKEN_STATUS_ACTIVE		0x80
#define QT_TOKEN_STATUS_HALTED		0x40
#define QT_TOKEN_STATUS_DATBUFERR	0x20
#define QT_TOKEN_STATUS_BABBLEDET	0x10
#define QT_TOKEN_STATUS_XACTERR		0x08
#define QT_TOKEN_STATUS_MISSEDUFRAME	0x04
#define QT_TOKEN_STATUS_SPLITXSTATE	0x02
#define QT_TOKEN_STATUS_PERR		0x01
#define QT_BUFFER_CNT				5

#define QT_LINK(from,to) from->qt_next = (((unint4)to) & ~0x1f)
#define QT_LINK_ALT(from,to) from->qt_altnext = (((unint4)to) & ~0x1f)

/* Queue Element Transfer Descriptor (qTD). */
typedef struct {
	/* this part defined by EHCI spec */
	unint4 qt_next;			/* see EHCI 3.5.1 */

	unint4 qt_altnext;			/* see EHCI 3.5.2 */
	unint4 qt_token;			/* see EHCI 3.5.3 */

	unint4 qt_buffer[QT_BUFFER_CNT];	/* see EHCI 3.5.4 */
	unint4 qt_buffer_hi[QT_BUFFER_CNT];	/* Appendix B */
	/* pad struct for 32 byte alignment */
	unint4 unused[3];
} qTD;

#define	QH_LINK_TERMINATE	1
#define	QH_LINK_TYPE_ITD	0
#define	QH_LINK_TYPE_QH		2
#define	QH_LINK_TYPE_SITD	4
#define	QH_LINK_TYPE_FSTN	6

#define QH_ENDPT1_RL(x)				(((x) & 0xf) << 28)	/* NAK Count Reload */
#define QH_ENDPT1_C(x)				(((x) & 0x1) << 27)	/* Control Endpoint Flag */
#define QH_ENDPT1_MAXPKTLEN(x)		(((x) & 0x7ff) << 16)	/* Maximum Packet Length */
#define QH_ENDPT1_H(x)				(((x) & 0x1) << 15)	/* Head of Reclamation List Flag */
#define QH_ENDPT1_DTC(x)			(((x) & 0x1) << 14)	/* Data Toggle Control */
#define QH_ENDPT1_DTC_IGNORE_QTD_TD	0x0
#define QH_ENDPT1_DTC_DT_FROM_QTD	0x1
#define QH_ENDPT1_EPS(x)			(((x) & 0x3) << 12)	/* Endpoint Speed */
#define QH_ENDPT1_EPS_FS			0x0
#define QH_ENDPT1_EPS_LS			0x1
#define QH_ENDPT1_EPS_HS			0x2
#define QH_ENDPT1_ENDPT(x)			(((x) & 0xf) << 8)	/* Endpoint Number */
#define QH_ENDPT1_I(x)				(((x) & 0x1) << 7)	/* Inactivate on Next Transaction */
#define QH_ENDPT1_DEVADDR(x)		(((x) & 0x7f) << 0)	/* Device Address */

#define QH_ENDPT2_MULT(x)			(((x) & 0x3) << 30)	/* High-Bandwidth Pipe Multiplier */
#define QH_ENDPT2_PORTNUM(x)		(((x) & 0x7f) << 23)	/* Port Number */
#define QH_ENDPT2_HUBADDR(x)		(((x) & 0x7f) << 16)	/* Hub Address */
#define QH_ENDPT2_UFCMASK(x)		(((x) & 0xff) << 8)	/* Split Completion Mask */
#define QH_ENDPT2_UFSMASK(x)		(((x) & 0xff) << 0)	/* Interrupt Schedule Mask */

/* Queue Head (QH). */
typedef struct {
	// link to the next queue head in the queue head list
	unint4 qh_link;
	// endpoints capabilities word 1
	unint4 qh_endpt1;
	// endpoints capabilities word 2
	unint4 qh_endpt2;
	// current queue transfer descriptor
	unint4 qh_curtd;

	qTD qh_overlay;

} QH;


#define USB_TYPE_STANDARD		(0x00 << 5)
#define USB_TYPE_CLASS			(0x01 << 5)
#define USB_TYPE_VENDOR			(0x02 << 5)
#define USB_TYPE_RESERVED		(0x03 << 5)

#define USB_RECIP_DEVICE		0x00
#define USB_RECIP_INTERFACE	0x01
#define USB_RECIP_ENDPOINT	0x02
#define USB_RECIP_OTHER			0x03



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
	unint1 fill[8];

} __attribute__((packed)) DeviceDescriptor;


typedef struct {

	unint1 bLength;  // == 0x7
	unint1 btype;	 // == 0x5
	unint1 bEndpointAddress;

	unint1 bmAttributes;
	unint2 wMaxPacketSize;
	unint1 bInterval;

} __attribute__((packed)) EndpointDescriptor;


typedef struct {

	unint1 bLength;  // == 0x9
	unint1 btype;	 // == 0x4
	unint1 bInterfaceNumber;

	unint1 bAlternateSetting;
	unint1 bNumEndpoints;
	unint1 bInterfaceClass;
	unint1 bInterfaceSubClass;
	unint1 bInterfaceProtocol;
	unint1 iInterface;

} __attribute__((packed)) InterfaceDescriptor;

typedef struct {

	unint1 bLength;
	unint1 btype;
	unint2 wtotalLength;

	unint1 bNumInterfaces;
	unint1 bConfigurationValue;
	unint1 iConfiguration;
	unint1 bmAttributes;

	unint1 bMaxPower;

} __attribute__((packed)) ConfigurationDescriptor;

typedef struct {
	unint1 bLength;
	unint1 btype;
	unint1 bnrPorts;

	unint2 wHubCharacteristics;
	unint1 bPwrOn2PwrGood;
	unint1 bHubContrCurrent;
	unint1 DeviceRemovable;

} __attribute__((packed)) HubDescriptor;


enum TransferType {Control, Isochronous, Bulk, Interrupt, UnknownTT};

enum TransferDirection {Out, In, Both, UnknownDir};

typedef struct {

	// the endpoint descriptor
   EndpointDescriptor descriptor __attribute__((aligned(4)));

   // each endpoint gets its own queue head
   QH* queue_head;

   qTD* q_int_transfer;

   TransferType type;

   TransferDirection direction;

   // the address of this endpoint at the device
   unint1 address;

   // for interrupt endpoints we need a polling frequency
   unint1 poll_frequency;

   // expected maximum receive size for interrupt transfers
   unint2 max_packet_size;

   // receive buffer for interrupt transfers
   unint1 *recv_buffer;

   unint1 setup_toggle;

   // data toggle used for bulk out transfers
   unint1 data_toggle;

   // data toggle used for bulk in transfers
   unint1 status_toggle;

} DeviceEndpoint;


class USB_EHCI_Host_Controller;

/*!
 *
 *  The USBDevice Class represents a generic attached USB Device.
 *  It holds information on the descriptors received from the device
 *  as well as the topology information (parent) this device is attached to.
 *
 */
class USBDevice {

private:
	ErrorT reactivateEp(int num);

public:
  // constructs a new USB device
  // a queue head is created inside the asynchronous list for this device
  USBDevice(USB_EHCI_Host_Controller *controller, USBDevice *parent, unint1 port, unint1 speed);

  ~USBDevice();

  static unint1 		addr_counter;

  // the device descriptor received
  DeviceDescriptor 		dev_descr;

  // the first interface descriptor received
  InterfaceDescriptor 	if_descr;

  // the associated device address
  unint1 				addr;

  // number of endpoints this device provides
  unint1 				numEndpoints;

  // port this device is connected on at the parent usb device
  unint1 				port;

  // the speed of this device, 0 = full, 1 = low, 2 = high
  unint1 				speed;

  // we support a maximum of five endpoints per device (4 + 1 control)
  DeviceEndpoint 		endpoints[5];

  // The controller this device is connected to
  USB_EHCI_Host_Controller *controller;

  // parent device (hub) or null
  USBDevice 			*parent;

  // some private data used by the device driver for this device
  void* 				dev_priv;

  // hub descriptor	(in case this device is a hub)
  HubDescriptor 		hub_descr;

  // pointer to the usb device drive
  USBDeviceDriver		*driver;

  // activates the endpoint and creates a queue head for async transfer
  ErrorT activateEndpoint(int num);

  // deactivates the device and stops all transfers
  ErrorT deactivate();

  // sets the address of the usb device and updates all queue heads for it
  ErrorT setAddress(unint2 addr);

};

class USBHub : public USBDeviceDriver {
public:
	USBHub(USBDevice *dev, USB_EHCI_Host_Controller *controller);

	virtual ~USBHub();

private:
	// The USB Device represnting the hub
	USBDevice* 					dev;

	// The hub descriptor received during enumeration
	HubDescriptor 				hub_descr;

	// The Controller used to communicate with this hub
	USB_EHCI_Host_Controller* 	controller;

	// The devices attached to the ports.. maximum 6 ports
	USBDevice*					portDevices[6];
public:

	ErrorT 	initialize() {return cOk;};

	ErrorT 	handleInterrupt();

	void 	handleStatusChange();

	ErrorT 	enumerate();
};




typedef struct {
	// status of this port
	int 		status;

	// the root device on this port
	USBDevice 	*root_device;

} PortInfo;


#define USB_DIR_IN  1
#define USB_DIR_OUT 0

/*!
 * \brief Generic USB EHCI Host Controller implementation
 *
 */
class USB_EHCI_Host_Controller {
private:
	unint4 			hc_base;

	unint4 			frame_list_size;

	// address are which is used for the asynchronous and periodic lists
	void* 			memory_base;

	unint4 			memory_size;

	// this flag indicates whether this object is operational
	bool 			operational;

	// ports
	PortInfo 		ports[3];

	// maximum of 10 registered devices
	USBDeviceDriver *registered_devices[10];

public:

	void 	registerDevice(USBDeviceDriver*);

	void 	unregisterDevice(USBDeviceDriver* drv);


	unint4 	operational_register_base;

	// address of the async queue head base registers
	unint4 	async_qh_reg;

	USB_EHCI_Host_Controller(unint4 ehci_dev_base, void* memory_base, unint4 memory_size);

	// tries to enumerate a given device and load the driver
	ErrorT 	enumerateDevice(USBDevice *dev);

	// sends a control msg (setup msg) with data read phase to a usb device
	int 	sendUSBControlMsg(USBDevice *dev, unint1 endpoint, unint1 *control_msg, unint1 direction = USB_DIR_IN, unint1 length = 0, unint1 *data = 0);

	int 	USBBulkMsg(USBDevice *dev, unint1 endpoint, unint1 direction, unint2 data_len, unint1 *data);

	void 	insertPeriodicQH(QH* qh, int poll_rate);

	void 	removefromPeriodic(QH* qh, int poll_rate);

	// try to initialize the device and the frame lists
	ErrorT 	Init();

	// handle EHCI IRQs
	void 	handleInterrupt();

	// destructor
	virtual ~USB_EHCI_Host_Controller();
};

#endif /* USBEHCIHOSTCONTROLLER_HH_ */
