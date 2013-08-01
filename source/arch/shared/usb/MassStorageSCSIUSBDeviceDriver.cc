/*
 * SMSC95xxUSBDeviceDriver.cc
 *
 *  Created on: 02.05.2013
 *      Author: dbaldin
 */

#include "MassStorageSCSIUSBDeviceDriver.hh"
#include "memtools.hh"
#include "kernel/Kernel.hh"
#include "inc/endian.h"

extern Kernel* theOS;

//extern unint4 cputole32(unint4 n);
//#define cputole32(x) x

typedef struct {
	unint1 dCBWSignature[4];
	unint1 dCBWTag[4];
	unint1 dCBWDataTransferLength[4];
	unint1 bmCBWFlags;
	unint1 bCBWDLUN;
	unint1 bCBWCBLength;
	unint1 CBWCB[16];
} CBW;


#define USB_BULK_CB_WRAP_LEN	31
#define USB_BULK_CB_SIG			0x43425355	/* Spells out USBC */
#define USB_BULK_IN_FLAG		0x80

#define SCSI_READ10			0x28
#define SCSI_INQUIRY		0x12
#define SCSI_INQUIRY_LEN	36U

#define CBW_SET_LUN(cbw,lun) cbw[13] = lun

static char cbwInquiry[31] __attribute__((aligned(4))) = {
'U','S','B','C',    			// CBW Signature
0x12,0x34,0x56,0x78,   			// CBW Tag
SCSI_INQUIRY_LEN,0x0,0x0,0x0, 	// CBW Data Transfer Length
USB_BULK_IN_FLAG, 0x0, 6,		// Data DIR, CBW LUN, CB Length
// CB
SCSI_INQUIRY,
0x0, 0x0, 0x0, SCSI_INQUIRY_LEN,
0x0, 0x0, 0x0, 0x0,
0x0 ,0x0, 0x0, 0x0,
0x0, 0x0, 0x0
};


MassStorageSCSIUSBDeviceDriver::MassStorageSCSIUSBDeviceDriver(USBDevice* dev)
:  USBDeviceDriver(), BlockDeviceDriver("sd0")
{
	this->dev 			= dev;
	this->bulkin_ep 	= 0;
	this->bulkout_ep 	= 0;
	this->int_ep 		= 0;
	this->sector_size 	= 512;
	this->myLUN 		= 0;
	LOG(ARCH,INFO,(ARCH,INFO,"MassStorageSCSIUSBDeviceDriver: new Device attached.."));

}

MassStorageSCSIUSBDeviceDriver::MassStorageSCSIUSBDeviceDriver(MassStorageSCSIUSBDeviceDriver* parent, int lun, char* name)
:  USBDeviceDriver(), BlockDeviceDriver(name)
{
	this->dev 			= parent->dev;
	this->bulkin_ep 	= parent->bulkin_ep;
	this->bulkout_ep 	= parent->bulkout_ep;
	this->int_ep 		= 0;
	this->myLUN 		= lun;
	this->sector_size 	= 512;

	LOG(ARCH,INFO,(ARCH,INFO,"MassStorageSCSIUSBDeviceDriver: created LUN %d as '%s'",lun,name));

	memset(&this->scsi_info,0,sizeof(InquiryData));
	int	error = performTransaction(cbwInquiry,(char*)&this->scsi_info,SCSI_INQUIRY_LEN);
	if (error < 0) {
		LOG(ARCH,ERROR,(ARCH,ERROR,"MassStorageSCSIUSBDeviceDriver: device inquiry failed.."));
	} else {
		theOS->getPartitionManager()->registerBlockDevice(this);
	}

}

// TODO: not multi threading capable
static volatile char csw[13];

