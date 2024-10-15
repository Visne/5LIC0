#ifndef VIRTUALCANBUS_HPP
#define VIRTUALCANBUS_HPP

#include <map>
#include <list>
#include <string>
#include "TagNode.hpp"

class VirtualCANBus {
private:
    
    typedef struct scheduled_bus_activity {
        float time_until; // Time until this message should be sent in seconds
        CANFDmessage_t msg;
    } scheduled_bus_activity_t;

    std::map<uint64_t, TagNode*> nodes_;

    std::list<scheduled_bus_activity_t> bus_queue_;

    // Basic logging functionality as std::format did not work (C++11)
    // Taken from stackoverflow: https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
    template <typename... Args>
    void log(const std::string &format, Args... args)
    {
        int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) + 1; // Extra space for '\0'
        if (size_s <= 0)
        {
            return;
        }
        auto size = static_cast<size_t>(size_s);
        std::unique_ptr<char[]> buf(new char[size]);
        std::snprintf(buf.get(), size, format.c_str(), args...);
        printf("[CANBUS]: %s\n", std::string(buf.get(), buf.get() + size - 1).c_str());
    }

public:
    uint64_t cluster_head_id;

    bool addNode(const uint64_t id, void (*scan_cb)(scan_data_msg_t, uint64_t), void (*price_update_cb)(unsigned long, uint64_t));
    bool removeNode(const uint64_t id);

    /* Resolves scheduled actions simulating the behavior of a CAN bus. Returns time in s until next this method should be called again */
    float simulateCANBus();
    /* Schedules a CAN message to be sent in time_until s*/
    void enqueueCANMessage(float time_until, CANFDmessage_t msg);

    void setProductId(uint64_t node_id, unsigned long product_id);

    void ProcessMessage(CANFDmessage_t msg);

    void PrintQueue(){
        for (auto msg : bus_queue_) {
            log("{ from %d to %d, command: %d } at t=%f", msg.msg.from, msg.msg.to, msg.msg.command, msg.time_until);
        }
    };

    CANFDmessage_t NewProductScanMsg(uint64_t node_id);
    CANFDmessage_t NewProductUpdateACK(uint64_t node_id);
    CANFDmessage_t NewProductUpdateRequestMsg(uint64_t node_id);
};

#endif // VIRTUALCANBUS_HPP