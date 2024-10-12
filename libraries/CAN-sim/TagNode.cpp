#include "TagNode.hpp"

void TagNode::SetNodeProduct(unsigned long product_id)
{
    product_.id = product_id;
}

void TagNode::UpdateNodeProduct(product_info_msg_t product_msg)
{
    product_.price = product_msg.price;
    product_.name = "";
    memcpy(&(product_.name), product_msg.product_name, product_msg.product_name_len);
}

scan_data_msg_t TagNode::GenerateScan()
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> random_customer_id(MIN_CUST_ID, MAX_CUST_ID);
    return {
        random_customer_id(rng), // Random customer
        product_.id
    };
}

/* Return time in s at which next tag scan will take place*/
int TagNode::GetNextSendTime(CANFDmessage_t* msg)
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(1, 10);

    msg->command = PRODUCT_SCAN;
    msg->to = id_; // TODO: implement cluster head instead
    msg->from = id_;
    msg->cb = scan_cb_;

    return dist(rng);
}

void TagNode::ProcessCommand(CANFDmessage_t msg)
{
    switch (msg.command){
        case PRODUCT_SCAN:
            (*scan_cb_)(GenerateScan());
            break;
        case PRICE_UPDATE:
            printf("Not implemented yet\n");
            break;
        default:
            printf("Unknown command\n");
    }
    return;
}