ErrorT MassStorageSCSIUSBDeviceDriver::performTransaction(char* bot_cbw, char* data,unint4 length) {


	CBW_SET_LUN(bot_cbw,myLUN);
	int direction = (bot_cbw[12] >> 7);

	int error = dev->controller->USBBulkMsg(dev,this->bulkout_ep,USB_DIR_OUT,31,(unint1*) &bot_cbw[0]);

	if (error < 0) {
		LOG(ARCH,ERROR,(ARCH,ERROR,"MassStorageSCSIUSBDeviceDriver: cbw send failed.."));
		ResetRecovery();
		return cError;
	}

	if (direction == 1) {
		// we are reading from the block device
		error = dev->controller->USBBulkMsg(dev,this->bulkin_ep,USB_DIR_IN,length,(unint1*)data);
		if (error < 0) {
			LOG(ARCH,ERROR,(ARCH,ERROR,"MassStorageSCSIUSBDeviceDriver: bulk msg recv failed.."));
			ResetRecovery();
			return cError;
		}

	} else {

		// we are reading from the block device
		error = dev->controller->USBBulkMsg(dev,this->bulkout_ep,USB_DIR_OUT,length,(unint1*)data);
		if (error < 0) {
			LOG(ARCH,ERROR,(ARCH,ERROR,"MassStorageSCSIUSBDeviceDriver: bulk msg send failed.."));
			ResetRecovery();
			return cError;
		}
	}

	// read csw
	error = dev->controller->USBBulkMsg(dev,this->bulkin_ep,USB_DIR_IN,13,(unint1*)csw);
	if (error < 0) {
		LOG(ARCH,ERROR,(ARCH,ERROR,"MassStorageSCSIUSBDeviceDriver: csw recv failed.."));
		ResetRecovery();
		return cError;
	}

	// check csw
	// todo use a 4 byte compare operation
	if ((csw[0] == 0x55) && (csw[1] == 0x53) && (csw[2] == 0x42) && (csw[3] == 0x53)) {

		// todo check tag
		// return data residue on command passed
		if (csw[12] == 0) return csw[8];
		else return csw[12];

	} else {
		ResetRecovery();
		return cError;
	}

	return cOk;

}


