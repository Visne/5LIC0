///FILE contains application specific datatypes and variables/macros. Methods called by servers or clients are stored in other headers
#pragma once
#include "../../shared/coap/coap-datatypes.h"

#define PRODUCT_ID_LEN 16
#define PRODUCT_DESCRIPT_LEN 48

#define TX_LEN_POST (PRODUCT_ID_LEN + CUSTOMER_ID_LEN + ORDER_DB_COMMAND_LEN + ORDER_QUANTITY_LEN + 12) //defines how much buffer space we need to process text based scans

#define CUSTOMER_ID_LEN 16
#define ORDER_DB_COMMAND_LEN 4
#define ORDER_QUANTITY_LEN 16

#define DB_SIZE 500 //used to indicate how manye elems are in play product database
#define CUSTOMER_DB_SIZE 100 //used to indicate how many customers are currently using system (testing)

#define TOGGLE_INTERVAL 10
#define TRANSX_SEP ":" //seperator used for packing/unpacking data for TX/RX

#define QUERY_URI "test/query"
#define SCAN_URI "test/scan"

//NOTE: all structs meant to be directly (or nearly so) transmitted over COAP should use char fields rather for easier transmission

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
    // EAN-13 product ID
    uint64_t product_id;
    char blankbuffer[PRODUCT_DESCRIPT_LEN]; //some data
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
    uint16_t customer_id;
    uint16_t product_id;
    uint16_t quantity;
    scan_type_t command;
} scan_data_t;

// Customer purchase database structs
typedef struct product_order_t {
    uint16_t product_id;
    uint16_t quantity;
    struct product_order_t *next; // For dynamic list of products -> new products are added here
} product_order_t;

// Structure to store each customer's data
typedef struct customer_tab_t {
    int customer_id;
    // Linked list of products
    product_order_t* products;
    struct customer_tab_t *next; // For dynamic list of customers --> new customers are added here
} customer_tab_t;