#include "coap.h"
#include <stddef.h>

coap_message_t coap_create_request(coap_method_t method,
                              const char* path,
                              coap_message_type_t type,
                              const void* payload,
                              size_t len)
{
    coap_message_t request;

    coap_init_message(&request, type, method, 0);
    coap_set_header_uri_path(&request, path);
    coap_set_payload(&request, payload, len);

    return request;
}
