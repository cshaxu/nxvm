#include "type.h"

#include "core/product/wait.h"

#include "core/product/wait_provider.h"

C_VOID core_product_wait_scope_initialize(core_product_wait_scope *scope,
    core_product_wait_provider provider, C_VOID *context)
{
    if (scope == STD_NULL) return;
    scope->provider = provider;
    scope->context = context;
}

C_VOID core_product_wait_milliseconds(const core_product_wait_scope *scope,
    uint32_t milliseconds)
{
    if (scope != STD_NULL && scope->provider != STD_NULL) {
        scope->provider(scope->context, milliseconds);
    }
}

C_VOID core_product_utils_sleep(const core_product_wait_scope *scope,
    uint32_t milisec)
{
    core_product_wait_milliseconds(scope, milisec);
}
