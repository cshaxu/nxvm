#include "../fixtures/window_frame.h"
static kvm_frame_exchange *frames;
#include "lib/types/test.h"
#include "lib/kvm-window/window.h"
#include "lib/kvm-window/motion.h"
#include "lib/kvm-window/geometry.h"
#include "lib/kvm-window/render.h"

static kvm_window window;
static kvm_window_frame_storage frame;
static const kvm_window_frame *received;

static void observe_expected_frame(void *opaque, const void *value)
{ *(lib_bool *)opaque = value == received; }

static void damage(void)
{
    const kvm_window_frame *old;
    lib_u32 surface[16] = {0};
    kvm_window_rect changed;
    lib_bool valid = LIB_FALSE;
    lib_test_assert(kvm_component_mailboxes_create(&window.base.mailboxes) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_mailboxes_select_notify(&window.base.mailboxes, LIB_NULL, LIB_NULL) == LIB_STATUS_OK);
    lib_test_assert(kvm_window_frame_prepare_graphics(&frame, 4u, 4u, 4u) == LIB_STATUS_OK);
    frame.data.graphics->width = frame.data.graphics->stride = 4u;
    frame.data.graphics->height = 4u;
    lib_test_assert(test_window_frame(&window, frames, &frame.frame) == LIB_STATUS_OK);
    received = kvm_component_mailboxes_take_frame(&window.base.mailboxes, LIB_NULL);
    lib_test_assert(received != LIB_NULL);
    /* First all-black frame still invalidates the entire surface. */
    lib_test_assert(kvm_window_render_frame(received, surface, 4, 4, &valid, &changed));
    lib_test_assert(changed.left == 0 && changed.top == 0 && changed.right == 4 && changed.bottom == 4);
    {
        const kvm_window_frame *pending = received;
        lib_test_assert(base_sync_event_wait(window.base.mailboxes.wake, 0u) == BASE_SYNC_WAIT_SIGNALED);
        frame.data.graphics->height = 5u;
        lib_test_assert(test_window_frame(&window, frames, &frame.frame) == LIB_STATUS_INVALID_ARGUMENT);
        frame.data.graphics->height = 4u; frame.data.graphics->stride = 3u;
        lib_test_assert(test_window_frame(&window, frames, &frame.frame) == LIB_STATUS_INVALID_ARGUMENT);
        kvm_window_frame_release(&frame);
        lib_test_assert(kvm_window_frame_prepare_text(&frame, 80u, 50u, 80u) == LIB_STATUS_OK);
        frame.data.text->base.text_columns = 80u; frame.data.text->base.text_rows = 25u;
        frame.data.text->base.font_height = KVM_WINDOW_FONT_HEIGHT + 1u;
        lib_test_assert(test_window_frame(&window, frames, &frame.frame) == LIB_STATUS_UNSUPPORTED);
        lib_test_assert(base_sync_event_wait(window.base.mailboxes.wake, 0u) == BASE_SYNC_WAIT_TIMED_OUT);
        lib_test_assert(received == pending && received->graphics && received->graphics->height == 4u);
        lib_test_assert(kvm_window_frame_copy(&frame, received));
    }
    kvm_component_mailboxes_finish_frame(&window.base.mailboxes, LIB_TRUE);
    /* A is overwritten by B before consumption; B contains both changes. */
    frame.data.graphics->pixels[5] = 0x112233u;
    lib_test_assert(test_window_frame(&window, frames, &frame.frame) == LIB_STATUS_OK);
    frame.data.graphics->pixels[10] = 0x445566u;
    lib_test_assert(test_window_frame(&window, frames, &frame.frame) == LIB_STATUS_OK);
    received = kvm_component_mailboxes_take_frame(&window.base.mailboxes, LIB_NULL);
    lib_test_assert(received != LIB_NULL);
    lib_test_assert(kvm_window_render_frame(received, surface, 4, 4, &valid, &changed));
    lib_test_assert(changed.left == 1 && changed.top == 1 && changed.right == 3 && changed.bottom == 3);
    lib_test_assert(surface[5] == 0x112233u && surface[10] == 0x445566u);
    lib_test_assert(!kvm_window_render_frame(received, surface, 4, 4, &valid, &changed));
    /* Failed output keeps its capture; old acknowledgement cannot erase new data. */
    old = received;
    frame.data.graphics->pixels[0] = 0x445566u;
    lib_test_assert(test_window_frame(&window, frames, &frame.frame) == LIB_STATUS_OK);
    kvm_component_mailboxes_finish_frame(&window.base.mailboxes, LIB_TRUE);
    received = kvm_component_mailboxes_take_frame(&window.base.mailboxes, LIB_NULL);
    lib_test_assert(received != LIB_NULL);
    lib_test_assert(received != old);
    lib_test_assert(kvm_window_render_frame(received, surface, 4, 4, &valid, &changed));
    lib_test_assert(changed.left == 0 && changed.top == 0 && changed.right == 1 && changed.bottom == 1);
    kvm_component_mailboxes_finish_frame(&window.base.mailboxes, LIB_TRUE);
    lib_test_assert(kvm_component_mailboxes_take_frame(&window.base.mailboxes, LIB_NULL) == LIB_NULL);
    /* Only the changed RGB pixel damages the surface. */
    frame.data.graphics->pixels[5] = 0xffu;
    lib_test_assert(kvm_window_render_frame(&frame.frame, surface, 4, 4, &valid, &changed));
    lib_test_assert(surface[5] == 0xffu && changed.left == 1 && changed.top == 1 &&
        changed.right == 2 && changed.bottom == 2);
    /* Republishing the same colour does not damage the surface. */
    frame.data.graphics->pixels[0] = 0x445566u;
    lib_test_assert(!kvm_window_render_frame(&frame.frame, surface, 4, 4, &valid, &changed));
    /* Row padding is not part of the displayed pixels. */
    frame.data.graphics->width = 3;
    valid = LIB_FALSE;
    lib_test_assert(kvm_window_render_frame(&frame.frame, surface, 3, 4, &valid, &changed));
    lib_test_assert(changed.right == 3 && changed.bottom == 4);
    frame.data.graphics->pixels[3] = 255;
    lib_test_assert(!kvm_window_render_frame(&frame.frame, surface, 3, 4, &valid, &changed));
    /* Recreated surfaces require full invalidation even for unchanged pixels. */
    valid = LIB_FALSE;
    lib_test_assert(kvm_window_render_frame(&frame.frame, surface, 3, 4, &valid, &changed));
    lib_test_assert(changed.left == 0 && changed.top == 0 && changed.right == 3 && changed.bottom == 4);
    kvm_component_mailboxes_destroy(&window.base.mailboxes);
}

