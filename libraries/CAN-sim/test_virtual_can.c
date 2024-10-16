#include <stdio.h>
#include <stdint.h>

extern uint8_t add_sender_endpoint(const char* id);
extern uint8_t add_receiver_endpoint(const char* id);
extern uint8_t send_data(const char* senderId, const char* data);
extern uint8_t receive_data(const char* receiverId, char* data);

int main() {
    add_sender_endpoint("sender1");
    add_receiver_endpoint("receiver1");

    send_data("sender1", "Test message");
    
    char buffer[256];
    receive_data("receiver1", buffer);
    printf("Received data: %s\n", buffer);

    return 0;
}