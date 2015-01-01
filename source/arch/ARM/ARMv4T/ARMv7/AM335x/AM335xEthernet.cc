/*
 * AM335xEthernet.cc
 *
 *  Created on: 30.12.2014
 *      Author: DanielBa
 */

#include "AM335xEthernet.hh"
#include "lwip/pbuf.h"
#include "netif/etharp.h"
#include "kernel/Kernel.hh"

extern Kernel* theOS;

#define SOP                 (1 << 31)
#define EOP                 (1 << 30)
#define OWNER_MAC           (1 << 29) /* packet is owned by the port */
#define EOQ                 (1 << 28)
#define TERDOWN_COMPLETE    (1 << 27)
#define CRC_PASS            (1 << 26)
#define PORTEN2             (1 << 20)
#define PORTEN1             (1 << 16)
#define PACKET_LEN          (1 << 0)

#ifndef ETH_IP4NETMASK
#define ETH_IP4NETMASK  255,255,255,0
#endif
#define ETH_IP4ADDR     192,168,1,100
#define ETH_IP4ADDR_GW  192,168,1,1

typedef struct cppi_tx_descriptor_t {
    volatile cppi_tx_descriptor_t* next_descr;
    volatile char*  buffer_pointer;
    volatile unint4 buffer_opt;
    volatile unint4 control;
    /* private data following */
    pbuf* p_buf; /* pointer to the pbuf holding the data. to be freed on completion */
} cppi_tx_descriptor_t;

typedef struct cppi_rx_descriptor_t {
    volatile cppi_rx_descriptor_t* next_descr;
    volatile char*  buffer_pointer;
    volatile unint4 buffer_opt;
    volatile unint4 control;
} cppi_rx_descriptor_t;


/* some receive area to be used by the dma to store new packets*/
static char rx_buffers[10][1500] ATTR_CACHE_INHIBIT;

/* descriptors to be used by dma for all ports */
static cppi_tx_descriptor_t txqueue[10] ATTR_CACHE_INHIBIT;
static cppi_rx_descriptor_t rxqueue[10] ATTR_CACHE_INHIBIT;

extern "C" err_t ethernet_input(struct pbuf *p, struct netif *netif);


static err_t low_level_output(struct netif *netif, struct pbuf *p) {

    /* get a free descriptor and send the packet */
    for (int i = 0; i < 10; i++) {
      if (!(txqueue[i].control & OWNER_MAC)) {
          /* this one is free */

          /* free last pbuf used by this descriptor */
          if (txqueue[i].p_buf != 0) {
              pbuf_free(txqueue[i].p_buf);
          }

          txqueue[i].buffer_pointer = (char*) p->payload;
          txqueue[i].buffer_opt     = p->len;
          txqueue[i].control        = SOP | EOP | OWNER_MAC |  p->len;
          txqueue[i].p_buf          = p;
          pbuf_ref(p);
      }
    }

    AM335xEthernet* ethdev = (AM335xEthernet*) netif->state;
    /* restart queue processing */
    ethdev->cpsw_stateram_regs->tx_hdp[0] = (unint4) &txqueue[0];

    return (ERR_OK);
}

static err_t ethernetif_init(struct netif *netif) {

#if LWIP_NETIF_HOSTNAME
    /* Initialize interface hostname */
    netif->hostname = "orcos";
#endif /* LWIP_NETIF_HOSTNAME */

    netif->name[0] = 'E';
    netif->name[1] = '0';

    /* We directly use etharp_output() here to save a function call.
     * You can instead declare your own function an call etharp_output()
     * from it if you have to do some checks before sending (e.g. if link
     * is available...) */
#if LWIP_ARP
    netif->ip4_output = etharp_output;
#else
#warning "AM335xEthernet device driver non operational without ARP support."
#endif
    netif->input      = ethernet_input;
    netif->linkoutput = low_level_output;
    netif->hwaddr_len = 6;

    // mac address is actually set by internal EFUSE registers (read only)
    // we may read them using the Control Module
    // mac_id0_lo Register (offset = 630h) [reset = device specific]
    // mac_id0_hi Register (offset = 634h) [reset = device specific]

  /*  for (int i = 0; i < netif->hwaddr_len; i++)
    {
       netif->hwaddr[i] = default_macaddr[i];
    }*/

   /* maximum transfer unit */
   netif->mtu = 1400;

   /* device capabilities */
   netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
   return (cOk);
}

