#include "contiki.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "sys/log.h"
#include <stdio.h>
#include <string.h>

#define LOG_MODULE "PriceServer"
#define LOG_LEVEL LOG_LEVEL_INFO

#define SERVER_PORT 1234
#define CLIENT_PORT 5678

static struct simple_udp_connection udp_conn;

// File containing prices for different product and customer combinations
static const char *price_file[5][5] = {
  {"Price: $1", "Price: $2", "Price: $3", "Price: $4", "Price: $5"},
  {"Price: $6", "Price: $7", "Price: $8", "Price: $9", "Price: $10"},
  {"Price: $11", "Price: $12", "Price: $13", "Price: $14", "Price: $15"},
  {"Price: $16", "Price: $17", "Price: $18", "Price: $19", "Price: $20"},
  {"Price: $21", "Price: $22", "Price: $23", "Price: $24", "Price: $25"}
};

PROCESS(udp_server_process, "UDP Server Process");
PROCESS(price_update_process, "Price Update Process");
AUTOSTART_PROCESSES(&udp_server_process, &price_update_process);

// Callback function 
static void udp_rx_callback(struct simple_udp_connection *c,
                            const uip_ipaddr_t *sender_addr,
                            uint16_t sender_port,
                            const uip_ipaddr_t *receiver_addr,
                            uint16_t receiver_port,
                            const uint8_t *data,
                            uint16_t datalen) {
  LOG_INFO("Received request from client\n");

  // Extract product ID and customer ID from the received data
  char received_data[64];
  strncpy(received_data, (const char *)data, datalen);
  received_data[datalen] = '\0';
  LOG_INFO("Received data: %s\n", received_data);

  // Parse product ID and customer ID
  int product_id, customer_id;
  if (sscanf(received_data, "Product ID: %d, Customer ID: %d", &product_id, &customer_id) == 2) {
    if (product_id >= 1 && product_id <= 5 && customer_id >= 1 && customer_id <= 5) {
      // Retrieve price from the file based on product ID and customer ID
      const char *price = price_file[product_id - 1][customer_id - 1];
      simple_udp_sendto(&udp_conn, price, strlen(price) + 1, sender_addr);
      LOG_INFO("Sent price information to client: %s\n", price);
    } else {
      LOG_INFO("Invalid product ID or customer ID\n");
    }
  } else {
    LOG_INFO("Failed to parse product ID and customer ID\n");
  }
}

PROCESS_THREAD(udp_server_process, ev, data) {
  PROCESS_BEGIN();

  // Initialize UDP connection
  simple_udp_register(&udp_conn, SERVER_PORT, NULL, CLIENT_PORT, udp_rx_callback);
  LOG_INFO("UDP server started on port %d\n", SERVER_PORT);

  PROCESS_END();
}

PROCESS_THREAD(price_update_process, ev, data) {
  PROCESS_BEGIN();

  while(1) {
    char new_price[32];
    int product_id, customer_id;

    printf("Enter product ID (1-5): ");
    if (scanf("%d", &product_id) == 1 && product_id >= 1 && product_id <= 5) {
      printf("Enter customer ID (1-5): ");
      if (scanf("%d", &customer_id) == 1 && customer_id >= 1 && customer_id <= 5) {
        printf("Enter new price: ");
        if (scanf("%s", new_price) == 1) {
          snprintf((char *)price_file[product_id - 1][customer_id - 1], 32, "Price: %s", new_price);
          LOG_INFO("Updated price information for Product ID: %d, Customer ID: %d to %s\n", product_id, customer_id, new_price);
        }
      }
    }
  }

  PROCESS_END();
}