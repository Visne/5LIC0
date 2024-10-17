#include "coap-engine.h"
#include "sys/log.h"

#define LOG_MODULE "CoAPScan"
#define LOG_LEVEL LOG_LEVEL_DBG

#include "../../shared/coap/coap-server-scan.h"

static void res_post_handler(coap_message_t* request, coap_message_t* response, uint8_t* buffer, uint16_t preferred_size, int32_t* offset);

// Global customer list head pointer (initially NULL)
customer_tab_t* customers = NULL;

RESOURCE(res_scan,
    "title=\"Customer scanning: ?len=0..\";rt=\"Text\"",
    NULL,
    res_post_handler,
    NULL,
    NULL);

static void res_post_handler(coap_message_t* request, coap_message_t* response, uint8_t* buffer, uint16_t preferred_size, int32_t* offset)
{
    // TODO: Return struct instead of string

    scan_data_t request_data = *(scan_data_t*) request->payload;

    //will be sent back to client for confirmation
    char return_msg[TX_LEN_POST];

    //updating database
    customer_tab_t* customer = find_or_add_customer(&customers, request_data.customer_id); //pointer to tab entry corresponding to customer data

    if (request_data.command == 4) {//wipe customer data from tab --> check out
        wipe_customer(&customers, request_data.customer_id, return_msg); //remove from database (infrequent)
        //sprintf(return_msg, "ERROR in database command access.\n"); //FIXME
    } else if ((request_data.command == 0) | (request_data.command == 1) | (request_data.command == 2)) {//if tab ADD, REMOVE or DELETE operation is requested
        product_order_t* scanned_product = find_or_add_product(&customer->products, request_data.product_id); //find location of product, if any (and add if not)
        // Modify product quantity based on command
        if (scanned_product != NULL) {
            modify_product_quantity(scanned_product, request_data.quantity, request_data.command, return_msg);
        } else {
            sprintf(return_msg, "ERROR in allocating memory for product");
        }
    } else { //invalid command i guess? Fix me later
        //sprintf(return_msg, "Sisko is the best captain \n");
        sprintf(return_msg, "ERROR in database command access.");
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
