#include "contiki.h"
#include "simple-udp.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "sys/log.h"

#define LOG_MODULE "Server"
#define LOG_LEVEL LOG_LEVEL_DBG

#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

typedef struct {
    long customer;
    long product;
} scan_data_t;

static struct simple_udp_connection connection;

static void on_message(struct simple_udp_connection *c,
                       const uip_ipaddr_t *sender,
                       uint16_t sender_port,
                       const uip_ipaddr_t *receiver,
                       uint16_t receiver_port,
                       const uint8_t *data,
                       uint16_t len) {
    scan_data_t* scan_data = (scan_data_t*) data;
    LOG_INFO("Received data: customer #%ld scanned product #%ld\n", scan_data->customer, scan_data->product);
}

PROCESS(server, "Server process");
AUTOSTART_PROCESSES(&server);

PROCESS_THREAD(server, ev, data) {
    PROCESS_BEGIN();

    if (NETSTACK_ROUTING.root_start()) {
        LOG_ERR("Failed to set up RPL root node\n");
        PROCESS_EXIT();
    }

    if (!simple_udp_register(&connection,
                             UDP_SERVER_PORT,
                             NULL,
                             UDP_CLIENT_PORT,
                             on_message)) {
        LOG_ERR("Failed to register UDP connection\n");
        PROCESS_EXIT();
    }

    PROCESS_END();
}
