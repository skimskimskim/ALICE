/**
 * Copyright (c) 2015, Swedish Institute of Computer Science.
 * All rights reserved.
 *
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
 */
/**
 * \file
 *         Orchestra: a slotframe dedicated to unicast data transmission. Designed for
 *         RPL storing mode only, as this is based on the knowledge of the children (and parent).
 *         If receiver-based:
 *           Nodes listen at a timeslot defined as hash(MAC) % ORCHESTRA_SB_UNICAST_PERIOD
 *           Nodes transmit at: for each nbr in RPL children and RPL preferred parent,
 *                                             hash(nbr.MAC) % ORCHESTRA_SB_UNICAST_PERIOD
 *         If sender-based: the opposite
 *
 * \author Simon Duquennoy <simonduq@sics.se>
 */
/* for hash time measure
#include "sys/clock.h"
#include "sys/rtimer.h"
*/
#include "contiki.h"
#include "orchestra.h"
#include "net/ipv6/uip-ds6-route.h"
#include "net/packetbuf.h"
#include "net/rpl/rpl-conf.h"
#include "net/mac/tsch/tsch-private.h"//ksh..
//#include "net/mac/tsch/tsch-asn.h"
#include <stdbool.h>

#if ORCHESTRA_UNICAST_SENDER_BASED && ORCHESTRA_COLLISION_FREE_HASH
#define UNICAST_SLOT_SHARED_FLAG    ((ORCHESTRA_UNICAST_PERIOD < (ORCHESTRA_MAX_HASH + 1)) ? LINK_OPTION_SHARED : 0)
#else
#define UNICAST_SLOT_SHARED_FLAG      LINK_OPTION_SHARED
#endif

#include "net/mac/tsch/tsch-log.h"

#define DEBUG DEBUG_PRINT
#include "net/net-debug.h"


#include "net/rpl/rpl.h"
#include "net/rpl/rpl-private.h"



uint16_t asfn_schedule=0; //absolute slotframe number for ALICE time varying scheduling


static uint16_t slotframe_handle = 0;
static struct tsch_slotframe *sf_unicast;

uint8_t link_option_rx = LINK_OPTION_RX ;
uint8_t link_option_tx = LINK_OPTION_TX | UNICAST_SLOT_SHARED_FLAG ; //ksh.. If it is a shared link, backoff will be applied.






/*---------------------------------------------------------------------------*/
static uint16_t
get_node_timeslot_us(const linkaddr_t *addr1, const linkaddr_t *addr2) //get timeslot for upstream
{
  if(addr1 != NULL && addr2 != NULL && ORCHESTRA_UNICAST_PERIOD > 0) { //ksh.
    return real_hash((ORCHESTRA_LINKADDR_HASH2(addr1, addr2)+asfn_schedule), (ORCHESTRA_UNICAST_PERIOD)); //link-based
  } else {
    return 0xffff;
  }
}
/*---------------------------------------------------------------------------*/
static uint16_t
get_node_timeslot_ds(const linkaddr_t *addr1, const linkaddr_t *addr2) //get timeslot for downstream
{
  if(addr1 != NULL && addr2 != NULL && ORCHESTRA_UNICAST_PERIOD > 0) { //ksh.
     return real_hash((ORCHESTRA_LINKADDR_HASH2(addr1, addr2)+asfn_schedule), (ORCHESTRA_UNICAST_PERIOD)); //link-based
  } else {
    return 0xffff;
  }
}
/*---------------------------------------------------------------------------*/
static uint16_t
get_node_channel_offset_us(const linkaddr_t *addr1, const linkaddr_t *addr2)
{
  int num_ch = (sizeof(TSCH_DEFAULT_HOPPING_SEQUENCE)/sizeof(uint8_t))-1; //ksh.

  if(addr1 != NULL && addr2 != NULL  && num_ch > 0) { //ksh.   
       return 1+real_hash((ORCHESTRA_LINKADDR_HASH2(addr1, addr2)+asfn_schedule),num_ch); //link-based
  } else {
    return 1+0; 
  }
}
/*---------------------------------------------------------------------------*/
static uint16_t
get_node_channel_offset_ds(const linkaddr_t *addr1, const linkaddr_t *addr2)
{
  int num_ch = (sizeof(TSCH_DEFAULT_HOPPING_SEQUENCE)/sizeof(uint8_t))-1; //ksh.

  if(addr1 != NULL && addr2 != NULL  && num_ch > 0) { //ksh.    
       return 1+real_hash((ORCHESTRA_LINKADDR_HASH2(addr1, addr2)+asfn_schedule),num_ch); //link-based
  } else {
    return 1+0; 
  }
}
/*---------------------------------------------------------------------------*/
uint16_t
is_root(){
  rpl_instance_t *instance =rpl_get_default_instance();
  if(instance!=NULL && instance->current_dag!=NULL){
       uint16_t min_hoprankinc = instance->min_hoprankinc;
       uint16_t rank=(uint16_t)instance->current_dag->rank;
       if(min_hoprankinc == rank){
          return 1;
       }
  }
  return 0;
}


