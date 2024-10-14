
#include "coap-engine.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../coap-headers/coap-server-scan.h"
#include "../coap-headers/coap-datatypes.h"


static void res_post_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
//static req_product_data_t unpack_get_payload(coap_message_t* request); //function returns a struct contianing the product data contained in the recieved coap packet

// Global customer list head pointer (initially NULL)
customer_tab_t* customers = NULL;
/*
// Initialize the customer database
void init_database(void) {
    customer_list_head = NULL;
}*/


RESOURCE(res_scan,
         "title=\"Customer scanning: ?len=0..\";rt=\"Text\"",
         NULL,
         res_post_handler,
         NULL,
         NULL);

static void res_post_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{

    //unpacking (text) request from client into struct
    scan_data_t request_data = unpack_scan_payload(request);//get struct corresponding to customer scan request
    //convert to appropriate and more usable forms
    int scan_customer_id = atoi(request_data.product_id);
    int scan_product_quantity = atoi(request_data.quantity);
    char scan_product_id[PRODUCT_ID_LEN];
    sprintf(scan_product_id, "%s", request_data.product_id);

    //will be sent back to client for confirmation
    char return_msg[TX_LEN_POST];

    //updating database
    customer_tab_t* customer = find_or_add_customer(&customers, scan_customer_id); //pointer to tab entry corresponding to customer data
    int commanded_action = atoi(request_data.command);
    //printf("%s",request_data.command);
    printf("EQ %d", commanded_action);
    //printf("Pid %s \n", request_data.product_id);
    //printf("Cid %s \n", request_data.customer_id);
    //printf("qty %s \n", request_data.quantity);
    if (commanded_action == 4) {//wipe customer data from tab --> check out
        //wipe_customer(customers, scan_customer_id, return_msg); //remove from database (infrequent)
        sprintf(return_msg, "ERROR in database command access.\n"); //we'll fix this later

    }
    else if ((commanded_action == 0) | (commanded_action == 1) | (commanded_action == 2)) {//if tab ADD, REMOVE or DELETE operation is requested
        product_order_t* scanned_product = find_or_add_product(&customer->products, scan_product_id); //find location of product, if any (and add if not)
        // Modify product quantity based on command
        modify_product_quantity(scanned_product, scan_product_quantity, commanded_action, return_msg);
    }
    else { //invalid command i guess? Fix me later
        //sprintf(return_msg, "Sisko is the best captain \n");
        sprintf(return_msg, "ERROR in database command access. \n");
    }
    const char *len = NULL;
    //char message[200]; //field to store data, will be culed later
    //sprintf(message, "%s%s%s%s%s%s%s\n",queried_product.product_id,TRANSX_SEP, queried_product.product_price,TRANSX_SEP, queried_product.product_description,TRANSX_SEP, queried_product.is_stocked); //construct a text message containing the database data that we'll send back to client
    
    int length = 200;
    if(coap_get_query_variable(request, "len", &len)) {
    length = atoi(len);
    if(length < 0) {
      length = 0;
    }
    if(length > REST_MAX_CHUNK_SIZE) {
      length = REST_MAX_CHUNK_SIZE;
    }
    memcpy(buffer, return_msg, length);
  } else {
    memcpy(buffer, return_msg, length);
  }
//now we send back the text-formatted database data
  coap_set_header_content_format(response, TEXT_PLAIN); 
  coap_set_header_etag(response, (uint8_t *)&length, 1);
  coap_set_payload(response, buffer, length); //send back acknowledgment
}
