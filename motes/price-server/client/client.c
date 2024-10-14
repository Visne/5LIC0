#include "contiki.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "sys/log.h"
#include <stdlib.h>

#define LOG_MODULE "PriceClient"
#define LOG_LEVEL LOG_LEVEL_INFO

#define SERVER_PORT 1234
#define CLIENT_PORT 5678
#define SEND_INTERVAL (CLOCK_SECOND * 3)

static struct simple_udp_connection udp_conn;

PROCESS(udp_client_process, "UDP Client Process");
AUTOSTART_PROCESSES(&udp_client_process);

PROCESS_THREAD(udp_client_process, ev, data) {
  static struct etimer periodic_timer;
  static char request_message[64];
  static int product_id;
  static int customer_id;

  PROCESS_BEGIN();

  // Initialize UDP connection
  simple_udp_register(&udp_conn, CLIENT_PORT, NULL, SERVER_PORT, NULL);
  LOG_INFO("UDP client started
");

  // Set a timer to send requests periodically
  etimer_set(&periodic_timer, SEND_INTERVAL);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    // Randomly generate valid product ID and customer ID (1-5)
    product_id = (random_rand() % 5) + 1;
    customer_id = (random_rand() % 5) + 1;

    // Create the request message
    snprintf(request_message, sizeof(request_message), "Product ID: %d, Customer ID: %d", product_id, customer_id);
    LOG_INFO("Sending request: %s
", request_message);

    // Send the request to the server
    uip_ipaddr_t dest_ipaddr;
    if(NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
      simple_udp_sendto(&udp_conn, request_message, strlen(request_message) + 1, &dest_ipaddr);
    } else {
      LOG_INFO("No route to server
");
    }

    // Reset the timer
    etimer_reset(&periodic_timer);
  }

  PROCESS_END();
}