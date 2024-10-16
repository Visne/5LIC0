#ifndef TYPES
#define TYPES

#include <stdint.h>
#include <string>
#include "./shared/can-commands.h"

// #define DEBUG_NODE //Uncomment to enable debug mode for nodes
// #define DEBUG_BUS  //Uncomment to enable debug mode for bus
#define MIN_CUST_ID 0
#define MAX_CUST_ID 100
#define MIN_TIME 8000
#define MAX_TIME 24000
#define CLUSTER_VIS_DIRECTORY "/home/quinten/contiki-ng/5LIC0/vis/"

// 29 bit ID + 64 byte payload = 541 bits per message. 1Mbit/s -> 1.000.000 / 541 =~ 1848 msgs per second
#define CAN_FREQ 1848
#define CAN_UNIT_STEP 0.0078125 // representation of time in seconds to send 1 CAN message, currently set to 1/CLOCK_SECOND

#define UNDEFINED_PRODUCT_ID 0 // Product ID 0 is forbidden



#endif