#ifndef LIB_TEST_WINDOW_FRAME_H
#define LIB_TEST_WINDOW_FRAME_H

#include "lib/kvm-window/window_interface.h"

/* Fixture values are copied only to prepare the producer-owned canvas. The
 * tested commit, notification and consumer transfer use production APIs. */
static lib_status test_window_frame(kvm_window *window, kvm_frame_exchange *frames,
    const kvm_window_frame *value)
{
    lib_status status;
    if (window == LIB_NULL || value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = kvm_window_frame_validate(value);
    if (status != LIB_STATUS_OK) return status;
    (void)kvm_window_frame_copy(kvm_frame_exchange_writer(frames), value);
    status = kvm_window_frame_commit(frames);
    return status == LIB_STATUS_OK ? kvm_window_publish_frame(window, frames) : status;
}

#endif
