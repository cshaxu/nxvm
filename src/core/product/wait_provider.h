#ifndef CORE_PRODUCT_WAIT_PROVIDER_H
#define CORE_PRODUCT_WAIT_PROVIDER_H

#include "type.h"


typedef C_VOID (*core_product_wait_provider)(C_VOID *context, uint32_t milliseconds);

typedef struct core_product_wait_scope {
    core_product_wait_provider provider;
    C_VOID *context;
} core_product_wait_scope;

C_VOID core_product_wait_scope_initialize(core_product_wait_scope *scope,
    core_product_wait_provider provider, C_VOID *context);

#endif
