#include "vm/platform/ux_frame.h"

type_status vm_platform_ux_frame_from_core(
    const core_platform_display_frame *source, ux_frame *destination)
{
    if (source == STD_NULL || destination == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    STD_MEMSET(destination, 0, sizeof(*destination));
    destination->valid = TYPE_TRUE;
    destination->sequence = (type_unsigned_32)source->generation;
    destination->graphics = source->kind == CORE_PLATFORM_DISPLAY_KIND_INDEXED_PIXELS;
    if (destination->graphics) {
        if (source->pixel_width > UX_GRAPHICS_MAX_WIDTH ||
            source->pixel_height > UX_GRAPHICS_MAX_HEIGHT) {
            return TYPE_STATUS_UNSUPPORTED;
        }
        destination->graphics_width = source->pixel_width;
        destination->graphics_height = source->pixel_height;
        STD_MEMCPY(destination->graphics_pixels, source->pixels,
            sizeof(source->pixels));
        STD_MEMCPY(destination->graphics_palette, source->palette_rgb,
            sizeof(source->palette_rgb));
    } else {
        destination->cursor_column = source->cursor_visible ? source->cursor_x : -1;
        destination->cursor_row = source->cursor_visible ? source->cursor_y : -1;
        destination->cursor_size = source->cursor_bottom >= source->cursor_top ?
            source->cursor_bottom - source->cursor_top + 1u : 0u;
        STD_MEMCPY(destination->text, source->characters, sizeof(source->characters));
        STD_MEMCPY(destination->attributes, source->attributes, sizeof(source->attributes));
        STD_MEMCPY(destination->text_palette, source->palette_rgb,
            sizeof(destination->text_palette));
        if (source->text_glyphs_present) {
            STD_MEMCPY(destination->font, source->text_glyphs,
                sizeof(destination->font));
            destination->font_height = 16u;
        }
    }
    return TYPE_STATUS_OK;
}
