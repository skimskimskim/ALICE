#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

#undef NULLRDC_CONF_802154_AUTOACK
#define NULLRDC_CONF_802154_AUTOACK       1


#define MAX_NODE_NUM 70 //ksh.. maximum number of nodes

#define SERVER_REPLY          1  //ksh..
#define WITH_COMPOWER   1 //ksh..
#define SERVER_WITH_COMPOWER  1//ksh..


/* Define as minutes */
#define RPL_CONF_DEFAULT_LIFETIME_UNIT   60

/* 10 minutes lifetime of routes */
#define RPL_CONF_DEFAULT_LIFETIME        10

#define RPL_CONF_DEFAULT_ROUTE_INFINITE_LIFETIME 1


//*********************************************************************
/* Set to run orchestra */
#ifndef WITH_ORCHESTRA
#define WITH_ORCHESTRA 1
#endif /* WITH_ORCHESTRA */

/* Set to enable TSCH security */
#ifndef WITH_SECURITY
#define WITH_SECURITY 0
#endif /* WITH_SECURITY */

/*******************************************************/
/********* Enable RPL non-storing mode *****************/
/*******************************************************/

#undef UIP_CONF_MAX_ROUTES
#define UIP_CONF_MAX_ROUTES MAX_NODE_NUM /* No need for routes */

#undef NBR_TABLE_CONF_MAX_NEIGHBORS
#define NBR_TABLE_CONF_MAX_NEIGHBORS MAX_NODE_NUM //ksh.. modified. original:10


#undef RPL_CONF_MOP
#define RPL_CONF_MOP RPL_MOP_STORING_NO_MULTICAST /* Mode of operation*/





#undef ORCHESTRA_CONF_RULES
#define ORCHESTRA_CONF_RULES {&eb_per_time_source, &default_common, &unicast_per_neighbor_rpl_storing} //slotframe identifier (handle) : {0 1 2}
#define ALICE_UNICAST_SF_ID 2 //slotframe handle of unicast slotframe
#define ALICE_BROADCAST_SF_ID 1 //slotframe handle of broadcast/default slotframe



/*******************************************************/
/********************* Enable TSCH *********************/
/*******************************************************/

/* Netstack layers */
#undef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC     tschmac_driver
#undef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC     nordc_driver
#undef NETSTACK_CONF_FRAMER
#define NETSTACK_CONF_FRAMER  framer_802154

/* IEEE802.15.4 frame version */
#undef FRAME802154_CONF_VERSION
#define FRAME802154_CONF_VERSION FRAME802154_IEEE802154E_2012

/* TSCH and RPL callbacks */
#define RPL_CALLBACK_PARENT_SWITCH tsch_rpl_callback_parent_switch
#define RPL_CALLBACK_NEW_DIO_INTERVAL tsch_rpl_callback_new_dio_interval
#define TSCH_CALLBACK_JOINING_NETWORK tsch_rpl_callback_joining_network
#define TSCH_CALLBACK_LEAVING_NETWORK tsch_rpl_callback_leaving_network



/* Needed for IoT-LAB M3 nodes */ //ksh..
#undef RF2XX_SOFT_PREPARE
#define RF2XX_SOFT_PREPARE 0
#undef RF2XX_WITH_TSCH
#define RF2XX_WITH_TSCH 1



/* Needed for CC2538 platforms only */
/* For TSCH we have to use the more accurate crystal oscillator
 * by default the RC oscillator is activated */
#undef SYS_CTRL_CONF_OSC32K_USE_XTAL
#define SYS_CTRL_CONF_OSC32K_USE_XTAL 1

/* Needed for cc2420 platforms only */
/* Disable DCO calibration (uses timerB) */
#undef DCOSYNCH_CONF_ENABLED
#define DCOSYNCH_CONF_ENABLED 0
/* Enable SFD timestamps (uses timerB) */
#undef CC2420_CONF_SFD_TIMESTAMPS
#define CC2420_CONF_SFD_TIMESTAMPS 1



