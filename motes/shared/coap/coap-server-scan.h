#pragma once
#include "../../shared/coap/coap-datatypes.h"

/*Database-related methods*/

// Find the location of the customer order sheet or create one
customer_tab_t* find_or_add_customer(customer_tab_t** head, int customer_id) {
    customer_tab_t* temp = *head; //start at begining of linked list

    while (temp != NULL) { //look thru every customer in database until match, or end of file
        if (temp->customer_id == customer_id) //if customer is currently present in database, stop looking
            return temp; //return pointer to customer data
        temp = temp->next; //if current customer in linked list is not the one looked for, look to next item
    }

    // Create new customer if not found
    customer_tab_t* new_customer = (customer_tab_t*)malloc(sizeof(customer_tab_t)); //alocate new space to a customer tab
    new_customer->customer_id = customer_id;
    new_customer->products = NULL;
    new_customer->next = *head; // Add to head of list
    *head = new_customer;

    return new_customer;
}

// Find the location of the customer product order entry or create one
product_order_t* find_or_add_product(product_order_t** product_list, uint16_t product_id) { //NOTE: first arg is the head of the linke dlist containing product data FOR A PREDETERMINED CUSTOMER
    product_order_t* temp = *product_list;

    while (temp != NULL) { //lets look thru every product the customer has scanned
        if (temp->product_id == product_id) //if the customer has already ordered the product
            return temp;
        temp = temp->next;
    }

    // Create new product entry if not found
    product_order_t* new_product = (product_order_t*)malloc(sizeof(product_order_t));
    new_product->product_id = product_id;
    new_product->quantity = 0;
    new_product->next = *product_list; // Add to head of list
    *product_list = new_product;

    return new_product;
}

void modify_product_quantity(product_order_t* product, int quantity, int command, char* output_msg) {
    if (command == 0) {  // ADD 
        product->quantity += quantity;
        sprintf(output_msg, "ADD.ID%hu%sQTY%d", product->product_id, TRANSX_SEP, product->quantity);
    } else if (command == 1) {  // REMOVE
        if (product->quantity > quantity) {
            product->quantity -= quantity;

        } else { //make sure we dont get negative amount of outstanding items in cart
            product->quantity = 0;
        }
        sprintf(output_msg, "REM.ID%hu%sQTY%d", product->product_id, TRANSX_SEP, product->quantity);
    } else if (command == 2) {  // DELETE product entry
        product->quantity = 0;
        sprintf(output_msg, "REM.ID%hu%sQTY%d", product->product_id, TRANSX_SEP, product->quantity);//returned for user/client
    }
}


// Remove customer and all their products
void wipe_customer(customer_tab_t** head, int customer_id, char* output_msg) { //WIPE operation
    customer_tab_t* temp = *head;
    customer_tab_t* prev = NULL;

    while (temp != NULL && temp->customer_id != customer_id) {
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL) return; // Customer not found

    // Free all product entries
    product_order_t* product = temp->products;
    while (product != NULL) {
        product_order_t* next_product = product->next;
        free(product);
        product = next_product;
    }

    // Remove customer from list
    if (prev != NULL) {
        prev->next = temp->next;
    }
    else {
        *head = temp->next;
    }

    free(temp);
    sprintf(output_msg, "Customer %d wiped %s", customer_id, "\0"); //returned for user/client
}
<<<<<<< HEAD
=======
*/
/*Data transmisison related methods*/



static scan_data_coap_t unpack_scan_payload(coap_message_t* request) { //turns raw POST request data into usable scan_data_coap_t payload
    const uint8_t* chunk;
    static scan_data_coap_t decoded_struct; //raw struct to write to
    /*
    if (request == NULL) { //borrowed from client chunk handler, checks if request is somehow empty
        //do something
    return;
    }*/

    int len = coap_get_payload(request, &chunk); //returns pointer to request data to chink while writing length of request to len
    char payloadbuffer[len + 10]; //building a buffer for text extraction
    sprintf(payloadbuffer, "%.*s\n", len, (char*)chunk); //write payload (from pointer chunk of length len) to payloadbuffer -> now we have text containing product info 

    char* pos; //pointer to seperator element
    pos = strtok(payloadbuffer, TRANSX_SEP); //find first (currently only) seperator
    //adjust based on return struct structure --> construct struct  
    sprintf(decoded_struct.customer_id, "%s", pos);
    pos = strtok(NULL, TRANSX_SEP);
    sprintf(decoded_struct.product_id, "%s", pos );
    pos = strtok(NULL, TRANSX_SEP);
    sprintf(decoded_struct.quantity, "%s", pos);
    pos = strtok(NULL, TRANSX_SEP);
    sprintf(decoded_struct.command, "%s", pos);
    //now we copy to output struct element
    return decoded_struct; //return completed struct
}
>>>>>>> 316b1b2a1607290622eabc65a63de9aa246281df
