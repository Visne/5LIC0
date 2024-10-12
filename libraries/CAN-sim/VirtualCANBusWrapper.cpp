#include "VirtualCANBus.hpp"
extern "C" {
    #include <stdint.h>
    #include <cstring>
    #include "contiki.h"
}

VirtualCANBus bus;

/* Wrapper functions */
extern "C" uint8_t add_sender_endpoint(const char* id) {
    return bus.addSenderEndpoint(id) ? 1 : 0;
}

extern "C" uint8_t add_receiver_endpoint(const char* id) {
    return bus.addReceiverEndpoint(id) ? 1 : 0;
}

extern "C" uint8_t remove_sender_endpoint(const char* id) {
    return bus.removeSenderEndpoint(id) ? 1 : 0;
}

extern "C" uint8_t remove_receiver_endpoint(const char* id) {
    return bus.removeReceiverEndpoint(id) ? 1 : 0;
}

extern "C" uint8_t send_data(const char* senderId, const char* data) {
    return bus.sendData(senderId, data) ? 1 : 0;
}

extern "C" uint8_t receive_data(const char* receiverId, char* data) {
    std::string strData;
    if (bus.receiveData(receiverId, strData)) {
        strcpy(data, strData.c_str());
        return 1;
    }
    return 0;
}