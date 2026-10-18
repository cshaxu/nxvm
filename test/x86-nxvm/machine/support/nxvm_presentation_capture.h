#include "x86-nxvm/machine/machine_interface.h"
#ifndef TEST_VM_PRESENTATION_CAPTURE_H
#define TEST_VM_PRESENTATION_CAPTURE_H
#include "lib/types/types_interface.h"


#include "guest_display.h"
#include "x86-nxvm/machine/machine_private.h"
#include "x86-nxvm/machine/display.h"
#include "x86-nxvm/machine/frame_interface.h"

/* Unbound owner tests refresh/inspect the guest snapshot without consuming driver
 * delivery. Bound observers read only the executor's published frame. */
static inline lib_status test_nxvm_machine_capture_presentation(const x86_nxvm_machine *machine,
    x86_nxvm_test_guest_display_frame *out_frame)
{
    static kvm_window_frame_storage frame;
    static kvm_console_text_frame console;
    lib_size cell;

    if (machine == LIB_NULL || out_frame == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (machine->executor != LIB_NULL) {
        /* The test reader cannot enter the bound executor's display driver. */
        if (!emulator_machine_copy_published_frame(machine->executor, &frame,
                emulator_machine_run_generation(machine->executor)))
            return LIB_STATUS_INVALID_STATE;
    } else if (x86_nxvm_machine_publish_display((x86_nxvm_machine *)machine, LIB_FALSE) !=
            LIB_STATUS_OK ||
        x86_nxvm_machine_frame_from_display(&machine->latest_snapshot,
            machine->display_generation, &frame, &console) != LIB_STATUS_OK)
        return LIB_STATUS_INVALID_STATE;
    lib_memory_set(out_frame, 0, sizeof(*out_frame));
    out_frame->generation = frame.frame.sequence;
    if (frame.frame.graphics) {
        out_frame->kind = X86_NXVM_TEST_GUEST_DISPLAY_KIND_INDEXED_PIXELS;
        out_frame->pixel_width = (lib_u16)frame.data.graphics.width;
        out_frame->pixel_height = (lib_u16)frame.data.graphics.height;
        if (machine->executor == LIB_NULL) {
            lib_memory_copy(out_frame->pixels, machine->latest_snapshot.pixels,
                sizeof(out_frame->pixels));
            lib_memory_copy(out_frame->palette_rgb, machine->latest_snapshot.palette_rgb,
                sizeof(out_frame->palette_rgb));
        } else {
            lib_size colours = 0u;
            for (cell = 0u; cell < (lib_size)out_frame->pixel_width * out_frame->pixel_height; ++cell) {
                lib_u32 rgb = frame.data.graphics.pixels[cell];
                lib_size index;
                for (index = 0u; index < colours && out_frame->palette_rgb[index] != rgb; ++index) {}
                if (index == colours) {
                    if (colours == X86_NXVM_TEST_GUEST_DISPLAY_PALETTE_ENTRIES)
                        return LIB_STATUS_UNSUPPORTED;
                    out_frame->palette_rgb[colours++] = rgb;
                }
                out_frame->pixels[cell] = (lib_u8)index;
            }
        }
        return LIB_STATUS_OK;
    }
    out_frame->kind = X86_NXVM_TEST_GUEST_DISPLAY_KIND_TEXT;
    out_frame->columns = (lib_u16)frame.data.text.base.text_columns;
    out_frame->rows = (lib_u16)frame.data.text.base.text_rows;
    out_frame->text_cell_height = (lib_u8)frame.data.text.base.font_height;
    out_frame->cursor_top = frame.data.text.base.cursor_top;
    out_frame->cursor_bottom = frame.data.text.base.cursor_bottom;
    out_frame->cursor_x = (lib_u16)frame.data.text.base.cursor_column;
    out_frame->cursor_y = (lib_u16)frame.data.text.base.cursor_row;
    out_frame->cursor_visible = frame.data.text.base.cursor_mode != KVM_CURSOR_DISABLED &&
        frame.data.text.base.cursor_phase;
    out_frame->text_glyphs_present = LIB_TRUE;
    for (cell = 0u; cell < X86_NXVM_TEST_GUEST_DISPLAY_MAX_COLUMNS *
            X86_NXVM_TEST_GUEST_DISPLAY_MAX_ROWS; ++cell) {
        out_frame->characters[cell] = (lib_u8)frame.data.text.base.cells[cell].glyph_index;
        out_frame->attributes[cell] = (lib_u8)(
            frame.data.text.base.cells[cell].foreground |
            (frame.data.text.base.cells[cell].background << 4u));
    }
    lib_memory_copy(out_frame->text_glyphs, frame.data.text.font,
        sizeof(out_frame->text_glyphs));
    lib_memory_copy(out_frame->palette_rgb, frame.data.text.base.text_palette,
        sizeof(out_frame->palette_rgb));
    return LIB_STATUS_OK;
}

#endif