static void rgb_coverage(void)
{
    lib_u32 surface[2048];
    lib_bool valid = LIB_FALSE;
    kvm_window_rect changed;
    kvm_window_frame_release(&frame);
    lib_test_assert(kvm_window_frame_prepare_graphics(&frame, 64u, 32u, 65u) == LIB_STATUS_OK);
    frame.data.graphics->width = 64u; frame.data.graphics->height = 32u; frame.data.graphics->stride = 65u;
    for (lib_u32 row = 0u; row < 32u; ++row) {
        for (lib_u32 col = 0u; col < 64u; ++col)
            frame.data.graphics->pixels[row * 65u + col] = 0x123400u + row * 64u + col;
        frame.data.graphics->pixels[row * 65u + 64u] = 0xffffffffu; /* Padding is not a colour. */
    }
    lib_test_assert(kvm_component_mailboxes_create(&window.base.mailboxes) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_mailboxes_select_notify(&window.base.mailboxes, LIB_NULL, LIB_NULL) == LIB_STATUS_OK);
    lib_test_assert(test_window_frame(&window, frames, &frame.frame) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_wait(window.base.mailboxes.wake, 0u) == BASE_SYNC_WAIT_SIGNALED);
    received = kvm_component_mailboxes_take_frame(&window.base.mailboxes, LIB_NULL);
    lib_test_assert(received != LIB_NULL);
    lib_test_assert(kvm_window_render_frame(received, surface, 64, 32, &valid, &changed));
    for (lib_u32 i = 0u; i < 2048u; ++i) lib_test_assert(surface[i] == 0x123400u + i);
    lib_test_assert(!kvm_window_render_frame(received, surface, 64, 32, &valid, &changed));
    frame.data.graphics->pixels[31u * 65u + 63u] = 0xff123456u;
    lib_test_assert(test_window_frame(&window, frames, &frame.frame) == LIB_STATUS_INVALID_ARGUMENT);
    {
        lib_bool matched = LIB_FALSE;
        lib_test_assert(kvm_frame_exchange_observe_latest(frames,
            observe_expected_frame, &matched) && matched);
    }
    lib_test_assert(base_sync_event_wait(window.base.mailboxes.wake, 0u) == BASE_SYNC_WAIT_TIMED_OUT);
    static kvm_window_frame_storage rejected_copy;
    lib_test_assert(!kvm_window_frame_copy(&rejected_copy, &frame.frame));
    lib_test_assert(received->graphics->pixels[31u * 65u + 63u] == 0x123bffu);
    frame.data.graphics->pixels[31u * 65u + 63u] = 0xabcdefu;
    frame.data.graphics->pixels[31u * 65u] = 0xfedcbau;
    lib_test_assert(kvm_window_render_frame(&frame.frame, surface, 64, 32, &valid, &changed));
    lib_test_assert(changed.left == 0 && changed.right == 64 && changed.top == 31 && changed.bottom == 32);
    lib_test_assert(surface[1984] == 0xfedcbau && surface[2047] == 0xabcdefu);
    kvm_component_mailboxes_destroy(&window.base.mailboxes);
}

