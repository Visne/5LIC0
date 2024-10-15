#ifndef TAGNODE_HPP
#define TAGNODE_HPP

#include <map>
#include <string.h>
#include "types.h"
#include <random>
#include <memory>
#include <stdexcept>
#include <math.h>

class TagNode
{
private:
    uint64_t id_;            // MAC address
    product_info_t product_; // Product currently being displayed
    uint64_t cluser_head_id_;// Logical CAN address of cluster head (send commands to this node)
    bool awaiting_ACK = false;

    void (*enqueue_message_)(float time_until, CANFDmessage_t msg);
    // Function callback pointers for commands
    void (*scan_cb_)(scan_data_msg_t, uint64_t);
    void (*product_update_cb_)(unsigned long, uint64_t, product_info_t*);

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
        printf("[NODE %ld]: %s\n", id_, std::string(buf.get(), buf.get() + size - 1).c_str());
    }

public:
    TagNode(
        uint64_t id, 
        void (*scanCb)(scan_data_msg_t, uint64_t), 
        void (*productUpdateCb)(unsigned long, uint64_t, product_info_t*),
        void (*enqueueMessage)(float, CANFDmessage_t)
    )
    {
        id_ = id;
        product_ = {
            0, 0, "UNDEFINED"};
        scan_cb_ = scanCb;
        product_update_cb_ = productUpdateCb;
        enqueue_message_ = enqueueMessage;
    }

    ~TagNode()
    {
    }

    /* Assign product ID to node, used to fetch and update product information */
    void SetNodeProduct(unsigned long product_id);

    /* Send a message on the bus to have local product information updated */
    void RequestProductInfo(unsigned long product_id);

    /* Replace local product information with given values */
    void UpdateNodeProduct(product_info_msg_t product_info);

    /* Generate a suitable (partly random) scan message to be submitted on the net */
    scan_data_msg_t GenerateScan();

    /* Return time in s at which next tag scan will take place, populates fields of msg to have correct info on message*/
    float GetNextSendTime(CANFDmessage_t *msg);

    void ProcessCommand(CANFDmessage_t msg);

    unsigned long GetProductId() { return product_.id; };
};

#endif // TAGNODE_HPP