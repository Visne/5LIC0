#pragma once
#include "coap-datatypes.h"
#include "coap-engine.h"
#include "contiki.h"
#include "contiki-net.h"
#include "coap-engine.h"
#include "coap-blocking-api.h"

product_info_t decode_product_response(coap_message_t* response) {

	const uint8_t* chunk;
	char responsebuffer[TX_LEN_REQ];
	int len = coap_get_payload(response, &chunk);
	sprintf(responsebuffer, "%.*s", len, (char*)chunk);

    product_info_t product_data;

    char* pos; //pointer to seperator element
    pos = strtok(responsebuffer, TRANSX_SEP); //find first (currently only) seperator

    //adjust based on response struct structure --> construct struct 
    sprintf(product_data.product_id, "%s%s", pos,"\0");
    pos = strtok(NULL, TRANSX_SEP);
    sprintf(product_data.product_price, "%s%s", pos,"\0");
    pos = strtok(NULL, TRANSX_SEP);
    sprintf(product_data.product_description, "%s%s", pos,"\0");
    pos = strtok(NULL, TRANSX_SEP);
    sprintf(product_data.is_stocked, "%s%s", pos, "\0");
    return product_data;//return reconstructed struct
}

void client_task(char* outputmsg, int node_mode, unsigned long long product_id, unsigned long long customer_id, int scan_qty, int scan_command) { //used to send coap product scan and query 


    if (node_mode == NODE_QUERY) { //query product info
        printf("Query ");


        req_product_data_t product;
        //writing to product info struct
        sprintf(product.product_id, "%llu", product_id);
        snprintf(product.blankbuffer, sizeof(product.blankbuffer), "Query info"); //can be changed depending on what we need

        //char outputmsg[TX_LEN_REQ]; //we'll define a buffer containing text corresponding to the product information request --> for now can only realistically send text through coap

        sprintf(outputmsg, "%s:%s%s", product.product_id, product.blankbuffer, "\0"); //creating string containing ID:data (data currently blank)
        //coap_set_payload(request, (char*)&outputmsg, sizeof(outputmsg) - 1); //set the struct to be the payload

        //return request;

    }
    else if (node_mode == NODE_SCAN) { //send a product scan
        //char customer_id_char[CUSTOMER_ID_LEN];
        //sprintf(customer_id_char, "%llu", customer_id);
        printf("Scan ");

        //char outputmsg[TX_LEN_POST]; //we'll define a buffer containing text corresponding to the product information request --> for now can only realistically send text through coap

        sprintf(outputmsg, "%llu%s%llu%s%d%s%d", customer_id, TRANSX_SEP, product_id, TRANSX_SEP,
            scan_qty, TRANSX_SEP,
            scan_command); //creating string containing ID:data (data currently blank)

    }
    else {
        printf("Something went wrong");
        //request = NULL;
        //return request;
    }
}