static void motion(void)
{
    kvm_window_motion mouse = {0};
    lib_i32 dx, dy, total_x = 0, total_y = 0;
    lib_test_assert(kvm_window_motion_move(&mouse, 0, 0, 200, 200, 100, 100, &dx, &dy));
    for (lib_i32 i = 1; i <= 10; ++i) {
        lib_test_assert(kvm_window_motion_move(&mouse, i, i, 200, 200, 100, 100, &dx, &dy));
        total_x += dx; total_y += dy;
    }
    lib_test_assert(total_x == 5 && total_y == 5);
    for (lib_i32 i = 9; i >= 0; --i) {
        lib_test_assert(kvm_window_motion_move(&mouse, i, i, 200, 200, 100, 100, &dx, &dy));
        total_x += dx; total_y += dy;
    }
    lib_test_assert(total_x == 0 && total_y == 0);
    lib_test_assert(kvm_window_motion_move(&mouse, 1, 1, 200, 200, 100, 100, &dx, &dy));
    lib_test_assert(dx == 0 && dy == 0);
    lib_test_assert(kvm_window_motion_move(&mouse, 0, 0, 200, 200, 100, 100, &dx, &dy));
    lib_test_assert(dx == 0 && dy == 0 && mouse.remainder_x == 0 && mouse.remainder_y == 0);
    lib_test_assert(kvm_window_motion_move(&mouse, 1, 1, 200, 200, 100, 100, &dx, &dy));
    lib_test_assert(kvm_window_motion_move(&mouse, 2, 2, 100, 100, 100, 100, &dx, &dy));
    lib_test_assert(dx == 1 && dy == 1 && mouse.remainder_x == 0 && mouse.remainder_y == 0);
}

