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
#include <SCLConfig.hh>

#define MODE_GMII   1
#define MODE_RGMII  2
#define MODE_MII    3

typedef struct {
    volatile unint4 tx_idver;            /* 0x00*/
    volatile unint4 tx_control;          /* 0x04*/
    volatile unint4 tx_teardown;         /* 0x08*/
    volatile unint4 reserved1;           /* 0x0c*/
    volatile unint4 rx_idver;            /* 0x10*/
    volatile unint4 rx_control;          /* 0x14*/
    volatile unint4 rx_teardown;         /* 0x18*/
    volatile unint4 cpdma_soft_reset;    /* 0x1c*/
    volatile unint4 dmacontrol;          /* 0x20*/
    volatile unint4 dmastatus;           /* 0x24*/
    volatile unint4 rx_buffer_offset;    /* 0x28*/
    volatile unint4 emcontrol;           /* 0x2c*/
    volatile unint4 tx_pri0_rate;        /* 0x30*/
    volatile unint4 tx_pri1_rate;        /* 0x34*/
    volatile unint4 tx_pri2_rate;        /* 0x38*/
    volatile unint4 tx_pri3_rate;        /* 0x3c*/
    volatile unint4 tx_pri4_rate;        /* 0x40*/
    volatile unint4 tx_pri5_rate;        /* 0x44*/
    volatile unint4 tx_pri6_rate;        /* 0x48*/
    volatile unint4 tx_pri7_rate;        /* 0x4c*/

    volatile unint4 reserved2[11];       /* 0x50 - 0x7c*/

    volatile unint4 tx_intstat_raw;      /* 0x80 */
    volatile unint4 tx_intstat_masked;   /* 0x84*/
    volatile unint4 tx_intmask_set;      /* 0x88*/
    volatile unint4 tx_intmask_clear;    /* 0x8c*/

    volatile unint4 cpdma_in_vector;     /* 0x90*/
    volatile unint4 cpdma_eoi_vector;    /* 0x94*/

    volatile unint4 reserved3[2];        /* 0x98 - 0x9c */

    volatile unint4 rx_intstat_raw;      /* 0xa0 */
    volatile unint4 rx_intstat_masked;   /* 0xa4*/
    volatile unint4 rx_intmask_set;      /* 0xa8*/
    volatile unint4 rx_intmask_clear;    /* 0xac*/

    volatile unint4 dma_intstat_raw;     /* 0xb0*/
    volatile unint4 dma_intstat_masked;  /* 0xb4*/
    volatile unint4 dma_intmask_set;     /* 0xb8*/
    volatile unint4 dma_intmask_clear;   /* 0xbc*/

    volatile unint4 rx0_pendthresh;      /* 0xc0 */
    volatile unint4 rx1_pendthresh;      /* 0xc4 */
    volatile unint4 rx2_pendthresh;      /* 0xc8 */
    volatile unint4 rx3_pendthresh;      /* 0xcc */
    volatile unint4 rx4_pendthresh;      /* 0xd0 */
    volatile unint4 rx5_pendthresh;      /* 0xd4 */
    volatile unint4 rx6_pendthresh;      /* 0xd8 */
    volatile unint4 rx7_pendthresh;      /* 0xdc */

    volatile unint4 rx0_freebuffer;      /* 0xe0 */
    volatile unint4 rx1_freebuffer;      /* 0xe4 */
    volatile unint4 rx2_freebuffer;      /* 0xe8 */
    volatile unint4 rx3_freebuffer;      /* 0xec */
    volatile unint4 rx4_freebuffer;      /* 0xf0 */
    volatile unint4 rx5_freebuffer;      /* 0xf4 */
    volatile unint4 rx6_freebuffer;      /* 0xf8 */
    volatile unint4 rx7_freebuffer;      /* 0xfc */

} cpsw_cpdma_regs_t;

typedef struct {
    volatile unint4 idver;           /* 0x00 */
    volatile unint4 reserved;        /* 0x04 */
    volatile unint4 control;         /* 0x08 */
    volatile unint4 reserved2;       /* 0x0c */
    volatile unint4 prescale;        /* 0x10 */
    volatile unint4 reserved3;       /* 0x14 */
    volatile unint4 unknown_vlan;    /* 0x18 */
    volatile unint4 reserved4;       /* 0x1c */
    volatile unint4 tblctl;          /* 0x20 */

} cpsw_ale_regs_t;

