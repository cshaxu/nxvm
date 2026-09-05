#include "router.h"

void ux_router_initialize(ux_router *router, ux_display_policy policy)
{
    if (router == NULL) return;
    router->policy = policy;
    router->target = policy == UX_DISPLAY_WINDOW ? UX_TARGET_WINDOW : UX_TARGET_CONSOLE;
    router->stable_text_frames = 0u;
}

ux_target ux_router_target(const ux_router *router)
{
    return router == NULL ? UX_TARGET_WINDOW : router->target;
}

ux_target ux_router_observe(ux_router *router, const ux_frame *frame)
{
    if (router == NULL || frame == NULL || frame->valid == 0u)
        return ux_router_target(router);
    if (router->policy == UX_DISPLAY_WINDOW) {
        router->target = UX_TARGET_WINDOW;
        return router->target;
    }
    if (frame->graphics != 0u) {
        router->target = UX_TARGET_WINDOW;
        router->stable_text_frames = 0u;
    } else if (router->target == UX_TARGET_WINDOW &&
        ++router->stable_text_frames >= UX_STABLE_TEXT_FRAMES) {
        router->target = UX_TARGET_CONSOLE;
        router->stable_text_frames = 0u;
    }
    return router->target;
}
