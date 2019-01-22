/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ip/uip.h"
#include "net/rpl/rpl.h"

#include "net/netstack.h"
#include "dev/button-sensor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"


#if WITH_COMPOWER & SERVER_WITH_COMPOWER
#include "powertrace.h"
#endif

#include "node-id.h"
#include "net/ipv6/uip-ds6-route.h"
#include "net/mac/tsch/tsch.h"
#include "net/rpl/rpl-private.h"
//#include "dev/temperature-sensor.h"
//#include "board.h"

#if WITH_ORCHESTRA
#include "orchestra.h"
#endif /* WITH_ORCHESTRA */



#define CONFIG_VIA_BUTTON PLATFORM_HAS_BUTTON

#if !ECHO_DOWNSTREAM_ENABLED //ksh..
#include "lib/list.h"
#endif




#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

#define UDP_EXAMPLE_ID  190


#define MAX_PAYLOAD_LEN		20

static struct uip_udp_conn *server_conn;
static int seq_id=1;




PROCESS(udp_server_process, "UDP server process");
AUTOSTART_PROCESSES(&udp_server_process);


#if !ECHO_DOWNSTREAM_ENABLED //ksh..
uip_ipaddr_t route_copy_list[MAX_NODE_NUM];
int num_child=0;

#endif



void
print_mac_states(){

#if WITH_COMPOWER & SERVER_WITH_COMPOWER

printf("m mactx: %d %d %d %d %d %d %d %u %u %u %u %u\n", mac_tx_up_ok_counter,  mac_tx_up_error_counter, mac_tx_down_ok_counter, mac_tx_down_error_counter, dc_radio, dc_tx, dc_listen,num_pktdrop_queue, num_pktdrop_mac, num_pktdrop_rpl, num_dis+num_dio+num_dao+num_dao_ack, tsch_queue_overflow);

#else
    PRINTF("m mactx: %d %d %d %d %d %d %d %d %d %d %d %d\n", mac_tx_up_ok_counter, mac_tx_up_collision_counter, mac_tx_up_noack_counter, mac_tx_up_deferred_counter, mac_tx_up_err_counter, mac_tx_up_err_fatal_counter,     mac_tx_down_ok_counter, mac_tx_down_collision_counter, mac_tx_down_noack_counter, mac_tx_down_deferred_counter, mac_tx_down_err_counter, mac_tx_down_err_fatal_counter);
#endif
}

void
send_packet(uip_ipaddr_t *ipaddr){

   if(seq_id > MAX_NUM_DOWNSTREAM_PACKETS) { 
      return; 
   }
   
    PRINTF("D sendmsg %d to %u %u time %u\n", seq_id++, ipaddr->u8[sizeof(ipaddr->u8)-2], ipaddr->u8[sizeof(ipaddr->u8)-1], (uint16_t) current_asn.ls4b);  //modified

    char buf[MAX_PAYLOAD_LEN];
    sprintf(buf, "%u", (uint16_t) current_asn.ls4b);

    uip_ipaddr_copy(&server_conn->ripaddr, ipaddr);
    uip_udp_packet_send(server_conn, buf, strlen(buf));
    uip_create_unspecified(&server_conn->ripaddr);



#if !ECHO_DOWNSTREAM_ENABLED
#if DO_SEQ_SEND_WITH_INTERVAL
    num_child-=1;
#endif
#endif

}


int address_flag=0;

#if !ECHO_DOWNSTREAM_ENABLED

/*
send_process0() : send msg to all the nodes in "pre-defined" list (even though a node is not included in the RPL route-list).
*/


static void
send_process1() /*For any experiment. Server sends msg to all nodes in its sub network.*/
{
    PRINTF("-------------SEND PROCESS START ! (MAKE DOWNSTREAM DESTINATION LIST)\n");


    uip_ds6_route_t *route;
    route = uip_ds6_route_head();


    num_child=0;


    while(route != NULL && num_child < MAX_NODE_NUM) {
      PRINTF("-- %u %u --- ", route->ipaddr.u8[sizeof(route->ipaddr.u8)-2], route->ipaddr.u8[sizeof(route->ipaddr.u8)-1]);  //      PRINT6ADDR(&route->ipaddr);
      uip_ipaddr_copy(&route_copy_list[num_child], &route->ipaddr);
	int i;
	for ( i=0;i<sizeof(route_copy_list->u8);i++){
	 printf("%u.", route_copy_list[num_child].u8[i]);
	}
	printf("\n");
       num_child++;

      route = uip_ds6_route_next(route);
    }


#if DO_SEQ_SEND_WITH_INTERVAL
//    printf("do_seq_send_with_interval==1\n");
#else
//    printf("do_seq_send_with_interval==0\n");
    while(num_child > 0){
        send_packet(&route_copy_list[num_child-1]);
    }    
#endif

  PRINTF("------------- SEND PROCESS END----------------------------\n");
  print_mac_states();

}

#endif

