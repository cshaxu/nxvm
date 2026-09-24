#include "lib/kvm-window/render.h"

lib_i32 kvm_window_frame_size(const kvm_window_frame *frame, lib_u32 *width, lib_u32 *height)
{
    if (kvm_window_frame_validate(frame) != LIB_STATUS_OK || !width || !height) return 0;
    *width = frame->graphics ? frame->image.width : frame->text.base.text_columns * 8u;
    *height = frame->graphics ? frame->image.height : frame->text.base.text_rows *
        (frame->text.base.font_height != 0u ? frame->text.base.font_height : KVM_WINDOW_FONT_HEIGHT);
    return 1;
}

static inline void kvm_window_update_pixel(lib_u32 *destination, lib_u32 colour,
    lib_u32 x, lib_u32 y, lib_i32 valid, kvm_window_rect *damage)
{
    if (valid && *destination == colour) return;
    *destination = colour;
    if ((lib_i32)x < damage->left) damage->left = (lib_i32)x;
    if ((lib_i32)y < damage->top) damage->top = (lib_i32)y;
    if ((lib_i32)x + 1 > damage->right) damage->right = (lib_i32)x + 1;
    if ((lib_i32)y + 1 > damage->bottom) damage->bottom = (lib_i32)y + 1;
}

lib_i32 kvm_window_render_frame(const kvm_window_frame *frame, lib_u32 *pixels,
    lib_u32 width, lib_u32 height, lib_i32 *valid, kvm_window_rect *changed)
{
    kvm_window_rect damage = {(lib_i32)width, (lib_i32)height, 0, 0};
    lib_u32 row;
    lib_u32 frame_width, frame_height, cell_height = KVM_WINDOW_FONT_HEIGHT;

    if (!pixels || !valid || !changed ||
        !kvm_window_frame_size(frame, &frame_width, &frame_height) ||
        width != frame_width || height != frame_height) return 0;
    if (!frame->graphics && frame->text.base.font_height)
        cell_height = frame->text.base.font_height;
    /* The surface is the rendered baseline, independent of skipped publications.
     * Compare resolved colours so palette changes need no separate cache. */
    for (row = 0u; row < height; ++row) {
        lib_u32 *destination = pixels + row * width;
        lib_u32 column;
        if (frame->graphics) {
            const lib_u8 *source = frame->image.pixels + row * frame->image.stride;
            for (column = 0u; column < width; ++column)
                kvm_window_update_pixel(&destination[column], frame->image.palette[source[column]],
                    column, row, *valid, &damage);
        } else {
            const kvm_window_text_frame *text = &frame->text;
            const kvm_text_cell *cells = text->base.cells + (row / cell_height) * KVM_TEXT_COLUMNS;
            for (column = 0u; column < text->base.text_columns; ++column) {
                const kvm_text_cell *cell = &cells[column];
                const lib_u8 *font = cell->glyph_bank ? text->secondary_font : text->font;
                lib_u8 bits = font[(lib_size)cell->glyph_index * KVM_WINDOW_FONT_HEIGHT + row % cell_height];
                lib_u32 bit;
                for (bit = 0u; bit < 8u; ++bit) {
                    lib_u32 x = column * 8u + bit;
                    lib_u32 colour = text->base.text_palette[
                        (bits & (0x80u >> bit)) ? cell->foreground : cell->background];
                    kvm_window_update_pixel(&destination[x], colour, x, row, *valid, &damage);
                }
            }
        }
    }
    *valid = 1;
    if (damage.right == 0) return 0;
    *changed = damage;
    return 1;
}
