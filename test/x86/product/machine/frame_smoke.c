#include "lib/types/types_interface.h"
#include <stdio.h>

#include "x86/product/machine/frame_interface.h"

lib_i32 main(void)
{
    x86_video_snapshot source = {0};
    static common_machine_frame destination;

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
    source.text_glyphs[4095u] = 0x5au;
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
        destination.window.text.font[4095u] != 0x5au) return 1;

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
    {
        common_machine_frame *before = lib_allocate(sizeof(*before));

        source.pixel_width = (lib_u16)(KVM_WINDOW_GRAPHICS_MAX_WIDTH + 1u);
        if (before == LIB_NULL) return 1;
        *before = destination;
        if (vm_machine_frame_from_display(&source, 9u, &destination) != LIB_STATUS_UNSUPPORTED ||
            lib_memory_compare(before, &destination, sizeof(destination)) != 0) return 1;
        lib_release(before);
    }
    return 0;
}
