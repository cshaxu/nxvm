#ifndef MYPC110_PRODUCT_CONTEXT_H
#define MYPC110_PRODUCT_CONTEXT_H

#include "app-mypc110/product_vdisplay.h"
#include "app-mypc110/product_card.h"

typedef struct mypc110_product_context {
    mypc110_vdisplay display;
    mypc110_product_cards cards;
    void *plan;
} mypc110_product_context;

#endif