/*---------------------------------------------------------------------------*/
static void
tcpip_handler(void)
{
  char *appdata;


  if(uip_newdata()) {
    appdata = (char *)uip_appdata;
    appdata[uip_datalen()] = 0;
    PRINTF("D rxvs %s from %u %u time %u\n", appdata, UIP_IP_BUF->srcipaddr.u8[sizeof(UIP_IP_BUF->srcipaddr.u8) - 2], UIP_IP_BUF->srcipaddr.u8[sizeof(UIP_IP_BUF->srcipaddr.u8) - 1], (uint16_t) current_asn.ls4b);  //modified


#if SERVER_REPLY
#if ECHO_DOWNSTREAM_ENABLED
    send_packet(&UIP_IP_BUF->srcipaddr);
    print_mac_states();
#endif
#endif

  }
}
/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTF("Server IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(state == ADDR_TENTATIVE || state == ADDR_PREFERRED) {
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTF("\n");
      /* hack to make address "final" */
      if (state == ADDR_TENTATIVE) {
	uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
      }
    }
  }
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
PROCESS_THREAD(udp_server_process, ev, data)
{
  static struct etimer et;

#if !ECHO_DOWNSTREAM_ENABLED
  static struct etimer periodic;
#if DO_SEQ_SEND_WITH_INTERVAL
  static struct etimer sequential_send;
#endif
#endif

  static struct etimer startAfter;

  uip_ipaddr_t ipaddr;
  struct uip_ds6_addr *root_if;

  PROCESS_BEGIN();
#if WITH_COMPOWER & SERVER_WITH_COMPOWER
  powertrace_start(CLOCK_SECOND*60);
#endif
  PROCESS_PAUSE();

  SENSORS_ACTIVATE(button_sensor);

  PRINTF("UDP server started. max nbr:%d max routes:%d\n",
         NBR_TABLE_CONF_MAX_NEIGHBORS, UIP_CONF_MAX_ROUTES);



#if UIP_CONF_ROUTER

  uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0x00ff, 0xfe00, 1);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_MANUAL);
  root_if = uip_ds6_addr_lookup(&ipaddr);
  if(root_if != NULL) {
    rpl_dag_t *dag;
    dag = rpl_set_root(RPL_DEFAULT_INSTANCE,(uip_ip6addr_t *)&ipaddr);
    uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
    rpl_set_prefix(dag, &ipaddr, 64);
    PRINTF("created a new RPL dag\n");
  } else {
    PRINTF("failed to create a new RPL DAG\n");
  }
#endif /* UIP_CONF_ROUTER */
  
  print_local_addresses();

  /* The data sink runs with a 100% duty cycle in order to ensure high 
     packet reception rates. */


 /* 3 possible roles:
   * - role_6ln: simple node, will join any network, secured or not
   * - role_6dr: DAG root, will advertise (unsecured) beacons
   * - role_6dr_sec: DAG root, will advertise secured beacons
   * */
  static int is_coordinator = 0;
  static enum { role_6ln, role_6dr, role_6dr_sec } node_role;
  node_role = role_6dr;//dr;

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
    node_role = role_6dr;
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

  etimer_set(&startAfter, START_AFTER_PERIOD);
#if !ECHO_DOWNSTREAM_ENABLED
  etimer_set(&periodic, SEND_INTERVAL_SERVER);
#if DO_SEQ_SEND_WITH_INTERVAL
  etimer_set(&sequential_send, SEQUENTIAL_SEND_INTERVAL);
#endif
#endif

  server_conn = udp_new(NULL, UIP_HTONS(UDP_CLIENT_PORT), NULL);
  if(server_conn == NULL) {
    PRINTF("No UDP connection available, exiting the process!\n");
    PROCESS_EXIT();
  }
  udp_bind(server_conn, UIP_HTONS(UDP_SERVER_PORT));

  PRINTF("Created a server connection with remote address ");
  PRINT6ADDR(&server_conn->ripaddr);
  PRINTF(" local/remote port %u/%u\n", UIP_HTONS(server_conn->lport),
         UIP_HTONS(server_conn->rport));


  while(1) {
    PROCESS_YIELD();
    if(ev == tcpip_event) {
      tcpip_handler();
    } else if (ev == sensors_event && data == &button_sensor) {
      PRINTF("Initiaing global repair\n");
      rpl_repair_root(RPL_DEFAULT_INSTANCE);
    } 

   if(etimer_expired(&startAfter)) {
#if !ECHO_DOWNSTREAM_ENABLED
     if(etimer_expired(&periodic)) {
       etimer_restart(&periodic);
       send_process1(); 
     }
#if DO_SEQ_SEND_WITH_INTERVAL
     if(etimer_expired(&sequential_send)){
       etimer_restart(&sequential_send);
       if(num_child>0) {        
      	 send_packet(&route_copy_list[num_child-1]);
       }
     }
#endif
#endif
   }//startAfter

  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
