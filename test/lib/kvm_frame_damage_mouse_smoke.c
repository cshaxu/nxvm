#include "lib/kvm-base/mailbox_interface.h"
#include "lib/kvm-window/win32/mouse.h"
#include "lib/kvm-window/geometry.h"
#include "lib/kvm-window/render.h"
#include <assert.h>

static kvm_component_mailboxes mailbox;
static kvm_frame frame, received;

static void damage(void)
{
    lib_u32 generation = 0u;
    assert(kvm_component_mailboxes_create(&mailbox) == LIB_STATUS_OK);
    frame.valid = frame.graphics = 1u;
    frame.graphics_width = frame.graphics_stride = 4u;
    frame.graphics_height = 4u;
    assert(kvm_component_mailboxes_publish_frame(&mailbox, &frame) == LIB_STATUS_OK);
    assert(kvm_component_mailboxes_capture_frame(&mailbox, &generation, &received));
    assert(received.dirty_right == 3 && received.dirty_bottom == 3);
    kvm_component_mailboxes_acknowledge_frame(&mailbox, generation);
    frame.dirty_left = frame.dirty_top = frame.dirty_right = frame.dirty_bottom = 1;
    frame.graphics_pixels[5] = 1u;
    assert(kvm_component_mailboxes_publish_frame(&mailbox, &frame) == LIB_STATUS_OK);
    frame.dirty_left = frame.dirty_top = frame.dirty_right = frame.dirty_bottom = 2;
    frame.graphics_pixels[10] = 2u;
    assert(kvm_component_mailboxes_publish_frame(&mailbox, &frame) == LIB_STATUS_OK);
    assert(kvm_component_mailboxes_capture_frame(&mailbox, &generation, &received));
    assert(received.dirty_left == 1 && received.dirty_top == 1 &&
        received.dirty_right == 2 && received.dirty_bottom == 2);
    assert(received.graphics_pixels[5] == 1u && received.graphics_pixels[10] == 2u);
    kvm_component_mailboxes_acknowledge_frame(&mailbox, generation);
    assert(!kvm_component_mailboxes_capture_frame(&mailbox, &generation, &received));
    assert(kvm_component_mailboxes_publish_frame(&mailbox, &frame) == LIB_STATUS_OK);
    assert(kvm_component_mailboxes_capture_frame(&mailbox, &generation, &received));
    assert(received.dirty_left == 2 && received.dirty_top == 2);
    /* Failed output has no acknowledgement; capture remains available.
     * A newer publication cannot be erased by the older write's success. */
    lib_u32 old = generation;
    assert(kvm_component_mailboxes_capture_frame(&mailbox, &generation, &received));
    assert(generation == old);
    assert(kvm_component_mailboxes_publish_frame(&mailbox, &frame) == LIB_STATUS_OK);
    kvm_component_mailboxes_acknowledge_frame(&mailbox, old);
    assert(kvm_component_mailboxes_capture_frame(&mailbox, &generation, &received));
    assert(generation != old);
    kvm_component_mailboxes_acknowledge_frame(&mailbox, generation);
    frame.graphics_palette[1] = 0xffu;
    assert(kvm_component_mailboxes_publish_frame(&mailbox, &frame) == LIB_STATUS_OK);
    assert(kvm_component_mailboxes_capture_frame(&mailbox, &generation, &received));
    assert(received.dirty_left == 0 && received.dirty_right == 3);
    frame.graphics_width = frame.graphics_stride = 3u;
    assert(kvm_component_mailboxes_publish_frame(&mailbox, &frame) == LIB_STATUS_OK);
    assert(kvm_component_mailboxes_capture_frame(&mailbox, &generation, &received));
    assert(received.dirty_left == 0 && received.dirty_right == 2);
    frame.graphics = 0u; frame.text_columns = 80u; frame.text_rows = 25u;
    assert(kvm_component_mailboxes_publish_frame(&mailbox, &frame) == LIB_STATUS_OK);
    frame.graphics = 1u;
    assert(kvm_component_mailboxes_publish_frame(&mailbox, &frame) == LIB_STATUS_OK);
    assert(kvm_component_mailboxes_capture_frame(&mailbox, &generation, &received));
    assert(received.dirty_left == 0 && received.dirty_bottom == 3);
    kvm_component_mailboxes_destroy(&mailbox);
}

