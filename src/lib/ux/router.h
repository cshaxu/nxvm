#ifndef UX_ROUTER_H
#define UX_ROUTER_H

#include "lib/ux/frame.h"

typedef enum ux_display_policy { UX_DISPLAY_WINDOW, UX_DISPLAY_CONSOLE } ux_display_policy;

typedef enum ux_target { UX_TARGET_WINDOW, UX_TARGET_CONSOLE } ux_target;

typedef struct ux_router {
    ux_display_policy policy;
    ux_target target;
    type_unsigned_32 stable_text_frames;
} ux_router;

#define UX_STABLE_TEXT_FRAMES 3u

void ux_router_initialize(ux_router *router, ux_display_policy policy);
ux_target ux_router_target(const ux_router *router);
/* Observe one copied frame.  `WINDOW` never routes to console. `CONSOLE`
 * switches immediately to graphics and only returns after stable text. */
ux_target ux_router_observe(ux_router *router, const ux_frame *frame);

#endif
