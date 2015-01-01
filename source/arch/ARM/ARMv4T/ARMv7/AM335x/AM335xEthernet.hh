/*
 * AM335xEthernet.hh
 *
 *  Created on: 30.12.2014
 *      Author: DanielBa
 */

#ifndef AM335XETHERNET_HH_
#define AM335XETHERNET_HH_

#include <hal/CommDeviceDriver.hh>
#include "lwip/netif.h"

#define MODE_GMII   1
#define MODE_RGMII  2
#define MODE_MII    3

typedef struct {
    unint4 tx_idver;            /* 0x00*/
    unint4 tx_control;          /* 0x04*/
    unint4 tx_teardown;         /* 0x08*/
    unint4 reserved1;           /* 0x0c*/
    unint4 rx_idver;            /* 0x10*/
    unint4 rx_control;          /* 0x14*/
    unint4 rx_teardown;         /* 0x18*/
    unint4 cpdma_soft_reset;    /* 0x1c*/
    unint4 dmacontrol;          /* 0x20*/
    unint4 dmastatus;           /* 0x24*/
    unint4 rx_buffer_offset;    /* 0x28*/
    unint4 emcontrol;           /* 0x2c*/
    unint4 tx_pri0_rate;        /* 0x30*/
    unint4 tx_pri1_rate;        /* 0x34*/
    unint4 tx_pri2_rate;        /* 0x38*/
    unint4 tx_pri3_rate;        /* 0x3c*/
    unint4 tx_pri4_rate;        /* 0x40*/
    unint4 tx_pri5_rate;        /* 0x44*/
    unint4 tx_pri6_rate;        /* 0x48*/
    unint4 tx_pri7_rate;        /* 0x4c*/

    unint4 reserved2[11];       /* 0x50 - 0x7c*/

    unint4 tx_intstat_raw;      /* 0x80 */
    unint4 tx_intstat_masked;   /* 0x84*/
    unint4 tx_intmask_set;      /* 0x88*/
    unint4 tx_intmask_clear;    /* 0x8c*/

    unint4 cpdma_in_vector;     /* 0x90*/
    unint4 cpdma_eoi_vector;    /* 0x94*/

    unint4 reserved3[2];        /* 0x98 - 0x9c */

    unint4 rx_intstat_raw;      /* 0xa0 */
    unint4 rx_intstat_masked;   /* 0xa4*/
    unint4 rx_intmask_set;      /* 0xa8*/
    unint4 rx_intmask_clear;    /* 0xac*/

    unint4 dma_intstat_raw;     /* 0xb0*/
    unint4 dma_intstat_masked;  /* 0xb4*/
    unint4 dma_intmask_set;     /* 0xb8*/
    unint4 dma_intmask_clear;   /* 0xbc*/

    unint4 rx0_pendthresh;      /* 0xc0 */
    unint4 rx1_pendthresh;      /* 0xc4 */
    unint4 rx2_pendthresh;      /* 0xc8 */
    unint4 rx3_pendthresh;      /* 0xcc */
    unint4 rx4_pendthresh;      /* 0xd0 */
    unint4 rx5_pendthresh;      /* 0xd4 */
    unint4 rx6_pendthresh;      /* 0xd8 */
    unint4 rx7_pendthresh;      /* 0xdc */

    unint4 rx0_freebuffer;      /* 0xe0 */
    unint4 rx1_freebuffer;      /* 0xe4 */
    unint4 rx2_freebuffer;      /* 0xe8 */
    unint4 rx3_freebuffer;      /* 0xec */
    unint4 rx4_freebuffer;      /* 0xf0 */
    unint4 rx5_freebuffer;      /* 0xf4 */
    unint4 rx6_freebuffer;      /* 0xf8 */
    unint4 rx7_freebuffer;      /* 0xfc */

} cpsw_cpdma_regs_t;

