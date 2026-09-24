#include "lib/types/types_interface.h"

#include "app-nxvm/machine/frame.h"

lib_status vm_machine_frame_from_display(
    const vm_machine_display_event *source, common_machine_frame *destination)
{
    static const lib_u16 cp437_controls[33u] = {
        0x0020u, 0x263au, 0x263bu, 0x2665u, 0x2666u, 0x2663u, 0x2660u,
        0x2022u, 0x25d8u, 0x25cbu, 0x25d9u, 0x2642u, 0x2640u, 0x266au,
        0x266bu, 0x263cu, 0x25bau, 0x25c4u, 0x2195u, 0x203cu, 0x00b6u,
        0x00a7u, 0x25acu, 0x21a8u, 0x2191u, 0x2193u, 0x2192u, 0x2190u,
        0x221fu, 0x2194u, 0x25b2u, 0x25bcu, 0x2302u
    };
    static const lib_u16 cp437_extended[128u] = {
        0x00c7u, 0x00fcu, 0x00e9u, 0x00e2u, 0x00e4u, 0x00e0u, 0x00e5u,
        0x00e7u, 0x00eau, 0x00ebu, 0x00e8u, 0x00efu, 0x00eeu, 0x00ecu,
        0x00c4u, 0x00c5u, 0x00c9u, 0x00e6u, 0x00c6u, 0x00f4u, 0x00f6u,
        0x00f2u, 0x00fbu, 0x00f9u, 0x00ffu, 0x00d6u, 0x00dcu, 0x00a2u,
        0x00a3u, 0x00a5u, 0x20a7u, 0x0192u, 0x00e1u, 0x00edu, 0x00f3u,
        0x00fau, 0x00f1u, 0x00d1u, 0x00aau, 0x00bau, 0x00bfu, 0x2310u,
        0x00acu, 0x00bdu, 0x00bcu, 0x00a1u, 0x00abu, 0x00bbu, 0x2591u,
        0x2592u, 0x2593u, 0x2502u, 0x2524u, 0x2561u, 0x2562u, 0x2556u,
        0x2555u, 0x2563u, 0x2551u, 0x2557u, 0x255du, 0x255cu, 0x255bu,
        0x2510u, 0x2514u, 0x2534u, 0x252cu, 0x251cu, 0x2500u, 0x253cu,
        0x255eu, 0x255fu, 0x255au, 0x2554u, 0x2569u, 0x2566u, 0x2560u,
        0x2550u, 0x256cu, 0x2567u, 0x2568u, 0x2564u, 0x2565u, 0x2559u,
        0x2558u, 0x2552u, 0x2553u, 0x256bu, 0x256au, 0x2518u, 0x250cu,
        0x2588u, 0x2584u, 0x258cu, 0x2590u, 0x2580u, 0x03b1u, 0x00dfu,
        0x0393u, 0x03c0u, 0x03a3u, 0x03c3u, 0x00b5u, 0x03c4u, 0x03a6u,
        0x0398u, 0x03a9u, 0x03b4u, 0x221eu, 0x03c6u, 0x03b5u, 0x2229u,
        0x2261u, 0x00b1u, 0x2265u, 0x2264u, 0x2320u, 0x2321u, 0x00f7u,
        0x2248u, 0x00b0u, 0x2219u, 0x00b7u, 0x221au, 0x207fu, 0x00b2u,
        0x25a0u, 0x00a0u
    };
    lib_size cell;

    if (source == LIB_NULL || destination == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (source->graphics && (source->pixel_width > KVM_WINDOW_GRAPHICS_MAX_WIDTH ||
        source->pixel_height > KVM_WINDOW_GRAPHICS_MAX_HEIGHT ||
        (lib_size)source->pixel_width * source->pixel_height > sizeof(source->pixels)))
        return LIB_STATUS_UNSUPPORTED;
    if (!source->graphics && (source->columns > KVM_TEXT_COLUMNS ||
        source->rows > KVM_TEXT_ROWS || source->text_cell_height == 0u ||
        source->text_cell_height > 32u)) return LIB_STATUS_UNSUPPORTED;
    *destination = (common_machine_frame){0};
    destination->window.valid = LIB_TRUE;
    destination->sequence = (lib_u32)source->generation;
    destination->window.graphics = source->graphics;
    if (destination->window.graphics) {
        destination->window.image.width = source->pixel_width;
        destination->window.image.height = source->pixel_height;
        destination->window.image.stride = source->pixel_width;
        lib_memory_copy(destination->window.image.pixels, source->pixels,
            (lib_size)source->pixel_width * source->pixel_height);
        lib_memory_copy(destination->window.image.palette, source->palette_rgb,
            sizeof(destination->window.image.palette));
        return LIB_STATUS_OK;
    }
    destination->window.text.base.text_columns = source->columns;
    destination->window.text.base.text_rows = source->rows;
    /* The CRTC cell height describes cursor raster coordinates. The copied
     * glyph asset remains 8x16, so preserve its rendering geometry and map
     * the CRTC interval into that one glyph coordinate system. */
    destination->window.text.base.font_height = 16u;
    destination->window.text.base.cursor_column = source->cursor_x;
    destination->window.text.base.cursor_row = source->cursor_y;
    destination->window.text.base.cursor_top = (lib_u8)((lib_u32)source->cursor_top *
        destination->window.text.base.font_height / source->text_cell_height);
    destination->window.text.base.cursor_bottom = (lib_u8)(
        (((lib_u32)source->cursor_bottom + 1u) * destination->window.text.base.font_height +
            source->text_cell_height - 1u) / source->text_cell_height - 1u);
    if (destination->window.text.base.cursor_top >= destination->window.text.base.font_height)
        destination->window.text.base.cursor_top = (lib_u8)(destination->window.text.base.font_height - 1u);
    if (destination->window.text.base.cursor_bottom >= destination->window.text.base.font_height)
        destination->window.text.base.cursor_bottom = (lib_u8)(destination->window.text.base.font_height - 1u);
    destination->window.text.base.cursor_visible = source->cursor_visible;
    destination->window.text.base.cursor_phase = source->cursor_visible;
    for (cell = 0u; cell < VM_MACHINE_EVENT_TEXT_CELLS; ++cell) {
        lib_u8 attribute = source->attributes[cell];
        destination->window.text.base.cells[cell] = (kvm_text_cell){
            source->characters[cell], 0u, attribute & 0x0fu, attribute >> 4u };
    }
    for (cell = 0u; cell < 256u; ++cell) {
        lib_u16 character = cell < 32u ? cp437_controls[cell] :
            cell == 127u ? cp437_controls[32u] :
            cell < 128u ? (lib_u16)cell : cp437_extended[cell - 128u];
        destination->characters.primary[cell] = character;
        destination->characters.secondary[cell] = character;
    }
    lib_memory_copy(destination->window.text.base.text_palette, source->palette_rgb,
        sizeof(destination->window.text.base.text_palette));
    if (source->glyphs_present) {
        lib_memory_copy(destination->window.text.font, source->glyphs,
            sizeof(destination->window.text.font));
    }
    return LIB_STATUS_OK;
}
