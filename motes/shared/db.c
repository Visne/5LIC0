#include "datatypes.h"
#include "sys/log.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define LOG_MODULE "Database"
#define LOG_LEVEL LOG_LEVEL_DBG

// Initialize database with testing data
void init_test_database(product_t* db, ean13_t size) {
    for (ean13_t i = 0; i < size; i++) {
        product_t product = {
            i,
            (2 * i + 2) * (0.115) * 100,
            true,
        };
        sprintf(product.description, "Product #%lu", i);

        db[i] = product;
    }
}

// Searches initialized database for matching EAN-13 product ID and returns info
product_t db_query_read(product_t* db, ean13_t product_id) {
    for (ean13_t i = 0; i < DB_SIZE; i++) {
        // Found match
        if (db[i].id == product_id) {
            product_t db_product = {
                product_id,
                db[i].price,
                true,
            };
            strcpy(db_product.description, db[i].description);

            return db_product;
        }
    }

    // No product with that ID in DB
    product_t db_product = { product_id, 0, false, "N/A" };
    return db_product;
}

// Find the location of the customer order sheet or create one
customer_tab_t* find_or_add_customer(customer_tab_t** head, customer_t customer_id) {
    customer_tab_t* temp = *head; //start at begining of linked list

    // Look through every customer in database until match
    while (temp != NULL) {
        // Found
        if (temp->customer_id == customer_id) {
            return temp;
        }

        temp = temp->next;
    }

    // Create new customer if not found
    customer_tab_t* new_customer = malloc(sizeof(customer_tab_t)); //alocate new space to a customer tab
    new_customer->customer_id = customer_id;
    new_customer->products = NULL;
    new_customer->next = *head; // Add to head of list
    *head = new_customer;

    return new_customer;
}

// Find the location of the customer product order entry or create one
// NOTE: first arg is the head of the linked list containing product data FOR A PREDETERMINED CUSTOMER
product_order_t* find_or_add_product(product_order_t** product_list, uint16_t product_id) {
    product_order_t* temp = *product_list;

    // Look through every product the customer has scanned until match
    while (temp != NULL) {
        // Found
        if (temp->product_id == product_id) {
            return temp;
        }

        temp = temp->next;
    }

    // Create new product entry if not found
    product_order_t* new_product = malloc(sizeof(product_order_t));
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
            LOG_INFO("ADD ID: %lu, QTY: %d\n", product->product_id, product->quantity);
            break;
        case REMOVE:
            if (product->quantity > quantity) {
                product->quantity -= quantity;
            } else {
                // Make sure we don't get negative amount of outstanding items in cart
                product->quantity = 0;
            }
            LOG_INFO("REMOVE ID: %lu, QTY: %d\n", product->product_id, product->quantity);
            break;
        case DELETE:
            product->quantity = 0;
            LOG_INFO("DELETE ID: %lu, QTY: %d\n", product->product_id, product->quantity);
            break;
        case WIPE:
            break;
    }
}

// Remove customer and all their products
void wipe_customer(customer_tab_t **head, customer_t customer_id) {
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
