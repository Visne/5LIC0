#include <tsch-const.h>
#include <tsch-types.h>
#include <tsch.h>
#include "contiki.h"
#include "simple-udp.h"
#include "net/netstack.h"
#include "tsch-schedule.h"
#include "sys/log.h"

#define LOG_MODULE "Server"
#define LOG_LEVEL LOG_LEVEL_DBG

#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

static void initialize_tsch_schedule(void);
static struct simple_udp_connection connection;

typedef struct {
    unsigned long customer_id;
    unsigned long long product_id;
} scan_data_t;


static void on_message(struct simple_udp_connection *c,
                       const uip_ipaddr_t *sender,
                       uint16_t sender_port,
                       const uip_ipaddr_t *receiver,
                       uint16_t receiver_port,
                       const uint8_t *data,
                       uint16_t len) {
    scan_data_t* scan_data = (scan_data_t*) data;
    LOG_INFO("Received data: customer #%lu scanned product #%llu\n", scan_data->customer_id, scan_data->product_id);
}

PROCESS(server, "Server process");
AUTOSTART_PROCESSES(&server);

PROCESS_THREAD(server, ev, data) {
    PROCESS_BEGIN();

    initialize_tsch_schedule();

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

static void initialize_tsch_schedule(void)
{
    struct tsch_slotframe *sf = tsch_schedule_add_slotframe(0, 1);

    tsch_schedule_add_link(sf,
                           LINK_OPTION_RX | LINK_OPTION_TX | LINK_OPTION_SHARED,
                           LINK_TYPE_ADVERTISING, &tsch_broadcast_address,
                           0, 0, 1);
}
