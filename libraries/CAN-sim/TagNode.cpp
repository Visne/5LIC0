#include "TagNode.hpp"

void TagNode::SetNodeProduct(ean13_t product_id)
{
    log("Updating product ID from %lld to %lld", product_.id, product_id);
    product_.id = product_id;
}

void TagNode::UpdateNodeProduct(product_t product_msg)
{
    log("Updating price from %d to %d", product_.price, product_msg.price);
    product_.price = product_msg.price;
    std::string name(product_msg.description);
    log("Updating name from %s to %s", product_.name, name);
    product_.name = name;
}

scan_data_msg_t TagNode::generateScan()
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> random_customer_id(MIN_CUST_ID, MAX_CUST_ID);
    customer_t customer_id = random_customer_id(rng);
    log("Generated scan: { cust: %ld, prod: %lld }", customer_id, product_.id);
    return {
        customer_id, // Random customer
        product_.id
    };
}

/* Return time in s at which next tag scan will take place*/
float TagNode::getNextSendTime()
{   
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(MIN_SCAN_PERIOD, MAX_SCAN_PERIOD);

    float t_next = dist(rng) / 1000.0; // Used as ms
    // Not randomized for now, just send 2s to get 8 messages/s on average
    
    #ifdef DEBUG_NODE
    log("Next message will be sent in %f seconds.", t_next);
    #endif
    return t_next;
}

// Lets the node send a scan submission wirelessly
void TagNode::sendProductScan() {
    if (product_.id == UNDEFINED_PRODUCT_ID) return; // Don't register scans if no product is assigned yet
    #ifdef DEBUG_NODE
    log("Calling scan callback", 0);
    #endif
    // Call the callback function
    (*scan_cb_)(generateScan(), id_);
    awaiting_ACK = true;
}

// Handles receiving the ACK for a product scan on the node's end
void TagNode::receiveScanAck() {
    log("Scan ACK received!", 0);
    awaiting_ACK = false;
}

bool TagNode::receiveProductUpdate(product_t data) {
    if (product_.id == data.id) {
        log("Received product update for prod %d", data.id);
        UpdateNodeProduct(data);
        return true;
    }
    return false;
}

void TagNode::sendProductUpdateReq() {
    // Call the callback function
    (*product_update_cb_)(product_.id, id_);
}

void TagNode::sendProductUpdateAck() {
    // log("Sending product update ACK", 0);
}

bool TagNode::wantsToApplyForClusterHead() {
    return true;
}

void TagNode::sendClusterHeadVote() {
    // log("Node applied for cluster head", 0);
}