#include "contiki.h"
#include "contiki-net.h"
#include "coap-engine.h"
#include "coap-blocking-api.h"
#include "sys/node-id.h"
#include "sys/log.h"

#include "../shared/coap/coap-datatypes.h"
#include "../shared/coap/utility.h"
#include "../shared/custom-schedule.h"

#define LOG_MODULE "Client"
#define LOG_LEVEL LOG_LEVEL_DBG

PROCESS(client, "Client process with customer scan");
AUTOSTART_PROCESSES(&client);

void scan_handler(coap_message_t *response) {
    const uint8_t *chunk;

    if (response == NULL) {
        LOG_ERR("Request timed out\n");
        return;
    }

    int len = coap_get_payload(response, &chunk);

    LOG_INFO("|%.*s\n", len, (char *) chunk);
}

void query_handler(coap_message_t *response) {
    if (response == NULL) {
        LOG_ERR("Request timed out\n");
        return;
    }

    product_info_t product = *(product_info_t*) response->payload;

    LOG_INFO("|%s %s %s %s\n", product.product_id, product.product_price, product.product_description, product.is_stocked);
}

PROCESS_THREAD(client, ev, data) {
    static struct etimer timer;
    coap_endpoint_t server_ep;
    uip_ipaddr_t root;
    coap_message_t request;

    PROCESS_BEGIN();

    initialize_tsch_schedule();

    etimer_set(&timer, TOGGLE_INTERVAL * node_id * CLOCK_SECOND);

    while (1) {
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
        etimer_reset(&timer);

        if (!NETSTACK_ROUTING.node_is_reachable() || !NETSTACK_ROUTING.get_root_ipaddr(&root)) {
            continue;
        }

        // Convert root IPv6 address to string
        char ip[UIPLIB_IPV6_MAX_STR_LEN];
        uiplib_ipaddr_snprint(ip, sizeof(ip), &root);

        coap_endpoint_parse(ip, strlen(ip), &server_ep);

        // If even node ID, send scans
        if (node_id % 2 == 0) {
            LOG_INFO("Scan\n");

            // Create payload for a scan for product id <node_id * 10000> and client id <node_id> and ADD 500 of the item
            scan_data_t scan = {
                node_id,
                node_id,
                500,
                0
            };

            request = coap_create_request(COAP_POST, SCAN_URI, COAP_TYPE_CON, &scan, sizeof(scan));

            // Send
            COAP_BLOCKING_REQUEST(&server_ep, &request, scan_handler);
        } else {
            LOG_INFO("Query\n");

            // Create payload for a product info query for product id <node_id * 10000>
            req_product_data_t product;
            sprintf(product.product_id, "%hu", node_id);
            snprintf(product.blankbuffer, sizeof(product.blankbuffer), "Query info"); //can be changed depending on what we need

            request = coap_create_request(COAP_GET, QUERY_URI, COAP_TYPE_CON, &product, sizeof(product));

            // Send :3
            COAP_BLOCKING_REQUEST(&server_ep, &request, query_handler);
        }

    }

    PROCESS_END();
}