typedef struct {
    volatile unint4 tx_hdp[8];       /* 0x00 - 0x1c */
    volatile unint4 rx_hdp[8];       /* 0x20 - 0x3c */
    volatile unint4 tx_cp[8];        /* 0x40 - 0x5c */
    volatile unint4 rx_cp[8];        /* 0x60 - 0x7c */
} cpsw_stateram_regs_t;

/* structure to access sub regions of CPSW_PORT
 * to be instantiated for P0 at offset 0x0, P1 at offset 0x100, P2 at offset 0x200 */
typedef struct {
    volatile unint4 control;
    volatile unint4 reserved;
    volatile unint4 max_blks;
    volatile unint4 blk_cnt;
    volatile unint4 tx_in_ctl;
    volatile unint4 port_vlan;
    volatile unint4 tx_pri_map;
    volatile unint4 cpdma_tx_pri_map;
    volatile unint4 cpdma_rx_ch_map;
    volatile unint4 reserved2[3];
    volatile unint4 rx_dscp_pri_map0;
    volatile unint4 rx_dscp_pri_map1;
    volatile unint4 rx_dscp_pri_map2;
    volatile unint4 rx_dscp_pri_map3;
    volatile unint4 rx_dscp_pri_map4;
    volatile unint4 rx_dscp_pri_map5;
    volatile unint4 rx_dscp_pri_map6;
    volatile unint4 rx_dscp_pri_map7;
} cpsw_port_regs_t;

typedef struct {
    volatile unint4 idver;
    volatile unint4 maccontrol;
    volatile unint4 macstatus;
    volatile unint4 soft_reset;
    volatile unint4 rx_maxlen;
    volatile unint4 bofftest;
    volatile unint4 rx_pause;
    volatile unint4 tx_Pause;
    volatile unint4 emcontrol;
    volatile unint4 rx_pri_map;
    volatile unint4 tx_gap;
} cpsw_sl_regs_t;

typedef struct {
    volatile unint4 idver;
    volatile unint4 control;
    volatile unint4 soft_reset;
    volatile unint4 stat_port_en;
    volatile unint4 ptype;
    volatile unint4 soft_idle;
    volatile unint4 thru_rate;
    volatile unint4 gap_thresh;
    volatile unint4 tx_start_wds;
    volatile unint4 flow_control;
    volatile unint4 vlan_ltype;
    volatile unint4 ts_ltype;
    volatile unint4 ts_dlr_ltype;
} cpsw_ss_regs_t;

typedef struct {
    volatile unint4 idver;
    volatile unint4 soft_reset;
    volatile unint4 control;
    volatile unint4 int_control;
    volatile unint4 c0_rx_thresh_en;  /* core 0 interrupt receive threshold enable */
    volatile unint4 c0_rx_en;         /* core 0 receive interrupt enable */
    volatile unint4 c0_tx_en;         /* core 0 transmit interrupt enable */
    volatile unint4 c0_misc_en;
} cpsw_wr_regs_t;


typedef struct {
    volatile unint4 good_rxframes;
    volatile unint4 broadcast_rxframes;
    volatile unint4 multicast_rxframes;
    volatile unint4 pause_rxframes;
    volatile unint4 rx_crc_errors;
} cpsw_stats_regs_t;


class AM335xEthernet : public CommDeviceDriver
{
public:
    volatile cpsw_wr_regs_t*         cpsw_wr_regs;
    volatile cpsw_ss_regs_t*         cpsw_ss_regs;
    volatile cpsw_sl_regs_t*         cpsw_sl_regs;
    volatile cpsw_port_regs_t*       cpsw_p0_regs;
    volatile cpsw_stateram_regs_t*   cpsw_stateram_regs;
    volatile cpsw_ale_regs_t*        cpsw_ale_regs;
    volatile cpsw_cpdma_regs_t*      cpsw_dma_regs;
    volatile cpsw_stats_regs_t*      cpsw_stats_regs;

    int intc_irq;
public:
    Mutex* mutex;

    AM335xEthernet(T_AM335xEthernet_Init * init);

    ~AM335xEthernet();

    ErrorT disableIRQ();

    ErrorT enableIRQ();

    ErrorT handleIRQ();

    ErrorT clearIRQ();

    void ale_addAddress(int type, char* address, int port);
};

#endif /* AM335XETHERNET_HH_ */
