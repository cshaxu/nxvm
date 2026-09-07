#ifndef UX_ROUTER_H
#define UX_ROUTER_H

#include "lib/base/base.h"

#define UX_WINDOW_TITLE_CAPACITY 128u

typedef enum ux_target {
    UX_TARGET_WINDOW,
    UX_TARGET_CONSOLE,
    UX_TARGET_NONE
} ux_target;

typedef struct ux_router {
    atomic_uint target;
    atomic_uint active_target;
    atomic_flag title_lock;
    lib_u32 title_generation;
    char window_title[UX_WINDOW_TITLE_CAPACITY];
} ux_router;

void ux_router_initialize(ux_router *router, ux_target target);
ux_target ux_router_target(const ux_router *router);
void ux_router_request(ux_router *router, ux_target target);
void ux_router_set_active_target(ux_router *router, ux_target target);
ux_target ux_router_active_target(const ux_router *router);
/* A title request is accepted only by an active native Window surface. */
lib_bool ux_router_request_window_title(ux_router *router, const char *title);
lib_status ux_router_capture_window_title(const ux_router *router,
    char *buffer, lib_u32 buffer_size, lib_u32 *out_generation);

#endif
