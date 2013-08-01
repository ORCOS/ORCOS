/*
 * USCommDeviceDriver.cc
 *
 *  Created on: 25.01.2013
 *      Author: Daniel
 */

#include "USCommDeviceDriver.hh"
#include "kernel/Kernel.hh"
#include "inc/types.hh"
#include "inc/memtools.hh"

extern Kernel* theOS;

#define VALID_MODULE(module_cb) ((module_cb->pf_module_init > LOG_MODULE_SPACE_START) &&  \
								(module_cb->pb_argument_area > LOG_MODULE_SPACE_START) && \
								(module_cb->i_max_argument_size > 0))


USCommDeviceDriver::USCommDeviceDriver(const char* dev_name, unint4 mmio_address, unint4 mmio_size, unint4 us_driver_addr) :
  CommDeviceDriver(dev_name), Module(us_driver_addr,us_driver_addr+4096,us_driver_addr+2048)
{
	this->mmio_address = mmio_address;
	this->us_driver_address = us_driver_addr;

	// try to read out the module control block
	module_cb = (ORCOS_MCB*) us_driver_addr;

	if (!VALID_MODULE(module_cb)) {
		valid_module = 0;
	    LOG(ARCH,ERROR,(ARCH,ERROR,"Module '%s' at 0x%x invalid.. Can not load Module..",dev_name, us_driver_addr));

	} else {
		valid_module = 1;
		// call initialization routine
	    LOG(ARCH,INFO,(ARCH,INFO,"Loading Module '%s' at 0x%x.",dev_name, us_driver_addr));
	}

    // add the mmio address space to the virtual address space of this module
    theOS->getHatLayer()->map((void*) mmio_address, (void*) mmio_address, mmio_size ,7,3,((Module*)this)->getId(), !ICACHE_ENABLE);

}

USCommDeviceDriver::~USCommDeviceDriver() {
	// TODO Auto-generated destructor stub
}

ErrorT USCommDeviceDriver::lowlevel_send(char *data, int len) {

	if (module_cb->i_max_argument_size < (size_t) len) {
		 LOG(ARCH,INFO,(ARCH,INFO,"USCommDeviceDriver::lowlevel_send(): data length too big: %d > %d.",len,module_cb->i_max_argument_size ));
		 return cError;
	}

	// place the argument passing structure inside the user space of the module
	ORCOS_module_args *args = (ORCOS_module_args*) module_cb->pb_argument_area;
	args->argument1 = (void*) (module_cb->pb_argument_area + sizeof(ORCOS_module_args));
	args->argument2 = (void*) len;
	memcpy(args->argument1,data,len);

	this->executeModuleFunction((void*) module_cb->pf_low_level_send, (void*) module_cb->pf_exit, (void*) 0);

	// return the result value
	return (ErrorT) args->argument1;

}

ErrorT USCommDeviceDriver::broadcast(packet_layer* packet,
		int2 fromProtocol_ID) {
	return cError;
}

ErrorT USCommDeviceDriver::multicast(packet_layer* packet, int2 fromProtocol_ID,
		unint4 dest_addr) {
	return cError;
}

const char* USCommDeviceDriver::getMacAddr() {
	return "0";
}

int1 USCommDeviceDriver::getMacAddrSize() {
	return 0;
}

unint2 USCommDeviceDriver::getMTU() {
	return cError;
}

int2 USCommDeviceDriver::getHardwareAddressSpaceId() {
	return cError;
}

const char* USCommDeviceDriver::getBroadcastAddr() {
	return "0";
}

ErrorT USCommDeviceDriver::enableIRQ() {
	return cError;
}

ErrorT USCommDeviceDriver::disableIRQ() {
	return cError;
}

ErrorT USCommDeviceDriver::clearIRQ() {
	return cError;
}
