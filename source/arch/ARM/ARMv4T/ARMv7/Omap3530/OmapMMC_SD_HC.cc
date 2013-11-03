/*
 * MMC.cc
 *
 *  Created on: 15.09.2013
 *      Author: dbaldin
 */

#include "OmapMMC_SD_HC.hh"
#include "inc/memio.h"

#include "kernel/Kernel.hh"
#include "inc/memtools.hh"

extern void kwait(int milliseconds);

extern Kernel* theOS;


unint4 buf[256];

OmapMMC_SD_HC::OmapMMC_SD_HC(T_OmapMMC_SD_HC_Init *init) : BlockDeviceDriver("mmc")
{
	baseAddress = init->Address;

	// enable MMC1-3 interface clocks
	unint4 CM_ICLKEN1_CORE = INW(0x48004a10);
	CM_ICLKEN1_CORE |= (1 << 24) | (1 << 29) | (1 << 30);
	OUTW(0x48004a10,CM_ICLKEN1_CORE);

	// enable MMC1-3 functional clocks
	unint4 CM_FCLKEN1_CORE = INW(0x48004a00);
	CM_FCLKEN1_CORE |= (1 << 24) | (1 << 29) | (1 << 30);
	OUTW(0x48004a00,CM_FCLKEN1_CORE);

	theOS->getBoard()->getExtPowerControl()->power_mmc_init();

	// reset module
	LOG(ARCH,INFO,(ARCH,INFO,"MMC/SD HC() Resetting."));

	OUTW(baseAddress + MMCHS_SYSCONFIG,0x2);
	//TODO: timeout
	while( (INW(baseAddress + MMCHS_SYSSTATUS) & 0x1) == 0x0) {};

	// soft reset all
	OUTW(baseAddress + MMCHS_SYSCTL,INW(baseAddress + MMCHS_SYSCTL) | (1 << 24));
	// TODO: timeout
	while( (INW(baseAddress + MMCHS_SYSCTL) & (1 << 24)) != 0x0) {};


	LOG(ARCH,INFO,(ARCH,INFO,"MMC/SD HC() Reset successful."));

	// activate 1.8v and 3v output
	unint4 value = INW(baseAddress + MMCHS_CAPA);
	value |= 0x7000000;
	OUTW(baseAddress + MMCHS_CAPA, value);

	// try initializing right now
	this->init();
}

