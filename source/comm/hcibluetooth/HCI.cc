/*
 * HCI.cc
 *
 *  Created on: 07.11.2012
 *      Author: kgilles
 */

#include "kernel/Kernel.hh"
#include "HCI.hh"
#include <types.hh>
#include "inc/sprintf.hh"
#include BoardCfd_hh

extern Kernel* theOS;

HCI::HCI() {
	// TODO Auto-generated constructor stub
#ifdef HAS_Board_HCICfd
	this->transportDevice = theOS->board->getHCI();
#endif
	//csr();
}

HCI::~HCI() {
	// TODO Auto-generated destructor stub
}

void HCI::reset(unint1 *ret_params)
{
	unint1 cmd_buffer[2];
	unint4 length = 1;
	unint ocf = 0x0003;
	unint ogf = HCI_CONTROLLER_BASEBAND;
	unint opcode = ocf | ogf << 10;
	cmd_buffer[0] = opcode & 0xFF;
	cmd_buffer[1] = opcode >> 8;
	printf("\n read scan enable command: \n");
	theOS->board->getUART()->writeBytes((const char *) cmd_buffer, 2);
	printf(" end of cmd \n");
	this->transportDevice->writeBytes((const char *) cmd_buffer, 2);
	command_complete_event((char *) ret_params, length);
	printf("\nResult: Status: %d \n", ret_params[0]);
}

unint1 HCI::write_Scan_Enable(int inq_en, int page_en) {
	// OCF 0x001A
	// parameter length: 1 byte
	unint i;
	unint1* status;
	*status = 1;
	unint4 readLength = 5;
	unint4 writeLength = 5;
	unint1 cmd_buffer[writeLength];
	unint1 readBuffer[readLength];
	for (i = 0; i < readLength; i++)
	{
		readBuffer[i] = 1;
	}
	unint ocf = 0x001A;
	unint ogf = HCI_CONTROLLER_BASEBAND;
	unint opcode = ocf | ogf << 10;
	cmd_buffer[0] = HCI_COMMAND;
	cmd_buffer[1] = opcode & 0xFF;
	cmd_buffer[2] = opcode >> 8;
	cmd_buffer[3] = 1;
	if (inq_en)
	{
		if (page_en)
		{
			cmd_buffer[4] = 0x03; // Inquiry enabled, page scan enabled: 0x03
		} else
		{
			cmd_buffer[4] = 0x01; // Inquiry enabled, page scan disabled: 0x01
		}
	} else
	{
		if (page_en)
		{
			cmd_buffer[4] = 0x02; // Inquiry disabled, page scan enabled: 0x02
		} else
		{
			cmd_buffer[4] = 0x00; // all scans disabled
		}
	}

	printf("\n write scan enable command: \n");
	theOS->board->getUART()->writeBytes((const char *) cmd_buffer, writeLength);
	printf(" end of cmd \n");
	this->transportDevice->writeBytes((const char *) cmd_buffer, writeLength);
	command_complete_event((char *) readBuffer, readLength);
	printf("\nResult: ret1: %d, ret2: %d, ret3: %d, ret4: %d, ret5: %d \n", readBuffer[0], readBuffer[1], readBuffer[2], readBuffer[3], readBuffer[4]);
	return readBuffer[0];
}

void HCI::read_Scan_Enable(unint1 *ret_params) {
	unint i;
	ErrorT err = cOk;
	unint4 readLength = 5;
	unint4 writeLength = 4;
	unint1 cmd_buffer[writeLength];
	unint1 readBuffer[readLength];
	for (i = 0; i < readLength; i++)
	{
		readBuffer[i] = 1;
	}
	unint ocf = 0x0019;
	unint ogf = HCI_CONTROLLER_BASEBAND;
	unint opcode = ocf | ogf << 10;
	cmd_buffer[0] = HCI_COMMAND;
	cmd_buffer[1] = opcode & 0xFF;
	cmd_buffer[2] = opcode >> 8;
	cmd_buffer[3] = 0x00;
	printf("\n read scan enable command: \n");
	theOS->board->getUART()->writeBytes((const char *) cmd_buffer, writeLength);
	printf(" end of cmd \n");
	this->transportDevice->writeBytes((const char *) cmd_buffer, writeLength);
	this->transportDevice->readBytes((char *) readBuffer, readLength);
	err = command_complete_event((char *) readBuffer, readLength);
	if (err != cOk) printf("\nError when reading HCI event! err code: %d\n", err);
	//printf("\n Result: %d \n", readBuffer[0]);
	printf("\nResult: ret1: %d, ret2: %d, ret3: %d, ret4: %d, ret5: %d\n", readBuffer[0], readBuffer[1], readBuffer[2], readBuffer[3], readBuffer[4]);
}

