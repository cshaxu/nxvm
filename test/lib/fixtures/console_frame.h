#ifndef LIB_TEST_CONSOLE_FRAME_H
#define LIB_TEST_CONSOLE_FRAME_H

#include "lib/kvm-console/console_interface.h"

static lib_status test_console_frame_copy(kvm_console_text_frame *writer,
    const kvm_console_text_frame *value)
{
    lib_status status;
    if (writer == LIB_NULL || value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = kvm_console_text_frame_validate(value);
    if (status != LIB_STATUS_OK) return status;
    status = kvm_console_frame_prepare(writer, value->base.text_columns,
        value->base.text_rows, value->base.stride);
    if (status != LIB_STATUS_OK) return status;
    kvm_text_cell *cells = writer->base.cells;
    lib_size capacity = writer->base.capacity;
    writer->base = value->base;
    writer->base.cells = cells;
    writer->base.capacity = capacity;
    *writer->characters = *value->characters;
    lib_memory_copy(cells, value->base.cells, capacity * sizeof(*cells));
    return LIB_STATUS_OK;
}

static lib_status test_console_frame(kvm_console *console, kvm_frame_exchange *frames,
    const kvm_console_text_frame *value)
{
    if (console == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    lib_status status = test_console_frame_copy(kvm_frame_exchange_writer(frames), value);
    if (status != LIB_STATUS_OK) return status;
    status = kvm_console_frame_commit(frames);
    return status == LIB_STATUS_OK ? kvm_console_publish_frame(console, frames) : status;
}

#endif
