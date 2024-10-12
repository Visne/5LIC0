#ifndef VIRTUALCANBUS_HPP
#define VIRTUALCANBUS_HPP

#include <map>
#include <list>
#include <string>
#include "TagNode.hpp"

class VirtualCANBus {
private:
    
    typedef struct scheduled_bus_activity {
        int time_until; // Time until this message should be sent
        CANFDmessage_t msg;
    } scheduled_bus_activity_t;

    std::map<uint64_t, TagNode*> nodes_;

    std::list<scheduled_bus_activity_t> bus_queue_;

public:
    bool addNode(const uint64_t id, void (*scan_cb)(scan_data_msg_t));
    bool removeNode(const uint64_t id);

    bool sendData(const std::string& data);
    bool receiveData(std::string& data);

    /* Resolves scheduled actions simulating the behavior of a CAN bus. Returns time in s until next this method should be called again */
    int simulateCANBus();
    /* Schedules a CAN message to be sent in time_until s*/
    void enqueueCANMessage(int time_until, CANFDmessage msg);
};

#endif // VIRTUALCANBUS_HPP