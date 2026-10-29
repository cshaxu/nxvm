#include "lib/types/types_interface.h"
#include "x86-nxvm/machine/frame_interface.h"

lib_i32 main(void)
{
    x86_video_snapshot source = {0};
    static kvm_window_frame_storage destination;
    static kvm_console_text_frame console;
    static lib_u8 before[sizeof(kvm_window_frame_storage)];
    const lib_size normalized_last = 255u * X86_VIDEO_TEXT_GLYPH_ROWS + 15u;

    source.kind = X86_VIDEO_KIND_TEXT;
    source.cursor_enabled = LIB_TRUE;
    source.columns = 80u;
    source.rows = 25u;
    source.text_cell_height = 8u;
    source.cursor_top = 6u;
    source.cursor_bottom = 7u;
    source.characters[0u] = 'A';
    source.attributes[0u] = 0x1eu;
    source.characters[1999u] = 'Z';
    source.attributes[1999u] = 0x4fu;
    source.palette_rgb[14u] = 0x00ffff00u;
    source.text_glyphs_present = LIB_TRUE;
    source.text_glyphs[normalized_last] = 0x5au;
    if (x86_nxvm_machine_frame_from_display(&source, 7u, &destination, &console) != LIB_STATUS_OK ||
        destination.frame.sequence != 7u ||
        destination.frame.graphics ||
        destination.data.text->base.cells[0u].glyph_index != 'A' ||
        destination.data.text->base.cells[0u].foreground != 0x0eu ||
        destination.data.text->base.cells[0u].background != 0x01u ||
        destination.data.text->base.cells[1999u].glyph_index != 'Z' ||
        destination.data.text->base.cells[1999u].foreground != 0x0fu ||
        destination.data.text->base.cells[1999u].background != 0x04u ||
        destination.data.text->base.text_palette[14u] != 0x00ffff00u ||
        destination.data.text->base.font_height != 16u ||
        destination.data.text->base.cursor[0].start != 12u ||
        destination.data.text->base.cursor[0].height != 4u ||
        console.characters->primary[0u] != 0x0020u ||
        console.characters->primary[219u] != 0x2588u ||
        destination.data.text->font[normalized_last] != 0x5au) return 1;

    source.rows = 43u;
    source.characters[3439u] = 'G';
    source.attributes[3439u] = 0x18u;
    source.text_font_selection = LIB_TRUE;
    source.text_foreground_mask = 7u;
    source.secondary_text_glyphs[normalized_last] = 0xa6u;
    if (x86_nxvm_machine_frame_from_display(&source, 8u, &destination, &console) != LIB_STATUS_OK ||
        destination.data.text->base.text_rows != 43u ||
        destination.data.text->base.cells[3439u].glyph_index != 'G' ||
        destination.data.text->base.cells[3439u].glyph_bank != 1u ||
        destination.data.text->base.cells[3439u].foreground != 0u ||
        destination.data.text->font[normalized_last] != 0x5au ||
        destination.data.text->secondary_font[normalized_last] != 0xa6u) return 1;
    source.text_font_selection = LIB_FALSE;
    source.text_foreground_mask = 15u;
    if (x86_nxvm_machine_frame_from_display(&source, 8u, &destination, &console) != LIB_STATUS_OK ||
        destination.data.text->base.cells[3439u].glyph_bank != 0u ||
        destination.data.text->base.cells[3439u].foreground != 8u) return 1;
    source.attributes[3439u] = 0x98u;
    source.text_foreground_mask = 7u;
    if (x86_nxvm_machine_frame_from_display(&source, 8u, &destination, &console) != LIB_STATUS_OK ||
        destination.data.text->base.cells[3439u].foreground != 0u ||
        destination.data.text->base.cells[3439u].background != 9u) return 1;
    source.text_foreground_mask = 0u;
    lib_memory_copy(before, &destination, sizeof(destination));
    source.text_foreground_mask = 8u;
    if (x86_nxvm_machine_frame_from_display(&source, 8u, &destination, &console) != LIB_STATUS_INVALID_ARGUMENT ||
        lib_memory_compare(before, &destination, sizeof(destination)) != 0) return 1;
    source.text_foreground_mask = 0u;

    {
        static const lib_u8 heights[] = {8u, 14u, 16u, 24u, 32u};
        const lib_size last = X86_VIDEO_TEXT_GLYPH_BYTES - 1u;
        source.text_cell_height = 32u;
        source.cursor_top = 30u;
        source.cursor_bottom = 31u;
        source.text_glyphs[last] = 0x80u;
        source.secondary_text_glyphs[last] = 0x40u;
        for (lib_size index = 0u; index < sizeof(heights); ++index) {
            source.text_glyph_height = heights[index];
            if (x86_nxvm_machine_frame_from_display(&source, 8u, &destination, &console) != LIB_STATUS_OK ||
                destination.data.text->base.font_height != heights[index] ||
                destination.data.text->font[last] != 0x80u ||
                destination.data.text->secondary_font[last] != 0x40u ||
                destination.data.text->base.cursor[0].start + destination.data.text->base.cursor[0].height != heights[index])
                return 1;
        }
        lib_memory_copy(before, &destination, sizeof(destination));
        source.text_glyph_height = 33u;
        if (x86_nxvm_machine_frame_from_display(&source, 8u, &destination, &console) != LIB_STATUS_UNSUPPORTED ||
            lib_memory_compare(before, &destination, sizeof(destination)) != 0) return 1;
        source.text_glyph_height = 0u;
        source.text_cell_height = 8u;
        source.cursor_top = 6u;
        source.cursor_bottom = 7u;
    }

    source.columns = 40u;
    source.rows = 2u;
    source.characters[40u] = 'X';
    source.attributes[40u] = 0xffu;
    source.characters[X86_VIDEO_MAX_COLUMNS] = 'B';
    source.attributes[X86_VIDEO_MAX_COLUMNS] = 0x1eu;
    if (x86_nxvm_machine_frame_from_display(&source, 8u, &destination, &console) != LIB_STATUS_OK ||
        destination.data.text->base.cells[40u].glyph_index != 0u ||
        destination.data.text->base.cells[40u].foreground != 0u ||
        destination.data.text->base.cells[X86_VIDEO_MAX_COLUMNS].glyph_index != 'B' ||
        destination.data.text->base.cells[X86_VIDEO_MAX_COLUMNS].foreground != 14u ||
        destination.data.text->base.cells[X86_VIDEO_MAX_COLUMNS].background != 1u) return 1;

    source.kind = X86_VIDEO_KIND_CGA_320X200X4;
    source.pixel_width = 320u;
    source.pixel_height = 200u;
    source.pixels[63999u] = 3u;
    source.palette_rgb[3u] = 0x00ff0000u;
    if (x86_nxvm_machine_frame_from_display(&source, 8u, &destination, &console) != LIB_STATUS_OK ||
        !destination.frame.graphics || destination.data.graphics->width != 320u ||
        destination.data.graphics->height != 200u ||
        destination.data.graphics->pixels[63999u] != 0x00ff0000u) return 1;
    lib_memory_copy(before, &destination, sizeof(destination));
    source.pixel_width = LIB_UINT16_MAX;
    if (x86_nxvm_machine_frame_from_display(&source, 9u, &destination, &console) != LIB_STATUS_UNSUPPORTED ||
        lib_memory_compare(before, &destination, sizeof(destination)) != 0) return 1;
    source.pixel_width = 0u;
    if (x86_nxvm_machine_frame_from_display(&source, 9u, &destination, &console) != LIB_STATUS_INVALID_ARGUMENT ||
        lib_memory_compare(before, &destination, sizeof(destination)) != 0) return 1;

    source.kind = X86_VIDEO_KIND_VGA_PLANAR16;
    source.pixel_width = 640u;
    source.pixel_height = 480u;
    source.pixels[0] = 15u;
    source.palette_rgb[15u] = 0x00ffffffu;
    if (x86_nxvm_machine_frame_from_display(&source, 9u, &destination, &console) != LIB_STATUS_OK ||
        !destination.frame.graphics || destination.data.graphics->width != 640u ||
        destination.data.graphics->height != 480u ||
        destination.data.graphics->pixels[0] != 0x00ffffffu) return 1;

    source.kind = X86_VIDEO_KIND_TEXT_PIXELS;
    source.columns = 0u;
    source.rows = 0u;
    source.text_cell_height = 0u;
    source.pixel_width = 720u;
    source.pixel_height = 512u;
    source.pixels[0] = 16u;
    source.palette_rgb[0] = 0xffffffu;
    source.palette_rgb[16] = 0u;
    source.pixels[720u * 512u - 1u] = 3u;
    if (x86_nxvm_machine_frame_from_display(&source, 9u, &destination, &console) != LIB_STATUS_OK ||
        kvm_window_frame_validate(&destination.frame) != LIB_STATUS_OK ||
        !destination.frame.graphics || destination.data.graphics->width != 720u ||
        destination.data.graphics->height != 512u || destination.data.graphics->stride != 720u ||
        destination.data.graphics->pixels[0] != 0u ||
        destination.data.graphics->pixels[1] != 0xffffffu ||
        destination.data.graphics->pixels[720u * 512u - 1u] != 0x00ff0000u) return 1;
    lib_memory_copy(before, &destination, sizeof(destination));
    source.pixel_height = 513u;
    if (x86_nxvm_machine_frame_from_display(&source, 10u, &destination, &console) != LIB_STATUS_UNSUPPORTED ||
        lib_memory_compare(before, &destination, sizeof(destination)) != 0) return 1;

    source = (x86_video_snapshot){0};
    source.kind = X86_VIDEO_KIND_TEXT;
    source.rows = 25u;
    source.text_cell_height = 8u;
    if (x86_nxvm_machine_frame_from_display(&source, 9u, &destination, &console) != LIB_STATUS_INVALID_ARGUMENT ||
        lib_memory_compare(before, &destination, sizeof(destination)) != 0) return 1;
    source.columns = 80u;
    source.rows = (lib_u16)(X86_VIDEO_MAX_ROWS + 1u);
    if (x86_nxvm_machine_frame_from_display(&source, 9u, &destination, &console) != LIB_STATUS_UNSUPPORTED ||
        lib_memory_compare(before, &destination, sizeof(destination)) != 0) return 1;
    source.rows = 25u;
    source.kind = (x86_video_kind)99;
    if (x86_nxvm_machine_frame_from_display(&source, 9u, &destination, &console) != LIB_STATUS_UNSUPPORTED ||
        lib_memory_compare(before, &destination, sizeof(destination)) != 0) return 1;

    source.kind = X86_VIDEO_KIND_TEXT;
    source.cursor_visible = LIB_TRUE;
    source.cursor_enabled = LIB_TRUE;
    source.cursor_top = 8u;
    source.cursor_bottom = 8u;
    if (x86_nxvm_machine_frame_from_display(&source, 10u, &destination, &console) != LIB_STATUS_OK ||
        destination.data.text->base.cursor_mode || destination.data.text->base.cursor_phase)
        return 1;
    source.cursor_top = 6u;
    source.cursor_bottom = 7u;
    source.cursor_x = 3u;
    source.cursor_y = 4u;
    for (lib_u32 phase = 0u; phase < 2u; ++phase) {
        source.cursor_visible = (lib_bool)phase;
        if (x86_nxvm_machine_frame_from_display(&source, 11u + phase, &destination, &console) != LIB_STATUS_OK ||
            destination.data.text->base.cursor_mode != KVM_CURSOR_SOURCE ||
            destination.data.text->base.cursor_phase != phase ||
            destination.data.text->base.cursor_column != 3 ||
            destination.data.text->base.cursor_row != 4) return 1;
    }
    return 0;
}
