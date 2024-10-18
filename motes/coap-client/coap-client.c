#include "contiki.h"
#include "contiki-net.h"
#include "coap-observe-client.h"
#include "coap-blocking-api.h"
#include "sys/node-id.h"
#include "sys/log.h"
#include "../../libraries/CAN-sim/shared/types.h"

#include "datatypes.h"
#include "utility.h"
#include "custom-schedule.h"

#define LOG_MODULE "Client"
#define LOG_LEVEL LOG_LEVEL_DBG
#define NR_OF_CAN_NODES 10

/*---------------------------------------------------------------------------*/
/* Defined in C++ code */
extern uint8_t init_can_bus(uint64_t id, void (*scan_callback)(scan_data_msg_t, uint64_t), void (*product_update_callback)(unsigned long, uint64_t), uint64_t cluster_id);
extern uint8_t add_node(uint64_t id, void (*scan_callback)(scan_data_msg_t, uint64_t), void (*product_update_callback)(unsigned long, uint64_t));
extern uint8_t remove_node(uint64_t id);
extern void send_can_message(CAN_command command, uint64_t target_node, CAN_data_t payload);
extern float simulate_can_bus();
extern void set_product_id(uint64_t node_id, ean13_t product_id);
extern void update_visualization(int clock);
/*---------------------------------------------------------------------------*/

#define TOGGLE_INTERVAL 10

PROCESS(client, "Client process with customer scan");
AUTOSTART_PROCESSES(&client);

static coap_observee_t *obs;
#define MAX_SCANS 50
typedef struct
{
    scan_submission scans[MAX_SCANS];
    int len;
} scan_submissions;
#define MAX_REQUESTS 50
typedef struct
{
    product_update_request requests[MAX_REQUESTS];
    int len;
} product_update_requests;

static scan_submissions node_scan_submissions;
static product_update_requests node_product_update_requests;
static scan_submission current_submission;
product_update_request current_request;
product_t current_request_response;

void scan_handler(coap_message_t *response)
{
    if (response == NULL)
    {
        LOG_ERR("Scan request timed out\n");
    }
}

void query_handler(coap_message_t *response)
{
    if (response == NULL)
    {
        LOG_ERR("Query request timed out\n");
        return;
    }

    product_t product = *(product_t *)response->payload;
    LOG_INFO("Query ID %lu: %s, price: %hu cents\n", product.id, product.description, product.price);
    current_request_response = product;
}

void notification_callback(coap_observee_t *subject, void* notification, coap_notification_flag_t flag) {
    LOG_INFO("Notification on URI: %s\n", subject->url);

    product_t product = *(product_t*) ((coap_message_t*) notification)->payload;

    switch (flag) {
        case NOTIFICATION_OK:
        case OBSERVE_OK:
            LOG_INFO("Product with ID %lu updated: %s (price: %hu, stocked: %hu)\n", product.id, product.description, product.price, product.is_stocked);
            break;
        case OBSERVE_NOT_SUPPORTED:
        case ERROR_RESPONSE_CODE:
        case NO_REPLY_FROM_SERVER:
            // TODO: More descriptive log message
            LOG_ERR("Something went wrong: %d\n", flag);
            obs = NULL;
            break;
    }
}

void scan_callback(scan_data_msg_t data, uint64_t calling_node)
{
    node_scan_submissions.scans[node_scan_submissions.len] = (scan_submission){
        calling_node,
        (scan_data_t){
            data.customer_id,
            data.product_id,
            1,
            (scan_type_t)ADD}};
    node_scan_submissions.len++;
}

void product_update_callback(ean13_t product_id, uint64_t calling_node)
{
    // printf("Product update for %ld generated by NODE#%ld\n", product_id, calling_node);

    node_product_update_requests.requests[node_product_update_requests.len] = (product_update_request){
        calling_node,
        product_id};
    node_product_update_requests.len++;
}

PROCESS_THREAD(client, ev, data)
{
    static struct etimer timer;
    coap_endpoint_t server_ep;
    uip_ipaddr_t root;
    static coap_message_t request;

    PROCESS_BEGIN();

    initialize_tsch_schedule();
    int can_started = init_can_bus(NR_OF_CAN_NODES, &scan_callback, &product_update_callback, node_id);
    printf("CAN started: %d\n", can_started);

    etimer_set(&timer, TOGGLE_INTERVAL * node_id * CLOCK_SECOND);

    float temp = simulate_can_bus();
    temp = temp;

    while (1)
    {
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
        update_visualization(clock_time());

        if (!NETSTACK_ROUTING.node_is_reachable() || !NETSTACK_ROUTING.get_root_ipaddr(&root))
        {
            // CAN bus is not simulated here since we'd otherwise throw away messages or let queue grow unbounded
            printf("Node not reachable, skipping iterations\n");
            etimer_set(&timer, CLOCK_SECOND);
            obs = NULL;
            continue;
        }
        

        // Convert root IPv6 address to string
        char ip[UIPLIB_IPV6_MAX_STR_LEN];
        uiplib_ipaddr_snprint(ip, sizeof(ip), &root);

        coap_endpoint_parse(ip, strlen(ip), &server_ep);

        if (obs == NULL)
        {
            obs = coap_obs_request_registration(&server_ep, UPDATE_URI, notification_callback, NULL);
        }

        for (int i = 0; i < node_scan_submissions.len; i++)
        {
            current_submission = (scan_submission) {
                .calling_node = node_scan_submissions.scans[i].calling_node,
                .scan = (scan_data_t){
                    node_scan_submissions.scans[i].scan.customer_id,
                    node_scan_submissions.scans[i].scan.product_id,
                    node_scan_submissions.scans[i].scan.quantity,
                    node_scan_submissions.scans[i].scan.command
                }
            };
            request = coap_create_request(COAP_POST, SCAN_URI, COAP_TYPE_CON, &current_submission.scan, sizeof(current_submission.scan));
            COAP_BLOCKING_REQUEST(&server_ep, &request, scan_handler);
            send_can_message(SCAN_ACK, current_submission.calling_node, (CAN_data_t){.empty = true});
        }
        node_scan_submissions.len = 0;

        for (int i = 0; i < node_product_update_requests.len; i++)
        {
            current_request = (product_update_request) { 
                node_product_update_requests.requests[i].calling_node,
                node_product_update_requests.requests[i].product_id
            };
            request = coap_create_request(COAP_GET, QUERY_URI, COAP_TYPE_CON, &(current_request), sizeof(current_request));
            COAP_BLOCKING_REQUEST(&server_ep, &request, query_handler);
            send_can_message(PRODUCT_UPDATE, current_request.calling_node, (CAN_data_t){ .product_info = current_request_response });
        }
        node_product_update_requests.len = 0;

        etimer_set(&timer, simulate_can_bus() * CLOCK_SECOND);
    }

    PROCESS_END();
}
