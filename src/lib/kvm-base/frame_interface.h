#ifndef KVM_FRAME_INTERFACE_H
#define KVM_FRAME_INTERFACE_H

#include "lib/types/types_interface.h"

/* Shared text fields only. Capacity is 80x50, not a default visible size.
 * Each stored row occupies KVM_TEXT_COLUMNS cells,
 * even when fewer columns are visible. Palette entries are 0x00RRGGBB.
 * Glyph indices select resources supplied by the receiving leaf's frame.
 * Zero dimensions are invalid; nonzero extents beyond this fixed capacity
 * are unsupported. Off-surface/hidden cursors do not invalidate a frame.
 * font_height == 0 means 16 scanlines. Inclusive cursor_top/bottom are clipped
 * to that height; a start beyond it is hidden. bottom < top retains the full
 * cell fallback. Console approximates the visible height, not its position. */
#define KVM_TEXT_COLUMNS 80u
#define KVM_TEXT_ROWS 50u

typedef struct kvm_text_cell {
    lib_u8 glyph_index;
    lib_u8 glyph_bank; /* 0 primary, 1 secondary */
    lib_u8 foreground; /* palette index 0..15 */
    lib_u8 background; /* palette index 0..15 */
} kvm_text_cell;

_Static_assert(sizeof(kvm_text_cell) == 4u, "Text cells must have no padding");

typedef struct kvm_text_frame {
    lib_u16 text_columns;
    lib_u16 text_rows;
    lib_i32 cursor_column;
    lib_i32 cursor_row;
    lib_u8 cursor_top;
    lib_u8 cursor_bottom;
    lib_u8 cursor_visible;
    lib_u8 cursor_phase;
    kvm_text_cell cells[KVM_TEXT_COLUMNS * KVM_TEXT_ROWS];
    lib_u32 text_palette[16u]; /* 0x00RRGGBB */
    lib_u32 font_height;
} kvm_text_frame;

static inline lib_status kvm_text_frame_validate(const kvm_text_frame *frame)
{
    if (frame == LIB_NULL || frame->text_columns == 0u || frame->text_rows == 0u)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (frame->text_columns > KVM_TEXT_COLUMNS || frame->text_rows > KVM_TEXT_ROWS)
        return LIB_STATUS_UNSUPPORTED;
    for (lib_size row = 0u; row < frame->text_rows; ++row) {
        for (lib_size column = 0u; column < frame->text_columns; ++column) {
            const kvm_text_cell *cell = &frame->cells[row * KVM_TEXT_COLUMNS + column];
            if (cell->foreground > 15u || cell->background > 15u ||
                cell->glyph_bank > 1u) return LIB_STATUS_INVALID_ARGUMENT;
        }
    }
    return LIB_STATUS_OK;
}

#endif
