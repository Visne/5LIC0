#pragma once

coap_message_t coap_create_request(coap_method_t method,
                                   const char* path,
                                   coap_message_type_t type,
                                   void* payload,
                                   size_t len);
