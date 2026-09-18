#include "lib/kvm-base/mailbox_interface.h"
#include <assert.h>

static kvm_frame source, destination;
static kvm_component_mailboxes mailbox;

static void check_copy(lib_size pixels)
{
    lib_size prefix = lib_offsetof(kvm_frame, graphics_pixels);
    lib_memory_set(&destination, 0xa5, sizeof(destination));
    assert(kvm_frame_copy(&destination, &source));
    assert(lib_memory_compare(&destination, &source, prefix + pixels) == 0);
    for (lib_size i = pixels; i < KVM_GRAPHICS_MAX_PIXELS; ++i)
        assert(destination.graphics_pixels[i] == 0xa5);
    assert(kvm_frame_copy(&source, &source));
}

int main(void)
{
    lib_u32 generation, old;
    lib_memory_set(&source, 0x3c, sizeof(source));
    source.valid = 1u;
    source.graphics = 0u;
    source.text_columns = 80u;
    source.text_rows = 25u;
    check_copy(0u); /* Even inactive graphics metadata/palette is copied. */
    assert(kvm_component_mailboxes_create(&mailbox) == LIB_STATUS_OK);
    lib_memory_set(mailbox.frame.graphics_pixels, 0x96,
        sizeof(mailbox.frame.graphics_pixels));
    assert(kvm_component_mailboxes_publish_frame(&mailbox, &source) == LIB_STATUS_OK);
    assert(kvm_component_mailboxes_capture_frame(&mailbox, &generation, &destination));
    assert(destination.text[1999] == source.text[1999]);
    for (lib_size i = 0u; i < KVM_GRAPHICS_MAX_PIXELS; ++i) {
        assert(mailbox.frame.graphics_pixels[i] == 0x96);
        assert(destination.graphics_pixels[i] == 0xa5);
    }
    old = generation;
    source.graphics = 1u;
    source.graphics_width = 3u;
    source.graphics_stride = 7u;
    source.graphics_height = 5u;
    check_copy(35u); /* Copy row padding, not just width*height. */
    assert(kvm_component_mailboxes_publish_frame(&mailbox, &source) == LIB_STATUS_OK);
    kvm_component_mailboxes_acknowledge_frame(&mailbox, old);
    assert(kvm_component_mailboxes_capture_frame(&mailbox, &generation, &destination));
    assert(generation != old && destination.dirty_right == 2 && destination.dirty_bottom == 4);
    assert(lib_memory_compare(destination.graphics_pixels, source.graphics_pixels, 35u) == 0);
    assert(destination.graphics_pixels[35] == 0xa5);
    source.graphics_width = source.graphics_stride = KVM_GRAPHICS_MAX_WIDTH;
    source.graphics_height = KVM_GRAPHICS_MAX_HEIGHT;
    check_copy(KVM_GRAPHICS_MAX_PIXELS);
    source.graphics = 0u;
    check_copy(0u); /* Graphics -> text never exposes the old pixels. */
    assert(kvm_component_mailboxes_publish_frame(&mailbox, &source) == LIB_STATUS_OK);
    assert(kvm_component_mailboxes_capture_frame(&mailbox, &generation, &destination));
    assert(!destination.graphics && destination.graphics_pixels[0] == 0xa5);
    kvm_component_mailboxes_acknowledge_frame(&mailbox, generation);
    assert(!kvm_component_mailboxes_capture_frame(&mailbox, &generation, &destination));
    source.graphics = 1u;
    source.graphics_stride = KVM_GRAPHICS_MAX_WIDTH + 1u;
    lib_memory_set(&destination, 0xa5, sizeof(destination));
    assert(!kvm_frame_copy(&destination, &source));
    assert(!kvm_frame_copy(&destination, LIB_NULL));
    assert(!kvm_frame_copy(LIB_NULL, &source));
    for (lib_size i = 0u; i < sizeof(destination); ++i)
        assert(((const lib_u8 *)&destination)[i] == 0xa5);
    kvm_component_mailboxes_destroy(&mailbox);
    return 0;
}
