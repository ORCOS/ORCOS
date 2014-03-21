

/*  <...> includes
 *********************************************************************/


/*  "..." includes
 *********************************************************************/
#include "kernel/Kernel.hh"


#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "netif/etharp.h"
#include "lwip_ethernetif.hh"
#include "lwip/netif.h"
#include "memtools.hh"
#include "lwip/stats.h"

extern Kernel* theOS;

extern "C" err_t ethernet_input(struct pbuf *p, struct netif *netif);

/*  defines
 *********************************************************************/

/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'n'

/*  global variables
 *********************************************************************/

static unsigned char ETH_LocalAddress[6] = { Board_ETH_UNIQUEID };

/*  function implementations
 *********************************************************************/

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void low_level_init(struct netif *netif) {
	int i;
    for (i = 0; i < netif->hwaddr_len; i++)
    {
        netif->hwaddr[i] = ETH_LocalAddress[i];
    }

    /* maximum transfer unit */
    netif->mtu = 1500; //MAX_FRAME_SIZE;

    /* device capabilities */
    /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP
            | NETIF_FLAG_LINK_UP;
}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become availale since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */

static char data[1500];

static err_t low_level_output(struct netif *netif, struct pbuf *p) {

	if (p->tot_len > 1400) {
		//pbuf_free(p);
		printf("ERROR len > 1400!\n");
		return ERR_MEM;
	}

	int pos = 0;

	struct pbuf *curp = p;

	while (curp != 0) {
		memcpy(&data[pos],curp->payload,curp->len);

		pos  += curp->len;
		curp = curp->next;
	}

	if (theOS->getBoard()->getETH() != 0)
		theOS->getBoard()->getETH()->lowlevel_send((char*)data,pos);

    LINK_STATS_INC(link.xmit);

    return ERR_OK; //cOk == ERR_OK
}

#if !LWIP_ARP
err_t
dummy_output(struct netif *netif, struct pbuf *q, struct ip4_addr *ipaddr) {
	return ERR_OK;
}
#endif

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t ethernetif_init(struct netif *netif) {

#if LWIP_NETIF_HOSTNAME
    /* Initialize interface hostname */
    netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

    /*
     * Initialize the snmp variables and counters inside the struct netif.
     * The last argument should be replaced with your link speed, in units
     * of bits per second.
     */
    //NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, ???);

    netif->name[0] = IFNAME0;
    netif->name[1] = IFNAME1;
    /* We directly use etharp_output() here to save a function call.
     * You can instead declare your own function an call etharp_output()
     * from it if you have to do some checks before sending (e.g. if link
     * is available...) */
#if LWIP_ARP
    netif->ip4_output = etharp_output;
#else
    netif->ip4_output = dummy_output;
#endif
    netif->input = ethernet_input;
    netif->linkoutput = low_level_output;

    netif->hwaddr_len = 6;

    /* initialize the hardware */
    low_level_init(netif);

    return ERR_OK;
}



