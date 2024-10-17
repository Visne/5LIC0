#ifndef VIRTUALCANBUS_HPP
#define VIRTUALCANBUS_HPP

#include <map>
#include <list>
#include <string>
#include <fstream>
#include "TagNode.hpp"

class VirtualCANBus
{
private:
    typedef struct scheduled_bus_activity
    {
        float time_until; // Time until this message should be sent in seconds
        CANFDmessage_t msg;
    } scheduled_bus_activity_t;

    std::map<uint64_t, TagNode *> nodes_;

    std::list<scheduled_bus_activity_t> bus_queue_;

    std::ofstream myfile_;  // Create an output file stream object
    std::string vis_file_;
    bool visualizing_ = false;

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
    VirtualCANBus() {
    }

    ~VirtualCANBus() {
        if (myfile_.is_open()) {
            myfile_.close();
        }
        
    }

    uint64_t cluster_head_id;

    // CAN BUS FUNCTIONS
    /* Add tag node to the bus */
    bool addNode(const uint64_t id, void (*scan_cb)(scan_data_msg_t, uint64_t), void (*price_update_cb)(unsigned long, uint64_t));
    /* Remove tag node from the bus */
    bool removeNode(const uint64_t id);
    /* Resolves scheduled actions simulating the behavior of a CAN bus. Returns time in s until next this method should be called again */
    float simulateCANBus();
    /* Schedules a CAN message to be sent in time_until s*/
    void enqueueCANMessage(float time_until, CANFDmessage_t msg);
    /* Method used to invoke appropriate actions at corresponding tag nodes when given message is up next on the bus */
    void ProcessMessage(CANFDmessage_t msg);
    
    // VISUALIZATION FUNCTIONS
    void openVisualizationFile(int shelf_id);
    void updateVisualization(int clock);


    // HELPER FUNCTIONS
    /* Sets a tag node's new product id (as if employee is setting it) and fires corresponding request for product info on the network */
    void setProductId(uint64_t node_id, unsigned long product_id);

    CANFDmessage_t NewProductScanMsg(uint64_t node_id)
    {
        CAN_data_t payload;
        payload.empty = true;
        CANFDmessage_t scan_msg = {
            PRODUCT_SCAN,
            cluster_head_id,
            node_id,
            payload};
        return scan_msg;
    };
    CANFDmessage_t NewProductUpdateACK(uint64_t node_id)
    {
        CAN_data_t payload;
        payload.empty = true;
        CANFDmessage_t scan_msg = {
            PRODUCT_UPDATE_ACK,
            node_id,
            cluster_head_id,
            payload};
        return scan_msg;
    };
    CANFDmessage_t NewProductUpdateRequestMsg(uint64_t node_id)
    {
        CAN_data_t payload;
        payload.empty = true;
        CANFDmessage_t request_msg = {
            REQUEST_PRODUCT_UPDATE,
            node_id, // Must be to calling node, not cluster head, as callback is made with info from calling node
            node_id,
            payload};
        return request_msg;
    };
    CANFDmessage_t NewClusterHeadElection()
    {
        CAN_data_t payload;
        payload.empty = true;
        CANFDmessage_t request_msg = {
            CLUSTER_HEAD_ELECTION,
            cluster_head_id, 
            cluster_head_id,
            payload
        };
        return request_msg;
    };
    CANFDmessage_t NewClusterHeadVote(uint64_t node_id)
        {
        CAN_data_t payload;
        payload.empty = true;
        CANFDmessage_t request_msg = {
            CLUSTER_HEAD_VOTE,
            cluster_head_id,
            node_id,
            payload
        };
        return request_msg;
    };

    // Function for padding strings
    std::string pad_to_length(const std::string& input, size_t n) {
        if (input.length() >= n) {
            return input.substr(0, n);  // Truncate if the input is longer than n
        }
        return input + std::string(n - input.length(), ' ');  // Pad with spaces
    };

    // Function for padding integers (converts int to string)
    std::string pad_to_length(unsigned long input, size_t n) {
        std::string str_input = std::to_string(input);  // Convert int to string
        return pad_to_length(str_input, n);  // Reuse string padding function
    };
};

#endif // VIRTUALCANBUS_HPP