void OmapMMC_SD_HC::init()
{
	// set CONTROL PAD value to closed loop
	OUTW(0x48002144, 0x100);

	LOG(ARCH,DEBUG,(ARCH,DEBUG,"MMC/SD HC() Setting Defaults.."));
	OUTW(baseAddress + MMCHS_CON,0x00000000);

	// TODO: if this doesn't work we should try with 1.8V
	//OUTW(baseAddress + MMCHS_HCTL,0x00000e00 | (1 <<24));
	// try SD card with 3.0V
	OUTW(baseAddress + MMCHS_HCTL,0x00000d00 | (1 <<24));

	// stop clock
	OUTW(baseAddress + MMCHS_SYSCTL,0x00000000);

	// set clock frequency
	OUTW(baseAddress + MMCHS_SYSCTL,0x0000a001 | (240 << 16));

	// wait until frequency is set
	while ( (INW(baseAddress + MMCHS_SYSCTL) & (1 << 1)) == 0x0 ) {};

	// provide clock to card
	OUTW(baseAddress + MMCHS_SYSCTL,INW(baseAddress + MMCHS_SYSCTL) | (1 << 2));

	// activate bus power
	OUTW(baseAddress + MMCHS_HCTL,INW(baseAddress + MMCHS_HCTL) | (1 <<8));

	// enable all interrupts...
	// must be done as all responses from the card are otherwise ignored ..
	/*
	 * IE_BADA | IE_CERR | IE_DEB | IE_DCRC | IE_DTO | IE_CIE |
		IE_CEB | IE_CCRC | IE_CTO | IE_BRR | IE_BWR | IE_TC | IE_CC
	 */
	OUTW(baseAddress + MMCHS_IE,0x307F0033);

	LOG(ARCH,INFO,(ARCH,INFO,"MMC/SD HC() INIT. Synchronizing with card.."));

	OUTW(baseAddress + MMCHS_STAT, STAT_CC);
	OUTW(baseAddress + MMCHS_CON, 0x2);
	OUTW(baseAddress + MMCHS_ARG, 0x0);
	OUTW(baseAddress + MMCHS_CMD, 0x0);

	// wait for initial clock generation to be stable
	kwait(1);
	while ( (INW(baseAddress + MMCHS_STAT) & STAT_CC) == 0x0 ) {};

	OUTW(baseAddress + MMCHS_STAT, STAT_CC);
	OUTW(baseAddress + MMCHS_CMD, 0x0);
	while ( (INW(baseAddress + MMCHS_STAT) & STAT_CC) == 0x0 ) {};

	// stop init
	OUTW(baseAddress + MMCHS_CON, 0x0);

	// clear all in stat
	OUTW(baseAddress + MMCHS_STAT, 0xffffffff);

	LOG(ARCH,DEBUG,(ARCH,DEBUG,"MMC/SD HC() Pre-Card Identification.."));

	// set clock frequency to 400khz
	//OUTW(baseAddress + MMCHS_HCTL,0x00000b00);
	OUTW(baseAddress + MMCHS_SYSCTL,0x00003C07);
	//OUTW(baseAddress + MMCHS_CON,0x00000001);

	// send CMD0 to set back all mmc cards to initialize mode
	sendCommand(0,0);


	// send CMD5
	if (sendCommand(0x05020000,0) == cOk) {
		LOG(ARCH,INFO,(ARCH,INFO,"MMC/SD HC() SDIO card detected."));
		//TODO follow SDIO Standard Specification to idenfity card type
		return;
	}

	LOG(ARCH,TRACE,(ARCH,TRACE,"MMC/SD HC() Software reset for MMCI cmd line.. "));

	OUTW(baseAddress + MMCHS_SYSCTL, INW(baseAddress + MMCHS_SYSCTL) | (1 << 25));
	while ( (INW(baseAddress + MMCHS_SYSCTL) & (1 << 25))  == 0x1) {};

	// send CMD8
	if (sendCommand(0x81a0000,0x01aa) == cOk) {
		LOG(ARCH,INFO,(ARCH,INFO,"MMC/SD HC() SD 2.0 Card detected."));
	}

	// send CMD55
	if (sendCommand(0x371a0000,0) == cError) {

		LOG(ARCH,INFO,(ARCH,INFO,"MMC/SD HC() MMC Card detected."));

		if (sendCommand(0x01020000,0) == cError) {
			LOG(ARCH,ERROR,(ARCH,ERROR,"MMC/SD HC() Error on MMC Card Init: CMD1 failed.."));
		} else {
			unint4 value = INW(baseAddress + MMCHS_RSP10);
			// todo wait until card is ready
		}
	} else {

		int busy = 1;
		int arg =  0x0; //0x1 | 1 << 28 | 1 << 30;
		while (busy)
		{
			// send ACMD41 with busy after response
			// do not check on error if busy after command
			if (sendCommand(0x29030000,arg) == cError) {
				LOG(ARCH,ERROR,(ARCH,ERROR,"MMC/SD HC() Can not detect card."));
				return;
			}

			unint4 value = INW(baseAddress + MMCHS_RSP10);
			LOG(ARCH,DEBUG,(ARCH,DEBUG,"MMC/SD HC() RSP10: 0x%x",value));
			//LOG(ARCH,INFO,(ARCH,INFO,"MMC() RSP10=0x%x",value));
			int ready = (value >> 31) & 0x1;
			if (!ready) {
				// try now with HCS card set // TODO: only do this if CMD8 succeeded
				arg =  value | 1 << 28 | 1 << 30;
				sendCommand(0x371a0000,0);
			} else {
				isHighCapacity = (value >> 30) & 0x1;
				LOG(ARCH,TRACE,(ARCH,TRACE,"MMC/SD HC() High Capacity SD : %d",isHighCapacity));
				busy = 0;
			}

		}
		LOG(ARCH,DEBUG,(ARCH,DEBUG,"MMC/SD HC() SD Card Init Stage Ready."));
	}

	// init MMC or SD x.0 card here

	// send ALL_SEND_CID to get Card Identification number
	if (sendCommand(0x02090000,0) == cError) {
		LOG(ARCH,WARN,(ARCH,WARN,"MMC/SD HC() reading CID failed.. card dropped.."));
		return;
	}

	/* CID Register
	 * from SD Specifcation:
	 * 	Manufacturer ID MID 8 [127:120]
		OEM/Application ID OID 16 [119:104]
		Product name PNM 40 [103:64]
		Product revision PRV 8 [63:56]
		Product serial number PSN 32 [55:24]
		reserved -- 4 [23:20]
		Manufacturing date MDT 12 [19:8]
		CRC7 checksum CRC 7 [7:1]
		not used, always 1 - 1 [0:0]
	 *
	 */


	unint4 value = INW(baseAddress + MMCHS_RSP10);
	//LOG(ARCH,INFO,(ARCH,INFO,"MMC() RSP10=0x%x",value));
	unint4 mdt = value & 0xffff;
	unint4 value2 = INW(baseAddress + MMCHS_RSP32);
	unint4 serial = (value >> 24) | ((value2 & 0xffffff) << 8);

	LOG(ARCH,INFO,(ARCH,INFO,"MMC/SD HC() MDT=0x%x, Product Serial: 0x%x",mdt,serial));
	unint4 revision = (value2 >> 8) & 0xff;
	// get product name
	unint4 value3 = INW(baseAddress + MMCHS_RSP54);
	unint4 value4 = INW(baseAddress + MMCHS_RSP76);
	char name[6];
	memcpy(&name[1],&value3,4);
	name[0] = value4 & 0xff;
	name[5] = 0x0;

	LOG(ARCH,INFO,(ARCH,INFO,"MMC/SD HC() Product Name: %s",name));

	// ask CARD for relative card address (RCA)
	sendCommand(0x031a0000,0x1 << 16);
	value = INW(baseAddress + MMCHS_RSP10);
	this->rca = value >> 16;

	LOG(ARCH,DEBUG,(ARCH,DEBUG,"MMC/SD HC() RCA: 0x%x",rca));

	// ask card for CSD
	sendCommand(0x09090000,rca << 16);
	// read CSD
	// [0:31]
	value = INW(baseAddress + MMCHS_RSP10);
	// [32:63]
	value2 = INW(baseAddress + MMCHS_RSP32);
	// [64:95]
	value3 = INW(baseAddress + MMCHS_RSP54);
	// [96:127]
	value4 = INW(baseAddress + MMCHS_RSP76);

	unint4 trans_speed = value4 & 0xff;
	unint4 block_size = (value3 >> 16) & 0xf;
	this->sector_size = 2 << block_size;
	LOG(ARCH,DEBUG,(ARCH,DEBUG,"MMC/SD HC() TRANS_SPEED: 0x%x, Block Size: %d",trans_speed, sector_size));

	// change speed to maximum supported by card
	// tran_speed = 0x32 = 25 mhz, 50 mhz otherwise

	#define CEN (1 << 2)
	// stop clock
	OUTW(baseAddress + MMCHS_SYSCTL,INW(baseAddress + MMCHS_SYSCTL) & ~(CEN));

	// set clock frequency
	OUTW(baseAddress + MMCHS_SYSCTL,0x0000a001 | (4 << 16));

	// wait until frequency is set
	while ( (INW(baseAddress + MMCHS_SYSCTL) & (1 << 1)) == 0x0 ) {};

	// provide clock to card
	OUTW(baseAddress + MMCHS_SYSCTL,INW(baseAddress + MMCHS_SYSCTL) | (CEN));

	//OUTW(baseAddress + MMCHS_HCTL,INW(baseAddress + MMCHS_HCTL) | (1 <<8));


	// CMD 7 with RCA in upper 16 bits as argument select that card
	sendCommand(0x071a0000, rca << 16);

	// CMD 55
	sendCommand(0x371a0000,rca << 16);

	// ACMD6
	if (sendCommand(0x061b0000, 0x2) == cOk) {
		// set to 4 bits data bus
		OUTW(baseAddress + MMCHS_HCTL,INW(baseAddress + MMCHS_HCTL) | (1 <<1));
	}

	// CMD 16 .. set block len
	sendCommand(0x101a0000, 512);
	this->sector_size = 512;

	LOG(ARCH,INFO,(ARCH,INFO,"MMC/SD HC() Card ready.."));

	theOS->getPartitionManager()->registerBlockDevice(this);

}

