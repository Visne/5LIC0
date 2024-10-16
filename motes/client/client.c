#include "contiki.h"
#include "net/ipv6/simple-udp.h"
#include "tsch.h"
#include "random.h"
#include "sys/log.h"

#include "../shared/datatypes.h"
#include "../shared/custom-schedule.h"

#define LOG_MODULE "Client"
#define LOG_LEVEL LOG_LEVEL_DBG

#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

static struct simple_udp_connection connection;

PROCESS(client, "Client  process");
AUTOSTART_PROCESSES(&client);

PROCESS_THREAD(client, ev, data) {
    static struct etimer timer;
    uip_ipaddr_t root;

    PROCESS_BEGIN();

    initialize_tsch_schedule();

    if (!simple_udp_register(&connection,
                             UDP_CLIENT_PORT,
                             NULL,
                             UDP_SERVER_PORT,
                             NULL))
    {
        LOG_ERR("Failed to register UDP connection\n");
        PROCESS_EXIT();
    }

    while (true) {
        if (NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&root)) {
            scan_data_coap_t scan_data = (scan_data_coap_t) {
                random_rand() % 5000,
                random_rand() % 5000,
            };

            LOG_INFO("Customer #%lu scanned product #%llu\n", scan_data.customer_id, scan_data.product_id);
            simple_udp_sendto(&connection, &scan_data, sizeof(scan_data_coap_t), &root);
        }

        etimer_set(&timer, 4000 + (random_rand() % 8000));
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    }

    PROCESS_END();
}
