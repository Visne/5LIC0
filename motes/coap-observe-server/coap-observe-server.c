#include "contiki.h"
#include "contiki-net.h"
#include "coap-engine.h"
#include "sys/log.h"

#define LOG_MODULE "ObsServer"
#define LOG_LEVEL LOG_LEVEL_DBG

extern coap_resource_t res_push;

PROCESS(coap_observe_server, "CoAP observe server");
AUTOSTART_PROCESSES(&coap_observe_server);

PROCESS_THREAD(coap_observe_server, ev, data)
{
    PROCESS_BEGIN();

    if (NETSTACK_ROUTING.root_start()) {
        LOG_ERR("Failed to set up RPL root node\n");
        PROCESS_EXIT();
    }

    coap_activate_resource(&res_push, "test/push");

    PROCESS_END();
}
