#ifndef VIRTUALCANBUSWRAPPER
#define VIRTUALCANBUSWRAPPER

#include "VirtualCANBus.hpp"
extern "C" {
    #include <stdint.h>
    #include <cstring>
    // #include "contiki.h"
}

VirtualCANBus bus;

void scanCallBack(scan_data_msg_t data) {
    unsigned long customer_id = data.customer_id;
    unsigned long long product_id = data.product_id;
    printf("Data: %ld, %lld\n", customer_id, product_id);
}

/* Wrapper functions */
extern "C" uint8_t add_sender_endpoint(uint64_t id) {
    return bus.addNode(id, &scanCallBack) ? 1 : 0;
}

extern "C" uint8_t add_receiver_endpoint(uint64_t id) {
    return bus.addNode(id, &scanCallBack) ? 1 : 0;
}

extern "C" uint8_t send_data(const char* data) {
    return bus.sendData(data) ? 1 : 0;
}

extern "C" uint8_t receive_data(char* data) {
    std::string strData;
    if (bus.receiveData(strData)) {
        strcpy(data, strData.c_str());
        return 1;
    }
    return 0;
}
#endif