#include "lib/kvm-base/frame_interface.h"
#include "lib/kvm-base/hotkey_interface.h"

#include <string.h>

typedef struct capture { kvm_input_event event; int count; } capture;
static int sink(void *opaque, const kvm_input_event *event)
{ capture *out = opaque; if (out == LIB_NULL || event == LIB_NULL) return 0; out->event = *event; ++out->count; return 1; }

int main(void)
{
    kvm_hotkey_registry registry;
    kvm_hotkey_matcher matcher;
    kvm_input_event key = { 0 };
    kvm_frame frame = { 0 };
    capture captured = { 0 };
    kvm_hotkey_registry_initialize(&registry);
    if (kvm_hotkey_registry_register(&registry, 'P', KVM_HOTKEY_MODIFIER_CONTROL |
            KVM_HOTKEY_MODIFIER_ALT, "pause") != LIB_STATUS_OK) return 1;
    kvm_hotkey_matcher_initialize(&matcher, &registry);
    key.type = KVM_EVENT_KEY; key.data.key.key = 'P'; key.data.key.pressed = LIB_TRUE;
    key.data.key.modifiers = KVM_KEY_MODIFIER_CONTROL | KVM_KEY_MODIFIER_ALT;
    if (!kvm_hotkey_matcher_submit(&matcher, &key, sink, &captured, LIB_TRUE) || captured.count != 1 ||
        captured.event.type != KVM_EVENT_HOTKEY || strcmp(captured.event.data.hotkey.identifier, "pause") != 0) return 1;
    frame.valid = 1u; frame.text_columns = KVM_TEXT_COLUMNS; frame.text_rows = KVM_TEXT_ROWS;
    frame.text_palette[1] = 0x00ff0000u;
    return kvm_frame_is_valid(&frame) ? 0 : 1;
}
