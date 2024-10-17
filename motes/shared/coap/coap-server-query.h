#pragma once

#include <stdint.h>
#include "../../shared/coap/coap-datatypes.h"

void init_test_database(product_info_t* db, unsigned long long size) { //initialized database with testing data
    for (unsigned long long i = 0; i < size; i++) {
        db[i].product_id = i + 1; // EAN-13 product ID
        db[i].product_price = (2 * i + 2) * (0.115) * 100; // Product price in cents
        sprintf(db[i].product_description, "Highly sought-after comoditity n %llu %s", i,"\0"); //a short product description 
        db[i].is_stocked = 1;
    }
}

product_info_t db_query_read(product_info_t* db, uint64_t product_id) { //searches initialized database for matching EAN13 product id and returns all info
    unsigned char product_found = 0; 

    unsigned long long i = 0; //index into database to be iterated with
    product_info_t db_product; //blank product information sheet that we'll send out

    while (product_found == 0) { //until found in database
        if (i >= DB_SIZE) { //no such element in DB
            db_product.product_id = product_id;
            db_product.product_price = 0;
            sprintf(db_product.product_description, "NA");
            db_product.is_stocked = 0;
            return db_product;
        }

        if (db[i].product_id == product_id) {//found match
            product_found = 1;
        }

        // Else keep looking
        i = i + 1;
    }

    db_product.product_id = product_id;
    db_product.product_price = db[i-1].product_price;
    sprintf(db_product.product_description, "%s", db[i - 1].product_description);
    db_product.is_stocked = 1;
    return db_product;
}


