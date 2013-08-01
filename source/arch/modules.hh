/*
 * modules.hh
 *
 *  Created on: 26.01.2013
 *      Author: Daniel
 */

#ifndef MODULES_HH_
#define MODULES_HH_


#if MODULE_IN_USERSPACE
#include "inc/types.hh"
#include "lib/orcos.hh"


#define ORCOS_MODULE(name,baseclass) class name : public UserSpace##baseclass
#define ORCOS_MODULE_CONSTRUCTOR(name) name( int4 a )

#define ORCOS_MODULE_CONSTRUCTOR_IMPL(name,baseclass) name::name(int4 a)
#define ORCOS_MODULE_INIT(name,arg_size) \
		extern"C" { \
		static char module_memory[sizeof(name)]; \
		name *the_module; \
		static char arguments[arg_size]; \
		void module_init(int4 a) { \
					the_module = new(&module_memory) name(a); \
					the_module->probe(); \
				} \
		void module_exit() { \
					syscall(cModuleReturnId); \
				} \
		ErrorT lowlevel_send(char* data, int len) { return the_module->lowlevel_send(data,len); } \
		const char* getMacAddr() { return the_module->getMacAddr(); } \
		int1 getMacAddrSize() { return the_module->getMacAddrSize(); } \
		unint2 getMTU () { return the_module->getMTU(); } \
		int2 getHardwareAddressSpaceId() { return the_module->getHardwareAddressSpaceId(); } \
		const char* getBroadcastAddr() { return the_module->getBroadcastAddr(); } \
		ErrorT enableIRQ() { return the_module->enableIRQ(); } \
		ErrorT disableIRQ() { return the_module->disableIRQ(); } \
		ErrorT clearIRQ() {	return  the_module->clearIRQ(); } \
		void module_recv() { return; } \
		ErrorT readByte( char* byte ) { return the_module->readByte(byte); } \
    	ErrorT writeByte( char byte ) { return the_module->writeByte(byte); } \
    	ErrorT readBytes( char *bytes, unint4 &length ) { return the_module->readBytes(bytes,length);  } \
    	ErrorT writeBytes( const char *bytes, unint4 length ) { return the_module->writeBytes(bytes,length); } \
		__volatile__ ORCOS_MCB module_mcb  __attribute__ ((section (".module_mcb"))) { \
					(unint4) &module_init, \
					(unint4) &module_exit, \
					(unint4) &arguments, arg_size, \
					(unint4) &lowlevel_send, \
					(unint4) &getMacAddr, \
					(unint4) &getMacAddrSize, \
					(unint4) &getMTU, \
					(unint4) &getHardwareAddressSpaceId, \
					(unint4) &getBroadcastAddr, \
					(unint4) &enableIRQ, \
					(unint4) &disableIRQ, \
					(unint4) &clearIRQ, \
					(unint4) &module_recv, \
					(unint4) &readByte, \
					(unint4) &writeByte, \
					(unint4) &readBytes, \
					(unint4) &writeBytes, \
					};  \
	}


#else
#define ORCOS_MODULE(name,baseclass) class name : public baseclass
#define ORCOS_MODULE_CONSTRUCTOR(name) name(const char *name, int4 a )

#define ORCOS_MODULE_CONSTRUCTOR_IMPL(name,baseclass) name::name(const char* name, int4 a) : baseclass(name)
#define ORCOS_MODULE_INIT(name,arg_size)

#endif

#endif /* MODULES_HH_ */
