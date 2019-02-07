# ALICE

## Overview
ALICE is an autonomous link-based TSCH cell scheduling solution. ALICE uses Contiki Orchestra code as its skeleton code.
ALICE uses three slotframes (EB, broadcast/default and unicast) as Orchestra does.
The main difference is unicast slotframe schedule which implements time-varying scheduling and link-based scheduling.

To use ALICE, enable Orchestra settings by following the instructions below.
Add `APPS += alice` to your makefile instead of `APPS += orchestra`.
Set up the following callbacks:

```
#define WITH_ALICE 1 
#if WITH_ALICE
#define ALICE_CALLBACK_PACKET_SELECTION alice_callback_packet_selection //ksh. packet_selection_callback.
#define ALICE_TSCH_CALLBACK_SLOTFRAME_START alice_callback_slotframe_start //ksh. slotframe_callback.
#endif
```

ALICE sample code location: ../../examples/ipv6/rpl-udp-alice-log/



When using this source code, please cite the following paper: 

Seohyang Kim, Hyung-Sin Kim, and Chongkwon Kim, ALICE: Autonomous Link-based Cell Scheduling for TSCH, In the 18th ACM/IEEE International Conference on Information Processing in Sensor Networks (IPSN'19), April 16-18, 2019, Montreal, Canada.




# Orchestra

## Overview

Orchestra is an autonomous scheduling solution for TSCH, where nodes maintain
their own schedule solely based on their local RPL state. There is no centralized
scheduler nor negociatoin with neighbors, i.e. no traffic overhead. The default
Orchestra rules can be used out-of-box in any RPL network, reducing contention
to a low level. Orchestra is described and evaluated in
[*Orchestra: Robust Mesh Networks Through Autonomously Scheduled TSCH*](http://www.simonduquennoy.net/papers/duquennoy15orchestra.pdf), ACM SenSys'15.

## Requirements

Orchestra requires a system running TSCH and RPL.
For sender-based unicast slots (`ORCHESTRA_UNICAST_SENDER_BASED`), it requires
RPL with downwards routing enabled (relies on DAO).

## Getting Started

To use Orchestra, add a couple global definitions, e.g in your `project-conf.h` file.

Disable 6TiSCH minimal schedule:

`#define TSCH_SCHEDULE_CONF_WITH_6TISCH_MINIMAL 0`

Enable TSCH link selector (allows Orchestra to assign TSCH links to outgoing packets):

`#define TSCH_CONF_WITH_LINK_SELECTOR 1`

Set up the following callbacks:

```
#define TSCH_CALLBACK_NEW_TIME_SOURCE orchestra_callback_new_time_source
#define TSCH_CALLBACK_PACKET_READY orchestra_callback_packet_ready
#define NETSTACK_CONF_ROUTING_NEIGHBOR_ADDED_CALLBACK orchestra_callback_child_added
#define NETSTACK_CONF_ROUTING_NEIGHBOR_REMOVED_CALLBACK orchestra_callback_child_removed
```

To use Orchestra, fist add it to your makefile `APPS` with `APPS += orchestra`.
 
Finally:
* add Orchestra to your makefile `APPS` with `APPS += orchestra`;
* start Orchestra by calling `orchestra_init()` from your application, after
including `#include "orchestra.h"`.

## Configuration

Orchestra comes with a number of pre-installed rules, `orchestra-rule-*.c`.
You can define your own by using any of these as a template.
A default Orchestra configuration is described in `orchestra-conf.h`, define your own
`ORCHESTRA_CONF_*` macros to override modify the rule set and change rules configuration.
