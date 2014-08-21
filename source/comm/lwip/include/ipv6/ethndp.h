#ifndef ETHNDP_H_
#define ETHNDP_H_

err_t ethndp_request(struct netif *netif, struct ip6_addr *ipaddr);

err_t ethndp_output(struct netif *netif, struct pbuf *q, struct ip6_addr *ipaddr);

#endif /* ETHNDP_H_ */
