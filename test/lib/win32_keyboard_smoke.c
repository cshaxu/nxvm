#include "lib/kvm-base/input_interface.h"
#include "lib/kvm-base/hotkey_interface.h"
#include "lib/types/win32/input.h"

#include <assert.h>

#ifdef _WIN32
typedef struct shared_keyboard_capture {
    lib_u8 keys[16];
    lib_u8 releases[16];
    lib_u8 modifiers[16];
    lib_u32 identities[16];
    lib_u32 flags[16];
    lib_u32 count;
} shared_keyboard_capture;

static lib_i32 capture_key(void *context, const kvm_input_event *event)
{
    shared_keyboard_capture *capture = (shared_keyboard_capture *)context;
    if (event == NULL || event->type != KVM_EVENT_KEY ||
        capture->count == sizeof(capture->keys)) return 0;
    capture->keys[capture->count] = (lib_u8)event->data.key.scan_code;
    capture->releases[capture->count] = (lib_u8)!event->data.key.pressed;
    capture->modifiers[capture->count] = event->data.key.modifiers;
    capture->identities[capture->count] = event->data.key.key;
    capture->flags[capture->count++] = event->data.key.flags;
    return 1;
}

typedef struct shared_hotkey_capture {
    kvm_hotkey_matcher matcher;
    kvm_input_event events[8];
    lib_u32 count;
} shared_hotkey_capture;

static lib_i32 capture_hotkey(void *context, const kvm_input_event *event)
{
    shared_hotkey_capture *capture = (shared_hotkey_capture *)context;
    if (capture == NULL || event == NULL || capture->count == 8u) return 0;
    capture->events[capture->count++] = *event;
    return 1;
}

static lib_i32 normalize_and_match(void *context, const kvm_input_event *event)
{
    shared_hotkey_capture *capture = (shared_hotkey_capture *)context;
    return capture != NULL && kvm_hotkey_matcher_submit(&capture->matcher,
        event, capture_hotkey, capture, LIB_TRUE);
}

static void assert_registered_raw_chord(lib_u32 trigger, const char *identifier)
{
    kvm_hotkey_registry registry;
    shared_hotkey_capture capture = { 0 };
    const lib_u8 control_alt = KVM_HOTKEY_MODIFIER_CONTROL |
        KVM_HOTKEY_MODIFIER_ALT;

    kvm_hotkey_registry_initialize(&registry);
    assert(kvm_hotkey_registry_register(&registry, trigger, control_alt,
        identifier) == LIB_STATUS_OK);
    kvm_hotkey_matcher_initialize(&capture.matcher, &registry);
    assert(kvm_keyboard_submit_record(&(kvm_keyboard_normalizer){ 0 }, NULL,
        &capture, normalize_and_match, &(kvm_keyboard_record){ KVM_KEYBOARD_TRANSITION,
            0x1du, LIB_WIN32_KEY_CONTROL, 0u, 0u, KVM_HOTKEY_MODIFIER_CONTROL, 1, 1u }));
    assert(kvm_keyboard_submit_record(&(kvm_keyboard_normalizer){ 0 }, NULL,
        &capture, normalize_and_match, &(kvm_keyboard_record){ KVM_KEYBOARD_TRANSITION,
            0x38u, LIB_WIN32_KEY_ALT, 0u, 0u, control_alt, 1, 1u }));
    assert(kvm_keyboard_submit_record(&(kvm_keyboard_normalizer){ 0 }, NULL,
        &capture, normalize_and_match, &(kvm_keyboard_record){ KVM_KEYBOARD_TRANSITION,
            (lib_u16)lib_win32_map_virtual_key((UINT)trigger, MAPVK_VK_TO_VSC), (lib_u16)trigger, 0u, 0u, control_alt, 1, 1u }));
    assert(capture.count == 1u && capture.events[0].type == KVM_EVENT_HOTKEY);
    assert(lib_text_compare((const char *)capture.events[0].data.hotkey.identifier, identifier) == 0);
    /* Every make and break in the matched raw chord is private to KVM. */
    assert(kvm_keyboard_submit_record(&(kvm_keyboard_normalizer){ 0 }, NULL,
        &capture, normalize_and_match, &(kvm_keyboard_record){ KVM_KEYBOARD_TRANSITION,
            (lib_u16)lib_win32_map_virtual_key((UINT)trigger, MAPVK_VK_TO_VSC), (lib_u16)trigger, 0u, 0u, control_alt, 0, 1u }));
    assert(kvm_keyboard_submit_record(&(kvm_keyboard_normalizer){ 0 }, NULL,
        &capture, normalize_and_match, &(kvm_keyboard_record){ KVM_KEYBOARD_TRANSITION,
            0x38u, LIB_WIN32_KEY_ALT, 0u, 0u, KVM_HOTKEY_MODIFIER_CONTROL, 0, 1u }));
    assert(kvm_keyboard_submit_record(&(kvm_keyboard_normalizer){ 0 }, NULL,
        &capture, normalize_and_match, &(kvm_keyboard_record){ KVM_KEYBOARD_TRANSITION,
            0x1du, LIB_WIN32_KEY_CONTROL, 0u, 0u, 0u, 0, 1u }));
    assert(capture.count == 1u);
    kvm_hotkey_matcher_discard(&capture.matcher);
}

