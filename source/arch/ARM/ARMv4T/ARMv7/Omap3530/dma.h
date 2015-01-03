/*
 * dma.h
 *
 *  Created on: 20.02.2014
 *    Copyright & Author: dbaldin
 */

#ifndef SOURCE_ARCH_ARM_ARMV4T_ARMV7_OMAP3530_DMA_H_
#define SOURCE_ARCH_ARM_ARMV4T_ARMV7_OMAP3530_DMA_H_

// TODO: create dms class that is initialized with base address to portable
// handle dma transfers

#define DMA4_CSDP(Channel) (*((volatile unsigned int *) (0x48056090 + 0x60*Channel)))
#define DMA4_CEN(Channel)  (*((volatile unsigned int *) (0x48056094 + 0x60*Channel)))
#define DMA4_CFN(Channel)  (*((volatile unsigned int *) (0x48056098 + 0x60*Channel)))
#define DMA4_CSSA(Channel) (*((volatile unsigned int *) (0x4805609C + 0x60*Channel)))
#define DMA4_CDSA(Channel) (*((volatile unsigned int *) (0x480560A0 + 0x60*Channel)))
#define DMA4_CCR(Channel)  (*((volatile unsigned int *) (0x48056080 + 0x60*Channel)))
#define DMA4_CSEI(Channel) (*((volatile unsigned int *) (0x480560A4 + 0x60*Channel)))
#define DMA4_CSFI(Channel) (*((volatile unsigned int *) (0x480560A8 + 0x60*Channel)))
#define DMA4_CDEI(Channel) (*((volatile unsigned int *) (0x480560AC + 0x60*Channel)))
#define DMA4_CDFI(Channel) (*((volatile unsigned int *) (0x480560B0 + 0x60*Channel)))

#endif // SOURCE_ARCH_ARM_ARMV4T_ARMV7_OMAP3530_DMA_H_
