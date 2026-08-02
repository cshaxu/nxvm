#include <stdint.h>
#include <stdio.h>

#include "core/product/utils.h"
#include "core/product/wait.h"
#include "core/product/wait_provider.h"

static uint32_t observed_milliseconds;

static void core_product_wait_smoke_provider(void *context, uint32_t milliseconds)
{
    uint32_t *calls = context;

    *calls += 1u;
    observed_milliseconds = milliseconds;
}

int main(void)
{
    uint32_t calls = 0u;

    core_product_wait_bind(core_product_wait_smoke_provider, &calls);
    utilsSleep(17u);
    if (calls != 1u || observed_milliseconds != 17u) {
        return 1;
    }
    core_product_wait_bind(NULL, NULL);
    utilsSleep(1u);
    if (calls != 1u) {
        return 1;
    }
    puts("M5:T14:S3:CORE-PRODUCT-WAIT:OK");
    return 0;
}