/*******************************************************/
/******************* Configure TSCH ********************/
/*******************************************************/

/* TSCH logging. 0: disabled. 1: basic log. 2: with delayed
 * log messages from interrupt */
#undef TSCH_LOG_CONF_LEVEL
#define TSCH_LOG_CONF_LEVEL 0 //ksh.. log level

/* IEEE802.15.4 PANID */
#undef IEEE802154_CONF_PANID
#define IEEE802154_CONF_PANID 0xabcd

/* Do not start TSCH at init, wait for NETSTACK_MAC.on() */
#undef TSCH_CONF_AUTOSTART
#define TSCH_CONF_AUTOSTART 0

/* 6TiSCH minimal schedule length.
 * Larger values result in less frequent active slots: reduces capacity and saves energy. */
#undef TSCH_SCHEDULE_CONF_DEFAULT_LENGTH
#define TSCH_SCHEDULE_CONF_DEFAULT_LENGTH 3

#if WITH_SECURITY

/* Enable security */
#undef LLSEC802154_CONF_ENABLED
#define LLSEC802154_CONF_ENABLED 1
/* TSCH uses explicit keys to identify k1 and k2 */
#undef LLSEC802154_CONF_USES_EXPLICIT_KEYS
#define LLSEC802154_CONF_USES_EXPLICIT_KEYS 1
/* TSCH uses the ASN rather than frame counter to construct the Nonce */
#undef LLSEC802154_CONF_USES_FRAME_COUNTER
#define LLSEC802154_CONF_USES_FRAME_COUNTER 0

#endif /* WITH_SECURITY */

#if WITH_ORCHESTRA

/* See apps/orchestra/README.md for more Orchestra configuration options */
#define TSCH_SCHEDULE_CONF_WITH_6TISCH_MINIMAL 0 /* No 6TiSCH minimal schedule */
#define TSCH_CONF_WITH_LINK_SELECTOR 1 /* Orchestra requires per-packet link selection */
/* Orchestra callbacks */
#define TSCH_CALLBACK_NEW_TIME_SOURCE orchestra_callback_new_time_source
#define TSCH_CALLBACK_PACKET_READY orchestra_callback_packet_ready
#define NETSTACK_CONF_ROUTING_NEIGHBOR_ADDED_CALLBACK orchestra_callback_child_added
#define NETSTACK_CONF_ROUTING_NEIGHBOR_REMOVED_CALLBACK orchestra_callback_child_removed




//KSH.. TSCH modification..............................................//
//#define BROADCAST_BACKOFF_ENABLED 0
#define ORCHESTRA_CONF_COMMON_SHARED_PERIOD 19 //ksh.. original: 31. (broadcast and default slotframe length)
#define ORCHESTRA_CONF_UNICAST_PERIOD 23 // 7, 11, 19, 23, 31, 43, 47, 59, 67, 71    
//#define ORCHESTRA_CONF_EBSF_PERIOD 397//.. original: 397. (EB slotframe)

//period 10  12 15 17 20 24 30 40 60 120 600
//KSH.. server-client application modification........................................//
#ifndef PERIOD 
#define PERIOD 30 //upstream/downstream send interval //15 (seq20)   30 (seq40)   60 (seq80)   90 (seq120)
#define PERIOD_SERVER PERIOD //downstream/downstream send interval //15 (seq20)   30 (seq40)   60 (seq80)   90 (seq120)
#endif

#define SEQUENTIAL_SEND_INTERVAL ((PERIOD_SERVER) * (CLOCK_SECOND) / (MAX_NODE_NUM))
//#define SEQUENTIAL_SEND_INTERVAL 40 //40// 20 // (T * CLOCK_SECOND) // openlab CLOCK_SECOND=100  //20 40 80 160


#define START_AFTER_PERIOD (30 * 60 * (CLOCK_SECOND)) //ksh.. RPL construction.. no udp packet during this time.. 


