#include "contiki.h"
#include "contiki-net.h"
#include "coap-observe-client.h"
#include "coap-blocking-api.h"
#include "sys/node-id.h"
#include "sys/log.h"

#include "datatypes.h"
#include "utility.h"
#include "custom-schedule.h"

#define LOG_MODULE "Client"
#define LOG_LEVEL LOG_LEVEL_DBG

PROCESS(client, "Client process with customer scan");
AUTOSTART_PROCESSES(&client);

static coap_observee_t *obs;

void scan_handler(coap_message_t *response) {
    if (response == NULL) {
        LOG_ERR("Scan request timed out\n");
    }
}

void query_handler(coap_message_t *response) {
    if (response == NULL) {
        LOG_ERR("Query request timed out\n");
        return;
    }

    product_info_t product = *(product_info_t*) response->payload;
    LOG_INFO("Query: %s (ID: %lu, price: %hu cents)\n", product.product_description, product.product_id, product.product_price);
}

void notification_callback(coap_observee_t *subject, void *notification, coap_notification_flag_t flag) {
    int len = 0;
    const uint8_t *payload = NULL;

    LOG_INFO("Notification on URI: %s\n", subject->url);
    if (notification) {
        len = coap_get_payload(notification, &payload);
    }
    switch (flag) {
        case NOTIFICATION_OK:
        case OBSERVE_OK:
            LOG_INFO("OK: %*s\n", len, (char *) payload);
            break;
        case OBSERVE_NOT_SUPPORTED:
        case ERROR_RESPONSE_CODE:
        case NO_REPLY_FROM_SERVER:
            // TODO: More descriptive log message
            LOG_ERR("Something went wrong: %d\n", flag);
            obs = NULL;
            break;
    }
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
            obs = NULL;
            continue;
        }

        // Convert root IPv6 address to string
        char ip[UIPLIB_IPV6_MAX_STR_LEN];
        uiplib_ipaddr_snprint(ip, sizeof(ip), &root);

        coap_endpoint_parse(ip, strlen(ip), &server_ep);

        if (obs == NULL) {
            obs = coap_obs_request_registration(&server_ep, UPDATE_URI, notification_callback, NULL);
        }

        // If even node ID, send scans
        if (node_id % 2 == 0) {
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
            // Create payload for a product info query for product id <node_id * 10000>
            req_product_data_t product;
            product.product_id = node_id;
            snprintf(product.blankbuffer, sizeof(product.blankbuffer), "Query info"); //can be changed depending on what we need

            request = coap_create_request(COAP_GET, QUERY_URI, COAP_TYPE_CON, &product, sizeof(product));

            // Send :3
            COAP_BLOCKING_REQUEST(&server_ep, &request, query_handler);
        }

    }

    PROCESS_END();
}






