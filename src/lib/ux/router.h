#ifndef UX_ROUTER_H
#define UX_ROUTER_H

#include "lib/base/base.h"

typedef enum ux_target {
    UX_TARGET_WINDOW,
    UX_TARGET_CONSOLE,
    UX_TARGET_NONE
} ux_target;

typedef struct ux_router {
    atomic_uint target;
} ux_router;

void ux_router_initialize(ux_router *router, ux_target target);
ux_target ux_router_target(const ux_router *router);
void ux_router_request(ux_router *router, ux_target target);

#endif