#define MAX_NUM_DOWNSTREAM_PACKETS 5000000 //ksh..


#define SEND_INTERVAL		((PERIOD) * (CLOCK_SECOND))
#define SEND_INTERVAL_SERVER	((PERIOD_SERVER) * (CLOCK_SECOND))
#define SEND_TIME		(random_rand() % (SEND_INTERVAL)) //backoff 




/**********************************************************************/
//KSH... Experimental setting.........................................//

#define ECHO_DOWNSTREAM_ENABLED 0 //ksh.. 1:echo, 0:not echo (independent)

#if !ECHO_DOWNSTREAM_ENABLED
#define DO_SEQ_SEND_WITH_INTERVAL 1 //ksh.. 1:sequential send with interval, 0:sequential send without interval (bursty)
#endif

#define FIXED_RPL_TOPOLOGY 0 //ksh.. creates fixed rpl topology //1: fixed RPL, 0: normal RPL //used for 2017 openmote-cc2538 SNU testbed



/**********************************************************************/
//KSH.. TSCH backoff window setting modification........................//
//#define TSCH_CONF_MAC_MIN_BE 1//original: 1
#define TSCH_CONF_MAC_MAX_BE 3 //original: 7

//KSH.. RPL modification..............................................//
#define RPL_MRHOF_CONF_SQUARED_ETX 0 //ksh.. mrhof using squared etx.// original value:0


#define RPL_CONF_WITH_PROBING 1 //ksh.. original: 1
//#define RPL_CONF_PROBING_INTERVAL (60 * CLOCK_SECOND) //ksh.. original: (120 * CLOCK_SECOND)
//#define RPL_PROBING_SEND_FUNC(instance, addr) dis_output((addr)) //ksh.. original: dio_output



#define TSCH_SCHEDULE_CONF_MAX_LINKS MAX_NODE_NUM //ksh.. as escalator..


#define RPL_CONF_DIS_INTERVAL 10 //ksh.. original: 60s

//#define RPL_CONF_DIS_START_DELAY 5//ksh.. original: 5s

//transmission power.  iot-lab . //cofigure: /contiki/platform/openlab/radio-rf2xx.c           possible parameters: openlab/net/phy_rf2xx/phy_rf2xx.c //#define RF2XX_TX_POWER  PHY_POWER_m17dBm //PHY_POWER_3dBm //ksh..

//#define RPL_CONF_DAO_DELAY (CLOCK_SECOND * 1) //ksh.. rpl dao delay. original:4sec
//#define RPL_CONF_DAO_RETRANSMISSION_TIMEOUT (2 * CLOCK_SECOND) //ksh.. if dao fails, retransmit. original: 5sec
//#define RPL_CONF_DAO_MAX_RETRANSMISSIONS 5 //ksh.. original: 5
//#define RPL_CONF_NOPATH_REMOVAL_DELAY 60 //ksh.. original:60s //since the type of [route's lifetime] is unsigned int, should not be set as 0.
//#define RPL_CONF_MAX_RANKINC  (3 * RPL_MIN_HOPRANKINC)  //ksh.. original:(7 * RPL_MIN_HOPRANKINC)



/*
 * RPL DAO ACK support. When enabled, DAO ACK will be sent and requested.
 * This will also enable retransmission of DAO when no ack is received.
 * */
/*
 * RPL REPAIR ON DAO NACK. When enabled, DAO NACK will trigger a local
 * repair in order to quickly find a new parent to send DAO's to.
 * NOTE: this is too agressive in some cases so use with care.
 * */

/* next_dis = RPL_DIS_INTERVAL / 2 +
 *    ((uint32_t)RPL_DIS_INTERVAL * (uint32_t)random_rand()) / RANDOM_RAND_MAX -
 *    RPL_DIS_START_DELAY;
 * */






