#ifndef UX_PRESENTER_INTERNAL_H
#define UX_PRESENTER_INTERNAL_H

#include "lib/ux/presenter.h"

lib_u32 ux_presenter_capture_target(const ux_presenter *presenter,
    ux_target *out_target);
lib_u32 ux_presenter_capture_window_title(const ux_presenter *presenter,
    char out_title[UX_WINDOW_TITLE_CAPACITY]);
lib_status ux_presenter_capture_frame(const ux_presenter *presenter,
    ux_frame *out_frame);
lib_u32 ux_presenter_frame_generation(const ux_presenter *presenter);
lib_u32 ux_presenter_capture_mouse_capturable(const ux_presenter *presenter,
    lib_bool *out_capturable);
lib_bool ux_presenter_take_mouse_release(ux_presenter *presenter);
void ux_presenter_set_mouse_capture_state(ux_presenter *presenter,
    ux_mouse_capture_state state);

#endif
