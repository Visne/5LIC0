#pragma once
#include "coap-datatypes.h"
#include "coap-engine.h"

product_info_coap_msg_t decode_product_response(coap_message_t* response) {

	const uint8_t* chunk;
	char responsebuffer[TX_LEN_REQ];
	int len = coap_get_payload(response, &chunk);
	sprintf(responsebuffer, "%.*s", len, (char*)chunk);

    product_info_coap_msg_t product_data;

    char* pos; //pointer to seperator element
    pos = strtok(responsebuffer, TRANSX_SEP); //find first (currently only) seperator

    //adjust based on response struct structure --> construct struct 
    sprintf(product_data.product_id, "%s%s", pos,"\0");
    pos = strtok(NULL, TRANSX_SEP);
    sprintf(product_data.product_price, "%s%s", pos,"\0");
    pos = strtok(NULL, TRANSX_SEP);
    sprintf(product_data.product_description, "%s%s", pos,"\0");
    pos = strtok(NULL, TRANSX_SEP);
    sprintf(product_data.is_stocked, "%s%s", pos, "\0");
    return product_data;//return reconstructed struct
}