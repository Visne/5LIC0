#pragma once
#include <stdint.h>
#include <stdbool.h>

#define PRODUCT_DESCRIPT_LEN 48

// Used to indicate how many elements are in test product database
#define DB_SIZE 500
// Used to indicate how many customers are currently using system (testing)
#define CUSTOMER_DB_SIZE 100

#define SCAN_URI "scan"
#define QUERY_URI "product/query"
#define UPDATE_URI "product/update"

typedef uint64_t ean13_t;
typedef uint32_t customer_t;

typedef enum {
    // Add customer to database or product to existing tab
    ADD,
    // Remove N product from customer tab
    REMOVE,
    // Wipe product entry from customer tab
    DELETE,
    // Remove customer entry and tab
    WIPE,
} scan_type_t;

// Symbolizes a product
typedef struct {
    ean13_t id;
    // Price in cents
    uint16_t price;
    // Short product descriptor
    bool is_stocked;
    char description[PRODUCT_DESCRIPT_LEN];
} product_t;

// Used by client to transmit scan requests
typedef struct {
    customer_t customer_id;
    ean13_t product_id;
    uint16_t quantity;
    scan_type_t command;
} scan_data_t;

// Customer purchase database structs
typedef struct product_order_t {
    ean13_t product_id;
    uint16_t quantity;
    // For dynamic list of products -> new products are added here
    struct product_order_t *next;
} product_order_t;

// Structure to store each customer's data
typedef struct customer_tab_t {
    customer_t customer_id;
    // Linked list of products
    product_order_t* products;
    // For dynamic list of customers --> new customers are added here
    struct customer_tab_t *next;
} customer_tab_t;