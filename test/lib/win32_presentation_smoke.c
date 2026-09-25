#include "lib/types/test.h"
#include "lib/types/win32/test.h"
#include "lib/kvm-window/frame_interface.h"
#include "lib/kvm-base/hotkey_interface.h"


typedef struct kvm_capture {
    kvm_input_event events[128];
    lib_u32 count;
} kvm_capture;

static lib_bool kvm_capture_event(void *opaque, const kvm_input_event *event)
{
    kvm_capture *capture = (kvm_capture *)opaque;
    if (capture == LIB_NULL || event == LIB_NULL || capture->count == 128u) return LIB_FALSE;
    capture->events[capture->count++] = *event;
    return LIB_TRUE;
}

int main(void)
{
    kvm_window_frame *frame = lib_allocate_zero(1u, sizeof(*frame));
    kvm_hotkey_registry registry;
    kvm_hotkey_matcher matcher;
    kvm_capture capture = { 0 };
    kvm_input_event event = { 0 };

    lib_test_assert(frame != LIB_NULL);
    frame->valid = LIB_TRUE;
    frame->graphics = 0u;
    frame->text.base.text_columns = 80u;
    frame->text.base.text_rows = 25u;
    frame->text.base.cells[0].glyph_index = 'X';
    lib_test_assert(kvm_window_frame_validate(frame) == LIB_STATUS_OK);

    kvm_hotkey_registry_initialize(&registry);
    lib_test_assert(kvm_hotkey_registry_register(&registry, 'P',
        KVM_HOTKEY_MODIFIER_CONTROL | KVM_HOTKEY_MODIFIER_ALT,
        "pause-toggle") == LIB_STATUS_OK);
    kvm_hotkey_matcher_initialize(&matcher, &registry);
    event.type = KVM_EVENT_KEY;
    event.data.key.key = KVM_HOTKEY_KEY_CONTROL;
    event.data.key.scan_code = 0x1du;
    event.data.key.pressed = LIB_TRUE;
    event.data.key.modifiers = KVM_HOTKEY_MODIFIER_CONTROL;
    lib_test_assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event,
        &capture, LIB_TRUE));
    event.data.key.key = KVM_HOTKEY_KEY_ALT;
    event.data.key.flags = 0u;
    event.data.key.scan_code = 0x38u;
    event.data.key.modifiers = KVM_HOTKEY_MODIFIER_CONTROL |
        KVM_HOTKEY_MODIFIER_ALT;
    lib_test_assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event,
        &capture, LIB_TRUE));
    event.type = KVM_EVENT_KEY;
    event.data.key.key = 'P';
    event.data.key.scan_code = 0x19u;
    event.data.key.pressed = LIB_TRUE;
    event.data.key.modifiers = KVM_HOTKEY_MODIFIER_CONTROL |
        KVM_HOTKEY_MODIFIER_ALT;
    lib_test_assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event,
        &capture, LIB_TRUE));
    lib_test_assert(capture.count == 1u && capture.events[0].type == KVM_EVENT_HOTKEY);
    lib_test_assert(lib_text_compare((const char *)capture.events[0].data.hotkey.identifier,
        "pause-toggle") == 0);
    /* Auto-repeat, then a second press while Ctrl/Alt stay held. Neither
       operation may forget the outstanding modifier breaks. */
    lib_test_assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    event.data.key.pressed = LIB_FALSE;
    lib_test_assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    event.data.key.pressed = LIB_TRUE;
    lib_test_assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    lib_test_assert(capture.count == 3u);
    lib_test_assert(capture.events[1].type == KVM_EVENT_HOTKEY);
    lib_test_assert(capture.events[2].type == KVM_EVENT_HOTKEY);
    event.data.key.pressed = LIB_FALSE;
    lib_test_assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event,
        &capture, LIB_TRUE));
    event.data.key.key = KVM_HOTKEY_KEY_ALT;
    event.data.key.flags = 0u;
    event.data.key.scan_code = 0x38u;
    event.data.key.pressed = LIB_TRUE;
    lib_test_assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    lib_test_assert(matcher.held_count == 2u); /* Held modifier repeat is consumed. */
    event.data.key.pressed = LIB_FALSE;
    lib_test_assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event,
        &capture, LIB_TRUE));
    event.data.key.key = KVM_HOTKEY_KEY_CONTROL;
    event.data.key.scan_code = 0x1du;
    event.data.key.modifiers = 0u;
    lib_test_assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event,
        &capture, LIB_TRUE));
    lib_test_assert(capture.count == 3u && matcher.held_count == 0u);
    /* Both physical Ctrl keys use LIB_WIN32_KEY_CONTROL, but a matched chord must
       suppress both breaks rather than leaking the second to the guest. */
    capture.count = 0u;
    kvm_hotkey_matcher_discard(&matcher);
    kvm_hotkey_matcher_initialize(&matcher, &registry);
    event.type = KVM_EVENT_KEY;
    event.data.key.key = KVM_HOTKEY_KEY_CONTROL;
    event.data.key.scan_code = 0x1du;
    event.data.key.pressed = LIB_TRUE;
    event.data.key.modifiers = KVM_HOTKEY_MODIFIER_CONTROL;
    lib_test_assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    event.data.key.flags = KVM_KEY_FLAG_EXTENDED;
    lib_test_assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    event.data.key.key = KVM_HOTKEY_KEY_ALT;
    event.data.key.flags = 0u;
    event.data.key.scan_code = 0x38u;
    event.data.key.modifiers = KVM_HOTKEY_MODIFIER_CONTROL |
        KVM_HOTKEY_MODIFIER_ALT;
    lib_test_assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    event.data.key.key = 'P';
    event.data.key.scan_code = 0x19u;
    lib_test_assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    lib_test_assert(capture.count == 1u && capture.events[0].type == KVM_EVENT_HOTKEY);
    event.data.key.pressed = LIB_FALSE;
    lib_test_assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    event.data.key.key = KVM_HOTKEY_KEY_ALT;
    event.data.key.flags = 0u;
    event.data.key.scan_code = 0x38u;
    lib_test_assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    event.data.key.key = KVM_HOTKEY_KEY_CONTROL;
    event.data.key.scan_code = 0x1du;
    lib_test_assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    event.data.key.flags = KVM_KEY_FLAG_EXTENDED;
    lib_test_assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    lib_test_assert(capture.count == 1u);
    event.data.key.flags = 0u;
    /* An uncompleted registered prefix is never swallowed: the original
       modifier and the mismatching key replay in their source order. */
    capture.count = 0u;
    kvm_hotkey_matcher_discard(&matcher);
    kvm_hotkey_matcher_initialize(&matcher, &registry);
    event.type = KVM_EVENT_KEY;
    event.data.key.key = KVM_HOTKEY_KEY_CONTROL;
    event.data.key.pressed = LIB_TRUE;
    event.data.key.modifiers = KVM_HOTKEY_MODIFIER_CONTROL;
    lib_test_assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    event.data.key.key = 'X';
    event.data.key.scan_code = 0x2du;
    event.data.key.modifiers = KVM_HOTKEY_MODIFIER_CONTROL;
    lib_test_assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    lib_test_assert(capture.count == 2u);
    lib_test_assert(capture.events[0].data.key.key == KVM_HOTKEY_KEY_CONTROL);
    lib_test_assert(capture.events[1].data.key.key == 'X');
    /* Once Ctrl was delivered with X, later Alt/P cannot consume that make. */
    event.data.key.key = KVM_KEY_ALT; event.data.key.scan_code = 0x38;
    event.data.key.modifiers = 3;
    lib_test_assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    event.data.key.key = 'P'; event.data.key.scan_code = 0x19;
    lib_test_assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    lib_test_assert(capture.count == 4 && capture.events[3].type == KVM_EVENT_KEY);

    /* Repeated pending make consumes no new physical-key slot. */
    capture.count = 0;
    kvm_hotkey_matcher_discard(&matcher);
    kvm_hotkey_matcher_initialize(&matcher, &registry);
    event.data.key.key = KVM_KEY_CONTROL; event.data.key.scan_code = 0x1d;
    event.data.key.modifiers = 1;
    for (lib_u32 i = 0; i != 100; ++i)
        lib_test_assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    lib_test_assert(matcher.held_count == 1 && capture.count == 0);
    event.data.key.key = KVM_KEY_ALT; event.data.key.scan_code = 0x38;
    event.data.key.modifiers = 3;
    lib_test_assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    event.data.key.key = 'P'; event.data.key.scan_code = 0x19;
    lib_test_assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    lib_test_assert(capture.count == 1 && capture.events[0].type == KVM_EVENT_HOTKEY);
    /* Many distinct held physical keys grow the single ledger; repeats do not. */
    capture.count = 0u;
    kvm_hotkey_matcher_discard(&matcher);
    kvm_hotkey_matcher_initialize(&matcher, &registry);
    event.data.key.key = 'P';
    event.data.key.modifiers = KVM_HOTKEY_MODIFIER_CONTROL | KVM_HOTKEY_MODIFIER_ALT;
    for (lib_u32 i = 0; i < 64; ++i) {
        event.data.key.scan_code = (lib_u16)(i + 1);
        lib_test_assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    }
    lib_test_assert(matcher.held_count == 64 && capture.count == 64);
    event.data.key.pressed = LIB_FALSE;
    for (lib_u32 i = 0; i < 64; ++i) {
        event.data.key.scan_code = (lib_u16)(i + 1);
        lib_test_assert(kvm_hotkey_matcher_submit(&matcher, &event, kvm_capture_event, &capture, LIB_TRUE));
    }
    lib_test_assert(matcher.held_count == 0 && capture.count == 64);
    kvm_hotkey_matcher_discard(&matcher);
    lib_release(frame);
    return 0;
}
