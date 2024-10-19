#include "contiki.h"
#include "net/routing/routing.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include <stdint.h>
#include <inttypes.h>
#include <time.h>
#include <stdlib.h>

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

#define SEND_INTERVAL      (10 * CLOCK_SECOND)
#define MAX_PRODUCTS 50
#define PRODUCT_ID_LEN 14
#define PRODUCT_PRICE_LEN 10
#define PRODUCT_DESCRIPT_LEN 50
#define PRODUCT_STOCKED_LEN 2

static struct simple_udp_connection udp_conn;
static uint32_t rx_count = 0;

PROCESS(udp_client_process, "UDP client");
AUTOSTART_PROCESSES(&udp_client_process);

/* Product info structure */
typedef struct { // symbolizes a product
    char product_id[PRODUCT_ID_LEN]; // EAN-13 product id
    char product_price[PRODUCT_PRICE_LEN]; // price in cents
    char product_description[PRODUCT_DESCRIPT_LEN]; // short product descriptor
    char is_stocked[PRODUCT_STOCKED_LEN]; // 1 if stocked, 0 if not
} product_info_t;

product_info_t products[MAX_PRODUCTS];
static uint32_t tx_count;
static uint32_t missed_tx_count;

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
  LOG_INFO("Received response '%.*s' from ", datalen, (char *) data);
  LOG_INFO_6ADDR(sender_addr);
#if LLSEC802154_CONF_ENABLED
  LOG_INFO_(" LLSEC LV:%d", uipbuf_get_attr(UIPBUF_ATTR_LLSEC_LEVEL));
#endif
  LOG_INFO_("\n");
  rx_count++;
}
/*---------------------------------------------------------------------------*/
void load_dataset() {
  srand(time(NULL));
  for(int i = 0; i < MAX_PRODUCTS; i++) {
    snprintf(products[i].product_id, PRODUCT_ID_LEN, "%d", i + 1);
    snprintf(products[i].product_price, PRODUCT_PRICE_LEN, "%d", (rand() % 50) + 1);
    snprintf(products[i].product_description, PRODUCT_DESCRIPT_LEN, "Product %d", i + 1);
    snprintf(products[i].is_stocked, PRODUCT_STOCKED_LEN, "1");
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer periodic_timer;
  static char str[128];
  uip_ipaddr_t dest_ipaddr;

  PROCESS_BEGIN();

  /* Load product dataset */
  load_dataset();

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);

  etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    if(NETSTACK_ROUTING.node_is_reachable() &&
        NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {

      /* Print statistics every 10th TX */
      if(tx_count % 10 == 0) {
        LOG_INFO("Tx/Rx/MissedTx: %" PRIu32 "/%" PRIu32 "/%" PRIu32 "\n",
                 tx_count, rx_count, missed_tx_count);
      }

      /* Send a request to the server for product information or update */
      int product_index = rand() % MAX_PRODUCTS;
      if(rand() % 2 == 0) {
        /* Send product information request */
        snprintf(str, sizeof(str), "%s", products[product_index].product_id);
        LOG_INFO("Sending product info request %"PRIu32" for Product ID %s to ", tx_count, str);
      } else {
        /* Send update request with new price and stock status */
        char new_price[PRODUCT_PRICE_LEN];
        char new_stock_status[PRODUCT_STOCKED_LEN];
        snprintf(new_price, PRODUCT_PRICE_LEN, "%d", (rand() % 50) + 1);
        snprintf(new_stock_status, PRODUCT_STOCKED_LEN, "%d", rand() % 2);
        snprintf(str, sizeof(str), "update %s %s %s", products[product_index].product_id, new_price, new_stock_status);
        LOG_INFO("Sending update request %"PRIu32" for Product ID %s with new price %s and stock status %s to ", tx_count, products[product_index].product_id, new_price, new_stock_status);
      }

      LOG_INFO_6ADDR(&dest_ipaddr);
      LOG_INFO_("\n");
      simple_udp_sendto(&udp_conn, str, strlen(str), &dest_ipaddr);
      tx_count++;
    } else {
      LOG_INFO("Not reachable yet\n");
      if(tx_count > 0) {
        missed_tx_count++;
      }
    }

    /* Add some jitter */
    etimer_set(&periodic_timer, SEND_INTERVAL
      - CLOCK_SECOND + (random_rand() % (2 * CLOCK_SECOND)));
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
