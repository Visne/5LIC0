#include "coap-engine.h"
#include "coap.h"
#include "sys/log.h"

#define LOG_MODULE "ResPush"
#define LOG_LEVEL LOG_LEVEL_DBG

static void res_get_handler(coap_message_t *request,
                            coap_message_t *response,
                            unsigned char *buffer,
                            unsigned short preferred_size,
                            signed int *offset);
static void res_periodic_handler(void);

PERIODIC_RESOURCE(res_push,
    "title=\"Periodic demo\";obs",
    res_get_handler,
    NULL,
    NULL,
    NULL,
    5000,
    res_periodic_handler);

static signed int event_counter = 0;

static void res_get_handler(coap_message_t *request,
                            coap_message_t *response,
                            unsigned char *buffer,
                            unsigned short preferred_size,
                            int *offset)
{
    coap_set_header_content_format(response, TEXT_PLAIN);
    coap_set_header_max_age(response, res_push.periodic->period / CLOCK_SECOND);
    coap_set_payload(response,
                     buffer,
                     snprintf((char *) buffer, preferred_size, "VERY LONG EVENT %lu", (unsigned long) event_counter));
}

static void res_periodic_handler() {
    LOG_INFO("Notifying observers\n");

    ++event_counter;

    coap_notify_observers(&res_push);
}
