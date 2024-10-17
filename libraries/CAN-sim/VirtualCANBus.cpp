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
        log("Looking at { from %d to %d, command: %d } at t=%f", next.msg.from, next.msg.to, next.msg.command, next.time_until);
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
            bus_queue_.pop_front();
            // Double check if target node exists, if not, skip
            if (nodes_.find(next.msg.to) != nodes_.end()) {
                
                 // Execute CAN command presently sent on the bus
                ProcessMessage(next.msg);
                
            
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
void VirtualCANBus::enqueueCANMessage(float time_until, CANmessage_t msg)
{
    #ifdef DEBUG_BUS
    log("Enqueueing message { from %d to %d, command: %d } at t=%f", msg.from, msg.to, msg.command, time_until);
    #endif
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
        // log("Comparing own time %f against present message's %f", scheduled_message.time_until, current.time_until);
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
bool VirtualCANBus::addNode(const uint64_t id, void (*scan_cb)(scan_data_msg_t, uint64_t), void (*product_update_cb)(unsigned long, uint64_t))
{
    if (nodes_.find(id) == nodes_.end())
    {
        if (cluster_head_id == 0) { cluster_head_id = id; };
        // Create and insert new node
        TagNode* new_node = new TagNode(id, scan_cb, product_update_cb);
        nodes_[id] = new_node;

        // Set it to start generating scans
        int t_next = new_node->getNextSendTime();
        enqueueCANMessage(t_next, NewProductScanMsg(id));        
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

void VirtualCANBus::setProductId(uint64_t node_id, unsigned long product_id)
{
    if (nodes_.find(node_id) != nodes_.end())
    {
        nodes_[node_id]->SetNodeProduct(product_id);
        // Generate new product update request
        enqueueCANMessage(0.0, NewProductUpdateRequestMsg(node_id));
    }
}

// Messages on the bus meant to invoke actions are not explicitly listened to by TagNode objects or the cluster head (cooja mote).
// Rather, callbacks to particular cooja mote functions are provided to TagNodes on creation, and invoked when the corresponding CAN message is up next on the bus.
void VirtualCANBus::ProcessMessage(CANmessage_t msg)
{
    switch (msg.command){
        // Represents a message initiating election of a new cluster head
        case CLUSTER_HEAD_ELECTION: {
            // If an election message is posted, all eligible tag nodes will send an application on the bus
            for (std::pair<uint64_t, TagNode *> node_: nodes_) {
                TagNode *node = node_.second;
                bool applying = node->wantsToApplyForClusterHead();
                if (applying) {
                    enqueueCANMessage(0.0, NewClusterHeadVote(node->GetNodeId()));
                }
            }
            // Queue new election.
            enqueueCANMessage(2, NewClusterHeadElection());
        }
        break;
        // Represents a message to attempt to claim the task of cluster head
        case CLUSTER_HEAD_VOTE: {
            if (nodes_.find(msg.from) != nodes_.end())
            {
                nodes_[msg.from]->sendClusterHeadVote();
            }
        }
        break;
        // Represents a "Product Scan event" being received by the cluster head
        case PRODUCT_SCAN: {
            if (nodes_.find(msg.from) != nodes_.end())
            {
                TagNode *node = nodes_[msg.from];
                // Invoke the callback to simulate the data that would have been submitted to cluster head instead, such that it may be forwarded to the wireless network.
                node->sendProductScan();

                // Queue new Product Scan event for this tag node.
                enqueueCANMessage(node->getNextSendTime(), NewProductScanMsg(msg.from));
            }
        }
        break;
        // Represents a product scan submission acknowledgement put on the bus after having been received wirelessly by the cluster head.
        case SCAN_ACK: {
            if (nodes_.find(msg.to) != nodes_.end())
            {
                nodes_[msg.to]->receiveScanAck();
            }
        }
        break;
        // Publish product update message on the bus for all tag nodes.
        case PRODUCT_UPDATE:{
            for (std::pair<uint64_t, TagNode *> node_: nodes_) {
                TagNode *node = node_.second;
                bool ack = node->receiveProductUpdate(msg.data.product_info);
                if (ack) {
                    enqueueCANMessage(0.0, NewProductUpdateACK(msg.to));
                }
            }
        }
        break;
        // Represents message put on the bus by an arbitrary tag node requesting updated info for its product id
        case REQUEST_PRODUCT_UPDATE: {
            if (nodes_.find(msg.to) != nodes_.end())
            {
                TagNode *node = nodes_[msg.to];
                // Invoke the callback to simulate the product info request that would have been submitted to cluster head normally, such that it may be forwarded to the wireless network.
                node->sendProductUpdateReq();
            }
        }
        // Acknowledgement message coming from tag node that it succesfully updated its product info
        case PRODUCT_UPDATE_ACK: {
            if (nodes_.find(msg.to) != nodes_.end())
            {
                nodes_[msg.to]->sendProductUpdateAck();
            }
        }
        break;
        default:
            printf("Unknown command\n");
    }
    return;
}

void VirtualCANBus::openVisualizationFile(int shelf_id) {
     {
        vis_file_ = CLUSTER_VIS_DIRECTORY "shelf_" + std::to_string(shelf_id) + ".txt";
        myfile_.open(vis_file_);  // Open a file or create it if it doesn't exist

        // Check if the file opened successfully
        if (myfile_.is_open()) {
            visualizing_ = true;
            log("File written successfully!", 0);
        } else {
            log("Unable to open visualization file!", 0);
        }
    }
}

void VirtualCANBus::updateVisualization(int clock)  {
    if (!visualizing_) return;

    myfile_.open(vis_file_);
    if(myfile_.is_open()) {
        #define MSEP_EDGE   "--+--"
        #define LSEP_EDGE   "+--"
        #define RSEP_EDGE   "--+"
        #define MSEP        "  |  "
        #define LSEP        "|  "
        #define RSEP        "  |"
        int columnwidth = 16;
        std::string hor_line;
        for (int i = 0; i < columnwidth; i++) {
            hor_line += "-";
        }
        //                      NODE ID                  PROD ID                  PRICE                    NAME                     AWAITING ACK
        myfile_ << LSEP_EDGE << hor_line << MSEP_EDGE << hor_line << MSEP_EDGE << hor_line << MSEP_EDGE << hor_line << MSEP_EDGE << hor_line << RSEP_EDGE"\n";
        myfile_ << LSEP << pad_to_length("NODE ID", columnwidth) << MSEP << pad_to_length("PROD ID", columnwidth) << MSEP << pad_to_length("PRICE", columnwidth) << MSEP << pad_to_length("NAME", columnwidth) << MSEP << pad_to_length("SCAN STATUS", columnwidth) << RSEP"\n";
        myfile_ << LSEP_EDGE << hor_line << MSEP_EDGE << hor_line << MSEP_EDGE << hor_line << MSEP_EDGE << hor_line << MSEP_EDGE << hor_line << RSEP_EDGE"\n";
        for (std::pair<uint64_t, TagNode *> node_: nodes_) {
            TagNode *node = node_.second;
            myfile_ << LSEP 
            << pad_to_length(node->GetNodeId(), columnwidth) << MSEP 
            << pad_to_length(node->GetProductId(), columnwidth) << MSEP
            << pad_to_length(node->GetProductPrice(), columnwidth) << MSEP
            << pad_to_length(node->GetProductName(), columnwidth) << MSEP
            << pad_to_length(node->GetAwaitingACK()? "AWAITING ACK" : "IDLE", columnwidth) << RSEP"\n";
        }
        myfile_ << LSEP_EDGE << hor_line << MSEP_EDGE << hor_line << MSEP_EDGE << hor_line << MSEP_EDGE << hor_line << MSEP_EDGE << hor_line << RSEP_EDGE"\n";
        myfile_ << "\n\n";
        myfile_ << "Clock: " << clock;
        myfile_.close();
    } else {
        log("File not open!", 0);
    }

    
}