void HCI::write_Inquiry_Scan_Activity(unint2 duration, unint2 interval) {
	// OCF: 0x001E
	unint4 cmdLength = 8;
	unint ocf = 0x001E;
	unint ogf = HCI_CONTROLLER_BASEBAND;
	unint opcode = ocf | ogf << 10;
	unint1 cmd_buffer[cmdLength];
	cmd_buffer[0] = HCI_COMMAND;
	cmd_buffer[1] = opcode & 0xFF;
	cmd_buffer[2] = opcode >> 8;
	cmd_buffer[3] = 0x04;
	cmd_buffer[4] = (unint1) 0x0800;
	cmd_buffer[5] = (unint1) (0x0800 >> 8);
	cmd_buffer[6] = (unint1) 0x0012;
	cmd_buffer[7] = (unint1) (0x0012 >> 8);

	printf("\n write inquiry scan activity command: \n");
	theOS->board->getUART()->writeBytes((const char *) cmd_buffer, cmdLength);
	printf(" end of cmd \n");
	this->transportDevice->writeBytes((const char *) cmd_buffer, cmdLength);
}

void HCI::read_Inquiry_Scan_Activity(unint1 *ret_params)
{
	unint4 writeLength = 4;
	unint1 cmd_buffer[writeLength];
	unint4 readLength = 5;
	unint ocf = 0x001D;
	unint ogf = HCI_CONTROLLER_BASEBAND;
	unint opcode = ocf | ogf << 10;
	cmd_buffer[0] = HCI_COMMAND;
	cmd_buffer[1] = opcode & 0xFF;
	cmd_buffer[2] = opcode >> 8;
	cmd_buffer[3] = 0;
	printf("\n read inquiry scan activity command: \n");
	theOS->board->getUART()->writeBytes((const char *) cmd_buffer, writeLength);
	printf(" end of cmd \n");
	this->transportDevice->writeBytes((const char *) cmd_buffer, writeLength);
	command_complete_event((char *) ret_params, readLength);
	printf("\nResult: Status: %d, Interval: %d \n", ret_params[0], (unint2) ret_params[1], (unint2) ret_params[3]);
}

void HCI::write_Current_IAC_LAP(int address) {
	// OCF: 0x003a
}

void HCI::read_Current_IAC_LAP(unint1 *ret_params)
{
	unint4 writeLength = 4;
	unint1 cmd_buffer[writeLength];
	unint4 readLength = 5;
	unint ocf = 0x0039;
	unint ogf = HCI_CONTROLLER_BASEBAND;
	unint opcode = ocf | ogf << 10;
	cmd_buffer[0] = HCI_COMMAND;
	cmd_buffer[1] = opcode & 0xFF;
	cmd_buffer[2] = opcode >> 8;
	cmd_buffer[3] = 0;
	printf("\n read current iac lap command: \n");
	theOS->board->getUART()->writeBytes((const char *) cmd_buffer, writeLength);
	printf(" end of cmd \n");
	this->transportDevice->writeBytes((const char *) cmd_buffer, writeLength);
	command_complete_event((char *) ret_params, readLength);
	printf("\nResult: Status: %d, IAC: %d \n", ret_params[0], ret_params[1]);
}

unint1 HCI::read_Local_Name(char* buf, int length)
{
	int i;
	unint1* status;
	unint4 readLength = 248;
	unint4 writeLength = 4;
	unint1 cmd_buffer[writeLength];
	if (length < readLength)
	{
		readLength = length;
	}
	unint ocf = 0x0014;
	unint ogf = HCI_CONTROLLER_BASEBAND;
	unint opcode = ocf | ogf << 10;
	cmd_buffer[0] = HCI_COMMAND;
	cmd_buffer[1] = opcode & 0xFF;
	cmd_buffer[2] = opcode >> 8;
	cmd_buffer[3] = 0;
	printf("\n read local name command: \n");
	theOS->board->getUART()->writeBytes((const char *) cmd_buffer, 3);
	printf(" end of cmd \n");

	this->transportDevice->writeBytes((const char *) cmd_buffer, writeLength);
	command_complete_event((char *) buf, readLength);
	return buf[0];
}

/*************************************************************
 * Link control commands
 ************************************************************/
void HCI::inquiry(unint LAP, unint1 length, unint1 num_responses)
{
	// OCF: 0x0001
	unint4 writeLength = 9;
	unint ocf = 0x0001;
	unint ogf = HCI_LINK_CONTROL;
	unint opcode = ocf | ogf << 10;
	unint1 cmd_buffer[writeLength];
	cmd_buffer[0] = HCI_COMMAND;
	cmd_buffer[1] = opcode & 0xFF;
	cmd_buffer[2] = opcode >> 8;
	cmd_buffer[3] = 5;
 	cmd_buffer[4] = (unint1) LAP;
	cmd_buffer[5] = (unint1) (LAP >> 8);
	cmd_buffer[6] = (unint1) (LAP >> 16);
	cmd_buffer[7] = length;
	cmd_buffer[8] = num_responses;
	this->transportDevice->writeBytes((const char *) cmd_buffer, writeLength);
}