typedef struct {
    unint4 idver;           /* 0x00 */
    unint4 reserved;        /* 0x04 */
    unint4 control;         /* 0x08 */
    unint4 reserved2;       /* 0x0c */
    unint4 prescale;        /* 0x10 */
    unint4 reserved3;       /* 0x14 */
    unint4 unknown_vlan;    /* 0x18 */
    unint4 reserved4;       /* 0x1c */
    unint4 tblctl;          /* 0x20 */

} cpsw_ale_regs_t;

typedef struct {
    unint4 tx_hdp[8];       /* 0x00 - 0x1c */
    unint4 rx_hdp[8];       /* 0x20 - 0x3c */
    unint4 tx_cp[8];        /* 0x40 - 0x5c */
    unint4 rx_cp[8];        /* 0x60 - 0x7c */
} cpsw_stateram_regs_t;

/* structure to access sub regions of CPSW_PORT
 * to be instantiated for P0 at offset 0x0, P1 at offset 0x100, P2 at offset 0x200 */
typedef struct {
    unint4 control;
    unint4 reserved;
    unint4 max_blks;
    unint4 blk_cnt;
    unint4 tx_in_ctl;
    unint4 port_vlan;
    unint4 tx_pri_map;
    unint4 cpdma_tx_pri_map;
    unint4 cpdma_rx_ch_map;
    unint4 reserved2[3];
    unint4 rx_dscp_pri_map0;
    unint4 rx_dscp_pri_map1;
    unint4 rx_dscp_pri_map2;
    unint4 rx_dscp_pri_map3;
    unint4 rx_dscp_pri_map4;
    unint4 rx_dscp_pri_map5;
    unint4 rx_dscp_pri_map6;
    unint4 rx_dscp_pri_map7;
} cpsw_port_regs_t;

typedef struct {
   unint4 idver;
   unint4 maccontrol;
   unint4 macstatus;
   unint4 soft_reset;
   unint4 rx_maxlen;
   unint4 bofftest;
   unint4 rx_pause;
   unint4 tx_Pause;
   unint4 emcontrol;
   unint4 rx_pri_map;
   unint4 tx_gap;
} cpsw_sl_regs_t;

typedef struct {
   unint4 idver;
   unint4 control;
   unint4 soft_reset;
   unint4 stat_port_en;
   unint4 ptype;
   unint4 soft_idle;
   unint4 thru_rate;
   unint4 gap_thresh;
   unint4 tx_start_wds;
   unint4 flow_control;
   unint4 vlan_ltype;
   unint4 ts_ltype;
   unint4 ts_dlr_ltype;
} cpsw_ss_regs_t;

typedef struct {
   unint4 idver;
   unint4 soft_reset;
   unint4 control;
   unint4 int_control;
   unint4 c0_rx_thresh_en;  /* core 0 interrupt receive threshold enable */
   unint4 c0_rx_en;         /* core 0 receive interrupt enable */
   unint4 c0_tx_en;         /* core 0 transmit interrupt enable */
   unint4 c0_misc_en;
} cpsw_wr_regs_t;

class AM335xEthernet : public CommDeviceDriver
{
public:
    cpsw_wr_regs_t*         cpsw_wr_regs;
    cpsw_ss_regs_t*         cpsw_ss_regs;
    cpsw_sl_regs_t*         cpsw_sl_regs;
    cpsw_port_regs_t*       cpsw_p0_regs;
    cpsw_port_regs_t*       cpsw_p1_regs;
    cpsw_port_regs_t*       cpsw_p2_regs;
    cpsw_stateram_regs_t*   cpsw_stateram_regs;
    cpsw_ale_regs_t*        cpsw_ale_regs;
    cpsw_cpdma_regs_t*      cpsw_dma_regs;

    struct netif tEMAC0Netif;
    int intc_irq;
public:
    explicit AM335xEthernet();

    ~AM335xEthernet();

    ErrorT disableIRQ();

    ErrorT enableIRQ();

    ErrorT handleIRQ();
};

#endif /* AM335XETHERNET_HH_ */
