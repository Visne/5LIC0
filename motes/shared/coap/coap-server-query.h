#pragma once
#include "../../shared/coap/coap-datatypes.h"

void init_test_database(product_info_t* db, unsigned long long int size) { //initialized database with testing data
    for (unsigned long long i = 0; i < size; i++) {
        sprintf(db[i].product_id, "%llu, %s", i+1, "\0"); //EAN13 product ID
        float price = (2*i + 2) * (0.115) * 100;
        sprintf(db[i].product_price, "%2.2f%s", price, "\0"); //product price in cents
        sprintf(db[i].product_description, "Highly sought-after comoditity n %llu %s", i,"\0"); //a short product description 
        sprintf(db[i].is_stocked, "1%s", "\0");
        
    }
}

product_info_t db_query_read(product_info_t* db, char* product_id) { //searches initialized database for matching EAN13 product id and returns all info
    char* pEnd1;
    char* pEnd2;
    unsigned char product_found = 0; 
    unsigned long long int searched_product_id; //converted equivalent to argument product_id. easier given that we're transmiting/recieving text
    unsigned long long int database_product_id;//working var corresponding to the EAN13 code stored in database currently being looked at
    char database_product_id_char[16];

    //for string to llu conversion
    searched_product_id = strtoull(product_id, &pEnd1, 10); //convert string rerpesentation of request EAD code into numerical one
    unsigned long long i = 0; //index into database to be iterated with
    product_info_t db_product; //blank product information sheet that we'll send out

    while (product_found == 0) { //until found in database
        if (i >= DB_SIZE) { //no such element in DB
            sprintf(db_product.product_id, "%s%s", product_id, "\0");
            sprintf(db_product.product_price, "NA%s", "\0");
            sprintf(db_product.product_description, "NA%s", "\0");
            sprintf(db_product.is_stocked, "0%s", "\0");
            return db_product;
        }

        sprintf(database_product_id_char, "%s", db[i].product_id); //copy currently indexed product id
        database_product_id = strtoull(database_product_id_char, &pEnd2, 10); //convert string rerpesentation of currently indexed database EAD code into numerical one
        if (database_product_id == searched_product_id) {//found match
            product_found = 1;
        }
        //else keep looking
        i = i + 1;
    }

    sprintf(db_product.product_id, "%s%s", product_id, "\0");
    sprintf(db_product.product_price, "%s%s", db[i-1].product_price, "\0");
    sprintf(db_product.product_description, "%s%s", db[i - 1].product_description, "\0");
    sprintf(db_product.is_stocked, "1%s", "\0");
    return db_product;
}


