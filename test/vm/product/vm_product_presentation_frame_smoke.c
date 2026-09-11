#include "type.h"

#include "vm/product/presentation_frame.h"

C_INT main(C_VOID)
{
    vm_machine_display_event source = {0};
    ui_frame destination = {0};

    source.graphics = TYPE_FALSE;
    source.generation = 7u;
    source.columns = 80u;
    source.rows = 25u;
    source.characters[0u] = 'A';
    source.attributes[0u] = 0x1eu;
    source.characters[1999u] = 'Z';
    source.attributes[1999u] = 0x4fu;
    source.palette_rgb[14u] = 0x00ffff00u;
    if (vm_product_presentation_frame_from_core(&source, &destination) != TYPE_STATUS_OK ||
        !destination.valid || destination.graphics || destination.text[0u] != 'A' ||
        destination.attributes[0u] != 0x1eu || destination.text[1999u] != 'Z' ||
        destination.attributes[1999u] != 0x4fu ||
        destination.text_palette[14u] != 0x00ffff00u) return 1;

    source.graphics = TYPE_TRUE;
    source.pixel_width = 320u;
    source.pixel_height = 200u;
    if (vm_product_presentation_frame_from_core(&source, &destination) != TYPE_STATUS_OK ||
        !destination.graphics || destination.graphics_width != 320u ||
        destination.graphics_height != 200u) return 1;
    return 0;
}
