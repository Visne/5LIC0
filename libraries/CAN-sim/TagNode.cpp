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
float TagNode::GetNextSendTime()
{   
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(1000, 3000);

    float t_next = dist(rng) / 1000.0; // Used as ms
    // Not randomized for now, just send 2s to get 8 messages/s on average
    
    #ifdef DEBUG_NODE
    log("Next message will be sent in %f seconds.", t_next);
    #endif
    return t_next;
}

// Lets the node send a scan submission wirelessly
void TagNode::sendProductScan() {
    #ifdef DEBUG_NODE
    log("Calling scan callback", 0);
    #endif
    if (product_.id == UNDEFINED_PRODUCT_ID) return; // Don't register scans if no product is assigned yet
    // Call the callback function
    (*scan_cb_)(GenerateScan(), id_);
    awaiting_ACK = true;
}

// Handles receiving the ACK for a product scan on the node's end
void TagNode::receiveScanAck() {
    #ifdef DEBUG_NODE
    log("ACK received!", 0);
    awaiting_ACK = false;
    #endif
}

bool TagNode::receiveProductUpdate(product_info_msg_t data) {
    if (product_.id == data.product_id) {
        char name[16];
        memcpy(name, data.product_name, data.product_name_len);
        log("New product info: { %ld, %d, %s }", data.product_id, data.price, name);
        return true;
    }
    return false;
}

void TagNode::sendProductUpdateReq() {
    #ifdef DEBUG_NODE
    log("Calling product update callback", 0);
    #endif
    // Call the callback function
    (*product_update_cb_)(product_.id, id_);
}

void TagNode::sendProductUpdateAck() {
    log("Sending product update ACK", 0);
}