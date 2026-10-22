#ifndef KVM_CONSOLE_FRAME_INTERFACE_H
#define KVM_CONSOLE_FRAME_INTERFACE_H

#include "lib/kvm-base/frame_interface.h"
#include "lib/kvm-base/frame_exchange_interface.h"

/* One BMP character (excluding surrogate code units) per glyph index. Both banks are copied frame data, not
 * resources retained from a caller. Each cell's glyph_bank selects the map. */
typedef struct kvm_console_character_map {
    lib_u16 primary[256u];
    lib_u16 secondary[256u];
} kvm_console_character_map;

typedef struct kvm_console_text_frame {
    kvm_text_frame base;
    kvm_console_character_map *characters;
} kvm_console_text_frame;

static inline void kvm_console_frame_release(kvm_console_text_frame *frame)
{
    lib_release(frame->characters);
    *frame = (kvm_console_text_frame){0};
}

static inline void kvm_console_frame_retire(void *slot, const void *latest)
{
    kvm_console_text_frame *frame = slot;
    const kvm_console_text_frame *current = latest;
    if (current == LIB_NULL || frame->base.text_columns != current->base.text_columns ||
        frame->base.text_rows != current->base.text_rows || frame->base.stride != current->base.stride)
        kvm_console_frame_release(frame);
}

/* Unheld writer only, initially zero-initialized. Equal specifications reuse
 * storage; allocation failure empties only this writer. */
static inline lib_status kvm_console_frame_prepare(kvm_console_text_frame *frame,
    lib_u32 columns, lib_u32 rows, lib_u32 stride)
{
    lib_size bytes;
    if (frame == LIB_NULL || kvm_frame_extent(columns, rows, stride,
            sizeof(kvm_text_cell), &bytes) != LIB_STATUS_OK ||
        bytes > LIB_SIZE_MAX - sizeof(*frame->characters)) return LIB_STATUS_INVALID_ARGUMENT;
    if (frame->characters && frame->base.text_columns == columns &&
        frame->base.text_rows == rows && frame->base.stride == stride) return LIB_STATUS_OK;
    kvm_console_frame_release(frame);
    frame->characters = lib_allocate_zero(1u, sizeof(*frame->characters) + bytes);
    if (frame->characters == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    frame->base.text_columns = columns; frame->base.text_rows = rows;
    frame->base.stride = stride;
    frame->base.capacity = bytes / sizeof(kvm_text_cell);
    frame->base.cells = (kvm_text_cell *)(frame->characters + 1);
    return LIB_STATUS_OK;
}

static inline lib_status kvm_console_text_frame_validate(const kvm_console_text_frame *frame)
{
    lib_status status;
    if (frame == LIB_NULL || frame->characters == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = kvm_text_frame_validate(&frame->base);
    if (status != LIB_STATUS_OK) return status;
    for (lib_size i = 0u; i < 256u; ++i) {
        if ((frame->characters->primary[i] >= 0xd800u && frame->characters->primary[i] <= 0xdfffu) ||
            (frame->characters->secondary[i] >= 0xd800u && frame->characters->secondary[i] <= 0xdfffu))
            return LIB_STATUS_INVALID_ARGUMENT;
    }
    return LIB_STATUS_OK;
}

static inline lib_status kvm_console_frame_commit(kvm_frame_exchange *frames)
{
    lib_status status;
    if (frames == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = kvm_console_text_frame_validate(kvm_frame_exchange_writer(frames));
    if (status == LIB_STATUS_OK) kvm_frame_exchange_publish(frames);
    return status;
}

#endif
