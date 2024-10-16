#include "contiki.h"
#include "contiki-net.h"
#include "coap-observe-client.h"
#include "sys/log.h"

#define LOG_MODULE "Observer"
#define LOG_LEVEL LOG_LEVEL_DBG

#define TOGGLE_INTERVAL 30
#define OBS_RESOURCE_URI "test/push"

static coap_observee_t *obs;

PROCESS(coap_observer, "CoAP observer");
AUTOSTART_PROCESSES(&coap_observer);

static void notification_callback(coap_observee_t *obse, void *notification, coap_notification_flag_t flag) {
    int len = 0;
    const uint8_t *payload = NULL;

    LOG_INFO("Notification on URI: %s\n", obse->url);
    if (notification) {
        len = coap_get_payload(notification, &payload);
    }
    switch (flag) {
        case NOTIFICATION_OK:
            LOG_INFO("NOTIFICATION OK: %*s\n", len, (char *) payload);
            break;
        case OBSERVE_OK: /* server accepeted observation request */
            LOG_INFO("OBSERVE_OK: %*s\n", len, (char *) payload);
            break;
        case OBSERVE_NOT_SUPPORTED:
            LOG_INFO("OBSERVE_NOT_SUPPORTED: %*s\n", len, (char *) payload);
            obs = NULL;
            break;
        case ERROR_RESPONSE_CODE:
            LOG_INFO("ERROR_RESPONSE_CODE: %*s\n", len, (char *) payload);
            obs = NULL;
            break;
        case NO_REPLY_FROM_SERVER:
            LOG_INFO("NO_REPLY_FROM_SERVER :(");
            obs = NULL;
            break;
    }
}

PROCESS_THREAD(coap_observer, ev, data) {
    PROCESS_BEGIN();

    static struct etimer et;
    coap_endpoint_t server_ep;
    uip_ipaddr_t root;

    etimer_set(&et, TOGGLE_INTERVAL * CLOCK_SECOND);

    while(1) {
        if (NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&root)) {
            // Convert root IPv6 address to string
            char ip[UIPLIB_IPV6_MAX_STR_LEN];
            uiplib_ipaddr_snprint(ip, sizeof(ip), &root);

            coap_endpoint_parse(ip, strlen(ip), &server_ep);

            if (!obs) {
                LOG_INFO("Starting observation\n");
                obs = coap_obs_request_registration(&server_ep, OBS_RESOURCE_URI, notification_callback, NULL);
            }
        } else {
            obs = NULL;
        }

        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
        etimer_reset(&et);
    }

    PROCESS_END();
}