/*---------------------------------------------------------------------------*/
static void
alice_RESCHEDULE_unicast_slotframe(void){ //ksh.  //remove current slotframe scheduling and re-schedule this slotframe.



  uint16_t timeslot_us, timeslot_ds, channel_offset_us, channel_offset_ds;
  uint16_t timeslot_us_p, timeslot_ds_p, channel_offset_us_p, channel_offset_ds_p; //parent's schedule
  uint8_t link_option_up, link_option_down;

//remove the whole links scheduled in the unicast slotframe
  struct tsch_link *l;
  l = list_head(sf_unicast->links_list);

 if(is_root()!=1){
//schedule the links between parent-node and current node
  if(l!=NULL){


     timeslot_us_p = get_node_timeslot_us(&linkaddr_node_addr, &orchestra_parent_linkaddr);
     timeslot_ds_p = get_node_timeslot_ds(&orchestra_parent_linkaddr, &linkaddr_node_addr);

     channel_offset_us_p = get_node_channel_offset_us(&linkaddr_node_addr, &orchestra_parent_linkaddr);
     channel_offset_ds_p = get_node_channel_offset_ds(&orchestra_parent_linkaddr, &linkaddr_node_addr);
     link_option_up=link_option_tx;
     link_option_down=link_option_rx;


     //parent uplink schedule
     l->link_options=link_option_up;
     l->timeslot=timeslot_us_p;
     l->channel_offset=channel_offset_us_p;
     l = list_item_next(l);     
     
     if(l!=NULL){
        //parent downlink schedule
        l->link_options=link_option_down;
        l->timeslot=timeslot_ds_p;
        l->channel_offset=channel_offset_ds_p;
        l = list_item_next(l);
     }
  }
//schedule the links between child-node and current node   //(lookup all route next hops)
 
 }//is_root()! end



  nbr_table_item_t *item = nbr_table_head(nbr_routes);
  while(l!=NULL && item!=NULL) {    

    linkaddr_t *addr = nbr_table_get_lladdr(nbr_routes, item);

    if(linkaddr_cmp(&orchestra_parent_linkaddr, addr)){
       item = nbr_table_next(nbr_routes, item);

       if(item==NULL){
         

#ifdef ALICE_TSCH_CALLBACK_SLOTFRAME_START //ksh sf update
           while(l!=NULL){
              
              //parent downlink schedule
              l->link_options=link_option_up;
              l->timeslot=timeslot_us;
              l->channel_offset=channel_offset_us;
              l = list_item_next(l);     
              
              if(l!=NULL){
                 //parent downlink schedule
                 l->link_options=link_option_down;
                 l->timeslot=timeslot_ds;
                 l->channel_offset=channel_offset_ds;
                 l = list_item_next(l);
              }
           }
#endif
           return;
       }
    }

    //ts and choff allocation

    timeslot_us = get_node_timeslot_us(addr, &linkaddr_node_addr); 
    timeslot_ds = get_node_timeslot_ds(&linkaddr_node_addr, addr); 

    channel_offset_us = get_node_channel_offset_us(addr, &linkaddr_node_addr);
    channel_offset_ds = get_node_channel_offset_ds(&linkaddr_node_addr, addr);


    //upstream link option
    if(timeslot_us==timeslot_us_p && channel_offset_us==channel_offset_us_p){
       link_option_up = link_option_tx | link_option_rx;
    }else{
       link_option_up = link_option_rx;
    }

    //downstream link option
    if(timeslot_ds==timeslot_ds_p && channel_offset_ds==channel_offset_ds_p){
       link_option_down = link_option_rx | link_option_tx;
    }else{
       link_option_down = link_option_tx;
    }

    //add links (upstream and downstream)

    //parent uplink schedule
    l->link_options=link_option_up;
    l->timeslot=timeslot_us;
    l->channel_offset=channel_offset_us;
    l = list_item_next(l);     
    
    if(l!=NULL){
       //parent downlink schedule
       l->link_options=link_option_down;
       l->timeslot=timeslot_ds;
       l->channel_offset=channel_offset_ds;
       l = list_item_next(l);
    }

    //move to the next item for while loop.
    item = nbr_table_next(nbr_routes, item);    
  } //while end..


}
/*---------------------------------------------------------------------------*/
static void
alice_schedule_unicast_slotframe(void){ //ksh.  //remove current slotframe scheduling and re-schedule this slotframe.


//  printf("- 4\n");
  uint16_t timeslot_us, timeslot_ds, channel_offset_us, channel_offset_ds;
  uint16_t timeslot_us_p, timeslot_ds_p, channel_offset_us_p, channel_offset_ds_p; //parent's schedule
  uint8_t link_option_up, link_option_down;

//remove the whole links scheduled in the unicast slotframe
  struct tsch_link *l;
  l = list_head(sf_unicast->links_list);
  while(l!=NULL) {    
    tsch_schedule_remove_link(sf_unicast, l);
    l = list_head(sf_unicast->links_list);
  }

 if(is_root()!=1){
//schedule the links between parent-node and current node

     timeslot_us_p = get_node_timeslot_us(&linkaddr_node_addr, &orchestra_parent_linkaddr);
     timeslot_ds_p = get_node_timeslot_ds(&orchestra_parent_linkaddr, &linkaddr_node_addr);

     channel_offset_us_p = get_node_channel_offset_us(&linkaddr_node_addr, &orchestra_parent_linkaddr);
     channel_offset_ds_p = get_node_channel_offset_ds(&orchestra_parent_linkaddr, &linkaddr_node_addr);
     link_option_up=link_option_tx;
     link_option_down=link_option_rx;
     tsch_schedule_add_link(sf_unicast, link_option_up, LINK_TYPE_NORMAL, &tsch_broadcast_address, timeslot_us_p, channel_offset_us_p);
     tsch_schedule_add_link(sf_unicast, link_option_down, LINK_TYPE_NORMAL, &tsch_broadcast_address, timeslot_ds_p, channel_offset_ds_p);
 }//is_root end

//schedule the links between child-node and current node   //(lookup all route next hops)
  nbr_table_item_t *item = nbr_table_head(nbr_routes);
  while(item != NULL) {
    linkaddr_t *addr = nbr_table_get_lladdr(nbr_routes, item);
    //ts and choff allocation

    timeslot_us = get_node_timeslot_us(addr, &linkaddr_node_addr); 
    timeslot_ds = get_node_timeslot_ds(&linkaddr_node_addr, addr); 

    channel_offset_us = get_node_channel_offset_us(addr, &linkaddr_node_addr);
    channel_offset_ds = get_node_channel_offset_ds(&linkaddr_node_addr, addr);

    //upstream link option
    if(timeslot_us==timeslot_us_p && channel_offset_us==channel_offset_us_p){
       link_option_up = link_option_tx | link_option_rx;
    }else{
       link_option_up = link_option_rx;
    }

    //downstream link option
    if(timeslot_ds==timeslot_ds_p && channel_offset_ds==channel_offset_ds_p){
       link_option_down = link_option_rx | link_option_tx;
    }else{
       link_option_down = link_option_tx;
    }

    //add links (upstream and downstream)
    tsch_schedule_add_link(sf_unicast, link_option_up, LINK_TYPE_NORMAL, &tsch_broadcast_address, timeslot_us, channel_offset_us);
    tsch_schedule_add_link(sf_unicast, link_option_down, LINK_TYPE_NORMAL, &tsch_broadcast_address, timeslot_ds, channel_offset_ds);

    //move to the next item for while loop.
    item = nbr_table_next(nbr_routes, item);
  }

}

