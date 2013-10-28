/*
 ORCOS - an Organic Reconfigurable Operating System
 Copyright (C) 2011 University of Paderborn

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ARMv7PtEntry_HH_
#define ARMv7PTEntry_HH_

#include <types.hh>
#include <Bitmap.hh>

#define ptTypeSection 0
#define ptTypeSuperSection 2

/*!
 * \brief This class provides a data structure to represent a proper page table entry
 * for the ARMv7 architecture
 */
class ARMv7PtEntry {
	// supersection 16MB, direct phys. address, no L2 table
	// section 1MB, direct phys. address, no L2 table
	// pagetable: points to L2 table containing small or large page (not used for now)
private:
	Bitmap ptL1Descriptor;

public:
	//! standard constructor
	ARMv7PtEntry() {
	}
	//! standard destructor
	~ARMv7PtEntry() {
	}

	Bitmap getDesc(void)
	{
		return ptL1Descriptor;
	}

	void setType(int type){
		switch (type)
		{
			case ptTypeSection:
			{
				ptL1Descriptor.clearBits( 0x40003 );
				ptL1Descriptor.setBits( 0x2 );
				break;
			}
			case ptTypeSuperSection:
			{
				ptL1Descriptor.clearBits( 0x40003 );
				ptL1Descriptor.setBits( 0x40002 );
				break;
			}
			default:
				break;
		}
	}
	void setDomain(int dom){
		ptL1Descriptor.clearBits( 0x1E0 );
		ptL1Descriptor.setBits( dom << 5);
	}
	void setCBit(int value){
		ptL1Descriptor.clearBits( 1 << 3 );
		ptL1Descriptor.setBits( (value & 0x1) << 3);
	}
	void setBBit(int value){
		ptL1Descriptor.clearBits( 1 << 2 );
		ptL1Descriptor.setBits( (value & 0x1) << 2);
	}
	void setSBit(){

	}
	void setXNBit(Bitmap permission){
		ptL1Descriptor.clearBits( 0x10 );
		ptL1Descriptor.setBits( permission << 4);
	}
	void setNSBit(){

	}
	void setTex(int value){
		ptL1Descriptor.clearBits( 0x7 << 12);
		ptL1Descriptor.setBits( (value & 0x7) << 12);
	}
	void setAP(Bitmap permission){
		ptL1Descriptor.clearBits( 0xC00 );
		ptL1Descriptor.setBits( permission << 10);
	}
	void setnGBit(bool nonGlobal){
		ptL1Descriptor.clearBits( 0x20000 );
		ptL1Descriptor.setBits( nonGlobal << 17);
	}
	void setBaseAddr(int type, void* physAddr){
		unint addr = ((unint)physAddr) >> 20;
		switch (type)
		{
			case ptTypeSection:
			{
				// use 12 msb
				ptL1Descriptor.clearBits( 0xFFF00000 );
				ptL1Descriptor.setBits( addr << 20);
				break;
			}
			case ptTypeSuperSection:
			{

				break;
			}
			default:
				break;
		}

	}
	void setSuperSectionBaseAddr(){
		// only [31:24], extended base address not supported
	}
};


#endif /* ARMv7PtEntry_HH_ */
