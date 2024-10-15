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
  unsigned long product_id;
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

typedef union CANFD_data {
    void (*cb) (scan_data_msg_t, uint64_t);
    product_info_msg_t product_info;
} CANFD_data_t;

/* Enum abstracting CANIDs, priorities are assigned top (highest) to bottom (lowest)*/
typedef enum CAN_command {
    PRODUCT_SCAN = 0,
    SCAN_ACK,
    PRODUCT_UPDATE,
    PRODUCT_UPDATE_ACK
} CAN_command_t;

/*---------------------------------------------------------------------------*/
/* Defined in C++ code */
extern uint8_t add_node(uint64_t id, void (*scan_callback) (scan_data_msg_t, uint64_t), void (*product_update_callback) (unsigned long, uint64_t, product_info_msg_t*));
extern uint8_t remove_node(uint64_t  id);
extern void send_can_message(CAN_command_t command, uint64_t target_node, CANFD_data_t payload);
extern float simulate_can_bus();
/*---------------------------------------------------------------------------*/
PROCESS(node_process, "Node process");
AUTOSTART_PROCESSES(&node_process);

static struct etimer timer;

int sendcount = 0;
int t_0;

void scan_callback(scan_data_msg_t data, uint64_t calling_node) {
    unsigned long customer_id = data.customer_id;
    unsigned long long product_id = data.product_id;
    printf("Data: %ld, %lld. Generated by NODE#%ld\n", customer_id, product_id, calling_node);
    sendcount++;
    // TODO, remove this and replace with actual method call upon receiving in client node.
    CANFD_data_t msg_data;
    msg_data.cb = NULL;
    send_can_message(SCAN_ACK, calling_node, msg_data);
}

void product_update_callback(unsigned long product_id, uint64_t calling_node, product_info_msg_t* product_info) {
    printf("NODE#%ld displaying product %ld requests updated information.\n", calling_node, product_id);
    char name[6] = "AUGURK";
    unsigned short price = 12;
    char *product_name = name;
    unsigned short product_name_len = 6;
    CANFD_data_t msg_data;
    msg_data.product_info = (product_info_msg_t) {
        product_id,
        price,
        product_name,
        product_name_len
    };
    send_can_message(PRODUCT_UPDATE, calling_node, msg_data);
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(node_process, ev, data)
{
  PROCESS_BEGIN();

  t_0 = clock_time();

  for (int i = 0; i < NR_OF_CAN_NODES; i++) {
    add_node(i, &scan_callback, &product_update_callback);
  }

  while (1) {
    float time_to_sleep = simulate_can_bus();
    printf("Simulated CAN bus, %f seconds until next message\n", time_to_sleep);
    etimer_set(&timer, time_to_sleep * CLOCK_SECOND);
    float time = ((float)clock_time() - t_0)/1000.0;
    float msgpersec = sendcount/time;
    printf("Current load = %f msg/s at clock time: %f\n", msgpersec, time);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