int main(void)
{
    shared_keyboard_capture capture = { 0 };
    kvm_keyboard_normalizer normalizer = { 0 };
    shared_hotkey_capture text = { 0 };
    assert(kvm_keyboard_submit_record(&normalizer, NULL, &text, capture_hotkey,
        &(kvm_keyboard_record){ .kind=KVM_KEYBOARD_CHARACTER, .utf16=0xd83du, .repeat_count=1u }));
    assert(text.count == 0u);
    assert(kvm_keyboard_submit_record(&normalizer, NULL, &text, capture_hotkey,
        &(kvm_keyboard_record){ .kind=KVM_KEYBOARD_CHARACTER, .utf16=0xde00u, .repeat_count=1u }));
    assert(text.count == 1u && text.events[0].type == KVM_EVENT_TEXT &&
        text.events[0].data.text.scalar == 0x1f600u);
    assert(!kvm_keyboard_submit_record(&normalizer, NULL, &text, capture_hotkey,
        &(kvm_keyboard_record){ .kind=KVM_KEYBOARD_CHARACTER, .utf16=0xdc00u, .repeat_count=1u }));
    assert(kvm_keyboard_submit_record(&normalizer, NULL, &text, capture_hotkey,
        &(kvm_keyboard_record){ .kind=KVM_KEYBOARD_CHARACTER, .utf16=0xd800u, .repeat_count=1u }));
    assert(kvm_keyboard_submit_record(&normalizer, NULL, &text, capture_hotkey,
        &(kvm_keyboard_record){ .kind=KVM_KEYBOARD_CHARACTER, .utf16=0xd800u, .repeat_count=1u }));
    assert(normalizer.pending_high_surrogate == 0xd800u);
    assert(kvm_keyboard_submit_record(&normalizer, NULL, &text, capture_hotkey,
        &(kvm_keyboard_record){ .kind=KVM_KEYBOARD_CHARACTER, .utf16=0xd800u, .repeat_count=1u }));
    assert(kvm_keyboard_submit_record(&normalizer, NULL, &text, capture_hotkey,
        &(kvm_keyboard_record){ .kind=KVM_KEYBOARD_CHARACTER, .utf16='a', .repeat_count=1u }));
    assert(kvm_keyboard_submit_record(&normalizer, NULL, &text, capture_hotkey,
        &(kvm_keyboard_record){ .kind=KVM_KEYBOARD_CHARACTER, .utf16=0x4e00u, .repeat_count=1u }));
    text.count = 8u;
    assert(!kvm_keyboard_submit_record(&normalizer, NULL, &text, capture_hotkey,
        &(kvm_keyboard_record){ .kind=KVM_KEYBOARD_CHARACTER, .utf16=0x4e00u, .repeat_count=1u }));

    /* The shared component preserves the host physical scan; each project maps it. */
    assert(kvm_keyboard_submit_record(&(kvm_keyboard_normalizer){ 0 }, NULL,
        &capture, capture_key, &(kvm_keyboard_record){ KVM_KEYBOARD_TRANSITION,
            0x1eu, 'A', 0u, 0u, 0u, 1, 1u }));
    assert(kvm_keyboard_submit_record(&(kvm_keyboard_normalizer){ 0 }, NULL,
        &capture, capture_key, &(kvm_keyboard_record){ KVM_KEYBOARD_TRANSITION,
            0x1eu, 'A', 0u, 0u, 0u, 0, 1u }));
    assert(capture.count == 2u);
    assert(capture.keys[0] == 0x1eu && capture.releases[0] == 0u);
    assert(capture.keys[1] == 0x1eu && capture.releases[1] == 1u);
    assert(capture.identities[0] == 'A' && capture.identities[1] == 'A');

    /* Esc is an ordinary original key-table entry (key 110), not a host
       stop command. */
    capture.count = 0u;
    assert(kvm_keyboard_submit_record(&(kvm_keyboard_normalizer){ 0 }, NULL,
        &capture, capture_key, &(kvm_keyboard_record){ KVM_KEYBOARD_TRANSITION,
            0x01u, LIB_WIN32_KEY_ESCAPE, 0u, 0u, 0u, 1, 1u }));
    assert(kvm_keyboard_submit_record(&(kvm_keyboard_normalizer){ 0 }, NULL,
        &capture, capture_key, &(kvm_keyboard_record){ KVM_KEYBOARD_TRANSITION,
            0x01u, LIB_WIN32_KEY_ESCAPE, 0u, 0u, 0u, 0, 1u }));
    assert(capture.count == 2u);
    assert(capture.keys[0] == 0x01u && capture.releases[0] == 0u);
    assert(capture.keys[1] == 0x01u && capture.releases[1] == 1u);

    /* A raw Console record from RDP can retain VK_RETURN while reporting no
       physical scan.  Both KVM leaves use this shared recovery path, so the
       guest still receives the normal Enter make/break pair. */
    capture.count = 0u;
    assert(kvm_keyboard_submit_record(&(kvm_keyboard_normalizer){ 0 }, NULL,
        &capture, capture_key, &(kvm_keyboard_record){ KVM_KEYBOARD_TRANSITION,
            0u, LIB_WIN32_KEY_RETURN, 0u, 0u, 0u, 1, 1u }));
    assert(kvm_keyboard_submit_record(&(kvm_keyboard_normalizer){ 0 }, NULL,
        &capture, capture_key, &(kvm_keyboard_record){ KVM_KEYBOARD_TRANSITION,
            0u, LIB_WIN32_KEY_RETURN, 0u, 0u, 0u, 0, 1u }));
    assert(capture.count == 2u);
    assert(capture.keys[0] == 0x1cu && capture.releases[0] == 0u);
    assert(capture.keys[1] == 0x1cu && capture.releases[1] == 1u);
    assert(capture.identities[0] == KVM_KEY_ENTER &&
        capture.identities[1] == KVM_KEY_ENTER);

    /* Extended state is a neutral KVM flag, not a copied Win32 control bit. */
    capture.count = 0u;
    assert(kvm_keyboard_submit_record(&(kvm_keyboard_normalizer){ 0 }, NULL,
        &capture, capture_key, &(kvm_keyboard_record){ KVM_KEYBOARD_TRANSITION,
            0xe04du, LIB_WIN32_KEY_RIGHT, 0u, KVM_INPUT_FLAG_EXTENDED, 0u, 1, 1u }));
    assert(capture.count == 1u && capture.identities[0] == KVM_KEY_RIGHT);
    assert(capture.flags[0] == KVM_KEY_FLAG_EXTENDED);
    assert(capture.keys[0] == 0x4du);
    assert(kvm_keyboard_submit_record(&(kvm_keyboard_normalizer){ 0 }, NULL,
        &capture, capture_key, &(kvm_keyboard_record){ KVM_KEYBOARD_TRANSITION,
            0x4du, LIB_WIN32_KEY_RIGHT, 0u, KVM_INPUT_FLAG_EXTENDED, 0u, 1, 1u }));
    assert(capture.keys[1] == capture.keys[0] && capture.flags[1] == capture.flags[0]);

    /* Raw Console input provides its own per-record modifier state.  These
       registrations must not depend on process-global GetKeyState(), which
       RDP does not reliably update for INPUT_RECORD delivery. */
    assert_registered_raw_chord('P', "pause-toggle");
    assert_registered_raw_chord('D', "send-ctrl-alt-del");
    assert_registered_raw_chord('F', "send-alt-enter");

    /* Window supplies a mask at its own native boundary; the common
       normalizer preserves that value rather than replacing it globally. */
    capture.count = 0u;
    assert(kvm_keyboard_submit_record(&(kvm_keyboard_normalizer){ 0 }, NULL,
        &capture, capture_key, &(kvm_keyboard_record){ KVM_KEYBOARD_TRANSITION,
            0x19u, 'P', 0u, 0u, KVM_HOTKEY_MODIFIER_CONTROL |
        KVM_HOTKEY_MODIFIER_ALT, 1, 1u }));
    assert(capture.count == 1u);
    assert(capture.modifiers[0] == (KVM_HOTKEY_MODIFIER_CONTROL |
        KVM_HOTKEY_MODIFIER_ALT));

    /* Only unmapped transitions ask the native adapter for character translation. */
    capture.count = 0;
    kvm_keyboard_record record = { .kind = KVM_KEYBOARD_TRANSITION, .key = 'A', .pressed = 1 };
    assert(kvm_keyboard_submit_record(&normalizer, NULL, &capture, capture_key, &record) ==
        KVM_KEYBOARD_ACCEPTED);
    record.pressed = 0;
    assert(kvm_keyboard_submit_record(&normalizer, NULL, &capture, capture_key, &record) ==
        KVM_KEYBOARD_ACCEPTED);
    assert(capture.count == 2);
    record.key = VK_PACKET; record.pressed = 1;
    assert(kvm_keyboard_submit_record(&normalizer, NULL, &capture, capture_key, &record) ==
        KVM_KEYBOARD_UNMAPPED);
    assert(capture.count == 2);

    /* UTF-16 input uses the active host layout to synthesize make/break;
       it never places text directly in guest memory. */
    capture.count = 0u;
    assert(kvm_keyboard_submit_record(&normalizer, NULL, &capture, capture_key,
        &(kvm_keyboard_record){ .kind=KVM_KEYBOARD_CHARACTER, .utf16=L'a', .repeat_count=1u }));
    assert(capture.count == 2u);
    assert(capture.keys[0] == 0x1eu && capture.releases[0] == 0u);
    assert(capture.keys[1] == 0x1eu && capture.releases[1] == 1u);
    return 0;
}
#else
int main(void) { return 0; }
#endif
