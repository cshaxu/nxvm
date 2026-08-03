#include "core/product/wait.h"
#include "core/product/wait_provider.h"

static _Thread_local core_product_wait_scope coreProductWaitScope;

void core_product_wait_scope_initialize(core_product_wait_scope *scope,
    core_product_wait_provider provider, void *context)
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

void core_product_wait_scope_leave(core_product_wait_scope previous)
{
    coreProductWaitScope = previous;
}

void core_product_wait_milliseconds(uint32_t milliseconds)
{
    if (coreProductWaitScope.provider != NULL) {
        coreProductWaitScope.provider(coreProductWaitScope.context, milliseconds);
    }
}

void utilsSleep(uint32_t milisec)
{
    core_product_wait_milliseconds(milisec);
}
