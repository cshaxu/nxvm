#ifndef KVM_CONSOLE_FRAME_INTERFACE_H
#define KVM_CONSOLE_FRAME_INTERFACE_H

#include "lib/kvm-base/frame_interface.h"

/* One BMP character (excluding surrogate code units) per glyph index. Both banks are copied frame data, not
 * resources retained from a caller. Each cell's glyph_bank selects the map. */
typedef struct kvm_console_character_map {
    lib_u16 primary[256u];
    lib_u16 secondary[256u];
} kvm_console_character_map;

typedef struct kvm_console_text_frame {
    kvm_text_frame base;
    kvm_console_character_map characters;
} kvm_console_text_frame;

static inline lib_status kvm_console_text_frame_validate(const kvm_console_text_frame *frame)
{
    lib_status status;
    if (frame == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = kvm_text_frame_validate(&frame->base);
    if (status != LIB_STATUS_OK) return status;
    for (lib_size i = 0u; i < 256u; ++i) {
        if ((frame->characters.primary[i] >= 0xd800u && frame->characters.primary[i] <= 0xdfffu) ||
            (frame->characters.secondary[i] >= 0xd800u && frame->characters.secondary[i] <= 0xdfffu))
            return LIB_STATUS_INVALID_ARGUMENT;
    }
    return LIB_STATUS_OK;
}

#endif