/**********************************************************************/
/*******   orchestra sender-based  vs. receiver-based    **************/
#define ORCHESTRA_CONF_UNICAST_SENDER_BASED 1 //1:sender-based 0:receiver-based
/**********************************************************************/
/******* ALICE : WITH_ALICE=1    , ORCHESTRA: UNDEFINE  ***************/
#define WITH_ALICE 1 //ALICE:1 ORCHESTRA:undefine.
/**********************************************************************/



#if WITH_ALICE
#define ALICE_CALLBACK_PACKET_SELECTION alice_callback_packet_selection //ksh. alice packet selection
#define ALICE_TSCH_CALLBACK_SLOTFRAME_START alice_callback_slotframe_start //ksh. alice time varying slotframe schedule
#endif
/**********************************************************************/
/**********************************************************************/



#define ORCHESTRA_ONE_CHANNEL_OFFSET 1 // ksh. 1: only one channel offset for unicast slotframe , 0: multiple channel offset for unicast slotframe 



#if WITH_ALICE || ORCHESTRA_CONF_UNICAST_SENDER_BASED //Orchestra SB
#define RPL_CONF_WITH_DAO_ACK 1 //ksh.. enable dao ack. original: 0.
#define RPL_CONF_RPL_REPAIR_ON_DAO_NACK 1 //ksh.. enable local repair. quickly find another parent. original: 0
#else //Orchestra RB
#define RPL_CONF_WITH_DAO_ACK 0 //ksh..  original: 0.
#define RPL_CONF_RPL_REPAIR_ON_DAO_NACK 0 
#endif

#endif /* WITH_ORCHESTRA */

/*******************************************************/
/************* Other system configuration **************/
/*******************************************************/

#if CONTIKI_TARGET_Z1
/* Save some space to fit the limited RAM of the z1 */
#undef UIP_CONF_TCP
#define UIP_CONF_TCP 0
#undef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM 3
#undef RPL_NS_CONF_LINK_NUM
#define RPL_NS_CONF_LINK_NUM  8
#undef NBR_TABLE_CONF_MAX_NEIGHBORS
#define NBR_TABLE_CONF_MAX_NEIGHBORS MAX_NODE_NUM //ksh.. modified. original:10
#undef UIP_CONF_ND6_SEND_NA
#define UIP_CONF_ND6_SEND_NA 0
#undef SICSLOWPAN_CONF_FRAG
#define SICSLOWPAN_CONF_FRAG 0

#if WITH_SECURITY
/* Note: on sky or z1 in cooja, crypto operations are done in S/W and
 * cannot be accommodated in normal slots. Use 65ms slots instead, and
 * a very short 6TiSCH minimal schedule length */
#undef TSCH_CONF_DEFAULT_TIMESLOT_LENGTH
#define TSCH_CONF_DEFAULT_TIMESLOT_LENGTH 65000
#undef TSCH_SCHEDULE_CONF_DEFAULT_LENGTH
#define TSCH_SCHEDULE_CONF_DEFAULT_LENGTH 2
/* Reduce log level to make space for security on z1 */
//#undef TSCH_LOG_CONF_LEVEL  //ksh.. undef
//#define TSCH_LOG_CONF_LEVEL 0 //ksh.. undef
#endif /* WITH_SECURITY */

#endif /* CONTIKI_TARGET_Z1 */

#if CONTIKI_TARGET_CC2538DK || CONTIKI_TARGET_ZOUL || \
  CONTIKI_TARGET_OPENMOTE_CC2538
#define TSCH_CONF_HW_FRAME_FILTERING    0
#endif /* CONTIKI_TARGET_CC2538DK || CONTIKI_TARGET_ZOUL \
       || CONTIKI_TARGET_OPENMOTE_CC2538 */

#if CONTIKI_TARGET_COOJA
#define COOJA_CONF_SIMULATE_TURNAROUND 0
#endif /* CONTIKI_TARGET_COOJA */

#endif /* __PROJECT_CONF_H__ */
