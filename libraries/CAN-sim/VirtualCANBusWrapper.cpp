#ifndef VIRTUALCANBUSWRAPPER
#define VIRTUALCANBUSWRAPPER

#include "VirtualCANBus.hpp"
extern "C" {
    #include <stdint.h>
    #include <cstring>
    #include "contiki.h"
}

static VirtualCANBus bus;

/* Wrapper functions */
extern "C" uint8_t add_node(uint64_t id, void (*scanCallBack)(scan_data_msg_t, uint64_t), void (*priceUpdateCallBack)(unsigned long, uint64_t, product_info_t*)) {
    return bus.addNode(id, scanCallBack, priceUpdateCallBack) ? 1 : 0;
}

extern "C" uint8_t remove_node(uint64_t id) {
    return bus.removeNode(id) ? 1 : 0;
}

extern "C" float simulate_can_bus() {
    float result = bus.simulateCANBus();
    return result;
}

extern "C" void send_can_message(CAN_command_t command, uint64_t target_node, CANFD_data_t payload) {
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
            CANFDmessage_t msg = {
                SCAN_ACK,
                target_node,
                bus.cluster_head_id,
                payload
            };
            bus.enqueueCANMessage(0.0, msg);
            break;
        }
        default:
            break;
    }
    return;
}

#endif