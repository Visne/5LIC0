#ifndef TYPES
#define TYPES

#include <stdint.h>
#include <string>

#define DEBUG_NODE //Uncomment to enable debug mode for nodes
#define DEBUG_BUS  //Uncomment to enable debug mode for bus
#define MIN_CUST_ID 0
#define MAX_CUST_ID 100

// 29 bit ID + 64 byte payload = 541 bits per message. 1Mbit/s -> 1.000.000 / 541 =~ 1848 msgs per second
#define CAN_FREQ 1848
#define CAN_UNIT_STEP 0.0078125 // representation of time in seconds to send 1 CAN message, currently set to 1/CLOCK_SECOND

typedef struct product_info_msg {
    /// Price of the product in cents
    unsigned short price;
    char* product_name;
    unsigned short product_name_len;
} product_info_msg_t;

typedef struct scan_data_msg {
    unsigned long customer_id;
    unsigned long long product_id;
} scan_data_msg_t;

/* Enum abstracting CANIDs, priorities are assigned top (highest) to bottom (lowest)*/
enum CAN_command {
    PRODUCT_SCAN = 0,
    SCAN_ACK,
    PRODUCT_UPDATE,
};

/* Model of CAN FD frame */
typedef struct CANFDmessage {
    CAN_command command; // 29 bits in real life equivalent, enum assumed to fit in this size
    uint64_t to;         // Node that is to execute given command
    uint64_t from;         // Node that sent given command (important for simulating CAN)
    void (*cb) (scan_data_msg_t, uint64_t);            // Pointer to the callback function for the given command
} CANFDmessage_t;

typedef struct product_info {
    unsigned long id;
    unsigned short price;
    std::string name;
} product_info_t;

#endif