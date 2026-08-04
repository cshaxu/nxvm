#include "type.h"




#include "core/product/utils.h"

#include "core/product/wait.h"

#include "core/product/wait_provider.h"

static uint32_t observed_milliseconds;

static C_VOID core_product_wait_smoke_provider(C_VOID *context, uint32_t milliseconds)
{
    uint32_t *calls = context;

    *calls += 1u;
    observed_milliseconds = milliseconds;
}

C_INT main(C_VOID)
{
    uint32_t calls = 0u;
    uint32_t nested_calls = 0u;
    core_product_wait_scope scope;
    core_product_wait_scope nested_scope;

    core_product_wait_scope_initialize(&scope, core_product_wait_smoke_provider,
        &calls);
    core_product_wait_milliseconds(&scope, 17u);
    if (calls != 1u || observed_milliseconds != 17u) {
        return 1;
    }
    core_product_wait_scope_initialize(&nested_scope,
        core_product_wait_smoke_provider, &nested_calls);
    core_product_wait_milliseconds(&nested_scope, 23u);
    if (calls != 1u || nested_calls != 1u || observed_milliseconds != 23u) {
        return 1;
    }
    core_product_wait_milliseconds(&scope, 31u);
    if (calls != 2u || nested_calls != 1u || observed_milliseconds != 31u) {
        return 1;
    }
    core_product_wait_milliseconds(STD_NULL, 1u);
    if (calls != 2u) {
        return 1;
    }
    puts("M5:T80:S6:WAIT-SCOPE:OK");
    return 0;
}
