// VirtualCANBus.cpp
#include "VirtualCANBus.hpp"
#include <cstring>

bool VirtualCANBus::addSenderEndpoint(const std::string& id) {
    if (senders.find(id) == senders.end()) {
        senders[id] = new SenderEndpoint();
        return true;
    }
    return false;
}

bool VirtualCANBus::addReceiverEndpoint(const std::string& id) {
    if (receivers.find(id) == receivers.end()) {
        receivers[id] = new ReceiverEndpoint();
        return true;
    }
    return false;
}

bool VirtualCANBus::removeSenderEndpoint(const std::string& id) {
    if (senders.find(id) != senders.end()) {
        delete senders[id];
        senders.erase(id);
        return true;
    }
    return false;
}

bool VirtualCANBus::removeReceiverEndpoint(const std::string& id) {
    if (receivers.find(id) != receivers.end()) {
        delete receivers[id];
        receivers.erase(id);
        return true;
    }
    return false;
}

bool VirtualCANBus::sendData(const std::string& senderId, const std::string& data) {
    // Logic to send data
    printf("%s\n", data.c_str());
    return senders.find(senderId) != senders.end();
}

bool VirtualCANBus::receiveData(const std::string& receiverId, std::string& data) {
    // Logic to receive data
    std::string temp = "Test\n";
    data = temp;
    return receivers.find(receiverId) != receivers.end();
}