/*---------------------------------------------------------------------------*/ //ksh. slotframe_callback. 
#ifdef ALICE_TSCH_CALLBACK_SLOTFRAME_START
void alice_callback_slotframe_start (uint16_t sfid, uint16_t sfsize){  
  asfn_schedule=sfid; //ksh.. update curr asfn_schedule.
  alice_RESCHEDULE_unicast_slotframe();
}
#endif
/*---------------------------------------------------------------------------*/ //ksh. packet_selection_callback. 
#ifdef ALICE_CALLBACK_PACKET_SELECTION
int alice_callback_packet_selection (uint16_t* ts, uint16_t* choff, const linkaddr_t rx_lladdr){


//schedule the links between parent-node and current node
  if(linkaddr_cmp(&orchestra_parent_linkaddr, &rx_lladdr)){
    *ts= get_node_timeslot_us(&linkaddr_node_addr, &orchestra_parent_linkaddr);
    *choff= get_node_channel_offset_us(&linkaddr_node_addr, &orchestra_parent_linkaddr);       
//    printf("ksh.. PCS.. parent : (%u,%u)\n", *ts, *choff);
    return 1;
  }

//schedule the links between child-node and current node   //(lookup all route next hops)
  nbr_table_item_t *item = nbr_table_head(nbr_routes);
  while(item != NULL) {
    linkaddr_t *addr = nbr_table_get_lladdr(nbr_routes, item);
    if(linkaddr_cmp(addr, &rx_lladdr)){
      *ts= get_node_timeslot_ds(&linkaddr_node_addr, addr);
      *choff= get_node_channel_offset_ds(&linkaddr_node_addr, addr); 

      return 1;
    } 
    item = nbr_table_next(nbr_routes, item);
  }

 
  *ts =0;
  *choff= ALICE_BROADCAST_SF_ID;
  //-----------------------

  return 0;
}
#endif //ALICE_CALLBACK_PACKET_SELECTION
/*---------------------------------------------------------------------------*/
static int
neighbor_has_uc_link(const linkaddr_t *linkaddr)
{
  if(linkaddr != NULL && !linkaddr_cmp(linkaddr, &linkaddr_null)) {
    if(orchestra_parent_knows_us 
       && linkaddr_cmp(&orchestra_parent_linkaddr, linkaddr)) {
      return 1;
    }
    if(nbr_table_get_from_lladdr(nbr_routes, (linkaddr_t *)linkaddr) != NULL  && !linkaddr_cmp(&orchestra_parent_linkaddr, linkaddr)) {
      return 1;
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
child_added(const linkaddr_t *linkaddr)
{
  alice_schedule_unicast_slotframe();
}
/*---------------------------------------------------------------------------*/
static void
child_removed(const linkaddr_t *linkaddr)
{
  alice_schedule_unicast_slotframe();
}
/*---------------------------------------------------------------------------*/
static int
select_packet(uint16_t *slotframe, uint16_t *timeslot, uint16_t *channel_offset)
{
  const linkaddr_t *dest = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);

  if(packetbuf_attr(PACKETBUF_ATTR_FRAME_TYPE) == FRAME802154_DATAFRAME && neighbor_has_uc_link(dest)) {
    if(slotframe != NULL) {
      *slotframe = slotframe_handle;
    }
    if(timeslot != NULL) {

        //if the destination is the parent node, schedule it in the upstream period, if the destination is the child node, schedule it in the downstream period.
        if(linkaddr_cmp(&orchestra_parent_linkaddr, dest)){
           *timeslot = get_node_timeslot_us(&linkaddr_node_addr, dest); //parent node (upstream)
        }else{
           *timeslot = get_node_timeslot_ds(&linkaddr_node_addr, dest);  //child node (downstream)
        }
    }
    if(channel_offset != NULL) { //ksh.
        //if the destination is the parent node, schedule it in the upstream period, if the destination is the child node, schedule it in the downstream period.
        if(linkaddr_cmp(&orchestra_parent_linkaddr, dest)){
           *channel_offset = get_node_channel_offset_us(&linkaddr_node_addr, dest); //child node (upstream)
        }else{
           *channel_offset = get_node_channel_offset_ds(&linkaddr_node_addr, dest); //child node (downstream)
        }
    }

    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
new_time_source(const struct tsch_neighbor *old, const struct tsch_neighbor *new)
{
  if(new != old) {   
    const linkaddr_t *new_addr = new != NULL ? &new->addr : NULL;
    if(new_addr != NULL) {
      linkaddr_copy(&orchestra_parent_linkaddr, new_addr);    
    } else {
      linkaddr_copy(&orchestra_parent_linkaddr, &linkaddr_null);
    }
    alice_schedule_unicast_slotframe(); 
  }
}
/*---------------------------------------------------------------------------*/
static void
init(uint16_t sf_handle)
{
  printf("ALICE PERIOD SIZE: ORCHESTRA_UNICAST_PERIOD : %d\n",  ORCHESTRA_UNICAST_PERIOD );

  slotframe_handle = sf_handle; //sf_handle=1
  /* Slotframe for unicast transmissions */
  sf_unicast = tsch_schedule_add_slotframe(slotframe_handle, ORCHESTRA_UNICAST_PERIOD);


#ifdef ALICE_TSCH_CALLBACK_SLOTFRAME_START
  asfn_schedule = alice_tsch_schedule_get_current_asfn(sf_unicast);//ksh..
#else
  asfn_schedule = 0; //sfid (ASN) will not be used.
#endif

}
/*---------------------------------------------------------------------------*/
struct orchestra_rule unicast_per_neighbor_rpl_storing = {
  init,
  new_time_source,
  select_packet,
  child_added,
  child_removed,
};
