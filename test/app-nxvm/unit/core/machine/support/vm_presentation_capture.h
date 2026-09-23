#ifndef TEST_VM_PRESENTATION_CAPTURE_H
#define TEST_VM_PRESENTATION_CAPTURE_H

#include "type.h"

#include "app-nxvm/devices/guest_display_frame.h"
#include "app-nxvm/machine/machine_private.h"

/* Test-only view of the one production Common frame. It never publishes or
 * stores a frame, so production has no second presentation route. */
static inline type_status test_vm_machine_capture_presentation(const vm_machine *machine,
    core_machine_guest_display_frame *out_frame)
{
    static common_machine_frame frame;
    STD_SIZE_T cell;

    if (machine == STD_NULL || out_frame == STD_NULL ||
        !vm_machine_copy_common_frame((vm_machine *)machine, &frame)) return TYPE_STATUS_INVALID_STATE;
    STD_MEMSET(out_frame, 0, sizeof(*out_frame));
    out_frame->generation = frame.sequence;
    if (frame.window.graphics) {
        out_frame->kind = CORE_MACHINE_GUEST_DISPLAY_KIND_INDEXED_PIXELS;
        out_frame->pixel_width = (type_unsigned_16)frame.window.image.width;
        out_frame->pixel_height = (type_unsigned_16)frame.window.image.height;
        STD_MEMCPY(out_frame->pixels, frame.window.image.pixels,
            sizeof(out_frame->pixels));
        STD_MEMCPY(out_frame->palette_rgb, frame.window.image.palette,
            sizeof(out_frame->palette_rgb));
        return TYPE_STATUS_OK;
    }
    out_frame->kind = CORE_MACHINE_GUEST_DISPLAY_KIND_TEXT;
    out_frame->columns = (type_unsigned_16)frame.window.text.base.text_columns;
    out_frame->rows = (type_unsigned_16)frame.window.text.base.text_rows;
    out_frame->text_cell_height = (type_unsigned_8)frame.window.text.base.font_height;
    out_frame->cursor_top = frame.window.text.base.cursor_top;
    out_frame->cursor_bottom = frame.window.text.base.cursor_bottom;
    out_frame->cursor_x = (type_unsigned_16)frame.window.text.base.cursor_column;
    out_frame->cursor_y = (type_unsigned_16)frame.window.text.base.cursor_row;
    out_frame->cursor_visible = frame.window.text.base.cursor_visible;
    out_frame->text_glyphs_present = TYPE_TRUE;
    for (cell = 0u; cell < CORE_MACHINE_GUEST_DISPLAY_MAX_COLUMNS *
            CORE_MACHINE_GUEST_DISPLAY_MAX_ROWS; ++cell) {
        out_frame->characters[cell] = (type_unsigned_8)frame.window.text.base.cells[cell].glyph_index;
        out_frame->attributes[cell] = (type_unsigned_8)(
            frame.window.text.base.cells[cell].foreground |
            (frame.window.text.base.cells[cell].background << 4u));
    }
    STD_MEMCPY(out_frame->text_glyphs, frame.window.text.font,
        sizeof(out_frame->text_glyphs));
    STD_MEMCPY(out_frame->palette_rgb, frame.window.text.base.text_palette,
        sizeof(out_frame->palette_rgb));
    return TYPE_STATUS_OK;
}

#endif