static void motion(void)
{
    kvm_win32_mouse mouse;
    int dx, dy, total_x = 0, total_y = 0;
    kvm_win32_mouse_reset(&mouse);
    assert(kvm_win32_mouse_move(&mouse, 0, 200, 200, 100, 100, &dx, &dy));
    for (int i = 1; i <= 10; ++i) {
        assert(kvm_win32_mouse_move(&mouse, (i << 16) | i, 200, 200, 100, 100, &dx, &dy));
        total_x += dx; total_y += dy;
    }
    assert(total_x == 5 && total_y == 5);
    for (int i = 9; i >= 0; --i) {
        assert(kvm_win32_mouse_move(&mouse, (i << 16) | i, 200, 200, 100, 100, &dx, &dy));
        total_x += dx; total_y += dy;
    }
    assert(total_x == 0 && total_y == 0);
    assert(kvm_win32_mouse_move(&mouse, 0x10001, 200, 200, 100, 100, &dx, &dy));
    assert(dx == 0 && dy == 0);
    assert(kvm_win32_mouse_move(&mouse, 0, 200, 200, 100, 100, &dx, &dy));
    assert(dx == 0 && dy == 0 && mouse.motion.remainder_x == 0 && mouse.motion.remainder_y == 0);
    assert(kvm_win32_mouse_move(&mouse, 0x10001, 200, 200, 100, 100, &dx, &dy));
    assert(kvm_win32_mouse_move(&mouse, 0x20002, 100, 100, 100, 100, &dx, &dy));
    assert(dx == 1 && dy == 1 && mouse.motion.remainder_x == 0 && mouse.motion.remainder_y == 0);
}

static void rendering(void)
{
    kvm_frame text = { 0 };
    kvm_window_rect display = { 0, 0, 640, 410 }, cursor;
    lib_u32 pixels[8 * 16];
    text.valid = 1; text.text_columns = 80; text.text_rows = 25;
    text.cursor_visible = 1; text.cursor_column = 0; text.cursor_row = 24;
    text.font_height = 16; text.cursor_top = 14; text.cursor_bottom = 15;
    assert(kvm_window_cursor_rect(&text, &display, &cursor));
    assert(cursor.bottom == 410 && cursor.top == 407);
    for (int row = 0; row < 25; ++row) {
        text.cursor_row = row;
        assert(kvm_window_cursor_rect(&text, &display, &cursor));
        assert(cursor.top >= row * 410 / 25);
        assert(cursor.bottom == (row + 1) * 410 / 25);
        assert(cursor.bottom - cursor.top <= 3);
    }
    text.text_columns = text.text_rows = 1;
    text.cursor_row = 0;
    display = (kvm_window_rect){10,20,18,36};
    for (unsigned top=0;top<16;++top) {
        text.cursor_top=(lib_u8)top; text.cursor_bottom=(lib_u8)top;
        assert(kvm_window_cursor_rect(&text,&display,&cursor));
        assert(cursor.top==20+(int)top && cursor.bottom==21+(int)top);
    }
    text.cursor_top=4; text.cursor_bottom=7;
    display.bottom=52;
    assert(kvm_window_cursor_rect(&text,&display,&cursor));
    assert(cursor.top==28 && cursor.bottom==36);
    text.cursor_top=15; text.cursor_bottom=255;
    assert(kvm_window_cursor_rect(&text,&display,&cursor) && cursor.bottom==52);
    text.cursor_top=16;
    assert(!kvm_window_cursor_rect(&text,&display,&cursor));
    text.cursor_top=9; text.cursor_bottom=8;
    assert(kvm_window_cursor_rect(&text,&display,&cursor) && cursor.top==20 && cursor.bottom==52);
    text.font_height=0;
    assert(kvm_window_cursor_rect(&text,&display,&cursor) && cursor.top==20 && cursor.bottom==52);
    text.font[0] = 0x80; text.attributes[0] = 0x21;
    text.text_palette[1] = 0x112233; text.text_palette[2] = 0x445566;
    kvm_window_render_text(&text, pixels, 8, 16);
    assert(pixels[0] == 0x112233 && pixels[1] == 0x445566 && pixels[127] == 0x445566);
}
int main(void) { damage(); motion(); rendering(); return 0; }
