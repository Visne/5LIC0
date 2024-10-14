#ifndef VIRTUALCANBUSWRAPPER
#define VIRTUALCANBUSWRAPPER

#include "VirtualCANBus.hpp"
extern "C" {
    #include <stdint.h>
    #include <cstring>
    #include "contiki.h"
}

static VirtualCANBus bus;

// void scanCallBack(scan_data_msg_t data) {
//     unsigned long customer_id = data.customer_id;
//     unsigned long long product_id = data.product_id;
//     printf("Data: %ld, %lld\n", customer_id, product_id);
// }

/* Wrapper functions */
extern "C" uint8_t add_node(uint64_t id, void (*scanCallBack)(scan_data_msg_t, uint64_t)) {
    return bus.addNode(id, scanCallBack) ? 1 : 0;
}

extern "C" uint8_t remove_node(uint64_t id) {
    return bus.removeNode(id) ? 1 : 0;
}

extern "C" float simulate_can_bus() {
    float result = bus.simulateCANBus();
    return result;
}

extern "C" void send_can_message(CAN_command command, uint64_t target_node, char* payload) {
    switch (command) {
        case SCAN_ACK: {
            CANFDmessage_t msg = {
                SCAN_ACK,
                target_node,
                bus.cluster_head_id,
                nullptr
            };
            bus.enqueueCANMessage(0.0, msg);
            // bus.simulateCANBus();
            break;
        }
        case PRODUCT_UPDATE: {
            printf("Not implemented yet\n");
        }
        default:
            break;
    }
    return;
}

#endif