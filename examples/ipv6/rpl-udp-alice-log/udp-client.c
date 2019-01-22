#include "contiki.h"
#include "lib/random.h"
#include "sys/ctimer.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ip/uip-udp-packet.h"
#include "sys/ctimer.h"
#if WITH_COMPOWER
#include "powertrace.h"
#endif
#include <stdio.h>
#include <string.h>

#include "dev/serial-line.h"
#include "net/ipv6/uip-ds6-route.h"

#include "node-id.h"
#include "net/rpl/rpl.h"
#include "net/mac/tsch/tsch.h"
#include "net/rpl/rpl-private.h"
#include "net/link-stats.h"
//#include "dev/temperature-sensor.h"
//#include "board.h"

#if WITH_ORCHESTRA
#include "orchestra.h"
#endif /* WITH_ORCHESTRA */



#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

#define UDP_EXAMPLE_ID  190

#define DEBUG DEBUG_FULL
#include "net/ip/uip-debug.h"


#define MAX_PAYLOAD_LEN		100 




#define CONFIG_VIA_BUTTON PLATFORM_HAS_BUTTON
#if CONFIG_VIA_BUTTON
#include "button-sensor.h"
#endif /* CONFIG_VIA_BUTTON */


static struct uip_udp_conn *client_conn;
static uip_ipaddr_t server_ipaddr;

/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client process");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/
static uint32_t seq_id;
static uint32_t reply;

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

clock_time_t current_time=0;
clock_time_t current_time1=0;
clock_time_t current_time2=0;

uint16_t asn_time=0;
uint16_t asn_time1=0;
uint16_t asn_time2=0;


uint32_t delay_SC=300001; //server->client delay


static void
tcpip_handler(void)
{




 char *appdata;




  char *str;
  current_time1 = clock_time();
  asn_time1 = (uint16_t) current_asn.ls4b;

  if(uip_newdata()) {
    str = (char *)uip_appdata;
    str[uip_datalen()] = 0; 
    reply++;
    uint16_t sv_snt = (uint16_t)atoi(str); //server sent time

    if(sv_snt <= (uint16_t) asn_time1) {
      delay_SC=(uint16_t)asn_time1 - sv_snt; 
    }

    printf("D rxvc %s (s:%u, r:%u) at %u delay: %u\n", str, seq_id, reply, sv_snt, delay_SC);  
    
  }
}

