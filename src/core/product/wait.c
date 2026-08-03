#include "type.h"

#include "core/product/wait.h"

#include "core/product/wait_provider.h"

static _Thread_local core_product_wait_scope coreProductWaitScope;

C_VOID core_product_wait_scope_initialize(core_product_wait_scope *scope,
    core_product_wait_provider provider, C_VOID *context)
{
    if (scope == NULL) return;
    scope->provider = provider;
    scope->context = context;
}

core_product_wait_scope core_product_wait_scope_enter(
    const core_product_wait_scope *scope)
{
    core_product_wait_scope previous = coreProductWaitScope;

    if (scope == NULL) {
        coreProductWaitScope.provider = NULL;
        coreProductWaitScope.context = NULL;
    } else {
        coreProductWaitScope = *scope;
    }
    return previous;
}

C_VOID core_product_wait_scope_leave(core_product_wait_scope previous)
{
    coreProductWaitScope = previous;
}

C_VOID core_product_wait_milliseconds(uint32_t milliseconds)
{
    if (coreProductWaitScope.provider != NULL) {
        coreProductWaitScope.provider(coreProductWaitScope.context, milliseconds);
    }
}

C_VOID core_product_utils_sleep(uint32_t milisec)
{
    core_product_wait_milliseconds(milisec);
}
