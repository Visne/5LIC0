#pragma once
#include "../../../motes/shared/coap/coap-datatypes.h"

/* Enum abstracting CAN commands, priorities are assigned top (highest) to bottom (lowest)*/
typedef enum CAN_command {
    CLUSTER_HEAD_ELECTION = 0,
    CLUSTER_HEAD_VOTE,
    PRODUCT_SCAN,
    SCAN_ACK,
    PRODUCT_UPDATE,
    PRODUCT_UPDATE_ACK,
    REQUEST_PRODUCT_UPDATE,
} CAN_command;

/* Structs used as product description middleware between cooja and CAN interface*/
typedef struct product_info_msg {
    /// Price of the product in cents
    unsigned long product_id;
    unsigned short price;
    char product_name[PRODUCT_DESCRIPT_LEN];
    unsigned short product_name_len;
} product_info_msg_t;

typedef struct scan_data_msg {
    unsigned long customer_id;
    unsigned long long product_id;
} scan_data_msg_t;

/* What the data field of a modeled CAN message may contain */
typedef union CAN_data {
    product_info_msg_t product_info;
    bool empty; // Used to indicate empty message (ACK)
} CAN_data_t;

/* Model of CAN FD frame */
typedef struct CANFDmessage {
    CAN_command command; // 29 bits in real life equivalent, enum assumed to fit in this size
    uint64_t to;         // Node that is to execute given command
    uint64_t from;         // Node that sent given command (important for simulating CAN)
    CAN_data_t data;            // Pointer to the callback function for the given command
} CANFDmessage_t;