/*---------------------------------------------------------------------------*/
static void
send_packet(void *ptr)
{

printf("send_packet()\n");
  rpl_instance_t *instance =rpl_get_default_instance();
  
  char buf[MAX_PAYLOAD_LEN];
  current_time2 = clock_time(); //printf("%u\n", current_time2);
  asn_time2 = (uint16_t) current_asn.ls4b; //printf("%u\n", asn_time2);

#ifdef SERVER_REPLY
  uint8_t num_used = 0;
  uip_ds6_nbr_t *nbr;

  nbr = nbr_table_head(ds6_neighbors);
  while(nbr != NULL) {
    nbr = nbr_table_next(ds6_neighbors, nbr);
    num_used++;
  }

#endif /* SERVER_REPLY */

  seq_id++;


  PRINTF("Data: send to %d %d Message Hello %d\n",server_ipaddr.u8[sizeof(server_ipaddr.u8) - 2], server_ipaddr.u8[sizeof(server_ipaddr.u8) - 1], seq_id);  //modified


int sz = sizeof(UIP_IP_BUF->srcipaddr.u8);

#if WITH_COMPOWER

  sprintf(buf, "%u %u %u %u %u %u %d %d %d %d %d %d 0 %d %d %d %u %u %u %u %u", seq_id, reply, asn_time2, delay_SC, rpl_get_parent_ipaddr(instance->current_dag->preferred_parent)->u8[sz-2], rpl_get_parent_ipaddr(instance->current_dag->preferred_parent)->u8[sz-1],    mac_tx_up_ok_counter,  mac_tx_up_error_counter, mac_tx_down_ok_counter, mac_tx_down_error_counter, tsch_queue_overflow, rpl_get_parent_link_stats(instance->current_dag->preferred_parent)->etx, dc_radio, dc_tx, dc_listen, num_pktdrop_queue, num_pktdrop_mac, num_pktdrop_rpl, num_dis+num_dio+num_dao+num_dao_ack, num_parent_switch);

#else

  sprintf(buf, "%u %u %u %u %d %d %d %d %d %d %d %d %d %d %d %d %d %d", seq_id, reply, asn_time2, delay_SC, rpl_get_parent_ipaddr(instance->current_dag->preferred_parent)->u8[sz-2], rpl_get_parent_ipaddr(instance->current_dag->preferred_parent)->u8[sz-1],    mac_tx_up_ok_counter, mac_tx_up_collision_counter, mac_tx_up_noack_counter, mac_tx_up_deferred_counter, mac_tx_up_err_counter, mac_tx_up_err_fatal_counter,      mac_tx_down_ok_counter, mac_tx_down_collision_counter, mac_tx_down_noack_counter, mac_tx_down_deferred_counter, mac_tx_down_err_counter, mac_tx_down_err_fatal_counter);

#endif


  uip_udp_packet_sendto(client_conn, buf, strlen(buf), &server_ipaddr, UIP_HTONS(UDP_SERVER_PORT));
  delay_SC=300001; //reset e2e latency
  printf("sent msg: %s \n",buf);

}
/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
  int i;
  uint8_t state;
  PRINTF("\n------------------------------------\n");
  PRINTF("Client IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTF("\n");
      /* hack to make address "final" */
      if (state == ADDR_TENTATIVE) {
	uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
      }
    }
  }
  PRINTF("------------------------------------\n\n");
}
/*---------------------------------------------------------------------------*/
static void
net_init(uip_ipaddr_t *br_prefix)
{
  uip_ipaddr_t global_ipaddr;

  if(br_prefix) { /* We are RPL root. Will be set automatically
                     as TSCH pan coordinator via the tsch-rpl module */
    memcpy(&global_ipaddr, br_prefix, 16);
    uip_ds6_set_addr_iid(&global_ipaddr, &uip_lladdr);
    uip_ds6_addr_add(&global_ipaddr, 0, ADDR_AUTOCONF);
    rpl_set_root(RPL_DEFAULT_INSTANCE, &global_ipaddr);
    rpl_set_prefix(rpl_get_any_dag(), br_prefix, 64);
    rpl_repair_root(RPL_DEFAULT_INSTANCE);
  }

  NETSTACK_MAC.on();
}
/*---------------------------------------------------------------------------*/
static void
set_global_address(void)
{
  uip_ipaddr_t ipaddr;
  uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);
  uip_ip6addr(&server_ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0x00ff, 0xfe00, 1);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer et;
  static struct etimer periodic;
 // static struct etimer periodic_parentCheck;
  static struct etimer startAfter;  
  static struct ctimer backoff_timer;
 // static struct ctimer backoff_timer_parentCheck;
#if WITH_COMPOWER
  static int print = 0;
#endif

  PROCESS_BEGIN();
#if WITH_COMPOWER
  powertrace_start(CLOCK_SECOND*60);
#endif
  PROCESS_PAUSE();

  set_global_address();

  PRINTF("UDP client process started nbr:%d routes:%d\n",
         NBR_TABLE_CONF_MAX_NEIGHBORS, UIP_CONF_MAX_ROUTES);

  print_local_addresses();

  /* new connection with remote host */
  client_conn = udp_new(NULL, UIP_HTONS(UDP_SERVER_PORT), NULL); 
  if(client_conn == NULL) {
    PRINTF("No UDP connection available, exiting the process!\n");
    PROCESS_EXIT();
  }
  udp_bind(client_conn, UIP_HTONS(UDP_CLIENT_PORT)); 

  PRINTF("Created a connection with the server ");
  PRINT6ADDR(&client_conn->ripaddr);
  PRINTF(" local/remote port %u/%u\n",
	UIP_HTONS(client_conn->lport), UIP_HTONS(client_conn->rport));

#if WITH_COMPOWER
  //powertrace_sniff(POWERTRACE_ON);
#endif






  static int is_coordinator = 0;
  static enum { role_6ln, role_6dr, role_6dr_sec } node_role;
  node_role = role_6ln;

  int coordinator_candidate = 0;

