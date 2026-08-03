#include "type.h"

#include "core/product/debug/debug_target.h"

static _Thread_local const core_product_debug_target *coreProductDebugScopeTarget;

C_VOID core_product_debug_scope_enter(const core_product_debug_target *target)
{
    coreProductDebugScopeTarget = target;
}

C_VOID core_product_debug_scope_leave(C_VOID)
{
    coreProductDebugScopeTarget = NULL;
}

const core_product_debug_target *core_product_debug_scope_target(C_VOID)
{
    return coreProductDebugScopeTarget;
}
