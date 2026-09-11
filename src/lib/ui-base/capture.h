#ifndef UI_CAPTURE_H
#define UI_CAPTURE_H

#include "lib/types/types_interface.h"

/* A presenter records only whether its native capture is active.  The native
 * backend owns the corresponding host calls; products never receive a native
 * handle or product input-device state through this value. */
typedef enum ui_capture_state {
    UI_CAPTURE_RELEASED,
    UI_CAPTURE_ACTIVE
} ui_capture_state;

typedef struct ui_capture {
    ui_capture_state state;
} ui_capture;

static inline void ui_capture_initialize(ui_capture *capture)
{
    if (capture != LIB_NULL) capture->state = UI_CAPTURE_RELEASED;
}

static inline void ui_capture_activate(ui_capture *capture)
{
    if (capture != LIB_NULL) capture->state = UI_CAPTURE_ACTIVE;
}

static inline void ui_capture_release(ui_capture *capture)
{
    if (capture != LIB_NULL) capture->state = UI_CAPTURE_RELEASED;
}

static inline lib_bool ui_capture_is_active(const ui_capture *capture)
{
    return capture != LIB_NULL && capture->state == UI_CAPTURE_ACTIVE;
}

#endif