AM335xEthernet::AM335xEthernet() : CommDeviceDriver("eth0")
{
    /*
     *  Step1 : Board initialization code must already have set the MII / GMII / RGMII mode of
     *          the ETH subsystem using the Control Module!
     *  Step2 : Pin muxing must be done before as well as this is board specific
     *  Step3 : Clocks must be provided as well as this is SOC specific
     *
     */

    //set pointers to hw regs using the init struct

    // Step4: Apply soft reset.

    cpsw_dma_regs->cpdma_soft_reset = 1;
    cpsw_ss_regs->soft_reset = 1;
    cpsw_sl_regs->soft_reset = 1;
    cpsw_wr_regs->soft_reset = 1;

    // TODO show ID versions

    // TODO wait for reset done

    /* Step5: initialize HDPs and CPs to NULL */
    memset(cpsw_stateram_regs,0,sizeof(cpsw_stateram_regs_t));
    memset(txqueue,0,sizeof(txqueue));
    memset(rxqueue,0,sizeof(rxqueue));

    /* initialize rx queue */
    for (int i = 0; i < 10; i++) {
        rxqueue[i].next_descr     = &rxqueue[i+1];
        rxqueue[i].buffer_pointer = rx_buffers[i];
        rxqueue[i].buffer_opt     = 1500;
        /* give ownership of this buffer to the hw */
        rxqueue[i].control        = OWNER_MAC;
    }
    /* last descriptor points to first one again */
    rxqueue[9].next_descr         = &rxqueue[0];


    /* set rx descriptor in hw register to the first one */
    cpsw_stateram_regs->rx_hdp[0] = (unint4) &rxqueue[0];

    /* initialize tx queue */
    for (int i = 0; i < 10; i++) {
          rxqueue[i].next_descr     = &rxqueue[i+1];
          rxqueue[i].buffer_pointer = 0;
          rxqueue[i].buffer_opt     = 0;
          /*nothin to transmit yet so do not let the hw own this descriptor */
          txqueue[i].control        = 0;
    }
    /* last descriptor ends here */
    txqueue[9].next_descr         = 0;
    txqueue[9].control            = EOP;

    cpsw_stateram_regs->tx_hdp[0] = (unint4) &txqueue[0];

    /* Step6: Configure Interrupts*/
    /* allow all interrupts */
    cpsw_dma_regs->rx_intmask_set = 0xff;
    /* route all interrupts to core 0 */
    cpsw_wr_regs->c0_rx_en        = 0xf;
    /* register at interrupt manager */
    intc_irq = init->INTC_IRQ;
    theOS->getInterruptManager()->registerIRQ(intc_irq,this,5000,0);
    /* unmask the irq to let it pass through */
    theOS->getBoard()->getInterruptController()->setIRQPriority(intc_irq,init->INTC_Priority);
    theOS->getBoard()->getInterruptController()->unmaskIRQ(intc_irq);

    /* configure control registers */
    cpsw_wr_regs->control       = (1 << 0) | (1 << 2);  /* no idle no standby mode */
    cpsw_ss_regs->stat_port_en  = 1;                    /* enable statistics on port 0 */
    cpsw_sl_regs->maccontrol    = 1;                    /* enable full duplex mode*/


    struct ip4_addr tgwAddr;
    struct ip4_addr eth_nm;
    struct ip4_addr tIpAddr;

    int addr[4] = {ETH_IP4NETMASK};
    IP4_ADDR(&eth_nm, addr[0], addr[1], addr[2], addr[3]);
    int addr2[4] = {ETH_IP4ADDR};
    IP4_ADDR(&tIpAddr, addr2[0], addr2[1], addr2[2], addr2[3]);
    int addr3[4] = {ETH_IP4ADDR_GW};
    IP4_ADDR(&tgwAddr, addr3[0], addr3[1], addr3[2], addr3[3]);

    /* save driver in netif as state */
    netif_add(&tEMAC0Netif, &tIpAddr, &eth_nm, &tgwAddr, (void*) this, &ethernetif_init, 0);
    netif_set_default(&tEMAC0Netif);

    /* enable tx and rx dma */
    cpsw_dma_regs->tx_control = 1;
    cpsw_dma_regs->rx_control = 1;

}

AM335xEthernet::~AM335xEthernet()
{

}

ErrorT AM335xEthernet::disableIRQ() {
    theOS->getBoard()->getInterruptController()->maskIRQ(intc_irq);
}


ErrorT AM335xEthernet::enableIRQ() {
    theOS->getBoard()->getInterruptController()->unmaskIRQ(intc_irq);
}

ErrorT AM335xEthernet::handleIRQ() {

    /* scan rx queue and process the data by passing it to the
     * network stack. afterwards ack packet*/

    for (int i = 0; i < 10; i++) {
        if (!(rxqueue[i].control & OWNER_MAC)) {
            /* queue is owned by host.. must be some data in it */
            int packet_len = rxqueue[i].control & 0x7ff;

            /* copy data from dma buffer to internal packet buffer.
             * Takes time but allows HW to receive new packets inside this buffer. */
            struct pbuf* ptBuf = pbuf_alloc(PBUF_RAW, (unint2) ( packet_len+10), PBUF_RAM);
            if (ptBuf != 0) {
                memcpy(ptBuf->payload, &rxqueue[i].buffer_pointer, packet_len);
                ethernet_input(ptBuf,&tEMAC0Netif);
                pbuf_free(ptBuf);
            }

            rxqueue[i].control |= OWNER_MAC;
            /* ACK packet. */
            cpsw_stateram_regs->rx_cp[0] = (unint4) &rxqueue[i];
        }
    }
    cpsw_dma_regs->cpdma_eoi_vector = 1;

    return (cOk);
}