static void rendering(void)
{
    static kvm_window_frame_storage text;
    kvm_window_rect display = { 0, 0, 640, 410 }, cursor;
    lib_u32 pixels[8 * 16];
    lib_bool valid = LIB_FALSE;
    kvm_window_rect changed;
    lib_test_assert(kvm_window_frame_prepare_text(&text, 80u, 50u, 80u) == LIB_STATUS_OK); text.data.text->base.text_columns = 80; text.data.text->base.text_rows = 25;
    text.data.text->base.cursor_mode = KVM_CURSOR_HOST; text.data.text->base.cursor_column = 0; text.data.text->base.cursor_row = 24;
    text.data.text->base.font_height = 16; text.data.text->base.cursor[0] = (kvm_text_cursor_span){ 14, 2u };
    lib_test_assert(kvm_window_cursor_rect(&text.frame, &display, &cursor));
    lib_test_assert(cursor.bottom == 410 && cursor.top == 407);
    text.data.text->base.font_height = 0;
    lib_test_assert(kvm_window_cursor_rect(&text.frame, &display, &cursor));
    lib_test_assert(cursor.bottom == 410 && cursor.top == 407);
    text.data.text->base.font_height = 16;
    for (lib_i32 row = 0; row < 25; ++row) {
        text.data.text->base.cursor_row = row;
        lib_test_assert(kvm_window_cursor_rect(&text.frame, &display, &cursor));
        lib_test_assert(cursor.top >= row * 410 / 25);
        lib_test_assert(cursor.bottom == (row + 1) * 410 / 25);
        lib_test_assert(cursor.bottom - cursor.top <= 3);
    }
    text.data.text->base.text_columns = text.data.text->base.text_rows = 1;
    text.data.text->base.cursor_row = 0;
    display = (kvm_window_rect){10,20,18,36};
    for (lib_u32 top=0;top<16;++top) {
        text.data.text->base.cursor[0] = (kvm_text_cursor_span){ (lib_u8)top, 1u };
        lib_test_assert(kvm_window_cursor_rect(&text.frame,&display,&cursor));
        lib_test_assert(cursor.top==20+(lib_i32)top && cursor.bottom==21+(lib_i32)top);
    }
    text.data.text->base.cursor[0] = (kvm_text_cursor_span){ 4, 4u };
    display.bottom=52;
    lib_test_assert(kvm_window_cursor_rect(&text.frame,&display,&cursor));
    lib_test_assert(cursor.top==28 && cursor.bottom==36);
    text.data.text->base.cursor[0] = (kvm_text_cursor_span){ 15, 241u };
    lib_test_assert(kvm_window_cursor_rect(&text.frame,&display,&cursor) && cursor.bottom==52);
    text.data.text->base.cursor[0].start=16;
    lib_test_assert(!kvm_window_cursor_rect(&text.frame,&display,&cursor));
    text.data.text->base.cursor[0] = (kvm_text_cursor_span){ 0u, LIB_UINT32_MAX };
    lib_test_assert(kvm_window_cursor_rect(&text.frame,&display,&cursor) && cursor.top==20 && cursor.bottom==52);
    text.data.text->base.font_height=0;
    lib_test_assert(kvm_window_cursor_rect(&text.frame,&display,&cursor) && cursor.top==20 && cursor.bottom==52);
    text.data.text->font[0] = 0x80;
    text.data.text->base.cells[0].foreground = 1; text.data.text->base.cells[0].background = 2;
    text.data.text->base.text_palette[1] = 0x112233; text.data.text->base.text_palette[2] = 0x445566;
    kvm_window_render_frame(&text.frame, pixels, 8, 16, &valid, &changed);
    lib_test_assert(pixels[0] == 0x112233 && pixels[1] == 0x445566 && pixels[127] == 0x445566);
    text.data.text->font[0] = 0x40;
    kvm_window_render_frame(&text.frame, pixels, 8, 16, &valid, &changed);
    lib_test_assert(pixels[0] == 0x445566 && pixels[1] == 0x112233);
    text.data.text->base.cells[0].glyph_bank = 1u;
    text.data.text->base.cells[0].foreground = 9u;
    text.data.text->base.text_palette[9] = 0x112233;
    text.data.text->secondary_font[0] = 0x20;
    kvm_window_render_frame(&text.frame, pixels, 8, 16, &valid, &changed);
    lib_test_assert(pixels[0] == 0x445566 && pixels[2] == 0x112233);
    /* Compare all original byte attributes, including intensity and bank coupling. */
    for (lib_u32 colour = 0; colour < 16; ++colour)
        text.data.text->base.text_palette[colour] = colour * 0x010101u;
    for (lib_u32 enabled = 0; enabled < 2; ++enabled) {
        for (lib_u32 attribute = 0; attribute < 256; ++attribute) {
            lib_u32 bits = enabled && (attribute & 8u) ? 0x20u : 0x40u;
            text.data.text->base.cells[0].foreground = attribute & 15u;
            text.data.text->base.cells[0].background = attribute >> 4;
            text.data.text->base.cells[0].glyph_bank = enabled && (attribute & 8u);
            kvm_window_render_frame(&text.frame, pixels, 8, 16, &valid, &changed);
            for (lib_u32 x = 0; x < 8; ++x)
                lib_test_assert(pixels[x] == ((bits & (0x80u >> x)) ?
                    (attribute & 15u) : (attribute >> 4)) * 0x010101u);
        }
    }
    /* Neutral producers may select bank 1 without colour bit 3. */
    text.data.text->base.cells[0].foreground = 1u;
    text.data.text->base.cells[0].glyph_bank = 1u;
    kvm_window_render_frame(&text.frame, pixels, 8, 16, &valid, &changed);
    lib_test_assert(pixels[2] == 0x010101u);
    /* Decoration changes alone damage old and new scanlines; glyph data stays intact. */
    text.data.text->base.cells[0].underline = 1u;
    text.data.text->base.underline_row = 12u;
    lib_test_assert(kvm_window_render_frame(&text.frame, pixels, 8, 16, &valid, &changed));
    for (lib_u32 x = 0; x < 8u; ++x) lib_test_assert(pixels[12u * 8u + x] == 0x010101u);
    text.data.text->base.underline_row = 13u;
    lib_test_assert(kvm_window_render_frame(&text.frame, pixels, 8, 16, &valid, &changed));
    lib_test_assert(changed.top == 12 && changed.bottom == 14);
    text.data.text->base.underline_row = 16u;
    lib_test_assert(kvm_window_render_frame(&text.frame, pixels, 8, 16, &valid, &changed));
    lib_test_assert(changed.top == 13 && changed.bottom == 14);
    text.data.text->base.cells[0].underline = 0u;
    lib_test_assert(!kvm_window_render_frame(&text.frame, pixels, 8, 16, &valid, &changed));
    {
        kvm_window_rect spans[2];
        text.data.text->base.cursor[0] = (kvm_text_cursor_span){12u, 4u};
        text.data.text->base.cursor[1] = (kvm_text_cursor_span){0u, 2u};
        display = (kvm_window_rect){0, 0, 8, 16};
        lib_test_assert(kvm_window_cursor_rects(&text.frame, &display, spans) == 2u);
        lib_test_assert(spans[0].top == 0 && spans[0].bottom == 2 &&
            spans[1].top == 12 && spans[1].bottom == 16);
        display.bottom = 1;
        lib_test_assert(kvm_window_cursor_rects(&text.frame, &display, spans) == 1u);
        lib_test_assert(spans[0].top == 0 && spans[0].bottom == 1);
    }
    /* A short visible row still has an 80-cell storage stride. */
    lib_u32 grid[16u * 32u];
    text.data.text->base.text_columns = text.data.text->base.text_rows = 2;
    text.data.text->font[3u * KVM_WINDOW_FONT_HEIGHT] = 0x80;
    text.data.text->secondary_font[5u * KVM_WINDOW_FONT_HEIGHT] = 0x40;
    for (lib_u32 row = 0; row < 2; ++row) {
        for (lib_u32 column = 0; column < 2; ++column) {
            lib_u8 fg = (lib_u8)(1u + row * 4u + column * 2u);
            text.data.text->base.cells[row * 80u + column] =
                (kvm_text_cell){ column ? 5u : 3u, (lib_u8)column, fg, (lib_u8)(fg + 1u), 0u };
        }
    }
    valid = LIB_FALSE;
    kvm_window_render_frame(&text.frame, grid, 16, 32, &valid, &changed);
    for (lib_u32 row = 0; row < 2; ++row)
        for (lib_u32 column = 0; column < 2; ++column)
            for (lib_u32 x = 0; x < 8; ++x) {
                lib_u32 fg = 1u + row * 4u + column * 2u;
                lib_test_assert(grid[row * 16u * 16u + column * 8u + x] ==
                    (x == column ? fg : fg + 1u) * 0x010101u);
            }
    kvm_window_frame_release(&text);
}
static void text_coverage(void)
{
    static lib_u32 guarded[132u * 8u * 60u * KVM_WINDOW_FONT_HEIGHT + 2u];
    const lib_u32 columns[] = {1u, 3u, 80u, 80u, 80u, 80u, 132u, 100u};
    const lib_u32 rows[] = {1u, 2u, 22u, 25u, 43u, 50u, 43u, 60u};
    const lib_u32 sentinel = 0xdeadbeefu;
    lib_bool valid = LIB_FALSE;
    kvm_window_rect changed;
    kvm_window_frame_release(&frame);
    for (lib_size grid = 0; grid < sizeof(rows)/sizeof(rows[0]); ++grid) {
        lib_test_assert(kvm_window_frame_prepare_text(&frame, columns[grid], rows[grid], columns[grid]+1u) == LIB_STATUS_OK);
        frame.data.text->base.text_palette[1] = 0x123456u;
        frame.data.text->base.text_palette[2] = 0xabcdefu;
        for (lib_u32 scan = 0; scan < KVM_WINDOW_FONT_HEIGHT; ++scan) {
            frame.data.text->font[KVM_WINDOW_FONT_HEIGHT + scan] = 0x55u;
            frame.data.text->secondary_font[KVM_WINDOW_FONT_HEIGHT + scan] = 0xaau;
        }
        for (lib_size i = 0; i < frame.data.text->base.capacity; ++i)
            frame.data.text->base.cells[i] = (kvm_text_cell){ (lib_u8)(i % 2u),
                (lib_u8)((i / 2u) % 2u), 1u, 2u, (lib_u8)(i % 3u == 0u) };
        for (lib_u32 font_height = 0; font_height <= KVM_WINDOW_FONT_HEIGHT; ++font_height) {
            lib_u32 width = columns[grid] * 8u;
            lib_u32 cell_height = font_height ? font_height : KVM_TEXT_FONT_DEFAULT_HEIGHT;
            lib_u32 height = rows[grid] * cell_height;
            lib_size count = (lib_size)width * height;
            frame.data.text->base.font_height = font_height;
            frame.data.text->base.underline_row = cell_height - 1u;
            for (lib_size i = 0; i < count + 2u; ++i) guarded[i] = sentinel;
            valid = LIB_FALSE;
            lib_test_assert(kvm_window_render_frame(&frame.frame, guarded + 1, width, height, &valid, &changed));
            lib_test_assert(changed.left == 0 && changed.top == 0 &&
                changed.right == (lib_i32)width && changed.bottom == (lib_i32)height);
            lib_test_assert(!kvm_window_render_frame(&frame.frame, guarded + 1, width, height, &valid, &changed));
            lib_test_assert(guarded[0] == sentinel && guarded[count + 1u] == sentinel);
            for (lib_size i = 0; i < count; ++i) {
                lib_size cell_index = (i / width / cell_height) * frame.data.text->base.stride + (i % width) / 8u;
                const kvm_text_cell *cell = &frame.data.text->base.cells[cell_index];
                lib_i32 foreground = cell->glyph_index && ((i % 2u) != cell->glyph_bank);
                foreground |= cell->underline && (i / width) % cell_height == cell_height - 1u;
                lib_test_assert(guarded[i + 1u] == (foreground ? 0x123456u : 0xabcdefu));
            }
        }
    }
    /* Last slot/last scanline and a 24-row bitmap retain the fixed stride. */
    {
        const lib_size last = (KVM_WINDOW_FONT_GLYPHS - 1u) * KVM_WINDOW_FONT_HEIGHT;
        static kvm_window_frame_storage tall;
        lib_test_assert(kvm_window_frame_prepare_text(&tall, 1u, 1u, 1u) == LIB_STATUS_OK);
        tall.data.text->base.text_columns = tall.data.text->base.text_rows = 1u;
        tall.data.text->base.font_height = KVM_WINDOW_FONT_HEIGHT;
        tall.data.text->base.cells[0] = (kvm_text_cell){255u, 0u, 1u, 2u, 0u };
        tall.data.text->base.text_palette[1] = 0x123456u;
        tall.data.text->base.text_palette[2] = 0xabcdefu;
        tall.data.text->font[last + 31u] = 0x80u;
        tall.data.text->secondary_font[last + 31u] = 0x40u;
        valid = LIB_FALSE;
        lib_test_assert(kvm_window_render_frame(&tall.frame, guarded + 1u, 8u, 32u, &valid, &changed));
        lib_test_assert(guarded[1u + 31u * 8u] == 0x123456u &&
            guarded[2u + 31u * 8u] == 0xabcdefu);
        lib_test_assert(!kvm_window_render_frame(&tall.frame, guarded + 1u, 8u, 32u, &valid, &changed));
        tall.data.text->base.cells[0].glyph_bank = 1u;
        lib_test_assert(kvm_window_render_frame(&tall.frame, guarded + 1u, 8u, 32u, &valid, &changed));
        lib_test_assert(changed.top == 31 && changed.bottom == 32 &&
            changed.left == 0 && changed.right == 2 &&
            guarded[1u + 31u * 8u] == 0xabcdefu &&
            guarded[2u + 31u * 8u] == 0x123456u);
        tall.data.text->base.font_height = 24u;
        tall.data.text->base.cells[0].glyph_bank = 0u;
        tall.data.text->font[last + 20u] = 0x80u;
        valid = LIB_FALSE;
        lib_test_assert(kvm_window_render_frame(&tall.frame, guarded + 1u, 8u, 24u, &valid, &changed));
        lib_test_assert(guarded[1u + 20u * 8u] == 0x123456u &&
            guarded[1u + 19u * 8u] == 0xabcdefu);
        kvm_window_frame_release(&tall);
    }
    /* Equal pixel extents still compare all rows when the cell grid changes. */
    lib_test_assert(kvm_window_frame_prepare_text(&frame, 80u, 50u, 80u) == LIB_STATUS_OK);
    frame.data.text->base.text_palette[1] = 0x123456u;
    frame.data.text->base.text_rows = 25;
    frame.data.text->base.font_height = 16;
    valid = LIB_FALSE;
    lib_test_assert(kvm_window_render_frame(&frame.frame, guarded + 1, 640, 400, &valid, &changed));
    frame.data.text->base.text_rows = 50;
    frame.data.text->base.font_height = 8;
    frame.data.text->base.cells[3999] = (kvm_text_cell){0, 0, 1, 1, 0u };
    lib_test_assert(kvm_window_render_frame(&frame.frame, guarded + 1, 640, 400, &valid, &changed));
    lib_test_assert(changed.bottom == 400 && guarded[640u * 400u] == 0x123456u);
    frame.data.text->base.cursor_mode = KVM_CURSOR_HOST;
    frame.data.text->base.cursor_column = 79;
    frame.data.text->base.cursor_row = 49;
    frame.data.text->base.cursor[0] = (kvm_text_cursor_span){ 6, 2u };
    {
        kvm_window_rect display = {0, 0, 640, 400}, cursor;
        lib_test_assert(kvm_window_cursor_rect(&frame.frame, &display, &cursor));
        lib_test_assert(cursor.left == 632 && cursor.top == 398 && cursor.bottom == 400);
    }
    /* Rejected geometry and invalid frames must not touch even the first pixel. */
    guarded[1] = sentinel;
    lib_test_assert(!kvm_window_render_frame(&frame.frame, guarded + 1, 1u, 1u, &valid, &changed));
    lib_test_assert(guarded[1] == sentinel);
    frame.frame.text = LIB_NULL;
    frame.frame.graphics = LIB_NULL;
    lib_test_assert(!kvm_window_render_frame(&frame.frame, guarded + 1, 80u * 8u,
        50u * KVM_TEXT_FONT_DEFAULT_HEIGHT, &valid, &changed));
    lib_test_assert(guarded[1] == sentinel);
}
int main(void)
{
    lib_test_assert(kvm_frame_exchange_create(&frames, sizeof(kvm_window_frame_storage), kvm_window_frame_retire) == LIB_STATUS_OK);
    damage(); rgb_coverage(); motion(); rendering(); text_coverage();
    kvm_frame_exchange_destroy(frames);
    kvm_window_frame_release(&frame);
    return 0;
}
