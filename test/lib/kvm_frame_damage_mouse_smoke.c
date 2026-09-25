#include "lib/types/test.h"
#include "lib/kvm-window/window.h"
#include "lib/kvm-window/motion.h"
#include "lib/kvm-window/geometry.h"
#include "lib/kvm-window/render.h"

static kvm_window window;
static kvm_window_frame frame, received;

static void damage(void)
{
    lib_u32 generation = 0u, old;
    lib_u32 surface[16] = {0};
    kvm_window_rect changed;
    lib_bool valid = LIB_FALSE;
    lib_test_assert(kvm_component_mailboxes_create(&window.base.mailboxes, &window.pending_frame,
        sizeof(window.pending_frame)) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_mailboxes_select_notify(&window.base.mailboxes, LIB_NULL, LIB_NULL) == LIB_STATUS_OK);
    frame.valid = frame.graphics = 1u;
    frame.image.width = frame.image.stride = 4u;
    frame.image.height = 4u;
    frame.image.palette[1] = 0x112233u;
    frame.image.palette[2] = 0x445566u;
    lib_test_assert(kvm_window_publish_frame(&window, &frame) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_mailboxes_capture_frame(&window.base.mailboxes, &generation, &received, sizeof(received)));
    /* First all-black frame still invalidates the entire surface. */
    lib_test_assert(kvm_window_render_frame(&received, surface, 4, 4, &valid, &changed));
    lib_test_assert(changed.left == 0 && changed.top == 0 && changed.right == 4 && changed.bottom == 4);
    {
        lib_u32 pending_generation = generation;
        lib_test_assert(base_sync_event_wait(window.base.mailboxes.wake, 0u) == BASE_SYNC_WAIT_SIGNALED);
        frame.image.height = KVM_WINDOW_GRAPHICS_MAX_HEIGHT + 1u;
        lib_test_assert(kvm_window_publish_frame(&window, &frame) == LIB_STATUS_UNSUPPORTED);
        frame.image.height = 4u; frame.image.stride = 3u;
        lib_test_assert(kvm_window_publish_frame(&window, &frame) == LIB_STATUS_INVALID_ARGUMENT);
        frame = (kvm_window_frame){ .valid = LIB_TRUE }; /* No graphics bytes in a text fixture. */
        frame.text.base.text_columns = 80u; frame.text.base.text_rows = 25u;
        frame.text.base.font_height = KVM_WINDOW_FONT_HEIGHT + 1u;
        lib_test_assert(kvm_window_publish_frame(&window, &frame) == LIB_STATUS_UNSUPPORTED);
        lib_test_assert(base_sync_event_wait(window.base.mailboxes.wake, 0u) == BASE_SYNC_WAIT_TIMED_OUT);
        lib_test_assert(kvm_component_mailboxes_capture_frame(&window.base.mailboxes, &generation, &received, sizeof(received)));
        lib_test_assert(generation == pending_generation && received.graphics && received.image.height == 4u);
        frame = received;
    }
    kvm_component_mailboxes_acknowledge_frame(&window.base.mailboxes, generation);
    /* A is overwritten by B before consumption; B contains both changes. */
    frame.image.pixels[5] = 1u;
    lib_test_assert(kvm_window_publish_frame(&window, &frame) == LIB_STATUS_OK);
    frame.image.pixels[10] = 2u;
    lib_test_assert(kvm_window_publish_frame(&window, &frame) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_mailboxes_capture_frame(&window.base.mailboxes, &generation, &received, sizeof(received)));
    lib_test_assert(kvm_window_render_frame(&received, surface, 4, 4, &valid, &changed));
    lib_test_assert(changed.left == 1 && changed.top == 1 && changed.right == 3 && changed.bottom == 3);
    lib_test_assert(surface[5] == 0x112233u && surface[10] == 0x445566u);
    lib_test_assert(!kvm_window_render_frame(&received, surface, 4, 4, &valid, &changed));
    /* Failed output keeps its capture; old acknowledgement cannot erase new data. */
    old = generation;
    frame.image.pixels[0] = 2u;
    lib_test_assert(kvm_window_publish_frame(&window, &frame) == LIB_STATUS_OK);
    kvm_component_mailboxes_acknowledge_frame(&window.base.mailboxes, old);
    lib_test_assert(kvm_component_mailboxes_capture_frame(&window.base.mailboxes, &generation, &received, sizeof(received)));
    lib_test_assert(generation != old);
    lib_test_assert(kvm_window_render_frame(&received, surface, 4, 4, &valid, &changed));
    lib_test_assert(changed.left == 0 && changed.top == 0 && changed.right == 1 && changed.bottom == 1);
    kvm_component_mailboxes_acknowledge_frame(&window.base.mailboxes, generation);
    lib_test_assert(!kvm_component_mailboxes_capture_frame(&window.base.mailboxes, &generation, &received, sizeof(received)));
    /* Palette-only changes affect exactly the pixels using that entry. */
    frame.image.palette[1] = 0xffu;
    lib_test_assert(kvm_window_render_frame(&frame, surface, 4, 4, &valid, &changed));
    lib_test_assert(surface[5] == 0xffu && changed.left == 1 && changed.top == 1 &&
        changed.right == 2 && changed.bottom == 2);
    frame.image.palette[255] = 0xffffffu;
    lib_test_assert(!kvm_window_render_frame(&frame, surface, 4, 4, &valid, &changed));
    /* Different indices with the same resolved colour do not damage the surface. */
    frame.image.palette[3] = frame.image.palette[2];
    frame.image.pixels[0] = 3;
    lib_test_assert(!kvm_window_render_frame(&frame, surface, 4, 4, &valid, &changed));
    /* Row padding is not part of the displayed pixels. */
    frame.image.width = 3;
    valid = LIB_FALSE;
    lib_test_assert(kvm_window_render_frame(&frame, surface, 3, 4, &valid, &changed));
    lib_test_assert(changed.right == 3 && changed.bottom == 4);
    frame.image.pixels[3] = 255;
    lib_test_assert(!kvm_window_render_frame(&frame, surface, 3, 4, &valid, &changed));
    /* Recreated surfaces require full invalidation even for unchanged pixels. */
    valid = LIB_FALSE;
    lib_test_assert(kvm_window_render_frame(&frame, surface, 3, 4, &valid, &changed));
    lib_test_assert(changed.left == 0 && changed.top == 0 && changed.right == 3 && changed.bottom == 4);
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
    kvm_window_frame text = { 0 };
    kvm_window_rect display = { 0, 0, 640, 410 }, cursor;
    lib_u32 pixels[8 * 16];
    lib_bool valid = LIB_FALSE;
    kvm_window_rect changed;
    text.valid = LIB_TRUE; text.text.base.text_columns = 80; text.text.base.text_rows = 25;
    text.text.base.cursor_visible = LIB_TRUE; text.text.base.cursor_column = 0; text.text.base.cursor_row = 24;
    text.text.base.font_height = 16; text.text.base.cursor_top = 14; text.text.base.cursor_bottom = 15;
    lib_test_assert(kvm_window_cursor_rect(&text, &display, &cursor));
    lib_test_assert(cursor.bottom == 410 && cursor.top == 407);
    text.text.base.font_height = 0;
    lib_test_assert(kvm_window_cursor_rect(&text, &display, &cursor));
    lib_test_assert(cursor.bottom == 410 && cursor.top == 407);
    text.text.base.font_height = 16;
    for (lib_i32 row = 0; row < 25; ++row) {
        text.text.base.cursor_row = row;
        lib_test_assert(kvm_window_cursor_rect(&text, &display, &cursor));
        lib_test_assert(cursor.top >= row * 410 / 25);
        lib_test_assert(cursor.bottom == (row + 1) * 410 / 25);
        lib_test_assert(cursor.bottom - cursor.top <= 3);
    }
    text.text.base.text_columns = text.text.base.text_rows = 1;
    text.text.base.cursor_row = 0;
    display = (kvm_window_rect){10,20,18,36};
    for (lib_u32 top=0;top<16;++top) {
        text.text.base.cursor_top=(lib_u8)top; text.text.base.cursor_bottom=(lib_u8)top;
        lib_test_assert(kvm_window_cursor_rect(&text,&display,&cursor));
        lib_test_assert(cursor.top==20+(lib_i32)top && cursor.bottom==21+(lib_i32)top);
    }
    text.text.base.cursor_top=4; text.text.base.cursor_bottom=7;
    display.bottom=52;
    lib_test_assert(kvm_window_cursor_rect(&text,&display,&cursor));
    lib_test_assert(cursor.top==28 && cursor.bottom==36);
    text.text.base.cursor_top=15; text.text.base.cursor_bottom=255;
    lib_test_assert(kvm_window_cursor_rect(&text,&display,&cursor) && cursor.bottom==52);
    text.text.base.cursor_top=16;
    lib_test_assert(!kvm_window_cursor_rect(&text,&display,&cursor));
    text.text.base.cursor_top=9; text.text.base.cursor_bottom=8;
    lib_test_assert(kvm_window_cursor_rect(&text,&display,&cursor) && cursor.top==20 && cursor.bottom==52);
    text.text.base.font_height=0;
    lib_test_assert(kvm_window_cursor_rect(&text,&display,&cursor) && cursor.top==20 && cursor.bottom==52);
    text.text.font[0] = 0x80;
    text.text.base.cells[0].foreground = 1; text.text.base.cells[0].background = 2;
    text.text.base.text_palette[1] = 0x112233; text.text.base.text_palette[2] = 0x445566;
    kvm_window_render_frame(&text, pixels, 8, 16, &valid, &changed);
    lib_test_assert(pixels[0] == 0x112233 && pixels[1] == 0x445566 && pixels[127] == 0x445566);
    text.text.font[0] = 0x40;
    kvm_window_render_frame(&text, pixels, 8, 16, &valid, &changed);
    lib_test_assert(pixels[0] == 0x445566 && pixels[1] == 0x112233);
    text.text.base.cells[0].glyph_bank = 1u;
    text.text.base.cells[0].foreground = 9u;
    text.text.base.text_palette[9] = 0x112233;
    text.text.secondary_font[0] = 0x20;
    kvm_window_render_frame(&text, pixels, 8, 16, &valid, &changed);
    lib_test_assert(pixels[0] == 0x445566 && pixels[2] == 0x112233);
    /* Compare all original byte attributes, including intensity and bank coupling. */
    for (lib_u32 colour = 0; colour < 16; ++colour)
        text.text.base.text_palette[colour] = colour * 0x010101u;
    for (lib_u32 enabled = 0; enabled < 2; ++enabled) {
        for (lib_u32 attribute = 0; attribute < 256; ++attribute) {
            lib_u32 bits = enabled && (attribute & 8u) ? 0x20u : 0x40u;
            text.text.base.cells[0].foreground = attribute & 15u;
            text.text.base.cells[0].background = attribute >> 4;
            text.text.base.cells[0].glyph_bank = enabled && (attribute & 8u);
            kvm_window_render_frame(&text, pixels, 8, 16, &valid, &changed);
            for (lib_u32 x = 0; x < 8; ++x)
                lib_test_assert(pixels[x] == ((bits & (0x80u >> x)) ?
                    (attribute & 15u) : (attribute >> 4)) * 0x010101u);
        }
    }
    /* Neutral producers may select bank 1 without colour bit 3. */
    text.text.base.cells[0].foreground = 1u;
    text.text.base.cells[0].glyph_bank = 1u;
    kvm_window_render_frame(&text, pixels, 8, 16, &valid, &changed);
    lib_test_assert(pixels[2] == 0x010101u);
    /* A short visible row still has an 80-cell storage stride. */
    lib_u32 grid[16u * 32u];
    text.text.base.text_columns = text.text.base.text_rows = 2;
    text.text.font[3u * 16u] = 0x80;
    text.text.secondary_font[5u * 16u] = 0x40;
    for (lib_u32 row = 0; row < 2; ++row) {
        for (lib_u32 column = 0; column < 2; ++column) {
            lib_u8 fg = (lib_u8)(1u + row * 4u + column * 2u);
            text.text.base.cells[row * KVM_TEXT_COLUMNS + column] =
                (kvm_text_cell){ column ? 5u : 3u, (lib_u8)column, fg, (lib_u8)(fg + 1u) };
        }
    }
    valid = LIB_FALSE;
    kvm_window_render_frame(&text, grid, 16, 32, &valid, &changed);
    for (lib_u32 row = 0; row < 2; ++row)
        for (lib_u32 column = 0; column < 2; ++column)
            for (lib_u32 x = 0; x < 8; ++x) {
                lib_u32 fg = 1u + row * 4u + column * 2u;
                lib_test_assert(grid[row * 16u * 16u + column * 8u + x] ==
                    (x == column ? fg : fg + 1u) * 0x010101u);
            }
}
static void text_coverage(void)
{
    static lib_u32 guarded[KVM_TEXT_COLUMNS * 8u * KVM_TEXT_ROWS * KVM_WINDOW_FONT_HEIGHT + 2u];
    const lib_u32 columns[] = {1u, 3u, 80u, 80u, 80u, KVM_TEXT_COLUMNS};
    const lib_u32 rows[] = {1u, 2u, 22u, 25u, 43u, KVM_TEXT_ROWS};
    const lib_u32 sentinel = 0xdeadbeefu;
    lib_bool valid = LIB_FALSE;
    kvm_window_rect changed;
    frame = (kvm_window_frame){ .valid = LIB_TRUE };
    frame.text.base.text_palette[1] = 0x123456u;
    frame.text.base.text_palette[2] = 0xabcdefu;
    for (lib_u32 scan = 0; scan < KVM_WINDOW_FONT_HEIGHT; ++scan) {
        frame.text.font[16u + scan] = 0x55u;
        frame.text.secondary_font[16u + scan] = 0xaau;
    }
    for (lib_u32 i = 0; i < KVM_TEXT_COLUMNS * KVM_TEXT_ROWS; ++i)
        frame.text.base.cells[i] = (kvm_text_cell){ (lib_u8)(i % 2u),
            (lib_u8)((i / 2u) % 2u), 1u, 2u };
    for (lib_size grid = 0; grid < sizeof(rows)/sizeof(rows[0]); ++grid) {
        frame.text.base.text_columns = columns[grid];
        frame.text.base.text_rows = rows[grid];
        for (lib_u32 font_height = 0; font_height <= KVM_WINDOW_FONT_HEIGHT; ++font_height) {
            lib_u32 width = columns[grid] * 8u;
            lib_u32 cell_height = font_height ? font_height : KVM_WINDOW_FONT_HEIGHT;
            lib_u32 height = rows[grid] * cell_height;
            lib_size count = (lib_size)width * height;
            frame.text.base.font_height = font_height;
            for (lib_size i = 0; i < count + 2u; ++i) guarded[i] = sentinel;
            valid = LIB_FALSE;
            lib_test_assert(kvm_window_render_frame(&frame, guarded + 1, width, height, &valid, &changed));
            lib_test_assert(changed.left == 0 && changed.top == 0 &&
                changed.right == (lib_i32)width && changed.bottom == (lib_i32)height);
            lib_test_assert(!kvm_window_render_frame(&frame, guarded + 1, width, height, &valid, &changed));
            lib_test_assert(guarded[0] == sentinel && guarded[count + 1u] == sentinel);
            for (lib_size i = 0; i < count; ++i) {
                lib_size cell_index = (i / width / cell_height) * KVM_TEXT_COLUMNS + (i % width) / 8u;
                const kvm_text_cell *cell = &frame.text.base.cells[cell_index];
                lib_i32 foreground = cell->glyph_index && ((i % 2u) != cell->glyph_bank);
                lib_test_assert(guarded[i + 1u] == (foreground ? 0x123456u : 0xabcdefu));
            }
        }
    }
    /* Equal pixel extents still compare all rows when the cell grid changes. */
    frame.text.base.text_rows = 25;
    frame.text.base.font_height = 16;
    valid = LIB_FALSE;
    lib_test_assert(kvm_window_render_frame(&frame, guarded + 1, 640, 400, &valid, &changed));
    frame.text.base.text_rows = 50;
    frame.text.base.font_height = 8;
    frame.text.base.cells[3999] = (kvm_text_cell){0, 0, 1, 1};
    lib_test_assert(kvm_window_render_frame(&frame, guarded + 1, 640, 400, &valid, &changed));
    lib_test_assert(changed.bottom == 400 && guarded[640u * 400u] == 0x123456u);
    frame.text.base.cursor_visible = LIB_TRUE;
    frame.text.base.cursor_column = 79;
    frame.text.base.cursor_row = 49;
    frame.text.base.cursor_top = 6;
    frame.text.base.cursor_bottom = 7;
    {
        kvm_window_rect display = {0, 0, 640, 400}, cursor;
        lib_test_assert(kvm_window_cursor_rect(&frame, &display, &cursor));
        lib_test_assert(cursor.left == 632 && cursor.top == 398 && cursor.bottom == 400);
    }
    /* Rejected geometry and invalid frames must not touch even the first pixel. */
    guarded[1] = sentinel;
    lib_test_assert(!kvm_window_render_frame(&frame, guarded + 1, 1u, 1u, &valid, &changed));
    lib_test_assert(guarded[1] == sentinel);
    frame.valid = LIB_FALSE;
    lib_test_assert(!kvm_window_render_frame(&frame, guarded + 1, KVM_TEXT_COLUMNS * 8u,
        KVM_TEXT_ROWS * KVM_WINDOW_FONT_HEIGHT, &valid, &changed));
    lib_test_assert(guarded[1] == sentinel);
}
int main(void) { damage(); motion(); rendering(); text_coverage(); return 0; }
