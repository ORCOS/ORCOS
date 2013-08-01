/*
 * HCI.hh
 *
 *  Created on: 07.11.2012
 *      Author: kgilles
 */

#ifndef HCI_HH_
#define HCI_HH_

#include <SCLConfig.hh>
#include Board_HCI_hh

#define HCI_LINK_CONTROL 0x01
#define HCI_POLICY 0x02
#define HCI_CONTROLLER_BASEBAND 0x03
#define HCI_COMMAND 0x01

/* !
 *\brief HCI control functions
 */
class HCI {
public:
	HCI();
	virtual ~HCI();

	//! HCI commands
	// Host Controller and Baseband Commands
	void reset(unint1 *ret_param);

	void read_Scan_Enable(unint1 *ret_params);
	void read_Inquiry_Scan_Activity(unint1 *ret_params);
	void read_Current_IAC_LAP(unint1 *ret_params);
	unint1 read_Local_Name(char* name, int length);

	unint1 write_Scan_Enable(int inq_en, int page_en);
	void write_Inquiry_Scan_Activity(unint2 duration, unint2 interval);
	void write_Current_IAC_LAP(int address);


	// HCI Policy Commands
	// Link Control Commands
	void inquiry(unint LAP, unint1 length, unint1 num_responses);
	void inquiry_cancel(unint1* ret_params);
	void create_Connection(unint8 bd_addr, unint2 packet_type, unint1 rep_mode, unint1 page_scan_mode, unint2 clock_offset, unint1 allow_role_switch);
	void accept_Connection_Request(unint8 bd_addr, unint1 role);

	// HCI event
	ErrorT command_complete_event(char* buffer, unint1 size);
	int csr(void);

	// HCI_Host_Buffer_Size

private:
	// transport device
	Board_HCICfdT transportDevice;
};

#endif /* HCI_HH_ */
