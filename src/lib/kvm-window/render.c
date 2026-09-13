#include "lib/kvm-window/render.h"

int kvm_window_frame_size(const kvm_frame *frame, lib_u32 *width, lib_u32 *height)
{
    if (!kvm_frame_is_valid(frame) || !width || !height) return 0;
    *width = frame->graphics ? frame->graphics_width : frame->text_columns * 8u;
    *height = frame->graphics ? frame->graphics_height : frame->text_rows *
        (frame->font_height != 0u && frame->font_height <= 16u ? frame->font_height : 16u);
    return 1;
}

void kvm_window_render_text(const kvm_frame *frame, lib_u32 *pixels, lib_u32 width, lib_u32 height)
{
    lib_u32 row;
    lib_u32 cell_height;

    if (!frame || !pixels || frame->graphics) return;
    cell_height = frame->font_height != 0u && frame->font_height <= 16u ? frame->font_height : 16u;
    if (width != frame->text_columns * 8u || height != frame->text_rows * cell_height) return;
    lib_memory_set(pixels, 0, (lib_size)width *
        height * sizeof(*pixels));
    for (row = 0u; row < frame->text_rows; ++row) {
        lib_u32 column;
        for (column = 0u; column < frame->text_columns; ++column) {
            lib_size index = (lib_size)row * KVM_TEXT_COLUMNS + column;
            lib_u8 character = frame->text[index];
            lib_u16 attribute = frame->attributes[index];
            lib_u32 scan;
            for (scan = 0u; scan < cell_height; ++scan) {
                const lib_u8 *font = frame->attribute_font_select != 0u &&
                    (attribute & 0x08u) != 0u ? frame->secondary_font : frame->font;
                lib_u8 bits = font[(lib_size)character * 16u + scan];
                lib_u32 *row_pixels = pixels +
                    ((lib_size)row * cell_height + scan) *
                    width + column * 8u;
                lib_u32 bit;
                for (bit = 0u; bit < 8u; ++bit)
                    row_pixels[bit] = frame->text_palette[
                        (bits & (0x80u >> bit)) != 0u ? attribute & 0x0fu :
                            (attribute >> 4) & 0x0fu];
            }
        }
    }
}

int kvm_window_render_graphics(const kvm_frame *frame, lib_u32 *pixels,
    lib_u32 width, lib_u32 height, lib_u32 *palette, int *valid, kvm_window_rect *changed)
{
    int full_refresh;
    lib_i32 left;
    lib_i32 top;
    lib_i32 right;
    lib_i32 bottom;
    lib_u32 row;

    if (!frame || !pixels || !changed || frame->graphics == 0u ||
        width != frame->graphics_width ||
        height != frame->graphics_height) return 0;
    full_refresh = !*valid || lib_memory_compare(palette,
        frame->graphics_palette, KVM_GRAPHICS_PALETTE_ENTRIES * sizeof(*palette)) != 0;
    left = full_refresh ? 0 : frame->dirty_left;
    top = full_refresh ? 0 : frame->dirty_top;
    right = full_refresh ? (lib_i32)frame->graphics_width - 1 : frame->dirty_right;
    bottom = full_refresh ? (lib_i32)frame->graphics_height - 1 : frame->dirty_bottom;
    if (left < 0) left = 0;
    if (top < 0) top = 0;
    if (right >= (lib_i32)frame->graphics_width) right = (lib_i32)frame->graphics_width - 1;
    if (bottom >= (lib_i32)frame->graphics_height) bottom = (lib_i32)frame->graphics_height - 1;
    if (right < left || bottom < top) return 0;
    for (row = (lib_u32)top; row <= (lib_u32)bottom; ++row) {
        const lib_u8 *source = frame->graphics_pixels + row * frame->graphics_stride;
        lib_u32 *destination = pixels + row * width;
        lib_u32 column;
        for (column = (lib_u32)left; column <= (lib_u32)right; ++column)
            destination[column] = frame->graphics_palette[source[column]];
    }
    lib_memory_copy(palette, frame->graphics_palette,
        KVM_GRAPHICS_PALETTE_ENTRIES * sizeof(*palette));
    *valid = 1;
    changed->left = left;
    changed->top = top;
    changed->right = right + 1;
    changed->bottom = bottom + 1;
    return 1;
}
