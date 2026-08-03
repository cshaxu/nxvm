#ifndef NTVDM64_CORE_PRODUCT_WAIT_PROVIDER_H
#define NTVDM64_CORE_PRODUCT_WAIT_PROVIDER_H

#include <stdint.h>

typedef void (*core_product_wait_provider)(void *context, uint32_t milliseconds);

typedef struct core_product_wait_scope {
    core_product_wait_provider provider;
    void *context;
} core_product_wait_scope;

void core_product_wait_scope_initialize(core_product_wait_scope *scope,
    core_product_wait_provider provider, void *context);
core_product_wait_scope core_product_wait_scope_enter(
    const core_product_wait_scope *scope);
void core_product_wait_scope_leave(core_product_wait_scope previous);

#endif
