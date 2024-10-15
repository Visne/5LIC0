#include "TagNode.hpp"

void TagNode::SetNodeProduct(unsigned long product_id)
{
    log("Updating product ID from %lld to %lld", product_.id, product_id);
    product_.id = product_id;
}

void TagNode::UpdateNodeProduct(product_info_msg_t product_msg)
{
    log("Updating price from %d to %d", product_.price, product_msg.price);
    product_.price = product_msg.price;
    char name[256];
    memcpy(&name, product_msg.product_name, product_msg.product_name_len);
    log("Updating name from %s to %s", product_.name, name);
    product_.name = name;
}

scan_data_msg_t TagNode::GenerateScan()
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> random_customer_id(MIN_CUST_ID, MAX_CUST_ID);
    unsigned long customer_id = random_customer_id(rng);
    #ifdef DEBUG_NODE
    log("Generated scan: { cust: %ld, prod: %lld }", customer_id, product_.id);
    #endif
    return {
        customer_id, // Random customer
        product_.id
    };
}

/* Return time in s at which next tag scan will take place*/
float TagNode::GetNextSendTime(CANFDmessage_t* msg)
{   
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(1000, 3000);

    msg->command = PRODUCT_SCAN;
    msg->to = id_; // TODO: implement cluster head instead
    msg->from = id_;
    msg->data.cb = scan_cb_;

    float t_next = dist(rng) / 1000.0; // Used as ms
    // Not randomized for now, just send 2s to get 8 messages/s on average
    
    #ifdef DEBUG_NODE
    log("Next message will be sent in %f seconds.", t_next);
    #endif
    return t_next;
}

// Messages on the bus are not explicitly listened to by TagNode objects.
// Rather, a callback to a particular TagNode is made via this method when the corresponding CAN message is up next on the bus.
void TagNode::ProcessCommand(CANFDmessage_t msg)
{
    switch (msg.command){
        case PRODUCT_SCAN: {
            #ifdef DEBUG_NODE
            log("Calling callback", 0);
            #endif
            CANFDmessage_t next_msg;
            int t_next = GetNextSendTime(&next_msg);
            enqueue_message_(t_next, next_msg);
            (*scan_cb_)(GenerateScan(), id_);
            awaiting_ACK = true;
            #ifdef DEBUG_NODE
            log("Survived callback", 0);
            #endif
            break;
        }
        case SCAN_ACK: {
            #ifdef DEBUG_NODE
            log("ACK received!", 0);
            awaiting_ACK = false;
            #endif
            break;
        }
        case PRODUCT_UPDATE:{
            product_info_msg_t data = msg.data.product_info;
            char name[16];
            memcpy(name, data.product_name, data.product_name_len);
            log("New product info: { %ld, %d, %s }", data.product_id, data.price, name);
            break;
        }
        case REQUEST_PRODCUCT_UPDATE: {
            if (product_.id == msg.data.product_info.product_id) {
                UpdateNodeProduct(msg.data.product_info);
            }   
        }
        default:
            printf("Unknown command\n");
    }
    return;
}