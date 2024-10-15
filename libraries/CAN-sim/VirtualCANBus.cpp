#include "VirtualCANBus.hpp"
#include <cstring>

float VirtualCANBus::simulateCANBus()
{
    float time_to_sleep = -1;
    #ifdef DEBUG_BUS
    log("Current queue status:", 0);
    for (auto msg : bus_queue_) {
        log("{ from %d to %d, command: %d } at t=%f", msg.msg.from, msg.msg.to, msg.msg.command, msg.time_until);
    }
    #endif
    // Loop until next frame is not directly up, or queue empty (should never happen)
    while (time_to_sleep < 0)
    {
        if (bus_queue_.empty())
        {
            return 1;
        }

        // Fetch next CAN message to be sent
        scheduled_bus_activity_t next = bus_queue_.front();
        #ifdef DEBUG_BUS
        log("Now processing{ from %d to %d, command: %d } at t=%f", next.msg.from, next.msg.to, next.msg.command, next.time_until);
        #endif
        // If not scheduled for next time unit, then thread can go to sleep until that time
        if (next.time_until > 0)
        {
            // Progress time that has passed for each frame by the amount we are about to
            for (scheduled_bus_activity_t& msg : bus_queue_)
            {
                msg.time_until -= next.time_until;
            }
            time_to_sleep = next.time_until;
            
        }
        else
        {
            // Double check if target node exists, if not, skip
            if (nodes_.find(next.msg.to) != nodes_.end()) {
                 // Execute CAN command presently sent on the bus
                nodes_[next.msg.to]->ProcessCommand(next.msg);
                bus_queue_.pop_front();

                // If current CAN message is a product scan, that means sending node is eligible to generate the next one
                if (next.msg.command == PRODUCT_SCAN) {
                    CANFDmessage_t next_msg;
                    int t_next = nodes_[next.msg.from]->GetNextSendTime(&next_msg);
                    enqueueCANMessage(t_next, next_msg);
                }
                if (next.msg.command == PRODUCT_UPDATE) {
                    product_info_msg_t rec_product_info = next.msg.data.product_info;
                    for (std::pair<uint64_t, TagNode *> node : nodes_ ) {
                        TagNode* current_node = node.second;
                        // If node is currently displaying this product, update info
                        if(current_node->GetProductId() == rec_product_info.product_id) {
                            current_node->UpdateNodeProduct(rec_product_info);
                        }
                    }
                }
            
                // Progress time that has passed for each frame by 1 step
                for (scheduled_bus_activity_t& msg : bus_queue_)
                {
                    msg.time_until -= CAN_UNIT_STEP;
                }
                time_to_sleep = CAN_UNIT_STEP;
            };
        }
    }
    return time_to_sleep;
}

/* Loops over current pending CAN messages queue, inserts new message where appropriate*/
void VirtualCANBus::enqueueCANMessage(float time_until, CANFDmessage_t msg)
{
    scheduled_bus_activity_t scheduled_message = {
        time_until,
        msg
    };
    // log("Looking for a spot for message: { from %d to %d, command: %d } at t=%d", msg.from, msg.to, msg.command, time_until);
    
    if (bus_queue_.empty()) {
        // log("Straightforwardly inserted", 0);
        bus_queue_.push_front(scheduled_message);
    }
    int i = 0;
    for (auto it = bus_queue_.begin(); it != bus_queue_.end(); ++it)
    {
        scheduled_bus_activity_t current = *it;
        // log("Comparing own time %d against present message's %d", scheduled_message.time_until, current.time_until);
        // In case of a message that will be sent sooner, move further back to the queue

        // Negative time = "now". Message has just been delayed more, but this means nothing for CAN's prioritization.
        float time_of_contending_message = std::max(current.time_until, (float)0.0); 

        if (scheduled_message.time_until > time_of_contending_message) {
            // log("He'll be earlier", 0);
            continue;
        }
        // In case of simultaneous messages (bus collision), order is determined by command prio, if higher prio (lower ID), then move back
        if (scheduled_message.time_until == time_of_contending_message) {
            // log("We're equal", 0);
            if (scheduled_message.msg.command >= current.msg.command) {
                // log("He has higher prio", 0);
                continue;
            }
        }

        // log("Inserted message at location %d", i);
        // Time_until of message next up is greater, or we have higher prio at equal time. Insert after current element.
        bus_queue_.insert(it, scheduled_message);
        
        return;
        i++;
    }
    //If you preceed one in the queue, join at the back
    bus_queue_.push_back(scheduled_message);

}

/* Adds a node to the bus */
bool VirtualCANBus::addNode(const uint64_t id, void (*scan_cb)(scan_data_msg_t, uint64_t), void (*product_update_cb)(unsigned long, uint64_t, product_info_t*))
{
    if (nodes_.find(id) == nodes_.end())
    {
        // Create and insert new node
        TagNode* new_node = new TagNode(id, scan_cb, product_update_cb);
        nodes_[id] = new_node;

        // Set it to start generating scans
        CANFDmessage msg;
        int t_next = new_node->GetNextSendTime(&msg);
        enqueueCANMessage(t_next, msg);        
        return true;
    }
    return false;
}

/* Removes a node from the bus */
bool VirtualCANBus::removeNode(const uint64_t id)
{
    if (nodes_.find(id) != nodes_.end())
    {
        delete nodes_[id];
        nodes_.erase(id);
        return true;
    }
    return false;
}

void setProductId(uint64_t node_id, unsigned long product_id) {
    if (nodes_.find(node_id) != nodes_.end())
    {
        nodes_[node_id].SetNodeProduct(node_id);
    }
}