#include "router.h"

void ux_router_initialize(ux_router *router, ux_target target)
{
    if (router == LIB_NULL) return;
    atomic_init(&router->target, (unsigned int)target);
}

ux_target ux_router_target(const ux_router *router)
{
    return router == LIB_NULL ? UX_TARGET_NONE :
        (ux_target)atomic_load(&router->target);
}

void ux_router_request(ux_router *router, ux_target target)
{
    if (router != LIB_NULL) atomic_store(&router->target, (unsigned int)target);
}
