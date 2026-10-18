#include "lib/types/test.h"
#include "lib/kvm-base/mailbox_interface.h"
#include "lib/kvm-window/frame_interface.h"
#include "lib/kvm-base/event_interface.h"

#if defined(KVM_WINDOW_FONT_DEFAULT_HEIGHT) || defined(KVM_WINDOW_FONT_MAX_HEIGHT) || defined(LIB_CONSOLE_LINE_MAX)
#error "Retired default/capacity names must not be exported"
#endif

_Static_assert(KVM_CURSOR_DISABLED == 0 && KVM_CURSOR_HOST == 1 &&
    KVM_CURSOR_SOURCE == 2, "cursor mode values");

static kvm_window_frame_storage source, destination;
static kvm_frame_exchange *frames;
static kvm_component_mailboxes mailbox;

static void check_validation(void)
{
    lib_memory_set(&source.frame, 0, sizeof(source));
    lib_test_assert(kvm_window_frame_validate(LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(kvm_window_frame_validate(&source.frame) == LIB_STATUS_INVALID_ARGUMENT);
    (void)kvm_window_frame_begin_text(&source);
    lib_test_assert(kvm_window_frame_validate(&source.frame) == LIB_STATUS_INVALID_ARGUMENT);
    source.data.text.base.text_columns = source.data.text.base.text_rows = 1u;
    lib_test_assert(kvm_window_frame_validate(&source.frame) == LIB_STATUS_OK);
    source.data.text.base.font_height = KVM_TEXT_FONT_DEFAULT_HEIGHT;
    source.data.text.base.text_columns = KVM_TEXT_COLUMNS;
    source.data.text.base.text_rows = KVM_TEXT_ROWS;
    source.data.text.base.cursor_mode = KVM_CURSOR_HOST;
    source.data.text.base.cursor_column = -1;
    source.data.text.base.cursor_row = 500;
    lib_test_assert(kvm_window_frame_validate(&source.frame) == LIB_STATUS_OK);
    for (lib_u32 height = 1u; height <= KVM_WINDOW_FONT_HEIGHT; ++height) {
        source.data.text.base.font_height = height;
        lib_test_assert(kvm_window_frame_validate(&source.frame) == LIB_STATUS_OK);
    }
    source.data.text.base.font_height = KVM_WINDOW_FONT_HEIGHT + 1u;
    lib_test_assert(kvm_window_frame_validate(&source.frame) == LIB_STATUS_UNSUPPORTED);
    source.data.text.base.font_height = 0u;
    (void)kvm_window_frame_begin_graphics(&source);
    lib_memory_set(&source.data.graphics, 0, sizeof(source.data.graphics));
    source.data.graphics.width = source.data.graphics.height = source.data.graphics.stride = 1u;
    lib_test_assert(kvm_window_frame_validate(&source.frame) == LIB_STATUS_OK);
    source.data.graphics.stride = 0u;
    lib_test_assert(kvm_window_frame_validate(&source.frame) == LIB_STATUS_INVALID_ARGUMENT);
    source.data.graphics.width = source.data.graphics.stride = KVM_WINDOW_GRAPHICS_MAX_WIDTH;
    source.data.graphics.height = KVM_WINDOW_GRAPHICS_MAX_HEIGHT;
    lib_test_assert(kvm_window_frame_validate(&source.frame) == LIB_STATUS_OK);
    source.data.graphics.height++;
    lib_test_assert(kvm_window_frame_validate(&source.frame) == LIB_STATUS_UNSUPPORTED);
    source.data.graphics.height = 1u;
    source.data.graphics.stride = (lib_u32)-1;
    lib_test_assert(kvm_window_frame_validate(&source.frame) == LIB_STATUS_UNSUPPORTED);
}

static void check_copy(void)
{
    lib_size bytes = source.frame.graphics != LIB_NULL ?
        lib_offsetof(kvm_window_graphics_frame, pixels) +
        (lib_size)source.data.graphics.stride * source.data.graphics.height * sizeof(lib_u32) :
        sizeof(kvm_window_text_frame);
    lib_memory_set(&destination, 0xa5, sizeof(destination));
    lib_test_assert(kvm_window_frame_copy(&destination, &source.frame));
    lib_test_assert(destination.frame.sequence == source.frame.sequence);
    lib_test_assert(lib_memory_compare(&destination.data, &source.data, bytes) == 0);
    lib_test_assert(source.frame.graphics != LIB_NULL ?
        destination.frame.graphics == &destination.data.graphics && destination.frame.text == LIB_NULL :
        destination.frame.text == &destination.data.text && destination.frame.graphics == LIB_NULL);
    for (lib_size i = bytes; i < sizeof(destination.data); ++i)
        lib_test_assert(((const lib_u8 *)&destination.data)[i] == 0xa5);
    lib_test_assert(kvm_window_frame_copy(&source, &source.frame));
}

int main(void)
{
    const kvm_window_frame *taken;
    lib_test_assert(sizeof(kvm_window_text_frame) == 32476);
    lib_test_assert(lib_offsetof(kvm_window_frame_storage, frame) == 0u);
    check_validation();
    (void)kvm_window_frame_begin_text(&source);
    source.frame.graphics = &source.data.graphics;
    lib_test_assert(kvm_window_frame_validate(&source.frame) == LIB_STATUS_INVALID_ARGUMENT);
    lib_memory_set(&source.frame, 0x3c, sizeof(source));
    (void)kvm_window_frame_begin_text(&source);
    source.data.text.base.text_columns = 80u;
    source.data.text.base.text_rows = 25u;
    source.data.text.base.font_height = 0u;
    source.data.text.base.cursor_mode = KVM_CURSOR_SOURCE;
    source.data.text.base.cursor_phase = LIB_TRUE;
    for (lib_size i = 0; i < KVM_TEXT_COLUMNS * KVM_TEXT_ROWS; ++i)
        source.data.text.base.cells[i] = (kvm_text_cell){ (lib_u8)i, (lib_u8)(i % 2u),
            (lib_u8)(i % 16u), (lib_u8)((i / 16u) % 16u) };
    check_copy(); /* No inactive graphics payload is copied. */
    lib_test_assert(kvm_component_mailboxes_create(&mailbox) == LIB_STATUS_OK);
    lib_test_assert(kvm_frame_exchange_create(&frames, sizeof(source)) == LIB_STATUS_OK);
    lib_test_assert(kvm_window_frame_copy(kvm_frame_exchange_writer(frames), &source.frame));
    lib_test_assert(kvm_window_frame_commit(frames) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_mailboxes_publish_frame(&mailbox, frames) == LIB_STATUS_OK);
    taken = kvm_component_mailboxes_take_frame(&mailbox, LIB_NULL);
    lib_test_assert(taken != LIB_NULL && lib_memory_compare(taken->text->base.cells,
        source.data.text.base.cells, sizeof(source.data.text.base.cells)) == 0);
    (void)kvm_window_frame_begin_graphics(&source);
    for (lib_size i = 0u; i < KVM_WINDOW_GRAPHICS_MAX_PIXELS; ++i)
        source.data.graphics.pixels[i] = (lib_u32)i;
    source.data.graphics.width = 3u;
    source.data.graphics.stride = 7u;
    source.data.graphics.height = 5u;
    check_copy(); /* Copy row padding, not just width*height. */
    lib_test_assert(kvm_window_frame_copy(kvm_frame_exchange_writer(frames), &source.frame));
    lib_test_assert(kvm_window_frame_commit(frames) == LIB_STATUS_OK);
    kvm_component_mailboxes_finish_frame(&mailbox, LIB_TRUE);
    taken = kvm_component_mailboxes_take_frame(&mailbox, LIB_NULL);
    lib_test_assert(taken != LIB_NULL && lib_memory_compare(taken->graphics->pixels,
        source.data.graphics.pixels, 35u * sizeof(source.data.graphics.pixels[0])) == 0);
    kvm_component_mailboxes_finish_frame(&mailbox, LIB_TRUE);
    source.data.graphics.width = source.data.graphics.stride = KVM_WINDOW_GRAPHICS_MAX_WIDTH;
    source.data.graphics.height = KVM_WINDOW_GRAPHICS_MAX_HEIGHT;
    check_copy();
    (void)kvm_window_frame_begin_text(&source);
    source.data.text.base.text_columns = 80u; source.data.text.base.text_rows = 25u;
    source.data.text.base.font_height = 0u;
    source.data.text.base.cursor_mode = KVM_CURSOR_DISABLED;
    source.data.text.base.cursor_phase = LIB_FALSE;
    for (lib_size i = 0; i < KVM_TEXT_COLUMNS * KVM_TEXT_ROWS; ++i)
        source.data.text.base.cells[i] = (kvm_text_cell){ (lib_u8)i, 1u, 15u, 2u };
    check_copy(); /* Graphics -> text never exposes the old pixels. */
    lib_test_assert(kvm_window_frame_copy(kvm_frame_exchange_writer(frames), &source.frame));
    lib_test_assert(kvm_window_frame_commit(frames) == LIB_STATUS_OK);
    taken = kvm_component_mailboxes_take_frame(&mailbox, LIB_NULL);
    lib_test_assert(taken != LIB_NULL && !taken->graphics);
    kvm_component_mailboxes_finish_frame(&mailbox, LIB_TRUE);
    lib_test_assert(kvm_component_mailboxes_take_frame(&mailbox, LIB_NULL) == LIB_NULL);
    (void)kvm_window_frame_begin_graphics(&source);
    source.data.graphics.stride = KVM_WINDOW_GRAPHICS_MAX_WIDTH + 1u;
    lib_memory_set(&destination.frame, 0xa5, sizeof(destination));
    lib_test_assert(!kvm_window_frame_copy(&destination, &source.frame));
    lib_test_assert(!kvm_window_frame_copy(&destination, LIB_NULL));
    lib_test_assert(!kvm_window_frame_copy(LIB_NULL, &source.frame));
    for (lib_size i = 0u; i < sizeof(destination); ++i)
        lib_test_assert(((const lib_u8 *)&destination.frame)[i] == 0xa5);
    kvm_window_frame_storage *writer = kvm_frame_exchange_writer(frames);
    writer->frame.text = LIB_NULL;
    writer->frame.graphics = LIB_NULL;
    lib_test_assert(kvm_window_frame_commit(frames) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(kvm_component_mailboxes_take_frame(&mailbox, LIB_NULL) == LIB_NULL);
    writer->frame.text = &writer->data.text;
    writer->frame.graphics = &writer->data.graphics;
    lib_test_assert(kvm_window_frame_commit(frames) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(kvm_component_mailboxes_take_frame(&mailbox, LIB_NULL) == LIB_NULL);
    kvm_component_mailboxes_destroy(&mailbox);
    kvm_frame_exchange_destroy(frames);
    return 0;
}
