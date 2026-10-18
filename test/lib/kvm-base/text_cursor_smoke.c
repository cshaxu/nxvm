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

static void check_validation(void)
{
    kvm_text_frame text = {0};
    lib_test_assert(kvm_text_frame_validate(LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(kvm_text_frame_validate(&text) == LIB_STATUS_INVALID_ARGUMENT);
    text.text_columns = 1u;
    lib_test_assert(kvm_text_frame_validate(&text) == LIB_STATUS_INVALID_ARGUMENT);
    text.text_rows = 1u;
    lib_test_assert(kvm_text_frame_validate(&text) == LIB_STATUS_OK);
    for (lib_i32 mode = -1; mode <= 3; ++mode) {
        text.cursor_mode = (kvm_cursor_mode)mode;
        lib_test_assert(kvm_text_frame_validate(&text) ==
            (mode >= KVM_CURSOR_DISABLED && mode <= KVM_CURSOR_SOURCE ?
                LIB_STATUS_OK : LIB_STATUS_INVALID_ARGUMENT));
    }
    text.cursor_mode = KVM_CURSOR_HOST;
    text.cursor_column = -1;
    text.cursor_row = 500;
    text.text_columns = KVM_TEXT_COLUMNS;
    text.text_rows = KVM_TEXT_ROWS;
    for (lib_u32 field = 0; field < 3; ++field) {
        lib_u8 *value = field == 0 ? &text.cells[3999].foreground :
            field == 1 ? &text.cells[3999].background : &text.cells[3999].glyph_bank;
        *value = field == 2 ? 1 : 15;
        lib_test_assert(kvm_text_frame_validate(&text) == LIB_STATUS_OK);
        ++*value;
        lib_test_assert(kvm_text_frame_validate(&text) == LIB_STATUS_INVALID_ARGUMENT);
        text.text_rows = 1u;
        lib_test_assert(kvm_text_frame_validate(&text) == LIB_STATUS_OK);
        text.text_rows = KVM_TEXT_ROWS;
        *value = 0u;
    }
    ++text.text_rows;
    lib_test_assert(kvm_text_frame_validate(&text) == LIB_STATUS_UNSUPPORTED);
    text.text_rows = 1u;
    ++text.text_columns;
    lib_test_assert(kvm_text_frame_validate(&text) == LIB_STATUS_UNSUPPORTED);
    text.text_columns = 1u;
    text.text_rows = 2u;
    text.cells[1].glyph_bank = 2u; /* Inactive column, not the next row. */
    lib_test_assert(kvm_text_frame_validate(&text) == LIB_STATUS_OK);
    text.cells[KVM_TEXT_COLUMNS].glyph_bank = 2u;
    lib_test_assert(kvm_text_frame_validate(&text) == LIB_STATUS_INVALID_ARGUMENT);
}

int main(void)
{
    kvm_text_frame text = {0};
    check_validation();
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
