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
        destination->graphics_stride = source->pixel_width;
        STD_MEMCPY(destination->graphics_pixels, source->pixels,
            sizeof(source->pixels));
        STD_MEMCPY(destination->graphics_palette, source->palette_rgb,
            sizeof(source->palette_rgb));
    } else {
        if (source->columns > UX_TEXT_COLUMNS || source->rows > UX_TEXT_ROWS)
            return TYPE_STATUS_UNSUPPORTED;
        destination->text_columns = source->columns;
        destination->text_rows = source->rows;
        destination->cursor_column = source->cursor_x;
        destination->cursor_row = source->cursor_y;
        destination->cursor_top = source->cursor_top;
        destination->cursor_bottom = source->cursor_bottom;
        destination->cursor_visible = source->cursor_visible;
        destination->cursor_phase = source->cursor_visible;
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
