#ifndef VIRTUALCANBUS_HPP
#define VIRTUALCANBUS_HPP

#include <map>
#include <string>

class SenderEndpoint;
class ReceiverEndpoint;

class VirtualCANBus {
private:
    std::map<std::string, SenderEndpoint*> senders;
    std::map<std::string, ReceiverEndpoint*> receivers;

public:
    bool addSenderEndpoint(const std::string& id);
    bool addReceiverEndpoint(const std::string& id);
    bool removeSenderEndpoint(const std::string& id);
    bool removeReceiverEndpoint(const std::string& id);

    bool sendData(const std::string& senderId, const std::string& data);
    bool receiveData(const std::string& receiverId, std::string& data);
};

class SenderEndpoint {
    // Logic for sender
};

class ReceiverEndpoint {
    // Logic for receiver
};

#endif // VIRTUALCANBUS_HPP