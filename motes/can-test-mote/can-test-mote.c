/*
 * Copyright (c) 2020, Institute of Electronics and Computer Science.
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
 */

/**
 * \file
 *         Application to test the CAN-generator application as used in a contiki mote.
 * \author
 *         Atis Elsts <atis.elsts@edi.lv>
 */

#include "contiki.h"
#include <stdio.h>
#include "sys/etimer.h"

#define NR_OF_CAN_NODES 16

typedef struct product_info_msg
{
  /// Price of the product in cents
  unsigned short price;
  char *product_name;
  unsigned short product_name_len;
} product_info_msg_t;

typedef struct scan_data_msg
{
  unsigned long customer_id;
  unsigned long long product_id;
} scan_data_msg_t;

/*---------------------------------------------------------------------------*/
/* Defined in C++ code */
extern uint8_t add_node(uint64_t id, void (*scan_callback) (scan_data_msg_t));
extern uint8_t remove_node(uint64_t  id);
extern uint8_t send_data(uint64_t  senderId, const char *data);
extern uint8_t receive_data(uint64_t receiverId, char *data);
extern float simulate_can_bus();
/*---------------------------------------------------------------------------*/
PROCESS(node_process, "Node process");
AUTOSTART_PROCESSES(&node_process);

static struct etimer et;
static struct etimer timer;

void scan_callback(scan_data_msg_t data) {
    unsigned long customer_id = data.customer_id;
    unsigned long long product_id = data.product_id;
    printf("Data: %ld, %lld\n", customer_id, product_id);
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(node_process, ev, data)
{
  PROCESS_BEGIN();

  for (int i = 0; i < NR_OF_CAN_NODES; i++) {
    add_node(i, &scan_callback);
  }

  while (1) {
    float time_to_sleep = simulate_can_bus();
    printf("Simulated CAN bus, %f seconds until next message\n", time_to_sleep);
    etimer_set(&timer, time_to_sleep * CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
  }

  /* Run CAN bus, sleeping when inactive */
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  /* Reset the etimer to trig again in 1 second */
  printf("First timer!\n");

  
  printf("Second timer\n");

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