#ifdef CONTIKI_TARGET_Z1
  /* Set node with MAC address c1:0c:00:00:00:00:01 as coordinator,
   * convenient in cooja for regression tests using z1 nodes
   * */
  extern unsigned char node_mac[8];
  unsigned char coordinator_mac[8] = { 0xc1, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };

  coordinator_candidate = (memcmp(node_mac, coordinator_mac, 8) == 0);
#elif CONTIKI_TARGET_COOJA
  coordinator_candidate = (node_id == 1);
#endif

  if(coordinator_candidate) {
    if(LLSEC802154_ENABLED) {
      node_role = role_6dr_sec;
    } else {
      node_role = role_6dr;
    }
  } else {
    node_role = role_6ln;
  }

#if CONFIG_VIA_BUTTON
  {
#define CONFIG_WAIT_TIME 5

    SENSORS_ACTIVATE(button_sensor);
    etimer_set(&et, CLOCK_SECOND * CONFIG_WAIT_TIME);

    while(!etimer_expired(&et)) {
      printf("Init: current role: %s. Will start in %u seconds. Press user button to toggle mode.\n",
             node_role == role_6ln ? "6ln" : (node_role == role_6dr) ? "6dr" : "6dr-sec",
             CONFIG_WAIT_TIME);
      PROCESS_WAIT_EVENT_UNTIL(((ev == sensors_event) &&
                                (data == &button_sensor) && button_sensor.value(0) > 0)
                               || etimer_expired(&et));
      if(ev == sensors_event && data == &button_sensor && button_sensor.value(0) > 0) {
        node_role = (node_role + 1) % 3;
        if(LLSEC802154_ENABLED == 0 && node_role == role_6dr_sec) {
          node_role = (node_role + 1) % 3;
        }
        etimer_restart(&et);
      }
    }
  }

#endif /* CONFIG_VIA_BUTTON */

  printf("Init: node starting with role %s\n",
         node_role == role_6ln ? "6ln" : (node_role == role_6dr) ? "6dr" : "6dr-sec");

  tsch_set_pan_secured(LLSEC802154_ENABLED && (node_role == role_6dr_sec));
  is_coordinator = node_role > role_6ln;

  if(is_coordinator) {
    uip_ipaddr_t prefix;
    uip_ip6addr(&prefix, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
    net_init(&prefix);
  } else {
    net_init(NULL);
  }

#if WITH_ORCHESTRA
  orchestra_init();
#endif /* WITH_ORCHESTRA */




  etimer_set(&startAfter, START_AFTER_PERIOD );
  etimer_set(&periodic, SEND_INTERVAL);
//  etimer_set(&periodic_parentCheck, PERIODIC_PARENT_CONNECTION_CHECK_INTERVAL);
  while(1) {
    PROCESS_YIELD();
    if(ev == tcpip_event) {
      tcpip_handler();
    }

    if(ev == serial_line_event_message && data != NULL) {
      char *str;
      str = data;
      if(str[0] == 'r') {
        uip_ds6_route_t *r;
        uip_ipaddr_t *nexthop;
        uip_ds6_defrt_t *defrt;
        uip_ipaddr_t *ipaddr;
        defrt = NULL;
        if((ipaddr = uip_ds6_defrt_choose()) != NULL) {
          defrt = uip_ds6_defrt_lookup(ipaddr);
        }
        if(defrt != NULL) {
          PRINTF("DefRT: :: -> %02d", defrt->ipaddr.u8[15]);
          PRINTF(" lt:%lu inf:%d\n", stimer_remaining(&defrt->lifetime),
                 defrt->isinfinite);
        } else {
          PRINTF("DefRT: :: -> NULL\n");
        }

        for(r = uip_ds6_route_head();
            r != NULL;
            r = uip_ds6_route_next(r)) {
          nexthop = uip_ds6_route_nexthop(r);
          PRINTF("Route: %02d -> %02d", r->ipaddr.u8[15], nexthop->u8[15]);
          /* PRINT6ADDR(&r->ipaddr); */
          /* PRINTF(" -> "); */
          /* PRINT6ADDR(nexthop); */
          PRINTF(" lt:%lu\n", r->state.lifetime);

        }
      }
    }


    if(etimer_expired(&startAfter) && etimer_expired(&periodic)) {
      etimer_restart(&periodic);
      ctimer_set(&backoff_timer, SEND_TIME, send_packet, NULL);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
