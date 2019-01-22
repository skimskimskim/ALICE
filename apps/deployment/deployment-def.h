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
 *         Deployment-related macros. Distinct from deployment.h as this
 *         is a stand-alone file (does not include any other file) that
 *         can be included globally from e.g. a project-conf.h file.
 *
 * \author Simon Duquennoy <simonduq@sics.se>
 */

#ifndef DEPLOYMENT_DEF_H
#define DEPLOYMENT_DEF_H

#define DEPLOYMENT_COOJA        1
#define DEPLOYMENT_MOTES        2
#define DEPLOYMENT_TWIST        3
#define DEPLOYMENT_INDRIYA      4
#define DEPLOYMENT_NESTESTBED   5

#ifndef DEPLOYMENT
#if CONTIKI_TARGET_JN5168
#define DEPLOYMENT DEPLOYMENT_MOTES
#else
#define DEPLOYMENT DEPLOYMENT_COOJA
#endif /* CONTIKI_TARGET_JN5168 */
#endif

#define IN_COOJA (DEPLOYMENT == DEPLOYMENT_COOJA)
#define IN_MOTES (DEPLOYMENT == DEPLOYMENT_MOTES)
#define IN_TWIST (DEPLOYMENT == DEPLOYMENT_TWIST)
#define IN_INDRIYA (DEPLOYMENT == DEPLOYMENT_INDRIYA)
#define IN_NESTESTBED (DEPLOYMENT == DEPLOYMENT_NESTESTBED)

#if IN_TWIST
#define ROOT_ID 137
#elif IN_INDRIYA
#define ROOT_ID 2
#define MAX_NODES 99
#elif IN_NESTESTBED
#define ROOT_ID 1
#define MAX_NODES 25
#else
#define ROOT_ID 1
#define MAX_NODES 99
#endif

#endif /* DEPLOYMENT_DEF_H */
