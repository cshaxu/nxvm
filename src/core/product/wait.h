#ifndef NTVDM64_CORE_PRODUCT_WAIT_H
#define NTVDM64_CORE_PRODUCT_WAIT_H

#include <stdint.h>

typedef void (*core_product_wait_provider)(void *context, uint32_t milliseconds);

void core_product_wait_bind(core_product_wait_provider provider, void *context);
void core_product_wait_milliseconds(uint32_t milliseconds);

#endif
