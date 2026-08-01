#include "core/product/debug/debug_target.h"

static const core_product_debug_target *coreProductDebugTarget;

void core_product_debug_bind_target(const core_product_debug_target *target)
{
    coreProductDebugTarget = target;
}

const core_product_debug_target *core_product_debug_get_target(void)
{
    return coreProductDebugTarget;
}
