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

#include <assembler.h>

#ifndef LEON3_IRQMP
#define LEON3_IRQMP

#define LEON3_IRQMP_BASE	 		0x80000200	// Base address
#define LEON3_IRQMP_MASK_OFFSET		0x40		// interrupt mask register
#define LEON3_IRQMP_CLEAR_OFFSET	0x0C		// interrupt clear register
#define LEON3_IRQMP_IP_OFFSET		0x04		// interrupt pending register
#define LEON3_IRQMP_BROADCAST		0x14		// interrupt broadcast register

/*!
 * \brief Implementation of the Leon3 multiprocessor interrupt controller
 */
class Leon3_IRQMP {

private:

	/*
	 * Used for multiprocessor configuration.
	 * Each cpu has its own force and mask register, which
	 * can be determined by cpu index.
	 */
	int cpuOffset;

public:
    Leon3_IRQMP();
    ~Leon3_IRQMP();

    /*!
     * \clear a given IRQ
     */
    inline void clearIRQ(short irqNum) {
        	OUTW(LEON3_IRQMP_BASE + LEON3_IRQMP_IP_OFFSET, INW(LEON3_IRQMP_BASE + LEON3_IRQMP_IP_OFFSET) & ~(1<<irqNum));
        }

    /*!
     * \brief enable all IRQs
     */
    inline void enableIRQ(int irqNum) {
        OUTW(LEON3_IRQMP_BASE + LEON3_IRQMP_MASK_OFFSET + cpuOffset, INW(LEON3_IRQMP_BASE + LEON3_IRQMP_MASK_OFFSET + cpuOffset) | (1 << irqNum));
    }
};
#endif /* LEON3_IRQMP */
