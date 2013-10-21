/*
 * MassStorageSCSIUSBDeviceDriver.hh
 *
 *  Created on: 02.05.2013
 *      Author: dbaldin
 */

#ifndef MASSSTORAGESCSIUSBDEVICEDRIVER_HH_
#define MASSSTORAGESCSIUSBDEVICEDRIVER_HH_

#include "USBDeviceDriverFactory.hh"
#include "hal/BlockDeviceDriver.hh"
#include "arch/shared/USBEHCIHostController.hh"


typedef struct __attribute__((packed)) {
  unint1 peripheral_qualifier : 3;
  unint1 device_type : 5;
  unint1 rmb : 1;
  unint1 reserved1 : 7;
  unint1 version;
  unint1 obsolete1 : 2;
  unint1 normaca : 1;
  unint1 hisup : 1;
  unint1 response_data_format : 4;
  unint1 additional_length;
  unint1 byte5;
  unint1 byte6;
  unint1 byte7;
  unint1 vendor_identification[8];
  unint1 product_identification[16];
  unint1 product_revision[4];
  unint1 drive_serial[8];
} InquiryData;

class MassStorageSCSIUSBDeviceDriver: public USBDeviceDriver, public BlockDeviceDriver {
public:

	USBDevice 	*dev;

	// endpoint number for bulk out transfer
	int 		bulkout_ep;

	// endpoint number for bulk in transfer
	int 		bulkin_ep;

	// interrupt endpoint number
	int 		int_ep;

	InquiryData scsi_info;

	// my logical unit number
	int			 myLUN;

	// next LUN msd of the same usb device
	MassStorageSCSIUSBDeviceDriver *next;

public:
  MassStorageSCSIUSBDeviceDriver(USBDevice* dev);

  virtual ~MassStorageSCSIUSBDeviceDriver();

private:
  // constructor for additional luns
  MassStorageSCSIUSBDeviceDriver(MassStorageSCSIUSBDeviceDriver* parent, int lun, char* name);

  ErrorT 	performTransaction(char* bot_cbw, char* data, unint4 length);

public:
  ErrorT 	initialize();

  ErrorT 	handleInterrupt();

  /*!
   * Static block size of 512 bytes for MassStorageDevices
   */
  unint4 	getBlockSize() { return 512; }

  // TODO overload character device driver methods to allow raw block access to the device

  /*!
   * \brief Reads 'blocks' blocks from block number 'blockNum' of this Logical Unit into
   * 		the buffer.
   */
  ErrorT 	readBlock(unint4 blockNum, char* buffer, unint4 blocks);

  /*!
   * \brief Tries to write 'blocks' blocks to block number 'blockNum' of this Logical Unit from
   * 		the buffer. If verify is set to true the written bytes are verifyed on the device.
   */
  ErrorT 	writeBlock(unint4 blockNum, char* buffer, unint4 blocks);


  ErrorT 	ResetRecovery();
};



class MassStorageSCSIUSBDeviceDriverFactory : public USBDeviceDriverFactory {

public:
	MassStorageSCSIUSBDeviceDriverFactory(char* name);

	//checks whether the given class,product device is supported by this driver
	bool 	isDriverFor(USBDevice* dev);

	// factory method which creates a new instance of this driver
	USBDeviceDriver* getInstance(USBDevice* dev) {
		return new MassStorageSCSIUSBDeviceDriver(dev);
	};

	virtual ~MassStorageSCSIUSBDeviceDriverFactory() {};
};

#endif /* MASSSTORAGESCSIUSBDEVICEDRIVER_HH_ */