ErrorT MassStorageSCSIUSBDeviceDriver::ResetRecovery() {

	unint1 msg[8] = {0x21,0xff,0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	int error = dev->controller->sendUSBControlMsg(dev,0,(unint1*) &msg);
	if (error < 0) {
		LOG(ARCH,ERROR,(ARCH,ERROR,"MassStorageSCSIUSBDeviceDriver: could not reset mass storage interface."));
		return cError;
	}
	// unstall the endpoints. order of unstalling is important
	unint1 msg2[8] = {0x02,0x01,0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	msg2[4] = this->bulkin_ep;
	error = dev->controller->sendUSBControlMsg(dev,0,(unint1*) &msg2);
	if (error < 0) {
		LOG(ARCH,ERROR,(ARCH,ERROR,"MassStorageSCSIUSBDeviceDriver: could not remove STALL at EP_IN."));
		return cError;
	}
	dev->endpoints[this->bulkout_ep].data_toggle = 0;

	msg2[4] = this->bulkout_ep;

	error = dev->controller->sendUSBControlMsg(dev,0,(unint1*) &msg2);
	if (error < 0) {
		LOG(ARCH,ERROR,(ARCH,ERROR,"MassStorageSCSIUSBDeviceDriver: could not remove STALL at EP_OUT."));
		return cError;
	}
	dev->endpoints[this->bulkin_ep].data_toggle = 0;

	return cOk;

}

ErrorT MassStorageSCSIUSBDeviceDriver::initialize() {
	// try to initialize the device

	// first check the endpoint information
	bool bulkinep, bulkoutep, intep = false;

	for (int i = 1; i <= dev->numEndpoints; i++) {
		if (dev->endpoints[i].type == Bulk) {
			// active it

			//dev->endpoints[i].max_packet_size = 64;
			if (dev->endpoints[i].direction == Out) {
				this->bulkout_ep = i; //dev->endpoints[i].address;
				bulkoutep = true;

			} else {
				this->bulkin_ep =  i; //dev->endpoints[i].address;;
				bulkinep = true;
			}

			dev->activateEndpoint(i);

		} else if(dev->endpoints[i].type == Interrupt)  {
			int_ep =  dev->endpoints[i].address;;
			dev->endpoints[i].poll_frequency = 200;
			intep = true;
		}
	}

	if (!(bulkinep & bulkoutep & !intep)) {
		LOG(ARCH,ERROR,(ARCH,ERROR,"MassStorageSCSIUSBDeviceDriver: probing failed.."));
		// deactivate eps again
		return cError;
	}

	dev->dev_priv = 0;

	// perform an initial reset recovery
	if (ResetRecovery() < 0) return cError;

	LOG(ARCH,INFO,(ARCH,INFO,"MassStorageSCSIUSBDeviceDriver: reset successful.."));

	// try getting the number of logical units
	unint4 lun = 1;
	unint1 msg2[8] = {0xa1,0xfe,0x00, 0x00, 0x00, 0x00, 0x01, 0x00};

	int error = dev->controller->sendUSBControlMsg(dev,0,(unint1*) &msg2,USB_DIR_IN,1,(unint1*)&lun);
	if (error < 0) {
		LOG(ARCH,ERROR,(ARCH,ERROR,"MassStorageSCSIUSBDeviceDriver: reading logical unit number failed.."));
		return cError;
	}

	if ((lun >= 0) && (lun <=3)) lun++;
	else lun = 1;
	LOG(ARCH,DEBUG,(ARCH,DEBUG,"MassStorageSCSIUSBDeviceDriver: Device LUN: %d",lun));

	// if lun > 1 we should setup multiple logical units inside the dev directory
	this->myLUN = lun - 1;

	// create additional logical unit representations
	for (unint1 i=1; i< lun; i++) {
		char* name = (char*) theOS->getMemManager()->alloc(4);
		name[0] = 's';
		name[1] = 'd';
		name[2] = '0' + i;
		name[3] = 0;
		new MassStorageSCSIUSBDeviceDriver(this,i,name);
	}

	memset(&this->scsi_info,0,sizeof(InquiryData));
	error = performTransaction(cbwInquiry,(char*)&this->scsi_info,SCSI_INQUIRY_LEN);
	if (error < 0) {
		LOG(ARCH,ERROR,(ARCH,ERROR,"MassStorageSCSIUSBDeviceDriver: device inquiry failed.."));
		return cError;
	}

	theOS->getPartitionManager()->registerBlockDevice(this);


	return cOk;
}

// TODO: not multi threading capable
static char cbwRead10[36] = {
		'U','S','B','C',    			// CBW Signature
		0x11,0x34,0x56,0x10,   			// CBW Tag
		0,0,0,0,
		USB_BULK_IN_FLAG, 0x0, 10,		// Data DIR, CBW LUN, CB Length
		// CB
		SCSI_READ10,
		0x0,0,0, 0,
		0, 0x0, 0x0, 0,
		0x0 ,0x0, 0x0, 0x0,
		0x0, 0x0, 0x0
		};;

ErrorT MassStorageSCSIUSBDeviceDriver::readBlock(unint4 blockNum, char* buffer, unint4 blocks) {

	// block size = 512
	// num blocks = (length / 512) +1;
	//int num_blocks = (length >> 9) +1;
	unint4 length = blocks << 9;


	// initialize read command
	// needs to be done this way for dma on physical address to work
	cbwRead10[8]  = (unint1)(length & 0xff);
	cbwRead10[9]  = (unint1) ((length & 0xff00) >> 8);
	cbwRead10[10] = (unint1) ((length & 0xff0000) >> 16);
	cbwRead10[11] = (unint1) ((length & 0xff000000) >> 24); 			// CBW Data Transfer Length

	cbwRead10[17] = (unint1) ((blockNum & 0xff000000) >> 24);
	cbwRead10[18] = (unint1) ((blockNum & 0xff0000) >> 16);
	cbwRead10[19] = (unint1)((blockNum & 0xff00) >> 8);
	cbwRead10[20] =	(unint1) (blockNum & 0xff);
	cbwRead10[23] = (unint1) blocks;


	int error = performTransaction(cbwRead10,buffer,length);
	if (error < 0) {
		LOG(ARCH,ERROR,(ARCH,ERROR,"MassStorageSCSIUSBDeviceDriver: reading device blocks failed %d. length: %d.",error,length));
		return cError;
	}

	return cOk;
}


ErrorT MassStorageSCSIUSBDeviceDriver::handleInterrupt() {

	ErrorT ret = cError;

	return ret;
}


MassStorageSCSIUSBDeviceDriver::~MassStorageSCSIUSBDeviceDriver() {

	// remove all instances (all luns) from the partition mananger
	LOG(ARCH,INFO,(ARCH,INFO,"MassStorageSCSIUSBDeviceDriver: removing all LUNs.."));
	// TODO delete other LUN instances

	// unregister this device
	theOS->getPartitionManager()->unregisterBlockDevice(this);

	// remove driver from dev directory
	theOS->getFileManager()->getDirectory("/dev")->remove(this);
}


//checks whether the given class,product device is supported by this driver
bool MassStorageSCSIUSBDeviceDriverFactory::isDriverFor(USBDevice* dev) {

	// we dont care for vendor id and product id as we are supporting generic SCSI MS Devices

	// if Mass Storage Device with SCSI command set and Bulk only transfer
	if ((dev->if_descr.bInterfaceClass == 8) && (dev->if_descr.bInterfaceSubClass == 6) && (dev->if_descr.bInterfaceProtocol == 0x50)) return true;

	return false;

}


MassStorageSCSIUSBDeviceDriverFactory::MassStorageSCSIUSBDeviceDriverFactory(char* name)
: USBDeviceDriverFactory(name)
{

}
