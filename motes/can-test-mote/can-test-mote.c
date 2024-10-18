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
#include "../../libraries/CAN-sim/shared/can-commands.h"

#define NR_OF_CAN_NODES 50

/*---------------------------------------------------------------------------*/
/* Defined in C++ code */
extern uint8_t init_can_bus(uint64_t id, void (*scan_callback) (scan_data_msg_t, uint64_t), void (*product_update_callback) (unsigned long, uint64_t));
extern uint8_t add_node(uint64_t id, void (*scan_callback) (scan_data_msg_t, uint64_t), void (*product_update_callback) (unsigned long, uint64_t));
extern uint8_t remove_node(uint64_t  id);
extern void send_can_message(CAN_command command, uint64_t target_node, CAN_data_t payload);
extern float simulate_can_bus();
extern void set_product_id(uint64_t node_id, ean13_t product_id);
extern void update_visualization(int clock);
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
    CAN_data_t msg_data;
    msg_data.empty = true;
    // printf("<-- THIS WOULD BE A NETWORK CALL TO SUBMIT SCAN -->\n");
    send_can_message(SCAN_ACK, calling_node, msg_data);
}

void product_update_callback(ean13_t product_id, uint64_t calling_node) {
    printf("NODE#%ld displaying product %ld requests updated information.\n", calling_node, product_id);
    unsigned short price = 2 * product_id + 1;
    CAN_data_t msg_data;
    // msg_data.product_info = (product_info_msg_t) {
    //     product_id,
    //     price,
    //     "",
    //     PRODUCT_DESCRIPT_LEN
    // };
    snprintf(msg_data.product_info.product_name, PRODUCT_DESCRIPT_LEN, "Product_%ld", product_id);
    printf("<-- THIS WOULD BE A NETWORK CALL TO FETCH DATA -->\n");
    send_can_message(PRODUCT_UPDATE, calling_node, msg_data);
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(node_process, ev, data)
{
  PROCESS_BEGIN();

  t_0 = clock_time();
  etimer_set(&timer, 5 * CLOCK_SECOND);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
  int can_started = init_can_bus(NR_OF_CAN_NODES, &scan_callback, &product_update_callback);

  if (can_started){
      while (1) {
          float time_to_sleep = simulate_can_bus();
          printf("Simulated CAN bus, %f seconds until next message\n", time_to_sleep);
          etimer_set(&timer, time_to_sleep * CLOCK_SECOND);
          // float time = ((float)clock_time() - t_0)/1000.0;
          // float msgpersec = sendcount/time;
          // printf("Current load = %f msg/s at clock time: %f\n", msgpersec, time);
          update_visualization(clock_time());
          PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
      }
  };

  

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
