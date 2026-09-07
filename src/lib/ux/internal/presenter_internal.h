#ifndef UX_PRESENTER_INTERNAL_H
#define UX_PRESENTER_INTERNAL_H

#include "lib/ux/presenter.h"

typedef enum ux_presenter_control_kind {
    UX_PRESENTER_CONTROL_TARGET,
    UX_PRESENTER_CONTROL_WINDOW_TITLE,
    UX_PRESENTER_CONTROL_STOP
} ux_presenter_control_kind;

typedef struct ux_presenter_control {
    ux_presenter_control_kind kind;
    ux_target target;
    char title[UX_WINDOW_TITLE_CAPACITY];
} ux_presenter_control;

lib_bool ux_presenter_take_control(ux_presenter *presenter,
    ux_presenter_control *out_control);
lib_status ux_presenter_capture_frame(const ux_presenter *presenter,
    ux_frame *out_frame);
lib_u32 ux_presenter_frame_generation(const ux_presenter *presenter);

#endif
