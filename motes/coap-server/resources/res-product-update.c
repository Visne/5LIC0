#include <stdio.h>
#include "coap-engine.h"

coap_resource_t res_product_update;

static void res_get_handler(coap_message_t *request,
                            coap_message_t *response,
                            uint8_t *buffer,
                            uint16_t preferred_size,
                            int32_t *offset)
{
    coap_set_payload(response, buffer, snprintf((char *) buffer, preferred_size, "hello!"));
}

static void notify() {
    coap_notify_observers(&res_product_update);
}

EVENT_RESOURCE(res_product_update,
               "title='Product updates'",
               res_get_handler,
               NULL,
               NULL,
               NULL,
               notify);
