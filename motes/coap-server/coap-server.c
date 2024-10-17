#include "contiki.h"
#include "coap-engine.h"
#include "tsch.h"
#include "sys/log.h"
#include "db.h"
#include "custom-schedule.h"

#define LOG_MODULE "Server"
#define LOG_LEVEL LOG_LEVEL_DBG

extern coap_resource_t res_scan;
extern coap_resource_t res_tagquery;
extern coap_resource_t res_product_update;

PROCESS(coap_server, "server process with product query and customer scanning");
AUTOSTART_PROCESSES(&coap_server);

PROCESS_THREAD(coap_server, ev, data)
{
    static struct etimer timer;

	PROCESS_BEGIN();

    initialize_tsch_schedule();

    if (NETSTACK_ROUTING.root_start()) {
        LOG_ERR("Failed to set up RPL root node\n");
        PROCESS_EXIT();
    }

	coap_activate_resource(&res_tagquery, QUERY_URI);
	coap_activate_resource(&res_scan, SCAN_URI);
    coap_activate_resource(&res_product_update, UPDATE_URI);

    etimer_set(&timer, 60 * CLOCK_SECOND);

    while (1) {
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
        etimer_reset(&timer);

        res_product_update.trigger();
    }

    PROCESS_END();
}
