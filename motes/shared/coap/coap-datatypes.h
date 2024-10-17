///FILE contains application specific datatypes and variables/macros. Methods called by servers or clients are stored in other headers
#pragma once
#include "../../shared/coap/coap-datatypes.h"

#define PRODUCT_ID_LEN 16
#define PRODUCT_DESCRIPT_LEN 48
#define PRODUCT_PRICE_LEN 16
#define PRODUCT_STOCKED_LEN 8

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

typedef struct { //symbolizes a product
    char product_id[PRODUCT_ID_LEN]; //EAN-13 product id
    char product_price[PRODUCT_PRICE_LEN]; //price in cents
    char product_description[PRODUCT_DESCRIPT_LEN]; //short product descriptor
    char is_stocked[PRODUCT_STOCKED_LEN]; //1 if stocked, 0 if not
} product_info_coap_msg_t;

typedef struct { //datastructure used for transmitting product information requests
    char product_id[PRODUCT_ID_LEN]; //EAN13 product ID
    char blankbuffer[PRODUCT_DESCRIPT_LEN]; //some data
} req_product_data_t;

typedef struct { //used by client to transmit scan requests
<<<<<<< HEAD
    uint16_t customer_id;
    uint16_t product_id;
    uint16_t quantity;
    // TODO: Turn into enum
    // Used to indicate between 0=ADD (add customer to database or product to existing tab), 1=REM (remove N product
    // from customer tab), 2=DELETE (wipe product entry from customer tab) or 3=WIPE (remove customer entry and tab)
    char command;
} scan_data_t;
=======
    char customer_id[CUSTOMER_ID_LEN];
    char product_id[PRODUCT_ID_LEN];
    char quantity[ORDER_QUANTITY_LEN];
    char command[ORDER_DB_COMMAND_LEN]; //used to indicate between 0=ADD (add customer to database or product to existing tab), 1=REM (remove N product from customer tab), 2=DELETE (wipe product entry from customer tab) or 3 =WIPE (remove customer entry and tab)
} scan_data_coap_t;
>>>>>>> 316b1b2a1607290622eabc65a63de9aa246281df

//customer purchase database structs
typedef struct product_order_t {
    uint16_t product_id;
    int quantity;
    struct product_order_t *next; // For dynamic list of products -> new products are added here
} product_order_t;

// Structure to store each customer's data
typedef struct customer_tab_t{
    int customer_id; //later look into doing this with chars instead
    product_order_t* products;  // Linked list of products
    struct customer_tab_t *next; // For dynamic list of customers --> new customers are added here
} customer_tab_t;