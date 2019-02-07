ALICE 
============================
ALICE is an autonomous link-based TSCH cell scheduling solution. ALICE uses Contiki Orchestra code as its skeleton code.
ALICE uses three slotframes (EB, broadcast/default and unicast) as Orchestra does.
The main difference is unicast slotframe schedule which implements time-varying scheduling and link-based scheduling.

ALICE source code location: ./apps/alice/ and ./core/net/mac/tsch/
ALICE example code location: ./examples/ipv6/rpl-udp-alice-log/

When using this source code, please cite the following paper: 

Seohyang Kim, Hyung-Sin Kim, and Chongkwon Kim, ALICE: Autonomous Link-based Cell Scheduling for TSCH, In the 18th ACM/IEEE International Conference on Information Processing in Sensor Networks (IPSN'19), April 16-18, 2019, Montreal, Canada.



The Contiki Operating System
============================

[![Build Status](https://travis-ci.org/contiki-os/contiki.svg?branch=master)](https://travis-ci.org/contiki-os/contiki/branches)

Contiki is an open source operating system that runs on tiny low-power
microcontrollers and makes it possible to develop applications that
make efficient use of the hardware while providing standardized
low-power wireless communication for a range of hardware platforms.

Contiki is used in numerous commercial and non-commercial systems,
such as city sound monitoring, street lights, networked electrical
power meters, industrial monitoring, radiation monitoring,
construction site monitoring, alarm systems, remote house monitoring,
and so on.

For more information, see the Contiki website:

[http://contiki-os.org](http://contiki-os.org)
