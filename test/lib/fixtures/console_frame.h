#ifndef LIB_TEST_CONSOLE_FRAME_H
#define LIB_TEST_CONSOLE_FRAME_H

#include "lib/kvm-console/console_interface.h"

static lib_status test_console_frame(kvm_console *console, kvm_frame_exchange *frames,
    const kvm_console_text_frame *value)
{
    lib_status status;
    if (console == LIB_NULL || value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *(kvm_console_text_frame *)kvm_frame_exchange_writer(frames) = *value;
    status = kvm_console_frame_commit(frames);
    return status == LIB_STATUS_OK ? kvm_console_publish_frame(console, frames) : status;
}

#endif
