#ifndef TAGNODE_HPP
#define TAGNODE_HPP

#include <map>
#include <string.h>
#include "types.h"
#include <random>

class TagNode {
    private:

    uint64_t id_;           // MAC address
    product_info_t product_; // Product currently being displayed
        
    // Function callback pointers for commands
    void (*scan_cb_) (scan_data_msg_t);

    public:
    TagNode(uint64_t id, void (*scanCb)(scan_data_msg_t)) {
        id_ = id;
        product_ = {
            0, 0, "UNDEFINED"
        };
        scan_cb_ = scanCb;
    }

    ~TagNode() {

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
    int GetNextSendTime(CANFDmessage_t* msg);

    void ProcessCommand(CANFDmessage_t msg);
};

#endif // TAGNODE_HPP