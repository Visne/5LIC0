#include "coap-engine.h"
#include "db.h"
#include "datatypes.h"
#include <stdio.h>

static product_t database[DB_SIZE];
static bool initialized = false;

static void res_get_handler(coap_message_t *request,
                            coap_message_t *response,
                            uint8_t *buffer,
                            uint16_t preferred_size,
                            int32_t *offset)
{
    if (!initialized) {
        // Set up database
        init_test_database(database, DB_SIZE);
    }

    if (request == NULL || request->payload == NULL) {
        printf("Tag query request or payload was null!\n");
        return;
    }

    // Returns the database entry for the product with this ID
    product_update_request update_request = *(product_update_request*) request->payload;
    product_t product = db_query_read(database, update_request.product_id);
    coap_set_payload(response, &product, sizeof(product));
}

RESOURCE(res_tagquery,
         "title='Tag query'",
         res_get_handler,
         NULL,
         NULL,
         NULL);