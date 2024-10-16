#include <stdio.h>
#include "contiki.h"
#include "contiki-net.h"
#include "coap-engine.h"
#include "coap-blocking-api.h"
#include "sys/node-id.h"

#include "../shared/coap/coap-datatypes.h"
#include "../shared/coap/coap-client-query.h"
#include "../shared/custom-schedule.h"

PROCESS(client_process_v1b, "client process with customer scan");
AUTOSTART_PROCESSES(&client_process_v1b);

static struct etimer et;

void
client_chunk_handler(coap_message_t* response)
{
    const uint8_t* chunk;

    if (response == NULL) {
        puts("Request timed out");
        return;
    }

    int len = coap_get_payload(response, &chunk);

    printf("|%.*s", len, (char*)chunk);
}

////start of process

PROCESS_THREAD(client_process_v1b, ev, data)
{
    coap_endpoint_t server_ep; //defines server ip --> mke sure that server in cooja uses the ones defined above
    uip_ipaddr_t root;

    PROCESS_BEGIN();

    initialize_tsch_schedule();

    static coap_message_t request[1];      //defines blank request as pointer
    etimer_set(&et, TOGGLE_INTERVAL* node_id * CLOCK_SECOND); //dummy timmer var, will be used to send db requests periodically

    while (1) {
        PROCESS_YIELD(); //surrenders hardware control to cooja/contiki scheduler
        if (etimer_expired(&et)) { //if timer is finished, we'll send a price request
            if (NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&root)) {
                // Convert root IPv6 address to string
                char ip[UIPLIB_IPV6_MAX_STR_LEN];
                uiplib_ipaddr_snprint(ip, sizeof(ip), &root);

                coap_endpoint_parse(ip, strlen(ip), &server_ep);


                if ((node_id % 2) == 0) { //if even node id, send scans
                    coap_init_message(request, COAP_TYPE_CON, COAP_POST, 0); //prepare
                    coap_set_header_uri_path(request, SCAN_URI);
                    //construct a payload to send to server with scan
                    char outputmsg[TX_LEN_POST];
                    client_task(outputmsg, NODE_SCAN, node_id, node_id, 500, 0); //create request for a scan for product id <node_id * 10000> and client id <node_id> and ADD 500 of the item
                    //send
                    coap_set_payload(request, (char*)&outputmsg, TX_LEN_POST - 1); //set the struct to be the payload
                    COAP_BLOCKING_REQUEST(&server_ep, request, client_chunk_handler); //send to server
                    
                    etimer_reset(&et);

                } else {
                    /* prepare request, TID is set by COAP_BLOCKING_REQUEST() */
                    coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0); //prepare
                    coap_set_header_uri_path(request, QUERY_URI);
                    //construct a payload to send to server with scan
                    char outputmsg[TX_LEN_REQ];
                    client_task(outputmsg, NODE_QUERY, node_id, 0, 0, 0); //create request for a product info query for product id <node_id * 10000> 
                    
                    //send :3
                    coap_set_payload(request, (char*)&outputmsg, TX_LEN_REQ- 1); //set the struct to be the payload
                    COAP_BLOCKING_REQUEST(&server_ep, request, client_chunk_handler); //send to server

                    
                    etimer_reset(&et);

                }
            }
        }
    }

    PROCESS_END();
}