OmapMMC_SD_HC::~OmapMMC_SD_HC() {

}

ErrorT OmapMMC_SD_HC::sendCommand(unint4 cmd, unint4 arg) {

	LOG(ARCH,DEBUG,(ARCH,DEBUG,"MMC/SD HC() Issuing CMD 0x%x, ARG: 0x%x",cmd,arg));
	// wait until cmd line is not used any more
	while(INW(baseAddress + MMCHS_PSTATE) & 0x1) {};

	OUTW(baseAddress + MMCHS_STAT, 0xffffffff);
	while (INW(baseAddress + MMCHS_STAT)) {
		OUTW(baseAddress + MMCHS_STAT, 0xffffffff);
		kwait(1);
	};

	OUTW(baseAddress + MMCHS_ARG, arg);
	// send ACMD41
	OUTW(baseAddress + MMCHS_CMD,cmd);

	while ( (INW(baseAddress + MMCHS_STAT) & (STAT_CC | STAT_CTO)) == 0x0 ) {};

	unint4 value = INW(baseAddress + MMCHS_RSP10);
	if ((value >> 16) != 0) {
		LOG(ARCH,DEBUG,(ARCH,DEBUG,"MMC/SD HC() CMD ERROR: 0x%x",value));
	}

	if ( ((INW(baseAddress + MMCHS_STAT) & (STAT_CC)) == 0x1)) return cOk;

	LOG(ARCH,WARN,(ARCH,WARN,"MMC/SD HC() CMD timed out"));

	// reset command line if timed out
	OUTW(baseAddress + MMCHS_SYSCTL, INW(baseAddress + MMCHS_SYSCTL) | (1 << 25));
	while ( (INW(baseAddress + MMCHS_SYSCTL) & (1 << 25))  == 0x1) {};


	return cError;
}

