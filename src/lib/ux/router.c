#include "router.h"

void ux_router_initialize(ux_router *router, ux_target target)
{
    if (router == LIB_NULL) return;
    router->target = target;
}

ux_target ux_router_target(const ux_router *router)
{
    return router == LIB_NULL ? UX_TARGET_WINDOW : router->target;
}

void ux_router_request(ux_router *router, ux_target target)
{
    if (router != LIB_NULL) router->target = target;
}
