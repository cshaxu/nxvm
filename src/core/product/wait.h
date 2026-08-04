#ifndef CORE_PRODUCT_WAIT_H
#define CORE_PRODUCT_WAIT_H

#include "type.h"
#include "core/product/wait_provider.h"

C_VOID core_product_wait_milliseconds(const core_product_wait_scope *scope,
    uint32_t milliseconds);

#endif
