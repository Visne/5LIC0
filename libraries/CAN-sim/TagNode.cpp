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
    log("Generated scan: { cust: %ld, prod: %lld }", customer_id, product_.id);
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
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, 2000);

    msg->command = PRODUCT_SCAN;
    msg->to = id_; // TODO: implement cluster head instead
    msg->from = id_;
    msg->cb = scan_cb_;

    float t_next = dist(rng) / 1000.0; // Used as ms
    // Not randomized for now, just send 2s to get 8 messages/s on average
    
    log("Next message will be sent in %f seconds.", t_next);
    return t_next;
}

void TagNode::ProcessCommand(CANFDmessage_t msg)
{
    switch (msg.command){
        case PRODUCT_SCAN:
            log("Calling callback", 0);
            (*scan_cb_)(GenerateScan());
            log("Survived callback", 0);
            break;
        case PRICE_UPDATE:
            printf("Not implemented yet\n");
            break;
        default:
            printf("Unknown command\n");
    }
    return;
}