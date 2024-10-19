#include "contiki.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

#define MAX_PRODUCTS 50
#define PRODUCT_ID_LEN 14
#define PRODUCT_PRICE_LEN 10
#define PRODUCT_DESCRIPT_LEN 50
#define PRODUCT_STOCKED_LEN 2

typedef struct { // symbolizes a product
    char product_id[PRODUCT_ID_LEN]; // EAN-13 product id
    char product_price[PRODUCT_PRICE_LEN]; // price in cents
    char product_description[PRODUCT_DESCRIPT_LEN]; // short product descriptor
    char is_stocked[PRODUCT_STOCKED_LEN]; // 1 if stocked, 0 if not
} product_info_t;

product_info_t products[MAX_PRODUCTS];
static struct simple_udp_connection udp_conn;

PROCESS(udp_server_process, "UDP server");
AUTOSTART_PROCESSES(&udp_server_process);

/* Function to load dataset */
void load_dataset() {
  srand(time(NULL));
  for(int i = 0; i < MAX_PRODUCTS; i++) {
    snprintf(products[i].product_id, PRODUCT_ID_LEN, "%d", i + 1);
    snprintf(products[i].product_price, PRODUCT_PRICE_LEN, "%d", (rand() % 50) + 1);
    snprintf(products[i].product_description, PRODUCT_DESCRIPT_LEN, "Product %d", i + 1);
    snprintf(products[i].is_stocked, PRODUCT_STOCKED_LEN, "1");
  }
}

/* Function to get product details by product_id */
int get_product_details(const char *product_id, product_info_t *product) {
  for(int i = 0; i < MAX_PRODUCTS; i++) {
    if(strcmp(products[i].product_id, product_id) == 0) {
      *product = products[i];
      return 1;
    }
  }
  return 0;
}

/* Function to update product details */
void update_product_details(const char *product_id, const char *new_price, const char *new_stock_status) {
  for(int i = 0; i < MAX_PRODUCTS; i++) {
    if(strcmp(products[i].product_id, product_id) == 0) {
      strncpy(products[i].product_price, new_price, PRODUCT_PRICE_LEN);
      strncpy(products[i].is_stocked, new_stock_status, PRODUCT_STOCKED_LEN);
      LOG_INFO("Updated Product ID %s to new price %s and stock status %s\n", product_id, new_price, new_stock_status);
      return;
    }
  }
  LOG_INFO("Product ID %s not found\n", product_id);
}

/* Function to handle incoming request */
void handle_request(const char *product_id, const uip_ipaddr_t *src_addr, uint16_t src_port) {
  product_info_t product;
  if(get_product_details(product_id, &product)) {
    char response[256];
    snprintf(response, sizeof(response), "Product ID: %s, Price: %s, Description: %s, Stocked: %s", 
             product.product_id, product.product_price, product.product_description, product.is_stocked);
    LOG_INFO("Sending response to ");
    LOG_INFO_6ADDR(src_addr);
    LOG_INFO(" : %s\n", response);
    simple_udp_sendto(&udp_conn, response, strlen(response), src_addr);
  } else {
    LOG_INFO("Product ID %s not found\n", product_id);
  }
}

/*---------------------------------------------------------------------------*/
static void
udp_rx_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
  LOG_INFO("Received request '%.*s' from ", datalen, (char *) data);
  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO("\n");

  char product_id[PRODUCT_ID_LEN];
  char new_price[PRODUCT_PRICE_LEN];
  char new_stock_status[PRODUCT_STOCKED_LEN];

  // Check if it's an update request
  if (sscanf((char *)data, "update %s %s %s", product_id, new_price, new_stock_status) == 3) {
    update_product_details(product_id, new_price, new_stock_status);
  } 
  // Check if it's a product information request
  else if (sscanf((char *)data, "%s", product_id) == 1) {
    handle_request(product_id, sender_addr, sender_port);
  } 
  else {
    LOG_INFO("Invalid request format\n");
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data)
{
  PROCESS_BEGIN();

  /* Load dataset */
  load_dataset();

  /* Initialize DAG root */
  NETSTACK_ROUTING.root_start();

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL,
                      UDP_CLIENT_PORT, udp_rx_callback);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
