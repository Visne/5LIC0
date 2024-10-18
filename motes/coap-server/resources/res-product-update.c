#include <stdio.h>
#include "coap-engine.h"
#include "datatypes.h"
#include "sys/log.h"

#define LOG_MODULE "ResUpdate"
#define LOG_LEVEL LOG_LEVEL_DBG

coap_resource_t res_product_update;

static void res_get_handler(coap_message_t *request,
                            coap_message_t *response,
                            uint8_t *buffer,
                            uint16_t preferred_size,
                            int32_t *offset)
{
    const int id = 2;
    static product_t product = {
        id,
        (2 * id + 2) * (0.115) * 100,
        true,
    };
    sprintf(product.description, "Updated Product #%d", id);

    coap_set_payload(response, &product, sizeof(product));
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
