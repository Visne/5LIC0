#include <tsch-const.h>
#include <tsch-types.h>
#include <tsch.h>
#include "contiki.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "tsch-schedule.h"
#include "sys/log.h"

#define LOG_MODULE "Client"
#define LOG_LEVEL LOG_LEVEL_DBG

#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

static void initialize_tsch_schedule(void);
static struct simple_udp_connection connection;

typedef struct {
    unsigned long customer_id;
    unsigned long long product_id;
} scan_data_t;

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
            scan_data_t scan_data = (scan_data_t) {
                random_rand() % 5000,
                random_rand() % 5000,
            };

            LOG_INFO("Customer #%lu scanned product #%llu\n", scan_data.customer_id, scan_data.product_id);
            simple_udp_sendto(&connection, &scan_data, sizeof(scan_data_t), &root);
        }

        etimer_set(&timer, 4000 + (random_rand() % 8000));
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    }

    PROCESS_END();
}

static void initialize_tsch_schedule(void)
{
    struct tsch_slotframe *sf = tsch_schedule_add_slotframe(0, 1);

    tsch_schedule_add_link(sf,
                           LINK_OPTION_RX | LINK_OPTION_TX | LINK_OPTION_SHARED,
                           LINK_TYPE_ADVERTISING, &tsch_broadcast_address,
                           0, 0, 1);
}
