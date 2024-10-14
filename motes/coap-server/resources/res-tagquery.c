#include "coap-engine.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../coap-headers/coap-server-query.h"
#include "../coap-headers/coap-datatypes.h"


static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
//static req_product_data_t unpack_get_payload(coap_message_t* request); //function returns a struct contianing the product data contained in the recieved coap packet

RESOURCE(res_tagquery,
         "title=\"Tag query: ?len=0..\";rt=\"Text\"",
         res_get_handler,
         NULL,
         NULL,
         NULL);

static void
res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{

    //set up database
    static product_info_t database[DB_SIZE];
    init_test_database(database, (unsigned long long int)DB_SIZE);

    //unpacking (text) request from client into struct
    req_product_data_t request_data = unpack_get_payload(request); 
    product_info_t queried_product = db_query_read(database, request_data.product_id); //returns the database entry for the product with this id
    
    //int db_id = atoi(request_data.product_id); //for now/FIXME : turns the product ID into an index into database
    //stores database data into char fields for later use
    //char database_price[16], database_id[16];
    //sprintf(database_id, "%s", database[db_id].product_id);
    //sprintf(database_price, "%s", database[db_id].product_price);
      
    const char *len = NULL;
    char message[200]; //field to store data, will be culed later
    sprintf(message, "%s%s%s%s%s%s%s\n",queried_product.product_id,TRANSX_SEP, queried_product.product_price,TRANSX_SEP, queried_product.product_description,TRANSX_SEP, queried_product.is_stocked); //construct a text message containing the database data that we'll send back to client
    
    int length = TX_LEN_REQ;
    if(coap_get_query_variable(request, "len", &len)) {
    length = atoi(len);
    if(length < 0) {
      length = 0;
    }
    if(length > REST_MAX_CHUNK_SIZE) {
      length = REST_MAX_CHUNK_SIZE;
    }
    memcpy(buffer, message, length);
  } else {
    memcpy(buffer, message, length);
  }
//now we send back the text-formatted database data
  coap_set_header_content_format(response, TEXT_PLAIN); 
  coap_set_header_etag(response, (uint8_t *)&length, 1);
  coap_set_payload(response, buffer, length);
}
