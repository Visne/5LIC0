#include "VirtualCANBus.hpp"
#include <cstring>

int VirtualCANBus::simulateCANBus()
{
    int time_to_sleep = -1;
    log("Current queue status:", 0);
    for (auto msg : bus_queue_) {
        log("{ from %d to %d, command: %d } at t=%d", msg.msg.from, msg.msg.to, msg.msg.command, msg.time_until);
    }
    // Loop until next frame is not directly up, or queue empty (should never happen)
    while (time_to_sleep < 0)
    {
        if (bus_queue_.empty())
        {
            return 1;
        }

        // Fetch next CAN message to be sent
        scheduled_bus_activity_t next = bus_queue_.front();
        
        log("Now processing{ from %d to %d, command: %d } at t=%d", next.msg.from, next.msg.to, next.msg.command, next.time_until);

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
                CANFDmessage_t next_msg;
                int t_next = nodes_[next.msg.from]->GetNextSendTime(&next_msg);
                
                enqueueCANMessage(t_next, next_msg);
                // Progress time that has passed for each frame by 1 step
                for (auto msg : bus_queue_)
                {
                    msg.time_until--;
                }
            };
        }
    }
    return time_to_sleep;
}

/* Loops over current pending CAN messages queue, inserts new message where appropriate*/
void VirtualCANBus::enqueueCANMessage(int time_until, CANFDmessage_t msg)
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
        if (scheduled_message.time_until > current.time_until) {
            // log("He'll be earlier", 0);
            continue;
        }
        // In case of simultaneous messages (bus collision), order is determined by command prio, if higher prio (lower ID), then move back
        if (scheduled_message.time_until == current.time_until) {
            // log("We're equal", 0);
            if (scheduled_message.msg.command < current.msg.command) {
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
bool VirtualCANBus::addNode(const uint64_t id, void (*scan_cb)(scan_data_msg_t))
{
    if (nodes_.find(id) == nodes_.end())
    {
        // Create and insert new node
        TagNode* new_node = new TagNode(id, scan_cb);
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

bool VirtualCANBus::sendData(const std::string &data)
{
    // Logic to send data
    printf("%s\n", data.c_str());
    return 1;
}

bool VirtualCANBus::receiveData(std::string &data)
{
    // Logic to receive data
    std::string temp = "Test\n";
    data = temp;
    return 1;
}