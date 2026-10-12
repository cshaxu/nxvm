#include "lib/types/types_interface.h"
#include "core/machine/frame_interface.h"

lib_i32 main(void)
{
    x86_video_snapshot source = {0};
    static emulator_machine_frame destination;
    static emulator_machine_frame before;
    const lib_size normalized_last = 255u * X86_VIDEO_TEXT_GLYPH_ROWS + 15u;

    source.kind = X86_VIDEO_KIND_TEXT;
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
    if (vm_machine_frame_from_display(&source, 7u, &destination) != LIB_STATUS_OK ||
        destination.sequence != 7u ||
        !destination.window.valid || destination.window.graphics ||
        destination.window.text.base.cells[0u].glyph_index != 'A' ||
        destination.window.text.base.cells[0u].foreground != 0x0eu ||
        destination.window.text.base.cells[0u].background != 0x01u ||
        destination.window.text.base.cells[1999u].glyph_index != 'Z' ||
        destination.window.text.base.cells[1999u].foreground != 0x0fu ||
        destination.window.text.base.cells[1999u].background != 0x04u ||
        destination.window.text.base.text_palette[14u] != 0x00ffff00u ||
        destination.window.text.base.font_height != 16u ||
        destination.window.text.base.cursor_top != 12u ||
        destination.window.text.base.cursor_bottom != 15u ||
        destination.characters.primary[0u] != 0x0020u ||
        destination.characters.primary[219u] != 0x2588u ||
        destination.window.text.font[normalized_last] != 0x5au) return 1;

    source.rows = 43u;
    source.characters[3439u] = 'G';
    source.attributes[3439u] = 0x18u;
    source.text_font_selection = LIB_TRUE;
    source.secondary_text_glyphs[normalized_last] = 0xa6u;
    if (vm_machine_frame_from_display(&source, 8u, &destination) != LIB_STATUS_OK ||
        destination.window.text.base.text_rows != 43u ||
        destination.window.text.base.cells[3439u].glyph_index != 'G' ||
        destination.window.text.base.cells[3439u].glyph_bank != 1u ||
        destination.window.text.base.cells[3440u].glyph_index != 0u ||
        destination.window.text.font[normalized_last] != 0x5au ||
        destination.window.text.secondary_font[normalized_last] != 0xa6u) return 1;
    source.text_font_selection = LIB_FALSE;
    if (vm_machine_frame_from_display(&source, 8u, &destination) != LIB_STATUS_OK ||
        destination.window.text.base.cells[3439u].glyph_bank != 0u) return 1;

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
            if (vm_machine_frame_from_display(&source, 8u, &destination) != LIB_STATUS_OK ||
                destination.window.text.base.font_height != heights[index] ||
                destination.window.text.font[last] != 0x80u ||
                destination.window.text.secondary_font[last] != 0x40u ||
                destination.window.text.base.cursor_bottom != heights[index] - 1u)
                return 1;
        }
        before = destination;
        source.text_glyph_height = 33u;
        if (vm_machine_frame_from_display(&source, 8u, &destination) != LIB_STATUS_UNSUPPORTED ||
            lib_memory_compare(&before, &destination, sizeof(destination)) != 0) return 1;
        source.text_glyph_height = 0u;
        source.text_cell_height = 8u;
        source.cursor_top = 6u;
        source.cursor_bottom = 7u;
    }

    source.columns = 40u;
    source.rows = 1u;
    source.characters[40u] = 'X';
    source.attributes[40u] = 0xffu;
    if (vm_machine_frame_from_display(&source, 8u, &destination) != LIB_STATUS_OK ||
        destination.window.text.base.cells[40u].glyph_index != 0u ||
        destination.window.text.base.cells[40u].foreground != 0u ||
        destination.window.text.base.cells[1999u].glyph_index != 0u) return 1;

    source.kind = X86_VIDEO_KIND_CGA_320X200X4;
    source.pixel_width = 320u;
    source.pixel_height = 200u;
    source.pixels[63999u] = 3u;
    source.palette_rgb[3u] = 0x00ff0000u;
    if (vm_machine_frame_from_display(&source, 8u, &destination) != LIB_STATUS_OK ||
        !destination.window.graphics || destination.window.image.width != 320u ||
        destination.window.image.height != 200u ||
        destination.window.image.pixels[63999u] != 3u ||
        destination.window.image.palette[3u] != 0x00ff0000u) return 1;
    before = destination;
    source.pixel_width = (lib_u16)(KVM_WINDOW_GRAPHICS_MAX_WIDTH + 1u);
    if (vm_machine_frame_from_display(&source, 9u, &destination) != LIB_STATUS_UNSUPPORTED ||
        lib_memory_compare(&before, &destination, sizeof(destination)) != 0) return 1;
    source.pixel_width = 0u;
    if (vm_machine_frame_from_display(&source, 9u, &destination) != LIB_STATUS_INVALID_ARGUMENT ||
        lib_memory_compare(&before, &destination, sizeof(destination)) != 0) return 1;

    source = (x86_video_snapshot){0};
    source.kind = X86_VIDEO_KIND_TEXT;
    source.rows = 25u;
    source.text_cell_height = 8u;
    if (vm_machine_frame_from_display(&source, 9u, &destination) != LIB_STATUS_INVALID_ARGUMENT ||
        lib_memory_compare(&before, &destination, sizeof(destination)) != 0) return 1;
    source.columns = 80u;
    source.rows = (lib_u16)(X86_VIDEO_MAX_ROWS + 1u);
    if (vm_machine_frame_from_display(&source, 9u, &destination) != LIB_STATUS_UNSUPPORTED ||
        lib_memory_compare(&before, &destination, sizeof(destination)) != 0) return 1;
    source.rows = 25u;
    source.kind = (x86_video_kind)99;
    if (vm_machine_frame_from_display(&source, 9u, &destination) != LIB_STATUS_UNSUPPORTED ||
        lib_memory_compare(&before, &destination, sizeof(destination)) != 0) return 1;

    source.kind = X86_VIDEO_KIND_TEXT;
    source.cursor_visible = LIB_TRUE;
    source.cursor_top = 8u;
    source.cursor_bottom = 8u;
    if (vm_machine_frame_from_display(&source, 10u, &destination) != LIB_STATUS_OK ||
        destination.window.text.base.cursor_visible || destination.window.text.base.cursor_phase)
        return 1;
    return 0;
}
