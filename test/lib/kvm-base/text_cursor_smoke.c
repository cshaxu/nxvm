#include "lib/types/test.h"
#include "lib/kvm-base/frame_interface.h"
#include "lib/kvm-base/event_interface.h"

_Static_assert(_Generic(((kvm_text_frame *)0)->cursor_phase, lib_bool: 1, default: 0), "cursor phase boolean");
_Static_assert(_Generic(((kvm_input_event *)0)->data.key.pressed, lib_bool: 1, default: 0), "key pressed boolean");
_Static_assert(_Generic(((kvm_input_event *)0)->data.mouse.relative, lib_bool: 1, default: 0), "mouse relative boolean");
_Static_assert(sizeof(kvm_text_cell) == 4u, "cell size");
_Static_assert(lib_offsetof(kvm_text_cell, glyph_index) == 0u, "glyph offset");
_Static_assert(lib_offsetof(kvm_text_cell, glyph_bank) == 1u, "bank offset");
_Static_assert(lib_offsetof(kvm_text_cell, foreground) == 2u, "foreground offset");
_Static_assert(lib_offsetof(kvm_text_cell, background) == 3u, "background offset");

int main(void)
{
    kvm_text_frame text = {0};
    lib_test_assert(sizeof(kvm_text_frame) == 16092);
    lib_u32 height, top, bottom;
    text = (kvm_text_frame){.text_columns = 80u, .text_rows = 25u,
        .cursor_mode = KVM_CURSOR_SOURCE, .cursor_top = 14u, .cursor_bottom = 31u};
    lib_test_assert(kvm_text_cursor_shape(&text, &height, &top, &bottom));
    lib_test_assert(height == 16u && top == 14u && bottom == 16u);
    text.cursor_phase = LIB_FALSE;
    lib_test_assert(kvm_text_cursor_shape(&text, &height, &top, &bottom));
    text.font_height = 32u;
    lib_test_assert(kvm_text_cursor_shape(&text, &height, &top, &bottom));
    lib_test_assert(height == 32u && top == 14u && bottom == 32u);
    text.font_height = 14u;
    lib_test_assert(!kvm_text_cursor_shape(&text, &height, &top, &bottom));
    text.cursor_bottom = 0u;
    lib_test_assert(kvm_text_cursor_shape(&text, &height, &top, &bottom));
    lib_test_assert(top == 0u && bottom == 14u);
    text.font_height = 512u;
    lib_test_assert(kvm_text_cursor_shape(&text, &height, &top, &bottom));
    lib_test_assert(top == 0u && bottom == 512u);
    text.cursor_row = 25;
    lib_test_assert(!kvm_text_cursor_shape(&text, &height, &top, &bottom));
    text.cursor_row = 0;
    text.cursor_mode = KVM_CURSOR_DISABLED;
    lib_test_assert(!kvm_text_cursor_shape(&text, &height, &top, &bottom));
    lib_test_assert(KVM_TEXT_FONT_DEFAULT_HEIGHT == 16u);
    return 0;
}
