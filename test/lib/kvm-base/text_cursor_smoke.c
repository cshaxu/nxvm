#include "lib/types/test.h"
#include "lib/kvm-base/frame_interface.h"
#include "lib/kvm-base/event_interface.h"

_Static_assert(_Generic(((kvm_text_frame *)0)->cursor_phase, lib_bool: 1, default: 0), "cursor phase boolean");
_Static_assert(_Generic(((kvm_input_event *)0)->data.key.pressed, lib_bool: 1, default: 0), "key pressed boolean");
_Static_assert(_Generic(((kvm_input_event *)0)->data.mouse.relative, lib_bool: 1, default: 0), "mouse relative boolean");
_Static_assert(sizeof(kvm_text_cell) == 5u, "cell size");
_Static_assert(lib_offsetof(kvm_text_cell, glyph_index) == 0u, "glyph offset");
_Static_assert(lib_offsetof(kvm_text_cell, glyph_bank) == 1u, "bank offset");
_Static_assert(lib_offsetof(kvm_text_cell, foreground) == 2u, "foreground offset");
_Static_assert(lib_offsetof(kvm_text_cell, background) == 3u, "background offset");

static void check_validation(void)
{
    kvm_text_frame text = {0};
    kvm_text_cell cells[80u * 50u] = {0};
    lib_test_assert(kvm_text_frame_validate(LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(kvm_text_frame_validate(&text) == LIB_STATUS_INVALID_ARGUMENT);
    text.text_columns = 1u;
    lib_test_assert(kvm_text_frame_validate(&text) == LIB_STATUS_INVALID_ARGUMENT);
    text.text_rows = 1u;
    text.cells = cells;
    text.stride = 80u;
    text.capacity = 80u * 50u;
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
    text.text_columns = 80u;
    text.text_rows = 50u;
    for (lib_u32 field = 0; field < 4; ++field) {
        lib_u8 *value = field == 0 ? &text.cells[3999].foreground :
            field == 1 ? &text.cells[3999].background :
            field == 2 ? &text.cells[3999].glyph_bank : &text.cells[3999].underline;
        *value = field >= 2 ? 1 : 15;
        lib_test_assert(kvm_text_frame_validate(&text) == LIB_STATUS_OK);
        ++*value;
        lib_test_assert(kvm_text_frame_validate(&text) == LIB_STATUS_INVALID_ARGUMENT);
        text.text_rows = 1u;
        lib_test_assert(kvm_text_frame_validate(&text) == LIB_STATUS_OK);
        text.text_rows = 50u;
        *value = 0u;
    }
    ++text.text_rows;
    lib_test_assert(kvm_text_frame_validate(&text) == LIB_STATUS_INVALID_ARGUMENT);
    text.text_rows = 1u;
    ++text.text_columns;
    lib_test_assert(kvm_text_frame_validate(&text) == LIB_STATUS_INVALID_ARGUMENT);
    text.text_columns = 1u;
    text.text_rows = 2u;
    text.cells[1].glyph_bank = 2u; /* Inactive column, not the next row. */
    lib_test_assert(kvm_text_frame_validate(&text) == LIB_STATUS_OK);
    text.cells[80u].glyph_bank = 2u;
    lib_test_assert(kvm_text_frame_validate(&text) == LIB_STATUS_INVALID_ARGUMENT);
}

int main(void)
{
    kvm_text_frame text = {0};
    check_validation();
    lib_u32 height;
    kvm_text_cursor_span spans[2];
    text = (kvm_text_frame){.text_columns = 80u, .text_rows = 25u,
        .cursor_mode = KVM_CURSOR_SOURCE, .cursor = {{14u, 18u}, {0u, 0u}}};
    lib_test_assert(kvm_text_cursor_shape(&text, &height, spans) == 1u);
    lib_test_assert(height == 16u && spans[0].start == 14u && spans[0].height == 2u);
    text.cursor_phase = LIB_FALSE;
    lib_test_assert(kvm_text_cursor_shape(&text, &height, spans) == 1u);
    text.font_height = 32u;
    lib_test_assert(kvm_text_cursor_shape(&text, &height, spans) == 1u);
    lib_test_assert(height == 32u && spans[0].start == 14u && spans[0].height == 18u);
    text.font_height = 14u;
    lib_test_assert(!kvm_text_cursor_shape(&text, &height, spans));
    text.cursor[0] = (kvm_text_cursor_span){0u, LIB_UINT32_MAX};
    lib_test_assert(kvm_text_cursor_shape(&text, &height, spans) == 1u);
    lib_test_assert(spans[0].start == 0u && spans[0].height == 14u);
    text.font_height = 512u;
    lib_test_assert(kvm_text_cursor_shape(&text, &height, spans) == 1u);
    lib_test_assert(spans[0].start == 0u && spans[0].height == 512u);
    text.font_height = 16u;
    text.cursor[0] = (kvm_text_cursor_span){12u, 4u};
    text.cursor[1] = (kvm_text_cursor_span){0u, 2u};
    lib_test_assert(kvm_text_cursor_shape(&text, &height, spans) == 2u);
    lib_test_assert(spans[0].start == 0u && spans[0].height == 2u && spans[1].start == 12u && spans[1].height == 4u);
    text.cursor[1] = (kvm_text_cursor_span){10u, 4u};
    lib_test_assert(kvm_text_cursor_shape(&text, &height, spans) == 1u);
    lib_test_assert(spans[0].start == 10u && spans[0].height == 6u);
    text.cursor[1] = (kvm_text_cursor_span){10u, 2u};
    lib_test_assert(kvm_text_cursor_shape(&text, &height, spans) == 1u);
    text.cursor[0].height = text.cursor[1].height = 0u;
    lib_test_assert(!kvm_text_cursor_shape(&text, &height, spans));
    text.cursor[0].height = 1u;
    text.cursor_row = 25;
    lib_test_assert(!kvm_text_cursor_shape(&text, &height, spans));
    text.cursor_row = 0;
    text.cursor_mode = KVM_CURSOR_DISABLED;
    lib_test_assert(!kvm_text_cursor_shape(&text, &height, spans));
    lib_test_assert(KVM_TEXT_FONT_DEFAULT_HEIGHT == 16u);
    return 0;
}
