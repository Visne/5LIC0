#ifndef TYPES
#define TYPES

#include <stdint.h>

// #define DEBUG_NODE //Uncomment to enable debug mode for nodes
// #define DEBUG_BUS  //Uncomment to enable debug mode for bus
#define MIN_CUST_ID 0
#define MAX_CUST_ID 100
#define MIN_SCAN_PERIOD 8000 // ms between scans
#define MAX_SCAN_PERIOD 24000 // ms between scans
#define CLUSTER_HEAD_ELECTION_PERIOD 5


#define CLUSTER_VIS_DIRECTORY "/home/quinten/contiki-ng/5LIC0/vis/"

// 29 bit ID + 64 byte payload = 541 bits per message. 1Mbit/s -> 1.000.000 / 541 =~ 1848 msgs per second
#define CAN_FREQ 1848
#define CAN_UNIT_STEP 0.0078125 // representation of time in seconds to send 1 CAN message, currently set to 1/CLOCK_SECOND

#define UNDEFINED_PRODUCT_ID 0 // Product ID 0 is forbidden

typedef struct product_info_msg {
    /// Price of the product in cents
    unsigned long product_id;
    unsigned short price;
    char* product_name;
    unsigned short product_name_len;
} product_info_msg_t;

typedef struct scan_data_msg {
    unsigned long customer_id;
    unsigned long long product_id;
} scan_data_msg_t;

/* Enum abstracting CANIDs, priorities are assigned top (highest) to bottom (lowest)*/
typedef enum {
    CLUSTER_HEAD_ELECTION = 0,
    CLUSTER_HEAD_VOTE,
    PRODUCT_SCAN,
    SCAN_ACK,
    PRODUCT_UPDATE,
    PRODUCT_UPDATE_ACK,
    REQUEST_PRODUCT_UPDATE,
} CAN_command;

/* Union abstracting possible contents of data field of CANFD message */
typedef union CAN_data {
    product_info_msg_t product_info;
    bool empty; // Used to indicate empty message (ACK)
} CAN_data_t;

/* Model of CAN FD frame */
typedef struct CANmessage {
    CAN_command command; // 29 bits in real life equivalent, enum assumed to fit in this size
    uint64_t to;         // Node that is to execute given command
    uint64_t from;         // Node that sent given command (important for simulating CAN)
    CAN_data_t data;            // Pointer to the callback function for the given command
} CANmessage_t;

#endif