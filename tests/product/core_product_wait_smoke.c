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
    uint32_t nested_calls = 0u;
    core_product_wait_scope scope;
    core_product_wait_scope nested_scope;
    core_product_wait_scope previous;
    core_product_wait_scope nested_previous;

    core_product_wait_scope_initialize(&scope, core_product_wait_smoke_provider,
        &calls);
    previous = core_product_wait_scope_enter(&scope);
    core_product_utils_sleep(17u);
    if (calls != 1u || observed_milliseconds != 17u) {
        return 1;
    }
    core_product_wait_scope_initialize(&nested_scope,
        core_product_wait_smoke_provider, &nested_calls);
    nested_previous = core_product_wait_scope_enter(&nested_scope);
    core_product_utils_sleep(23u);
    if (calls != 1u || nested_calls != 1u || observed_milliseconds != 23u) {
        return 1;
    }
    core_product_wait_scope_leave(nested_previous);
    core_product_utils_sleep(31u);
    if (calls != 2u || nested_calls != 1u || observed_milliseconds != 31u) {
        return 1;
    }
    core_product_wait_scope_leave(previous);
    core_product_utils_sleep(1u);
    if (calls != 2u) {
        return 1;
    }
    puts("M5:T80:S6:WAIT-SCOPE:OK");
    return 0;
}
