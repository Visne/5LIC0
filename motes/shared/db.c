#include "datatypes.h"
#include "sys/log.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define LOG_MODULE "Database"
#define LOG_LEVEL LOG_LEVEL_DBG

// Initialize database with testing data
void init_test_database(product_info_t* db, uint64_t size) {
    for (unsigned long long i = 0; i < size; i++) {
        db[i].product_id = i + 1;
        db[i].product_price = (2 * i + 2) * (0.115) * 100;
        sprintf(db[i].product_description, "Highly sought commoditity n %llu", i);
        db[i].is_stocked = 1;
    }
}

// Searches initialized database for matching EAN-13 product ID and returns info
product_info_t db_query_read(product_info_t* db, uint64_t product_id) {
    bool product_found = false;

    unsigned long long i = 0; //index into database to be iterated with
    product_info_t db_product; //blank product information sheet that we'll send out

    while (product_found == 0) { //until found in database
        if (i >= DB_SIZE) { //no such element in DB
            db_product.product_id = product_id;
            db_product.product_price = 0;
            sprintf(db_product.product_description, "NA");
            db_product.is_stocked = 0;
            return db_product;
        }

        if (db[i].product_id == product_id) {//found match
            product_found = true;
        }

        // Else keep looking
        i = i + 1;
    }

    db_product.product_id = product_id;
    db_product.product_price = db[i-1].product_price;
    sprintf(db_product.product_description, "%s", db[i - 1].product_description);
    db_product.is_stocked = 1;
    return db_product;
}

// Find the location of the customer order sheet or create one
customer_tab_t* find_or_add_customer(customer_tab_t** head, uint32_t customer_id) {
    customer_tab_t* temp = *head; //start at begining of linked list

    while (temp != NULL) { //look thru every customer in database until match, or end of file
        if (temp->customer_id == customer_id) //if customer is currently present in database, stop looking
            return temp; //return pointer to customer data
        temp = temp->next; //if current customer in linked list is not the one looked for, look to next item
    }

    // Create new customer if not found
    customer_tab_t* new_customer = (customer_tab_t*) malloc(sizeof(customer_tab_t)); //alocate new space to a customer tab
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
    product_order_t* new_product = (product_order_t*) malloc(sizeof(product_order_t));
    new_product->product_id = product_id;
    new_product->quantity = 0;
    new_product->next = *product_list; // Add to head of list
    *product_list = new_product;

    return new_product;
}

void modify_product_quantity(product_order_t* product, uint16_t quantity, scan_type_t command) {
    switch (command) {
        case ADD:
            product->quantity += quantity;
            LOG_INFO("ADD ID: %hu, QTY: %d\n", product->product_id, product->quantity);
            break;
        case REMOVE:
            if (product->quantity > quantity) {
                product->quantity -= quantity;
            } else {
                // Make sure we don't get negative amount of outstanding items in cart
                product->quantity = 0;
            }
            LOG_INFO("REMOVE ID: %hu, QTY: %d\n", product->product_id, product->quantity);
            break;
        case DELETE:
            product->quantity = 0;
            LOG_INFO("DELETE ID: %hu, QTY: %d\n", product->product_id, product->quantity);
            break;
        case WIPE:
            break;
    }
}

// Remove customer and all their products
void wipe_customer(customer_tab_t **head, uint32_t customer_id) {
    customer_tab_t* temp = *head;
    customer_tab_t* prev = NULL;

    while (temp != NULL && temp->customer_id != customer_id) {
        prev = temp;
        temp = temp->next;
    }

    // Customer not found
    if (temp == NULL) {
        return;
    }

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
    } else {
        *head = temp->next;
    }

    free(temp);
    LOG_DBG("Customer %u wiped\n", customer_id);
}
