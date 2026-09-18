#ifndef KVM_FRAME_INTERFACE_H
#define KVM_FRAME_INTERFACE_H

#include "lib/types/types_interface.h"

/* Value-only ABI shared between a project runtime and the Win32 presenter.
 * It intentionally carries no renderer pointer, application-state pointer, or lock. */
/* Palette entries are platform-neutral 0x00RRGGBB values. They never carry
 * Win32 COLORREF byte order; a native presenter converts only at its boundary. */
#define KVM_TEXT_COLUMNS 80u
#define KVM_TEXT_ROWS 25u
#define KVM_GRAPHICS_MAX_WIDTH 1280u
#define KVM_GRAPHICS_MAX_HEIGHT 768u
#define KVM_GRAPHICS_MAX_PIXELS (KVM_GRAPHICS_MAX_WIDTH * KVM_GRAPHICS_MAX_HEIGHT)
#define KVM_GRAPHICS_PALETTE_ENTRIES 256u

typedef struct kvm_frame {
    lib_u32 sequence;
    lib_u32 graphics;
    lib_u32 valid;
    lib_u16 text_columns;
    lib_u16 text_rows;
    lib_i32 cursor_column;
    lib_i32 cursor_row;
    lib_u8 cursor_top;
    lib_u8 cursor_bottom;
    lib_u8 cursor_visible;
    lib_u8 cursor_phase;
    lib_u8 text[KVM_TEXT_COLUMNS * KVM_TEXT_ROWS];
    lib_u16 attributes[KVM_TEXT_COLUMNS * KVM_TEXT_ROWS];
    lib_u32 text_palette[16u]; /* 0x00RRGGBB */
    lib_u8 font[256u * 16u];
    lib_u8 secondary_font[256u * 16u];
    lib_u32 font_height;
    lib_u32 attribute_font_select;
    lib_u32 graphics_width;
    lib_u32 graphics_height;
    lib_u32 graphics_stride;
    lib_i32 dirty_left;
    lib_i32 dirty_top;
    lib_i32 dirty_right;
    lib_i32 dirty_bottom;
    lib_u32 graphics_palette[KVM_GRAPHICS_PALETTE_ENTRIES]; /* 0x00RRGGBB */
    lib_u8 graphics_pixels[KVM_GRAPHICS_MAX_PIXELS];
} kvm_frame;

static inline lib_bool kvm_frame_is_valid(const kvm_frame *frame)
{
    if (frame == LIB_NULL || frame->valid == 0u) return LIB_FALSE;
    if (frame->graphics != 0u) {
        return frame->graphics_width != 0u &&
            frame->graphics_width <= KVM_GRAPHICS_MAX_WIDTH &&
            frame->graphics_height != 0u &&
            frame->graphics_height <= KVM_GRAPHICS_MAX_HEIGHT &&
            frame->graphics_stride >= frame->graphics_width &&
            frame->graphics_stride <= KVM_GRAPHICS_MAX_WIDTH;
    }
    return frame->text_columns != 0u && frame->text_columns <= KVM_TEXT_COLUMNS &&
        frame->text_rows != 0u && frame->text_rows <= KVM_TEXT_ROWS;
}

/* Copies the entire metadata/text/font/palette prefix. Pixels are meaningful
 * only in graphics mode, within stride * height; the remaining capacity is
 * untouched. Invalid arguments leave destination unchanged. Self-copy is OK. */
static inline lib_bool kvm_frame_copy(kvm_frame *destination,
    const kvm_frame *source)
{
    lib_size bytes = lib_offsetof(kvm_frame, graphics_pixels);
    if (destination == LIB_NULL || !kvm_frame_is_valid(source)) return LIB_FALSE;
    if (destination == source) return LIB_TRUE;
    if (source->graphics != 0u)
        bytes += (lib_size)source->graphics_stride * source->graphics_height;
    lib_memory_copy(destination, source, bytes);
    return LIB_TRUE;
}

#endif
