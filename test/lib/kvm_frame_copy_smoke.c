#include "lib/types/test.h"
#include "lib/types/file.h"
#include "lib/kvm-base/mailbox_interface.h"
#include "lib/kvm-window/frame_interface.h"
#include "lib/kvm-console/frame_interface.h"

static kvm_window_frame source, destination, storage;
static kvm_component_mailboxes mailbox;

_Static_assert(sizeof(kvm_text_cell) == 4u, "cell size");
_Static_assert(lib_offsetof(kvm_text_cell, glyph_index) == 0u, "glyph offset");
_Static_assert(lib_offsetof(kvm_text_cell, glyph_bank) == 1u, "bank offset");
_Static_assert(lib_offsetof(kvm_text_cell, foreground) == 2u, "foreground offset");
_Static_assert(lib_offsetof(kvm_text_cell, background) == 3u, "background offset");

static void check_validation(void)
{
    kvm_console_text_frame text = {0};
    source = (kvm_window_frame){0};
    lib_test_assert(kvm_window_frame_validate(LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(kvm_window_frame_validate(&source) == LIB_STATUS_INVALID_ARGUMENT);
    source.valid = 1u;
    lib_test_assert(kvm_window_frame_validate(&source) == LIB_STATUS_INVALID_ARGUMENT);
    source.text.base.text_columns = source.text.base.text_rows = 1u;
    lib_test_assert(kvm_window_frame_validate(&source) == LIB_STATUS_OK);
    source.text.base.font_height = KVM_WINDOW_FONT_HEIGHT;
    source.text.base.text_columns = KVM_TEXT_COLUMNS;
    source.text.base.text_rows = KVM_TEXT_ROWS;
    source.text.base.cursor_visible = 1u;
    source.text.base.cursor_column = -1;
    source.text.base.cursor_row = 500;
    lib_test_assert(kvm_window_frame_validate(&source) == LIB_STATUS_OK);
    source.text.base.font_height++;
    lib_test_assert(kvm_window_frame_validate(&source) == LIB_STATUS_UNSUPPORTED);
    text.base = source.text.base; /* Console does not own bitmap bounds. */
    lib_test_assert(kvm_console_text_frame_validate(&text) == LIB_STATUS_OK);
    source.text.base.font_height = 0u;
    for (lib_u32 field = 0; field < 3; ++field) {
        lib_u8 *value = field == 0 ? &source.text.base.cells[1999].foreground :
            field == 1 ? &source.text.base.cells[1999].background : &source.text.base.cells[1999].glyph_bank;
        *value = field == 2 ? 1 : 15;
        lib_test_assert(kvm_window_frame_validate(&source) == LIB_STATUS_OK);
        ++*value;
        lib_test_assert(kvm_window_frame_validate(&source) == LIB_STATUS_INVALID_ARGUMENT);
        source.text.base.text_rows = 1;
        lib_test_assert(kvm_window_frame_validate(&source) == LIB_STATUS_OK); /* Invisible tail. */
        source.text.base.text_rows = 25;
        *value = 0;
    }
    source.text.base.text_rows++;
    lib_test_assert(kvm_window_frame_validate(&source) == LIB_STATUS_UNSUPPORTED);
    text.base.text_columns++;
    lib_test_assert(kvm_console_text_frame_validate(&text) == LIB_STATUS_UNSUPPORTED);
    text.base.text_columns = 0u;
    lib_test_assert(kvm_console_text_frame_validate(&text) == LIB_STATUS_INVALID_ARGUMENT);
    text.base.text_columns = 1u;
    text.characters.primary[255] = 0xd800u;
    lib_test_assert(kvm_console_text_frame_validate(&text) == LIB_STATUS_INVALID_ARGUMENT);
    text.characters.primary[255] = 0xd7ffu;
    text.characters.secondary[0] = 0xdfffu;
    lib_test_assert(kvm_console_text_frame_validate(&text) == LIB_STATUS_INVALID_ARGUMENT);
    text.characters.secondary[0] = 0xe000u;
    lib_test_assert(kvm_console_text_frame_validate(&text) == LIB_STATUS_OK);
    source.graphics = 1u;
    source.image.width = source.image.height = source.image.stride = 1u;
    lib_test_assert(kvm_window_frame_validate(&source) == LIB_STATUS_OK);
    source.image.stride = 0u;
    lib_test_assert(kvm_window_frame_validate(&source) == LIB_STATUS_INVALID_ARGUMENT);
    source.image.width = source.image.stride = KVM_WINDOW_GRAPHICS_MAX_WIDTH;
    source.image.height = KVM_WINDOW_GRAPHICS_MAX_HEIGHT;
    lib_test_assert(kvm_window_frame_validate(&source) == LIB_STATUS_OK);
    source.image.height++;
    lib_test_assert(kvm_window_frame_validate(&source) == LIB_STATUS_UNSUPPORTED);
    source.image.height = 1u;
    source.image.stride = (lib_u32)-1;
    lib_test_assert(kvm_window_frame_validate(&source) == LIB_STATUS_UNSUPPORTED);
}

static void check_copy(void)
{
    lib_size bytes = kvm_window_frame_size_bytes(&source);
    lib_memory_set(&destination, 0xa5, sizeof(destination));
    lib_test_assert(kvm_window_frame_copy(&destination, &source));
    lib_test_assert(lib_memory_compare(&destination, &source, bytes) == 0);
    for (lib_size i = bytes; i < sizeof(destination); ++i)
        lib_test_assert(((const lib_u8 *)&destination)[i] == 0xa5);
    lib_test_assert(kvm_window_frame_copy(&source, &source));
}

int main(void)
{
    lib_u32 generation, old;
    lib_test_assert(sizeof(kvm_text_frame) == 8084);
    lib_test_assert(sizeof(kvm_window_text_frame) == 16276);
    lib_test_assert(sizeof(kvm_console_text_frame) == 9108);
    lib_test_assert(sizeof(kvm_window_frame) == 984084);
    check_validation();
    lib_memory_set(&source, 0x3c, sizeof(source));
    source.valid = 1u;
    source.graphics = 0u;
    source.text.base.text_columns = 80u;
    source.text.base.text_rows = 25u;
    source.text.base.font_height = 0u;
    for (lib_size i = 0; i < KVM_TEXT_COLUMNS * KVM_TEXT_ROWS; ++i)
        source.text.base.cells[i] = (kvm_text_cell){ (lib_u8)i, (lib_u8)(i % 2u),
            (lib_u8)(i % 16u), (lib_u8)((i / 16u) % 16u) };
    check_copy(); /* No inactive graphics payload is copied. */
    lib_test_assert(kvm_component_mailboxes_create(&mailbox, &storage, sizeof(storage)) == LIB_STATUS_OK);
    lib_memory_set(&storage, 0x96, sizeof(storage));
    lib_test_assert(kvm_component_mailboxes_publish_frame(&mailbox, &source,
        kvm_window_frame_size_bytes(&source)) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_mailboxes_capture_frame(&mailbox, &generation, &destination, sizeof(destination)));
    lib_test_assert(lib_memory_compare(destination.text.base.cells, source.text.base.cells,
        sizeof(source.text.base.cells)) == 0);
    for (lib_size i = kvm_window_frame_size_bytes(&source); i < sizeof(source); ++i) {
        lib_test_assert(((const lib_u8 *)&storage)[i] == 0x96);
        lib_test_assert(((const lib_u8 *)&destination)[i] == 0xa5);
    }
    old = generation;
    lib_test_assert(kvm_component_mailboxes_publish_frame(&mailbox, &source, sizeof(storage) + 1u) == LIB_STATUS_LIMIT_EXCEEDED);
    lib_test_assert(mailbox.frame_generation == old);
    lib_test_assert(!kvm_component_mailboxes_capture_frame(&mailbox, &generation, &destination, 1u));
    source.graphics = 1u;
    source.image.width = 3u;
    source.image.stride = 7u;
    source.image.height = 5u;
    check_copy(); /* Copy row padding, not just width*height. */
    lib_test_assert(kvm_component_mailboxes_publish_frame(&mailbox, &source,
        kvm_window_frame_size_bytes(&source)) == LIB_STATUS_OK);
    kvm_component_mailboxes_acknowledge_frame(&mailbox, old);
    lib_test_assert(kvm_component_mailboxes_capture_frame(&mailbox, &generation, &destination, sizeof(destination)));
    lib_test_assert(generation != old);
    lib_test_assert(lib_memory_compare(destination.image.pixels, source.image.pixels, 35u) == 0);
    lib_test_assert(destination.image.pixels[35] == 0xa5);
    source.image.width = source.image.stride = KVM_WINDOW_GRAPHICS_MAX_WIDTH;
    source.image.height = KVM_WINDOW_GRAPHICS_MAX_HEIGHT;
    check_copy();
    source.graphics = 0u;
    source.text.base.text_columns = 80u; source.text.base.text_rows = 25u;
    source.text.base.font_height = 0u;
    for (lib_size i = 0; i < KVM_TEXT_COLUMNS * KVM_TEXT_ROWS; ++i)
        source.text.base.cells[i] = (kvm_text_cell){ (lib_u8)i, 1u, 15u, 2u };
    check_copy(); /* Graphics -> text never exposes the old pixels. */
    lib_test_assert(kvm_component_mailboxes_publish_frame(&mailbox, &source,
        kvm_window_frame_size_bytes(&source)) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_mailboxes_capture_frame(&mailbox, &generation, &destination, sizeof(destination)));
    lib_test_assert(!destination.graphics && ((const lib_u8 *)&destination)[kvm_window_frame_size_bytes(&source)] == 0xa5);
    kvm_component_mailboxes_acknowledge_frame(&mailbox, generation);
    lib_test_assert(!kvm_component_mailboxes_capture_frame(&mailbox, &generation, &destination, sizeof(destination)));
    source.graphics = 1u;
    source.image.stride = KVM_WINDOW_GRAPHICS_MAX_WIDTH + 1u;
    lib_memory_set(&destination, 0xa5, sizeof(destination));
    lib_test_assert(!kvm_window_frame_copy(&destination, &source));
    lib_test_assert(!kvm_window_frame_copy(&destination, LIB_NULL));
    lib_test_assert(!kvm_window_frame_copy(LIB_NULL, &source));
    for (lib_size i = 0u; i < sizeof(destination); ++i)
        lib_test_assert(((const lib_u8 *)&destination)[i] == 0xa5);
    kvm_component_mailboxes_destroy(&mailbox);
    return 0;
}
