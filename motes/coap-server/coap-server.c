#include <string.h>
#include "contiki.h"
#include "coap-engine.h"

//extern coap_resource_t res_tagquery;
extern coap_resource_t res_scan;
extern coap_resource_t res_tagquery;


PROCESS(server_coap_v1b, "server process with product query and customer scanning");
AUTOSTART_PROCESSES(&server_coap_v1b);

PROCESS_THREAD(server_coap_v1b, ev, data)
{	
	//init customer database
	//const  unsigned char customer_db_init = 0; //0 as long as customer database is initialized
	//static customer_tab_t *customers = NULL;
	 
	PROCESS_BEGIN();
	//etimer_set(&et, TOGGLE_INTERVAL * CLOCK_SECOND);
	coap_activate_resource(&res_tagquery, "test/query");
	coap_activate_resource(&res_scan, "test/scan");
	
		
	while (1) { PROCESS_WAIT_EVENT();} //loops and waits for client price requests
	PROCESS_END();
}
