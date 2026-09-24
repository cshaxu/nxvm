#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/machine/frame.h"

lib_i32 main(void)
{
    vm_machine_display_event source = {0};
    static common_machine_frame destination;

    source.graphics = LIB_FALSE;
    source.generation = 7u;
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
    if (vm_machine_frame_from_display(&source, &destination) != LIB_STATUS_OK ||
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
        destination.characters.primary[219u] != 0x2588u) return 1;

    source.graphics = LIB_TRUE;
    source.pixel_width = 320u;
    source.pixel_height = 200u;
    if (vm_machine_frame_from_display(&source, &destination) != LIB_STATUS_OK ||
        !destination.window.graphics || destination.window.image.width != 320u ||
        destination.window.image.height != 200u) return 1;
    {
        common_machine_frame *before = lib_allocate(sizeof(*before));

        source.pixel_width = (lib_u16)(KVM_WINDOW_GRAPHICS_MAX_WIDTH + 1u);
        if (before == LIB_NULL) return 1;
        *before = destination;
        if (vm_machine_frame_from_display(&source, &destination) != LIB_STATUS_UNSUPPORTED ||
            lib_memory_compare(before, &destination, sizeof(destination)) != 0) return 1;
        lib_release(before);
    }
    return 0;
}
