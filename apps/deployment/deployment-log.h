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
 *         Header file for deployment-log.c, and a set of macros for RPL logging
 *
 * \author Simon Duquennoy <simonduq@sics.se>
 */

#ifndef LOG_H
#define LOG_H

#if WITH_LOG

/* Used to identify packets carrying RPL log */
#define LOG_MAGIC 0xcafebabe

/* Data structure copied at the end of all data packets, making it possible
 * to trace packets at every hop, from every layer. */
struct app_data {
  uint32_t magic;
  uint32_t seqno;
  uint16_t src;
  uint16_t dest;
  uint8_t hop;
  uint8_t ping;
  uint16_t dummy_for_padding;
};

/* Copy an appdata to another with no assumption that the addresses are aligned */
void appdata_copy(void *dst, void *src);
/* Get dataptr from the packet currently in uIP buffer */
struct app_data *appdataptr_from_uip();
/* Get dataptr from a buffer */
struct app_data *appdataptr_from_buffer(const void *b, int len);
/* Get dataptr from the current packetbuf */
struct app_data *appdataptr_from_packetbuf();
/* Get dataptr from a queuebuf */
struct app_data *appdataptr_from_queuebuf(const void *q);
/* Log information about a data packet along with RPL routing information */
void log_appdataptr(void *dataptr);
/* Starts logging process */
void log_start();
/* Print all neighbors (RPL "parents"), their link metric and rank */
void rpl_print_neighbor_list();

#if WITH_DEPLOYMENT
#define LOG(...) printf(__VA_ARGS__)
#define LOGA(appdataptr, ...) { printf(__VA_ARGS__); log_appdataptr(appdataptr); }
#define LOGU(...) LOGA(appdataptr_from_uip(), __VA_ARGS__)
#define LOGP(...) LOGA(appdataptr_from_packetbuf(), __VA_ARGS__)
#define LOG_IPADDR(addr) uip_debug_ipaddr_print(addr)
#define LOG_LLADDR(addr) uip_debug_lladdr_print(addr)
#if WITH_LOG_HOP_COUNT
#define LOG_INC_HOPCOUNT_FROM_PACKETBUF() { struct app_data *dataptr = appdataptr_from_packetbuf(); \
                                                if(dataptr != NULL) { dataptr->hop++; } }
#else
#define LOG_INC_HOPCOUNT_FROM_PACKETBUF() { }
#endif

#define LOG_APPDATAPTR_FROM_BUFFER(b, l) appdataptr_from_buffer(b, l)
#define LOG_APPDATAPTR_FROM_PACKETBUF() appdataptr_from_packetbuf()
#define LOG_APPDATAPTR_FROM_QUEUEBUF(q) appdataptr_from_queuebuf(q)
#define LOG_NODEID_FROM_LINKADDR(linkaddr) node_id_from_linkaddr((const linkaddr_t *)linkaddr)
#define LOG_NODEID_FROM_IPADDR(ipaddr) node_id_from_ipaddr((const uip_ipaddr_t *)ipaddr)
#define LOG_PRINT_NEIGHBOR_LIST rpl_print_neighbor_list

#else /* WITH_DEPLOYMENT */

#define LOG(...) printf(__VA_ARGS__)
#define LOG_NL(...) { printf(__VA_ARGS__); printf("\n"); }
#define LOGA(appdataptr, ...) LOG_NL(__VA_ARGS__)
#define LOGU(...) LOG_NL(__VA_ARGS__)
#define LOGP(...) LOG_NL(__VA_ARGS__)
#define LOG_IPADDR(addr) uip_debug_ipaddr_print(addr)
#define LOG_LLADDR(addr) uip_debug_lladdr_print(addr)
#define LOG_INC_HOPCOUNT_FROM_PACKETBUF() { }

#define LOG_APPDATAPTR_FROM_BUFFER(b, l) NULL
#define LOG_APPDATAPTR_FROM_PACKETBUF() NULL
#define LOG_APPDATAPTR_FROM_QUEUEBUF(q) NULL
#define LOG_NODEID_FROM_LINKADDR(addr) ((addr) ? (addr)->u8[7] : 0)
#define LOG_NODEID_FROM_IPADDR(addr) ((addr) ? (addr)->u8[15] : 0)
#define LOG_PRINT_NEIGHBOR_LIST rpl_print_neighbor_list

#endif /* WITH_DEPLOYMENT */

#endif /* WITH_LOG */

#endif /* LOG */
