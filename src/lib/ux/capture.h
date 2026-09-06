#ifndef UX_CAPTURE_H
#define UX_CAPTURE_H

#include "lib/base/base.h"

/* A presenter records only whether its native capture is active.  The native
 * backend owns the corresponding host calls; products never receive a native
 * handle or product input-device state through this value. */
typedef enum ux_capture_state {
    UX_CAPTURE_RELEASED,
    UX_CAPTURE_ACTIVE
} ux_capture_state;

typedef struct ux_capture {
    ux_capture_state state;
} ux_capture;

static inline void ux_capture_initialize(ux_capture *capture)
{
    if (capture != LIB_NULL) capture->state = UX_CAPTURE_RELEASED;
}

static inline void ux_capture_activate(ux_capture *capture)
{
    if (capture != LIB_NULL) capture->state = UX_CAPTURE_ACTIVE;
}

static inline void ux_capture_release(ux_capture *capture)
{
    if (capture != LIB_NULL) capture->state = UX_CAPTURE_RELEASED;
}

static inline lib_bool ux_capture_is_active(const ux_capture *capture)
{
    return capture != LIB_NULL && capture->state == UX_CAPTURE_ACTIVE;
}

#endif
