#ifndef NTVDM64_CORE_PRODUCT_WAIT_PROVIDER_H
#define NTVDM64_CORE_PRODUCT_WAIT_PROVIDER_H

#include <stdint.h>

typedef void (*core_product_wait_provider)(void *context, uint32_t milliseconds);

void core_product_wait_bind(core_product_wait_provider provider, void *context);

#endif
