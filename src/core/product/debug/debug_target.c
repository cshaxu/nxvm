#include "core/product/debug/debug_target.h"

static _Thread_local const core_product_debug_target *coreProductDebugScopeTarget;

void core_product_debug_scope_enter(const core_product_debug_target *target)
{
    coreProductDebugScopeTarget = target;
}

void core_product_debug_scope_leave(void)
{
    coreProductDebugScopeTarget = NULL;
}

const core_product_debug_target *core_product_debug_scope_target(void)
{
    return coreProductDebugScopeTarget;
}
