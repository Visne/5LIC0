#include "coap-engine.h"
#include "db.h"
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

    // Returns the database entry for the product with this ID
    product_t product = db_query_read(database, *request->payload);
    coap_set_payload(response, &product, sizeof(product));
}

RESOURCE(res_tagquery,
         "title='Tag query'",
         res_get_handler,
         NULL,
         NULL,
         NULL);
