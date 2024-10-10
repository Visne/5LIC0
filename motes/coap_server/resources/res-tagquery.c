/*
 * Copyright (c) 2013, Institute for Pervasive Computing, ETH Zurich
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *      Example resource
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 */

#include <stdlib.h>
#include <string.h>
#include "coap-engine.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define DB_SIZE 20 //used to indicate how manye elems ar ein play database

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
//static req_product_data_t unpack_get_payload(coap_message_t* request); //function returns a struct contianing the product data contained in the recieved coap packet

typedef struct database_elem {
    char product_id[16]; //product id
    char product_price[16]; //price in euro
} database_elem_t;

typedef struct req_product_data {
    char product_id[16];
    char blankbuffer[16];
} req_product_data_t;




void init_database(database_elem_t* db, int size) { //initialized database with testing data
    for (int i = 0; i < size; i++) {
       sprintf(db[i].product_id, "%d", i+1);
       float price = (7 * i + 5) * (0.33);
       sprintf(db[i].product_price, "%2.2f", price);
        //sprintf(db[i].product_price, "%2.2f", i * 1.5);
    }
}


/*
 * A handler function named [resource name]_handler must be implemented for each RESOURCE.
 * A buffer for the response payload is provided through the buffer pointer. Simple resources can ignore
 * preferred_size and offset, but must respect the REST_MAX_CHUNK_SIZE limit for the buffer.
 * If a smaller block size is requested for CoAP, the REST framework automatically splits the data.
 */
RESOURCE(res_tagquery,
         "title=\"Tag query: ?len=0..\";rt=\"Text\"",
         res_get_handler,
         NULL,
         NULL,
         NULL);

static req_product_data_t unpack_get_payload(coap_message_t* request){
    const uint8_t* chunk;


    //if (request == NULL) { //borrowed from client chunk handler, checks if request is somehow empty
        //do something
      //  return;
    //}

    int len = coap_get_payload(request, &chunk); //returns pointer to request data to chink while writing length of request to len
    char payloadbuffer[len + 10]; //building a buffer for text extraction
    char payload_id[len + 10]; //building blank arrays to contain payload information
    char payload_blankdata[len + 10]; 
    sprintf(payloadbuffer, "%.*s \n", len, (char*)chunk); //write payload (from pointer chunk of length len) to payloadbuffer -> now we have text containing product info
    char* pos; //pointer to ':' element
    pos = strchr(payloadbuffer, ':'); //find pointer to part of payload which corresponds to seperator
    int id_length = pos - payloadbuffer; //how many chars is the id? compare address of ':' to start of string
    int databuffer_length = (payloadbuffer + len) - pos - 1; //(end of string pointer) - position of seperator = len(":databuffertext"). We remove 1 to get rid of seperator contribution
    strncpy(payload_id, payloadbuffer, id_length); //copies the product id to string
    strncpy(payload_blankdata, (payloadbuffer + id_length + 1), databuffer_length); //copies data after seperator to buffer
    //now we copy to output struct element
    static req_product_data_t decoded_struct;
    sprintf(decoded_struct.product_id, "%s", payload_id);
    sprintf(decoded_struct.blankbuffer, "%s", payload_blankdata);
    return decoded_struct; //return completed struct
}

static void
res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{

    //set up database
    static database_elem_t database[DB_SIZE];
    init_database(database, 10);
    //populate dummy database for testing
    /*
    sprintf(database[0].product_id, "%d", 1);
    sprintf(database[0].product_price, "%2.2f", 29.99);
    sprintf(database[1].product_id, "%d", 2);
    sprintf(database[1].product_price, "%2.2f", 42.5);
    sprintf(database[2].product_id, "%d", 3);
    sprintf(database[2].product_price, "%2.2f", 19.99);
    sprintf(database[3].product_id, "%d", 4);
    sprintf(database[3].product_price, "%2.2f", 35.49);
    sprintf(database[4].product_id, "%d", 5);
    sprintf(database[4].product_price, "%2.2f", 5.89);
    sprintf(database[5].product_id, "%d", 6);
    sprintf(database[5].product_price, "%2.2f", 2.99);
    sprintf(database[6].product_id, "%d", 7);
    sprintf(database[6].product_price, "%2.2f", 0.99);
    sprintf(database[7].product_id, "%d", 8);
    sprintf(database[7].product_price, "%2.2f", 78.99);
    sprintf(database[8].product_id, "%d", 9);
    sprintf(database[8].product_price, "%2.2f", 76.99);
    sprintf(database[9].product_id, "%d", 10);
    sprintf(database[9].product_price, "%2.2f", 7.99);
    */

    req_product_data_t request_data = unpack_get_payload(request); //we store text request data here
    int db_id = atoi(request_data.product_id);

    //store the fetched data in chars for later submission
    char database_price[16];
    char database_id[16];
    sprintf(database_id, "%s", database[db_id].product_id);
    sprintf(database_price, "%s", database[db_id].product_price);
       
    
  //get payload data from request
  

  const char *len = NULL;
  char message[70];
  sprintf(message, "%s:%s \n",database_id, database_price); //sending back product id
  //static int a = 5;
  //static double b = 5.1;
  /* Some data that has the length up to REST_MAX_CHUNK_SIZE. For more, see the chunk resource. */
  //sprintf(message, "%s:%s\n", database[2].product_id, database[2].product_price);
  int length = 12;       

    if(coap_get_query_variable(request, "len", &len)) {
    length = atoi(len);
    if(length < 0) {
      length = 0;
    }
    if(length > REST_MAX_CHUNK_SIZE) {
      length = REST_MAX_CHUNK_SIZE;
    }
    memcpy(buffer, message, length);
  } else {
    memcpy(buffer, message, length);
  }

  coap_set_header_content_format(response, TEXT_PLAIN); /* text/plain is the default, hence this option could be omitted. */
  coap_set_header_etag(response, (uint8_t *)&length, 1);
  coap_set_payload(response, buffer, length);
}
