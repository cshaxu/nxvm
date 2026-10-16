#include "x86-nxvm/machine/machine_interface.h"
#ifndef TEST_VM_PRESENTATION_CAPTURE_H
#define TEST_VM_PRESENTATION_CAPTURE_H
#include "lib/types/types_interface.h"


#include "guest_display.h"
#include "x86-nxvm/machine/machine_private.h"
#include "x86-nxvm/machine/display.h"

/* Unbound owner tests refresh/inspect the sole cache without consuming driver
 * delivery. Bound observers read only the executor's published frame. */
static inline lib_status test_nxvm_machine_capture_presentation(const x86_nxvm_machine *machine,
    core_machine_guest_display_frame *out_frame)
{
    static emulator_machine_frame frame;
    lib_size cell;

    if (machine == LIB_NULL || out_frame == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (machine->executor != LIB_NULL) {
        /* The test reader cannot enter the bound executor's display driver. */
        if (!emulator_machine_copy_published_frame(machine->executor, &frame,
                emulator_machine_run_generation(machine->executor)))
            return LIB_STATUS_INVALID_STATE;
    } else if (x86_nxvm_machine_publish_display((x86_nxvm_machine *)machine, LIB_FALSE) !=
            LIB_STATUS_OK ||
        !emulator_machine_frame_copy(&frame, &machine->latest_frame))
        return LIB_STATUS_INVALID_STATE;
    lib_memory_set(out_frame, 0, sizeof(*out_frame));
    out_frame->generation = frame.sequence;
    if (frame.window.graphics) {
        out_frame->kind = CORE_MACHINE_GUEST_DISPLAY_KIND_INDEXED_PIXELS;
        out_frame->pixel_width = (lib_u16)frame.window.image.width;
        out_frame->pixel_height = (lib_u16)frame.window.image.height;
        lib_memory_copy(out_frame->pixels, frame.window.image.pixels,
            sizeof(out_frame->pixels));
        lib_memory_copy(out_frame->palette_rgb, frame.window.image.palette,
            sizeof(out_frame->palette_rgb));
        return LIB_STATUS_OK;
    }
    out_frame->kind = CORE_MACHINE_GUEST_DISPLAY_KIND_TEXT;
    out_frame->columns = (lib_u16)frame.window.text.base.text_columns;
    out_frame->rows = (lib_u16)frame.window.text.base.text_rows;
    out_frame->text_cell_height = (lib_u8)frame.window.text.base.font_height;
    out_frame->cursor_top = frame.window.text.base.cursor_top;
    out_frame->cursor_bottom = frame.window.text.base.cursor_bottom;
    out_frame->cursor_x = (lib_u16)frame.window.text.base.cursor_column;
    out_frame->cursor_y = (lib_u16)frame.window.text.base.cursor_row;
    out_frame->cursor_visible = frame.window.text.base.cursor_mode != KVM_CURSOR_DISABLED &&
        frame.window.text.base.cursor_phase;
    out_frame->text_glyphs_present = LIB_TRUE;
    for (cell = 0u; cell < CORE_MACHINE_GUEST_DISPLAY_MAX_COLUMNS *
            CORE_MACHINE_GUEST_DISPLAY_MAX_ROWS; ++cell) {
        out_frame->characters[cell] = (lib_u8)frame.window.text.base.cells[cell].glyph_index;
        out_frame->attributes[cell] = (lib_u8)(
            frame.window.text.base.cells[cell].foreground |
            (frame.window.text.base.cells[cell].background << 4u));
    }
    lib_memory_copy(out_frame->text_glyphs, frame.window.text.font,
        sizeof(out_frame->text_glyphs));
    lib_memory_copy(out_frame->palette_rgb, frame.window.text.base.text_palette,
        sizeof(out_frame->palette_rgb));
    return LIB_STATUS_OK;
}

#endif
