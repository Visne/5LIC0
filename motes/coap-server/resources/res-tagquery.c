#include "coap-engine.h"
#include "db.h"
#include <stdio.h>

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

RESOURCE(res_tagquery,
         "title=\"Tag query: ?len=0..\";rt=\"Text\"",
         res_get_handler,
         NULL,
         NULL,
         NULL);

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
    // Set up database
    static product_t database[DB_SIZE];
    init_test_database(database, DB_SIZE);

    product_update_request update_request = *(product_update_request*) request->payload;
    printf("Query for product %ld\n", update_request.product_id);
    // Returns the database entry for the product with this ID
    product_t queried_product = db_query_read(database, update_request.product_id);

    //int db_id = atoi(request_data.product_id); //for now/FIXME : turns the product ID into an index into database
    //stores database data into char fields for later use
    //char database_price[16], database_id[16];
    //sprintf(database_id, "%s", database[db_id].product_id);
    //sprintf(database_price, "%s", database[db_id].product_price);

    coap_set_payload(response, &queried_product, sizeof(queried_product));
}