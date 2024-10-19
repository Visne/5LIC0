#ifndef VIRTUALCANBUSWRAPPER
#define VIRTUALCANBUSWRAPPER

#include "VirtualCANBus.hpp"
#include "../../motes/shared/db.h"
extern "C" {
    #include <stdint.h>
    #include <cstring>
    #include "contiki.h"
}

static VirtualCANBus bus;

extern "C" uint8_t init_can_bus(uint64_t nr_of_nodes, void (*scanCallBack)(scan_data_msg_t, uint64_t), void (*priceUpdateCallBack)(unsigned long, uint64_t), uint64_t node_id) {
    for (int i = 0; i < (int) nr_of_nodes; i++) {
        int id = node_id * NR_OF_NODES_PER_CLUSTER + i;
        if (bus.addNode(id, scanCallBack, priceUpdateCallBack)) {
            int product_id = rand() % DB_SIZE;
            if ((id % NR_OF_NODES_PER_CLUSTER) == 7) { product_id = 42; };
            bus.setProductId(id, product_id);
        } else {
            return 0;
        }
    }
    if (node_id % 10 == 1) {
        bus.openVisualizationFile(node_id);
    }
    bus.openLoggingFile(node_id);
    
    bus.updateVisualization(0);
    bus.enqueueCANMessage(2, bus.NewClusterHeadElection());
    return 1;
}

/* Wrapper functions */
extern "C" uint8_t remove_node(uint64_t id) {
    return bus.removeNode(id) ? 1 : 0;
}

extern "C" float simulate_can_bus(int clockt_time) {
    float result = bus.simulateCANBus(clockt_time);
    return result;
}

extern "C" void send_can_message(CAN_command command, uint64_t target_node, CAN_data_t payload) {
    switch (command) {
        case SCAN_ACK: {
            CANmessage_t msg = {
                SCAN_ACK,
                target_node,
                bus.cluster_head_id,
                true
            };
            bus.enqueueCANMessage(0.0, msg);
            // bus.simulateCANBus();
            break;
        }
        case PRODUCT_UPDATE: {
            CANmessage_t msg = {
                PRODUCT_UPDATE,
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

extern "C" void update_visualization(int clock) {
    bus.updateVisualization(clock);
}
#endif