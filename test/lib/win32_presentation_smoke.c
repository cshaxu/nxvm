#include "lib/kvm-window/frame_interface.h"
#include "lib/kvm-base/hotkey_interface.h"

#include <assert.h>
#include <stdlib.h>

typedef struct kvm_capture {
    kvm_input_event events[128];
    lib_u32 count;
} kvm_capture;

static lib_i32 kvm_capture_event(void *opaque, const kvm_input_event *event)
{
    kvm_capture *capture = (kvm_capture *)opaque;
    if (capture == NULL || event == NULL || capture->count == 128u) return 0;
    capture->events[capture->count++] = *event;
    return 1;
}

int main(void)
{
    kvm_window_frame *frame = lib_allocate_zero(1u, sizeof(*frame));
    kvm_hotkey_registry registry;
    kvm_hotkey_matcher matcher;
    kvm_capture capture = { 0 };
    kvm_input_event event = { 0 };

    assert(frame != NULL);
    frame->valid = 1u;
    frame->graphics = 0u;
    frame->text.base.text_columns = 80u;
    frame->text.base.text_rows = 25u;
    frame->text.base.cells[0].glyph_index = 'X';
    assert(kvm_window_frame_validate(frame) == LIB_STATUS_OK);

    kvm_hotkey_registry_initialize(&registry);
    assert(kvm_hotkey_registry_register(&registry, 'P',
        KVM_HOTKEY_MODIFIER_CONTROL | KVM_HOTKEY_MODIFIER_ALT,
        "pause-toggle") == LIB_STATUS_OK);
    kvm_hotkey_matcher_initialize(&matcher, &registry);
    event.type = KVM_EVENT_KEY;
    event.data.key.key = KVM_HOTKEY_KEY_CONTROL;
    event.data.key.scan_code = 0x1du;
    event.data.key.pressed = 1u;
    event.data.key.modifiers = KVM_HOTKEY_MODIFIER_CONTROL;
    assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event,
        &capture, LIB_TRUE));
    event.data.key.key = KVM_HOTKEY_KEY_ALT;
    event.data.key.flags = 0u;
    event.data.key.scan_code = 0x38u;
    event.data.key.modifiers = KVM_HOTKEY_MODIFIER_CONTROL |
        KVM_HOTKEY_MODIFIER_ALT;
    assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event,
        &capture, LIB_TRUE));
    event.type = KVM_EVENT_KEY;
    event.data.key.key = 'P';
    event.data.key.scan_code = 0x19u;
    event.data.key.pressed = 1u;
    event.data.key.modifiers = KVM_HOTKEY_MODIFIER_CONTROL |
        KVM_HOTKEY_MODIFIER_ALT;
    assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event,
        &capture, LIB_TRUE));
    assert(capture.count == 1u && capture.events[0].type == KVM_EVENT_HOTKEY);
    assert(lib_text_compare((const char *)capture.events[0].data.hotkey.identifier,
        "pause-toggle") == 0);
    /* Auto-repeat, then a second press while Ctrl/Alt stay held. Neither
       operation may forget the outstanding modifier breaks. */
    assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    event.data.key.pressed = 0u;
    assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    event.data.key.pressed = 1u;
    assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    assert(capture.count == 3u);
    assert(capture.events[1].type == KVM_EVENT_HOTKEY);
    assert(capture.events[2].type == KVM_EVENT_HOTKEY);
    event.data.key.pressed = 0u;
    assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event,
        &capture, LIB_TRUE));
    event.data.key.key = KVM_HOTKEY_KEY_ALT;
    event.data.key.flags = 0u;
    event.data.key.scan_code = 0x38u;
    event.data.key.pressed = 1u;
    assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    assert(matcher.held_count == 2u); /* Held modifier repeat is consumed. */
    event.data.key.pressed = 0u;
    assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event,
        &capture, LIB_TRUE));
    event.data.key.key = KVM_HOTKEY_KEY_CONTROL;
    event.data.key.scan_code = 0x1du;
    event.data.key.modifiers = 0u;
    assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event,
        &capture, LIB_TRUE));
    assert(capture.count == 3u && matcher.held_count == 0u);
    /* Both physical Ctrl keys use VK_CONTROL, but a matched chord must
       suppress both breaks rather than leaking the second to the guest. */
    capture.count = 0u;
    kvm_hotkey_matcher_discard(&matcher);
    kvm_hotkey_matcher_initialize(&matcher, &registry);
    event.type = KVM_EVENT_KEY;
    event.data.key.key = KVM_HOTKEY_KEY_CONTROL;
    event.data.key.scan_code = 0x1du;
    event.data.key.pressed = 1u;
    event.data.key.modifiers = KVM_HOTKEY_MODIFIER_CONTROL;
    assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    event.data.key.flags = KVM_KEY_FLAG_EXTENDED;
    assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    event.data.key.key = KVM_HOTKEY_KEY_ALT;
    event.data.key.flags = 0u;
    event.data.key.scan_code = 0x38u;
    event.data.key.modifiers = KVM_HOTKEY_MODIFIER_CONTROL |
        KVM_HOTKEY_MODIFIER_ALT;
    assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    event.data.key.key = 'P';
    event.data.key.scan_code = 0x19u;
    assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    assert(capture.count == 1u && capture.events[0].type == KVM_EVENT_HOTKEY);
    event.data.key.pressed = 0u;
    assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    event.data.key.key = KVM_HOTKEY_KEY_ALT;
    event.data.key.flags = 0u;
    event.data.key.scan_code = 0x38u;
    assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    event.data.key.key = KVM_HOTKEY_KEY_CONTROL;
    event.data.key.scan_code = 0x1du;
    assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    event.data.key.flags = KVM_KEY_FLAG_EXTENDED;
    assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    assert(capture.count == 1u);
    event.data.key.flags = 0u;
    /* An uncompleted registered prefix is never swallowed: the original
       modifier and the mismatching key replay in their source order. */
    capture.count = 0u;
    kvm_hotkey_matcher_discard(&matcher);
    kvm_hotkey_matcher_initialize(&matcher, &registry);
    event.type = KVM_EVENT_KEY;
    event.data.key.key = KVM_HOTKEY_KEY_CONTROL;
    event.data.key.pressed = 1u;
    event.data.key.modifiers = KVM_HOTKEY_MODIFIER_CONTROL;
    assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    event.data.key.key = 'X';
    event.data.key.scan_code = 0x2du;
    event.data.key.modifiers = KVM_HOTKEY_MODIFIER_CONTROL;
    assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    assert(capture.count == 2u);
    assert(capture.events[0].data.key.key == KVM_HOTKEY_KEY_CONTROL);
    assert(capture.events[1].data.key.key == 'X');
    /* Once Ctrl was delivered with X, later Alt/P cannot consume that make. */
    event.data.key.key = KVM_KEY_ALT; event.data.key.scan_code = 0x38;
    event.data.key.modifiers = 3;
    assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    event.data.key.key = 'P'; event.data.key.scan_code = 0x19;
    assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    assert(capture.count == 4 && capture.events[3].type == KVM_EVENT_KEY);

    /* Repeated pending make consumes no new physical-key slot. */
    capture.count = 0;
    kvm_hotkey_matcher_discard(&matcher);
    kvm_hotkey_matcher_initialize(&matcher, &registry);
    event.data.key.key = KVM_KEY_CONTROL; event.data.key.scan_code = 0x1d;
    event.data.key.modifiers = 1;
    for (lib_u32 i = 0; i != 100; ++i)
        assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    assert(matcher.held_count == 1 && capture.count == 0);
    event.data.key.key = KVM_KEY_ALT; event.data.key.scan_code = 0x38;
    event.data.key.modifiers = 3;
    assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    event.data.key.key = 'P'; event.data.key.scan_code = 0x19;
    assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    assert(capture.count == 1 && capture.events[0].type == KVM_EVENT_HOTKEY);
    /* Many distinct held physical keys grow the single ledger; repeats do not. */
    capture.count = 0u;
    kvm_hotkey_matcher_discard(&matcher);
    kvm_hotkey_matcher_initialize(&matcher, &registry);
    event.data.key.key = 'P';
    event.data.key.modifiers = KVM_HOTKEY_MODIFIER_CONTROL | KVM_HOTKEY_MODIFIER_ALT;
    for (lib_u32 i = 0; i < 64; ++i) {
        event.data.key.scan_code = (lib_u16)(i + 1);
        assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    }
    assert(matcher.held_count == 64 && capture.count == 64);
    event.data.key.pressed = 0;
    for (lib_u32 i = 0; i < 64; ++i) {
        event.data.key.scan_code = (lib_u16)(i + 1);
        assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    }
    assert(matcher.held_count == 0 && capture.count == 64);
    kvm_hotkey_matcher_discard(&matcher);
    lib_release(frame);
    return 0;
}
