#pragma once
#include <stdint.h>
#include <stdbool.h>

#define PRODUCT_DESCRIPT_LEN 48

#define DB_SIZE 500 //used to indicate how manye elems are in play product database
#define CUSTOMER_DB_SIZE 100 //used to indicate how many customers are currently using system (testing)

#define TOGGLE_INTERVAL 10

#define QUERY_URI "test/query"
#define SCAN_URI "test/scan"
#define UPDATE_URI "product/update"

// Symbolizes a product
typedef struct {
    // EAN-13 product ID
    uint64_t product_id;
    // Price in cents
    uint16_t product_price;
    // Short product descriptor
    char product_description[PRODUCT_DESCRIPT_LEN];
    bool is_stocked;
} product_info_t;

// Datastructure used for transmitting product information requests
typedef struct {
<<<<<<< Updated upstream
    // EAN-13 product ID
    uint64_t product_id;
    char blankbuffer[PRODUCT_DESCRIPT_LEN];
} req_product_data_t;

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

// Used by client to transmit scan requests
typedef struct {
    uint32_t customer_id;
    uint16_t product_id;
    uint16_t quantity;
    scan_type_t command;
} scan_data_t;

// Customer purchase database structs
typedef struct product_order_t {
    uint16_t product_id;
    uint16_t quantity;
    // For dynamic list of products -> new products are added here
    struct product_order_t *next;
} product_order_t;

// Structure to store each customer's data
typedef struct customer_tab_t {
    uint32_t customer_id;
    // Linked list of products
    product_order_t* products;
    // For dynamic list of customers --> new customers are added here
    struct customer_tab_t *next;
} customer_tab_t;
