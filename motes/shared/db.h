#pragma once
#include "datatypes.h"

void init_test_database(product_info_t* db, uint64_t size);
product_info_t db_query_read(product_info_t* db, uint64_t product_id);
customer_tab_t* find_or_add_customer(customer_tab_t** head, uint32_t customer_id);
product_order_t* find_or_add_product(product_order_t** product_list, uint16_t product_id);
void modify_product_quantity(product_order_t* product, uint16_t quantity, scan_type_t command);
void wipe_customer(customer_tab_t **head, uint32_t customer_id);
