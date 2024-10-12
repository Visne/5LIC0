#include "VirtualCANBus.hpp"
#include <cstring>

int VirtualCANBus::simulateCANBus()
{
    // Loop until next frame is not directly up, or queue empty (should never happen)
    while (true)
    {
        if (bus_queue_.empty())
        {
            return 1;
        }
        // Fetch next CAN message to be sent
        scheduled_bus_activity_t next = bus_queue_.front();

        // If not scheduled for next time unit, then thread can go to sleep until that time
        if (next.time_until > 1)
        {
            return next.time_until;
        }
        else
        {
            // Double check if target node exists
            if (nodes_.find(next.msg.to) == nodes_.end()) continue;

            // Execute CAN command presently sent on the bus
            nodes_[next.msg.to]->ProcessCommand(next.msg);
            bus_queue_.pop_front();
            // Progress time that has passed for each frame by 1 step
            for (auto msg : bus_queue_)
            {
                msg.time_until--;
            }
            CANFDmessage_t next_msg;
            int t_next = nodes_[next.msg.from]->GetNextSendTime(&next_msg);
            
            enqueueCANMessage(t_next, next_msg);
        }
    }
}

/* Loops over current pending CAN messages queue, inserts new message where appropriate*/
void VirtualCANBus::enqueueCANMessage(int time_until, CANFDmessage_t msg)
{
    scheduled_bus_activity_t scheduled_message = {
        time_until,
        msg
    };

    for (auto it = bus_queue_.begin(); it != bus_queue_.end(); ++it)
    {
        // In case of a message that will be sent sooner, move further back to the queue
        if (scheduled_message.time_until < time_until) {
            continue;
        }
        // In case of simultaneous messages (bus collision), order is determined by command prio, if higher prio (lower ID), then move back
        if (scheduled_message.time_until == time_until) {
            if (scheduled_message.msg.command < msg.command) {
                continue;
            }
        }

        // Time_until of message next up is greater, or we have higher prio at equal time. Insert.
        bus_queue_.insert(it, scheduled_message);
    }
}

/* Adds a node to the bus */
bool VirtualCANBus::addNode(const uint64_t id, void (*scan_cb)(scan_data_msg_t))
{
    if (nodes_.find(id) == nodes_.end())
    {
        nodes_[id] = new TagNode(id, scan_cb);
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