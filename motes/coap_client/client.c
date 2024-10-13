#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "coap-engine.h"
#include "coap-blocking-api.h"
#include "sys/node-id.h"
#include "../coap-headers/coap-datatypes.h"




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
    static coap_endpoint_t server_ep; //defines server ip --> mke sure that server in cooja uses the ones defined above
    
    PROCESS_BEGIN();
    static coap_message_t request[1];      //defines blank request as pointer
    coap_endpoint_parse(SERVER_EP, strlen(SERVER_EP), &server_ep);
    etimer_set(&et, TOGGLE_INTERVAL* node_id * CLOCK_SECOND); //dummy timmer var, will be used to send db requests periodically

    while (1) {
        PROCESS_YIELD(); //surrenders hardware control to cooja/contiki scheduler
        if (etimer_expired(&et)) { //if timer is finished, we'll send a price request
            //printf("node id %d\n", node_id);
            //two test cases

            if ((node_id % 2) == 0) { //if even node id, send scans 

                /* prepare request, TID is set by COAP_BLOCKING_REQUEST() */
                coap_init_message(request, COAP_TYPE_CON, COAP_POST, 0); //prepare 
                coap_set_header_uri_path(request, "test/scan"); //we'll poll the "product" ressource (at test/hello until i can figure out why any other uri doenst work)

                scan_data_t scan; // a scan
                //lets create a few database access commands (order 500 cigarettes)
                sprintf(scan.customer_id, "%d", node_id*55); //arbitrary
                sprintf(scan.product_id, "%d", node_id * 100000);  //ID for cigarettes
                sprintf(scan.quantity, "%d", 500); // 500
                sprintf(scan.command, "%d", 0); //ADD to inventory

                char outputmsg[TX_LEN_POST]; //we'll define a buffer containing text corresponding to the product information request --> for now can only realistically send text through coap

                sprintf(outputmsg, "%s%s%s%s%s%s%s", scan.customer_id, TRANSX_SEP, scan.product_id, TRANSX_SEP, scan.quantity, TRANSX_SEP, scan.command); //creating string containing ID:data (data currently blank)
                //printf("%s\n",outputmsg);
                coap_set_payload(request, (char*)&outputmsg, sizeof(outputmsg) - 1); //set the struct to be the payload
                COAP_BLOCKING_REQUEST(&server_ep, request, client_chunk_handler); //send to server

                //printf("\n--Done--\n");
                etimer_reset(&et);

            }
            else {
                /* prepare request, TID is set by COAP_BLOCKING_REQUEST() */
                coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0); //prepare 
                coap_set_header_uri_path(request, "test/query"); //we'll poll the "product" ressource (at test/hello until i can figure out why any other uri doenst work)

                req_product_data_t product;
                //writing to product info struct 
                sprintf(product.product_id, "%d", node_id);
                snprintf(product.blankbuffer, sizeof(product.blankbuffer), "StoredInfo");

                char outputmsg[TX_LEN_REQ]; //we'll define a buffer containing text corresponding to the product information request --> for now can only realistically send text through coap

                sprintf(outputmsg, "%s:%s%s", product.product_id, product.blankbuffer, "\0"); //creating string containing ID:data (data currently blank)
                coap_set_payload(request, (char*)&outputmsg, sizeof(outputmsg) - 1); //set the struct to be the payload
                COAP_BLOCKING_REQUEST(&server_ep, request, client_chunk_handler); //send to server

                //printf("\n--Done--\n");
                etimer_reset(&et);

            }
        }
    }

    PROCESS_END();
}






