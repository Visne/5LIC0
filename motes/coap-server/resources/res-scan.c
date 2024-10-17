#include "coap-engine.h"
#include "sys/log.h"

#define LOG_MODULE "CoAPScan"
#define LOG_LEVEL LOG_LEVEL_DBG

#include "../../shared/coap/coap-server-scan.h"

static void res_post_handler(coap_message_t* request, coap_message_t* response, uint8_t* buffer, uint16_t preferred_size, int32_t* offset);

// Global customer list head pointer (initially NULL)
customer_tab_t* customers = NULL;

RESOURCE(res_scan,
    "title='Customer scanning'",
    NULL,
    res_post_handler,
    NULL,
    NULL);

static void res_post_handler(coap_message_t* request, coap_message_t* response, uint8_t* buffer, uint16_t preferred_size, int32_t* offset)
{
    // TODO: Return struct instead of string

    scan_data_t request_data = *(scan_data_t*) request->payload;

    // Will be sent back to client for confirmation
    char return_msg[TX_LEN_POST];

    // Updating database
    customer_tab_t* customer = find_or_add_customer(&customers, request_data.customer_id);

    product_order_t* scanned_product;
    switch (request_data.command) {
        case ADD:
        case REMOVE:
        case DELETE:
            // Find location of product, if any (and add if not)
            scanned_product = find_or_add_product(&customer->products, request_data.product_id);

            // Modify product quantity based on command
            if (scanned_product == NULL) {
                LOG_ERR("ERROR in allocating memory for product");
                return;
            }

            modify_product_quantity(scanned_product, request_data.quantity, request_data.command, return_msg);
            break;
        case WIPE:
            wipe_customer(&customers, request_data.customer_id);
            return;
    }

    const char* len = NULL;

    int length = 200;
    if (coap_get_query_variable(request, "len", &len)) {
        length = atoi(len);
        if (length < 0) {
            length = 0;
        }
        if (length > REST_MAX_CHUNK_SIZE) {
            length = REST_MAX_CHUNK_SIZE;
        }
    }

    // Now we send back the text-formatted database data
    coap_set_header_content_format(response, TEXT_PLAIN);
    coap_set_payload(response, &return_msg, length); //send back acknowledgment
}
