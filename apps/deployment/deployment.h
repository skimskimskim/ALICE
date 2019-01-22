/*
 * Copyright (c) 2014, Swedish Institute of Computer Science.
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
 *         Header file for deployment.c
 *
 * \author Simon Duquennoy <simonduq@sics.se>
 */

#ifndef DEPLOYMENT_H
#define DEPLOYMENT_H

#if WITH_DEPLOYMENT

#include "contiki-conf.h"
#include "deployment-def.h"
#include "sys/node-id.h"
#include "net/ip/uip.h"
#include "net/linkaddr.h"

/* Returns the total number of nodes in the deployment */
uint16_t get_node_id();
/* Returns a node-index from a node's linkaddr */
uint16_t node_index_from_linkaddr(const linkaddr_t *addr);
/* Returns a node-id from a node's link-layer address */
uint16_t node_id_from_linkaddr(const linkaddr_t *addr);
/* Returns a node-id from a node's IPv6 address */
uint16_t node_id_from_ipaddr(const uip_ipaddr_t *addr);
/* Returns a node-index from a node-id */
uint16_t get_node_index_from_id(uint16_t id);
/* Returns a node-id from a node's absolute index in the deployment */
uint16_t get_node_id_from_index(uint16_t index);
/* Sets an IPv6 from a link-layer address */
void set_ipaddr_from_linkaddr(uip_ipaddr_t *ipaddr, const linkaddr_t *lladdr);
/* Sets an IPv6 from a link-layer address */
void set_ipaddr_from_id(uip_ipaddr_t *ipaddr, uint16_t id);
/* Sets an linkaddr from a link-layer address */
void set_linkaddr_from_id(linkaddr_t *lladdr, uint16_t id);
/* Initializes global IPv6 and creates DODAG */
int deployment_init(uip_ipaddr_t *ipaddr, uip_ipaddr_t *br_prefix, int root_id);

/* Our absolute index in the id_mac table */
extern uint16_t node_index;

#endif /* WITH_DEPLOYMENT */

#endif /* DEPLOYMENT_H */