void HCI::inquiry_cancel(unint1* ret_params)
{
	unint4 writeLength = 4;
	unint4 readLength = 1;
	unint1 cmd_buffer[writeLength];
	unint ocf = 0x0002;
	unint ogf = HCI_LINK_CONTROL;
	unint opcode = ocf | ogf << 10;
	cmd_buffer[0] = HCI_COMMAND;
	cmd_buffer[0] = opcode & 0xFF;
	cmd_buffer[1] = opcode >> 8;
	cmd_buffer[3] = 0;
	this->transportDevice->writeBytes((const char *) cmd_buffer, writeLength);
	command_complete_event((char *) ret_params, readLength);
}


void HCI::create_Connection(unint8 bd_addr, unint2 packet_type, unint1 rep_mode, unint1 page_scan_mode, unint2 clock_offset, unint1 allow_role_switch)
{
	// OCF: 0x0005
	unint4 writeLength = 17;
	unint1 cmd_buffer[writeLength];
	unint ocf = 0x0005;
	unint ogf = HCI_LINK_CONTROL;
	unint opcode = ocf | ogf << 10;
	cmd_buffer[0] = HCI_COMMAND;
	cmd_buffer[1] = opcode & 0xFF;
	cmd_buffer[2] = opcode >> 8;
	cmd_buffer[3] = 13;
	cmd_buffer[4] = (unint1) bd_addr;
	cmd_buffer[5] = (unint1) (bd_addr >> 8);
	cmd_buffer[6] = (unint1) (bd_addr >> 16);
	cmd_buffer[7] = (unint1) (bd_addr >> 24);
	cmd_buffer[8] = (unint1) (bd_addr >> 32);
	cmd_buffer[9] = (unint1) (bd_addr >> 40);
	cmd_buffer[10] = (unint1) (packet_type);
	cmd_buffer[11] = (unint1) (packet_type >> 8);
	cmd_buffer[12] = rep_mode;
	cmd_buffer[13] = page_scan_mode;
	cmd_buffer[14] = (unint1) (clock_offset);
	cmd_buffer[15] = (unint1) (clock_offset >> 8);
	cmd_buffer[16] = allow_role_switch;
	this->transportDevice->writeBytes((const char *) cmd_buffer, writeLength);
}


void HCI::accept_Connection_Request(unint8 bd_addr, unint1 role)
{
	// OCF: 0x0009
	unint4 writeLength = 11;
	unint1 cmd_buffer[writeLength];
	unint ocf = 0x0009;
	unint ogf = HCI_LINK_CONTROL;
	unint opcode = ocf | ogf << 10;
	cmd_buffer[0] = HCI_COMMAND;
	cmd_buffer[1] = opcode & 0xFF;
	cmd_buffer[2] = opcode >> 8;
	cmd_buffer[3] = 7;
	cmd_buffer[4] = (unint1) bd_addr;
	cmd_buffer[5] = (unint1) (bd_addr >> 8);
	cmd_buffer[6] = (unint1) (bd_addr >> 16);
	cmd_buffer[7] = (unint1) (bd_addr >> 24);
	cmd_buffer[8] = (unint1) (bd_addr >> 32);
	cmd_buffer[9] = (unint1) (bd_addr >> 40);
	cmd_buffer[10] = (unint1) (role);
	this->transportDevice->writeBytes((const char *) cmd_buffer, writeLength);
}

/*************************************************************
 * Link control commands
 ************************************************************/

ErrorT HCI::command_complete_event(char* buffer, unint1 size)
{
	ErrorT err;
    unint4 readLength;
    char header[3] = {0,0,0};
    int i = 10;

    printf("Reading ... \n");
    // HCI event packet has packet type 0x04
    while (i > 0) {
        err = this->transportDevice->readByte(header);
        /*if (err != cOk)
            return err;*/
        i--;
        printf("%d ", header[0]);
        if (header[0] == 0x04)
            break;
    }

    // check command complete event code
	err = this->transportDevice->readByte(header + 1);
	if (err != cOk)
		return err;
	if (header[1] != 0x0E)
		return cError;

    // get parameter total length
	err = this->transportDevice->readByte(header + 2);
	if (err != cOk)
		return err;

    // check size
    if (header[2] < size)
        readLength = header[2];
    else
    	readLength = size;

    // read parameters
    err = this->transportDevice->readBytes(buffer, readLength);

    return err;
}
