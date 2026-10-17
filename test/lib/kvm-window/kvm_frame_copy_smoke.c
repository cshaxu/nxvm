#include "lib/types/test.h"
#include "lib/kvm-base/mailbox_interface.h"
#include "lib/kvm-window/frame_interface.h"
#include "lib/kvm-base/event_interface.h"

#if defined(KVM_WINDOW_FONT_DEFAULT_HEIGHT) || defined(KVM_WINDOW_FONT_MAX_HEIGHT) || defined(LIB_CONSOLE_LINE_MAX)
#error "Retired default/capacity names must not be exported"
#endif

#define ASSERT_BOOLEAN(type, member) \
    _Static_assert(_Generic(((type *)0)->member, lib_bool: 1, default: 0), \
        #type "." #member " must use the boolean value contract")
ASSERT_BOOLEAN(kvm_window_frame, valid);
ASSERT_BOOLEAN(kvm_window_frame, graphics);
_Static_assert(KVM_CURSOR_DISABLED == 0 && KVM_CURSOR_HOST == 1 &&
    KVM_CURSOR_SOURCE == 2, "cursor mode values");
#undef ASSERT_BOOLEAN

static kvm_window_frame source, destination, storage;
static kvm_component_mailboxes mailbox;

static void check_validation(void)
{
    source = (kvm_window_frame){0};
    lib_test_assert(kvm_window_frame_validate(LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(kvm_window_frame_validate(&source) == LIB_STATUS_INVALID_ARGUMENT);
    source.valid = LIB_TRUE;
    lib_test_assert(kvm_window_frame_validate(&source) == LIB_STATUS_INVALID_ARGUMENT);
    source.text.base.text_columns = source.text.base.text_rows = 1u;
    lib_test_assert(kvm_window_frame_validate(&source) == LIB_STATUS_OK);
    for (lib_i32 mode = -1; mode <= 3; ++mode) {
        source.text.base.cursor_mode = (kvm_cursor_mode)mode;
        lib_test_assert(kvm_window_frame_validate(&source) ==
            (mode >= KVM_CURSOR_DISABLED && mode <= KVM_CURSOR_SOURCE ?
                LIB_STATUS_OK : LIB_STATUS_INVALID_ARGUMENT));
    }
    source.text.base.cursor_mode = KVM_CURSOR_DISABLED;
    source.text.base.font_height = KVM_TEXT_FONT_DEFAULT_HEIGHT;
    source.text.base.text_columns = KVM_TEXT_COLUMNS;
    source.text.base.text_rows = KVM_TEXT_ROWS;
    source.text.base.cursor_mode = KVM_CURSOR_HOST;
    source.text.base.cursor_column = -1;
    source.text.base.cursor_row = 500;
    lib_test_assert(kvm_window_frame_validate(&source) == LIB_STATUS_OK);
    for (lib_u32 height = 1u; height <= KVM_WINDOW_FONT_HEIGHT; ++height) {
        source.text.base.font_height = height;
        lib_test_assert(kvm_window_frame_validate(&source) == LIB_STATUS_OK);
    }
    source.text.base.font_height = KVM_WINDOW_FONT_HEIGHT + 1u;
    lib_test_assert(kvm_window_frame_validate(&source) == LIB_STATUS_UNSUPPORTED);
    source.text.base.font_height = 0u;
    for (lib_u32 field = 0; field < 3; ++field) {
        lib_u8 *value = field == 0 ? &source.text.base.cells[3999].foreground :
            field == 1 ? &source.text.base.cells[3999].background : &source.text.base.cells[3999].glyph_bank;
        *value = field == 2 ? 1 : 15;
        lib_test_assert(kvm_window_frame_validate(&source) == LIB_STATUS_OK);
        ++*value;
        lib_test_assert(kvm_window_frame_validate(&source) == LIB_STATUS_INVALID_ARGUMENT);
        source.text.base.text_rows = 1;
        lib_test_assert(kvm_window_frame_validate(&source) == LIB_STATUS_OK); /* Invisible tail. */
        source.text.base.text_rows = KVM_TEXT_ROWS;
        *value = 0;
    }
    source.text.base.text_rows++;
    lib_test_assert(kvm_window_frame_validate(&source) == LIB_STATUS_UNSUPPORTED);
    source.graphics = LIB_TRUE;
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
    lib_test_assert(sizeof(kvm_window_text_frame) == 32476);
    lib_test_assert(sizeof(kvm_window_frame) == 984080);
    check_validation();
    lib_memory_set(&source, 0x3c, sizeof(source));
    source.valid = LIB_TRUE;
    source.graphics = LIB_FALSE;
    source.text.base.text_columns = 80u;
    source.text.base.text_rows = 25u;
    source.text.base.font_height = 0u;
    source.text.base.cursor_mode = KVM_CURSOR_SOURCE;
    source.text.base.cursor_phase = LIB_TRUE;
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
    source.graphics = LIB_TRUE;
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
    source.graphics = LIB_FALSE;
    source.text.base.text_columns = 80u; source.text.base.text_rows = 25u;
    source.text.base.font_height = 0u;
    source.text.base.cursor_mode = KVM_CURSOR_DISABLED;
    source.text.base.cursor_phase = LIB_FALSE;
    for (lib_size i = 0; i < KVM_TEXT_COLUMNS * KVM_TEXT_ROWS; ++i)
        source.text.base.cells[i] = (kvm_text_cell){ (lib_u8)i, 1u, 15u, 2u };
    check_copy(); /* Graphics -> text never exposes the old pixels. */
    lib_test_assert(kvm_component_mailboxes_publish_frame(&mailbox, &source,
        kvm_window_frame_size_bytes(&source)) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_mailboxes_capture_frame(&mailbox, &generation, &destination, sizeof(destination)));
    lib_test_assert(!destination.graphics && ((const lib_u8 *)&destination)[kvm_window_frame_size_bytes(&source)] == 0xa5);
    kvm_component_mailboxes_acknowledge_frame(&mailbox, generation);
    lib_test_assert(!kvm_component_mailboxes_capture_frame(&mailbox, &generation, &destination, sizeof(destination)));
    source.graphics = LIB_TRUE;
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
