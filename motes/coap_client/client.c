#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "coap-engine.h"
#include "coap-blocking-api.h"
#include "sys/node-id.h"

/* FIXME: This server address is hard-coded for Cooja and link-local for unconnected border router. */
#define SERVER_EP "coap://[fe80::201:1:1:1]"
#define CLIENT_ID 1 //this will be different per node
#define TOGGLE_INTERVAL 10

PROCESS(client_process_v1a, "client process with limited product information request");
AUTOSTART_PROCESSES(&client_process_v1a);

static struct etimer et;

//client response handling block
void client_chunk_handler(coap_message_t* response)
{
    const uint8_t* chunk;

    if (response == NULL) {
        puts("Request timed out");
        return;
    }

    int len = coap_get_payload(response, &chunk);

    printf("Product ID | Product price (EUR): %.*s", len, (char*)chunk);
} 

//datastructure containing a product id that the client can send to request product info
typedef struct client_product_id { //a struct that the client sends product information for price/info get requests -> contains product ID for now
    char product_id[16]; //the product data, eg EAN13 code
    char blankbuffer[32]; //some data
} client_product_id_t;
static client_product_id_t product; //blank product information structure

////start of process

PROCESS_THREAD(client_process_v1a, ev, data)
{
    static coap_endpoint_t server_ep; //defines server ip --> mke sure that server in cooja uses the ones defined above
    
    PROCESS_BEGIN();
    static coap_message_t request[1];      //defines blank request as pointer
    coap_endpoint_parse(SERVER_EP, strlen(SERVER_EP), &server_ep);
    etimer_set(&et, TOGGLE_INTERVAL * CLOCK_SECOND); //dummy timmer var, will be used to send price update requests periodically

    while (1) {
        PROCESS_YIELD(); //surrenders hardware control to cooja/contiki scheduler
        if (etimer_expired(&et)) { //if timer is finished, we'll send a price request
            //printf("node id %d\n", node_id);
            
            /* prepare request, TID is set by COAP_BLOCKING_REQUEST() */
            coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0); //prepare 
            coap_set_header_uri_path(request, "test/query"); //we'll poll the "product" ressource (at test/hello until i can figure out why any other uri doenst work)

            //writing to product info struct 
            sprintf(product.product_id, "%d", node_id);
            snprintf(product.blankbuffer, sizeof(product.blankbuffer), "StoredInfo");

            char outputmsg[64]; //we'll define a buffer containing text corresponding to the product information request --> for now can only realistically send text through coap
            
            sprintf(outputmsg, "%s:%s \n", product.product_id, product.blankbuffer); //creating string containing ID:data (data currently blank)
            coap_set_payload(request, (char*)&outputmsg, sizeof(outputmsg) - 1); //set the struct to be the payload
            COAP_BLOCKING_REQUEST(&server_ep, request, client_chunk_handler); //send to server

            //printf("\n--Done--\n");
            etimer_reset(&et);
        }
    }

    PROCESS_END();
}






