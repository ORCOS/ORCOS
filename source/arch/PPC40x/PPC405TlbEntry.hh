/*
 ORCOS - an Organic Reconfigurable Operating System
 Copyright (C) 2008 University of Paderborn

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

#ifndef PPC405TLBENTRY_HH_
#define PPC405TLBENTRY_HH_

#include <types.hh>
#include <BigEndBitmap.hh>

/*!
 * \brief This class provides a data structure to represent a proper TLB entry
 * for the PowerPC405 architecture
 *
 * Such a TLB entry consists of three parts: the TaskID for the entry (to distingiush between different processes),
 * the TAG portion and the DATA portion. This class provides methods to set and get the values of the different fields
 * and flags which are inside the TAG and DATA portions.
 * For the meanings of the single fields and flags, read the PPC405 User Manual
 */
class PPC405TlbEntry {
private:
    //! Bitmap (Big Endian) representing the TAG-Portion of the TLB Entry
    BigEndBitmap tag;
    //! Bitmap (Big Endian) representing the DATA-Portion of the TLB Entry
    BigEndBitmap data;
    //! The TaskID of the TLB Entry (indicating to which process the mapping belongs)
    TaskIdT procID;
public:
    //! standard constructor
    PPC405TlbEntry() {
    }
    //! standard destructor
    ~PPC405TlbEntry() {
    }

    //! Sets the TID field of the TLB Entry
    void setProcID(TaskIdT pid) {
        procID = pid;
    }
    //! Gets the TID field of the TLB Entry
    TaskIdT getProcID() {
        return procID;
    }

    //! Gets the TAG-Portion of the TLB Entry
    BitmapT getTag() {
        return tag;
    }
    //! Sets the complete TAG-Portion of the TLB Entry
    void setTag(BitmapT b) {
        tag = b;
    }

    //! Sets the EPN field(= the logical address) of the TAG-Portion
    void setEffectivePage(BitmapT);
    //! Gets the EPN field(= the logical address) of the TAG-Portion
    BitmapT getEffectivePage();

    //! Sets the SIZE field of the TAG-Portion
    void setSize(byte s) {
        tag.clearBits(0x380);
        tag.setBits(s << 7);
    }
    //! Gets the SIZE field of the TAG-Portion
    byte getSize() {
        return tag >> 7 & 0x7;
    }

    //! Sets the V flag of the TAG-Portion to valid (=1)
    void setValid() {
        tag.setBit(25);
    }
    //! Sets the V flag of the TAG-Portion to not valid (=0)
    void setNotValid() {
        tag.clearBit(25);
    }
    //! Checks if V flag of the TAG-Portion is set to valid
    bool isValid() {
        return tag.isSet(25);
    }

    //! Sets the E flag of the TAG-Portion to little-endian (=1)
    void setLittleEndian() {
        tag.setBit(26);
    }
    //! Sets the E flag of the TAG-Portion to big-endian (=0)
    void setBigEndian() {
        tag.clearBit(26);
    }
    //! Checks if the E flag of the TAG-Portion is set to little-endian
    bool isLittleEndian() {
        return tag.isSet(26);
    }
    //! Checks if the E flag of the TAG-Portion is set to big-endian
    bool isBigEndian() {
        return !isLittleEndian();
    }

    //! Sets the U0 flag of the TAG-Portion to 1
    void setUserDef() {
        tag.setBit(27);
    }
    //! Sets the U0 flag of the TAG-Portion to 0
    void setNotUserDef() {
        tag.clearBit(27);
    }
    //! Checks if U0 flag is set
    bool isUserDef() {
        return tag.isSet(27);
    }

    //! Gets the DATA-Portion of the TLB Entry
    BitmapT getData() {
        return data;
    }
    //! Sets the complete DATA-Portion of the TLB Entry
    void setData(BitmapT b) {
        data = b;
    }

    //! Sets the RPN field (= physical address) of the DATA-Portion
    void setRealPage(BitmapT);
    //! Gets the RPN field (= physical address) of the DATA-Portion
    BitmapT getRealPage();

    //! Sets the EX flag of the DATA-Portion (= Rights:Set to executable)
    void setExecutable() {
        data.setBit(22);
    }
    //! Deletes the EX flag of the DATA-Portion (= Rights: Set to non-executable)
    void setNotExecutable() {
        data.clearBit(22);
    }
    //! Check if EX flag is set (=Rights: is executable?)
    bool isExecutable() {
        return data.isSet(22);
    }

    //! Sets the WR flag of the DATA-Portion (= Rights: Set to writable)
    void setWritable() {
        data.setBit(23);
    }
    //! Deletes the WR flag of the DATA-Portion (= Rights: Set to non-writable)
    void setNotWritable() {
        data.clearBit(23);
    }
    //! Check if WR flag is set (= Rights: is writable?)
    bool isWritable() {
        return data.isSet(23);
    }

    //! Sets the ZSEL field of the DATA-Portion
    void setZoneSelect(byte s) {
        data.clearBits(0xF0);
        data.setBits(s << 4);
    }
    //! Gets the ZSEL field of the DATA-Portion
    byte getZoneSelect() {
        return data >> 4 & 0xF;
    }

    //! Sets the W flag of the DATA-Portion
    void setWriteThrough() {
        data.setBit(28);
    }
    //! Deletes the W flag of the DATA-Portion
    void setNoWriteThrough() {
        data.clearBit(28);
    }
    //! Checks if the W flag of the DATA-Portion is set
    bool isWriteThrough() {
        return data.isSet(28);
    }

    //! Sets the I flag of the DATA-Portion
    void setCacheInhibit() {
        data.setBit(29);
    }
    //! Deletes the I flag of the DATA-Portion
    void setNoCacheInhibit() {
        data.clearBit(29);
    }
    //! Checks if the I flag of the DATA-Portion is set
    bool isCacheInhibit() {
        return data.isSet(29);
    }

    //! Sets the G flag of the DATA-Portion
    void setGuarded() {
        data.setBit(31);
    }
    //! Deletes the G flag of the DATA-Portion
    void setNotGuarded() {
        data.clearBit(31);
    }
    //! Checks if the G flag of the DATA-Portion is set
    bool isGuarded() {
        return data.isSet(31);
    }
};

#endif /*PPC405TLBENTRY_HH_*/
