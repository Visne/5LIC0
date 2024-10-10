#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "coap-engine.h"



#define TOGGLE_INTERVAL 10

extern coap_resource_t
res_tagquery; 
//res_pricequery;

PROCESS(server_coap_v1a, "server process with limited product information requestr");
AUTOSTART_PROCESSES(&server_coap_v1a);

PROCESS_THREAD(server_coap_v1a, ev, data)
{
	PROCESS_BEGIN();
	//etimer_set(&et, TOGGLE_INTERVAL * CLOCK_SECOND);
	coap_activate_resource(&res_tagquery, "test/query");
	//coap_activate_resource(&res_pricequery, "test/query");

	
	while (1) { PROCESS_WAIT_EVENT();} //loops and waits for client price requests
	PROCESS_END();
}
