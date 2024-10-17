#include "contiki.h"
#include "coap-engine.h"
#include "tsch.h"
#include "sys/log.h"

#include "../shared/custom-schedule.h"
#include "../../shared/coap/coap-datatypes.h"

#define LOG_MODULE "Server"
#define LOG_LEVEL LOG_LEVEL_DBG

extern coap_resource_t res_scan;
extern coap_resource_t res_tagquery;
extern coap_resource_t res_product_update;

PROCESS(server_coap_v1b, "server process with product query and customer scanning");
AUTOSTART_PROCESSES(&server_coap_v1b);

PROCESS_THREAD(server_coap_v1b, ev, data)
{
	PROCESS_BEGIN();

    initialize_tsch_schedule();

    if (NETSTACK_ROUTING.root_start()) {
        LOG_ERR("Failed to set up RPL root node\n");
        PROCESS_EXIT();
    }

	coap_activate_resource(&res_tagquery, QUERY_URI);
	coap_activate_resource(&res_scan, SCAN_URI);
    coap_activate_resource(&res_product_update, "product/update");

    PROCESS_END();
}