ErrorT OmapMMC_SD_HC::readBlock(unint4 blockNum, unint1* buffer, unint4 length) {

	length *= 512;
	LOG(ARCH,DEBUG,(ARCH,DEBUG,"MMC/SD HC()::readBlock, blockNum: %d length: %d, buffer: %x",blockNum, length, buffer));

	// wait until we are allowed to use data lines
	while ((INW(baseAddress + MMCHS_PSTATE) & (1 << 1))) {}

	OUTW(baseAddress + MMCHS_BLK, 0x200);

	if (isHighCapacity == 0) blockNum *= 512;

	// CMD 17 (send single block)
	if (sendCommand(0x113a0010,blockNum) == cError) {
		return cError;
	}

	// buffer must be 4 bytes aligned
	unint4* buf = (unint4*) buffer;

	unint4 timeout = 100;
	// TODO: remove 1 ms wait .. we could be much faster
	while (((INW(baseAddress + MMCHS_STAT) & (1 << 5)) == 0) && (timeout)) {timeout--; kwait(1);};

	unint4 mmc_stat = INW(baseAddress + MMCHS_STAT);
	if (mmc_stat & (1<< 15)) {
		LOG(ARCH,WARN,(ARCH,WARN,"MMC/SD HC()::readBlock failed: STAT: %x",mmc_stat));
		return cError;
	}

	if (timeout == 0) {
		LOG(ARCH,WARN,(ARCH,WARN,"MMC/SD HC()::readBlock timeout.."));
		return cError;
	}

	while ((INW(baseAddress + MMCHS_PSTATE) & (1<<11)) == 0) {};

	// check brr
	while ((length > 0)) {
		*buf = INW(baseAddress + MMCHS_DATA);
		buf++;
		length -= 4;
	}

	return cOk;

	// CMD 12 (Stop transmission)
	//sendCommand(0x0c1a0000,0x0);

}

ErrorT OmapMMC_SD_HC::writeBlock(unint4 blockNum, unint1* buffer, unint4 length) {

	length *= 512;
	LOG(ARCH,DEBUG,(ARCH,DEBUG,"MMC/SD HC()::writeBlock, blockNum: %d length: %d, buffer: %x",blockNum, length, buffer));

	// wait until we are allowed to use data lines
	while ((INW(baseAddress + MMCHS_PSTATE) & (1 << 1))) {}

	OUTW(baseAddress + MMCHS_BLK, 0x200);

	if (isHighCapacity == 0) blockNum *= 512;

	// CMD 24 (write single block)
	if (sendCommand(0x183a0000,blockNum) == cError) {
		return cError;
	}

	unint4 timeout = 100;
	// TODO: remove 1 ms wait .. we could be much faster
	while (((INW(baseAddress + MMCHS_STAT) & (1 << 4)) == 0) && (timeout)) {timeout--; kwait(1);};

	// test error bit
	unint4 mmc_stat = INW(baseAddress + MMCHS_STAT);
	if (mmc_stat & (1<< 15)) {
		LOG(ARCH,WARN,(ARCH,WARN,"MMC/SD HC()::writeBlock failed: STAT: %x",mmc_stat));
		return cError;
	}

	if (timeout == 0) {
		LOG(ARCH,WARN,(ARCH,WARN,"MMC/SD HC()::writeBlock timeout.."));
		return cError;
	}

	// no error.. start writing data .. first wait until we are allowed to write
	while ((INW(baseAddress + MMCHS_PSTATE) & (1<<10)) == 0) {};

	unint4* buf = (unint4*) buffer;

	// now write everything
	while ((length > 0)) {
		OUTW(baseAddress + MMCHS_DATA,*buf);
		buf++;
		length -= 4;
	}

	return cOk;


}
