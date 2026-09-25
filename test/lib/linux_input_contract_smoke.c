#include "lib/types/test.h"
#include "lib/kvm-base/input_interface.h"
#include "lib/kvm-base/linux/input.h"

static kvm_input_event received;
static lib_i32 capture(void *context, const kvm_input_event *event)
{ (void)context; received = *event; return 1; }
int main(void)
{
    kvm_keyboard_normalizer state = { 0 };
    lib_test_assert(kvm_keyboard_submit_record(&(kvm_keyboard_normalizer){ 0 }, LIB_NULL,
        LIB_NULL, capture, &(kvm_keyboard_record){ KVM_KEYBOARD_TRANSITION,
            0u, KVM_LINKVM_KEY_UP, 0u, 0u, 0u, 1, 1u }));
    lib_test_assert(received.type == KVM_EVENT_KEY && received.data.key.key == KVM_KEY_UP &&
        received.data.key.scan_code == 0u);
    lib_test_assert(kvm_keyboard_submit_record(&state, LIB_NULL, LIB_NULL, capture,
        &(kvm_keyboard_record){ .kind=KVM_KEYBOARD_CHARACTER, .utf16='a', .repeat_count=1u }));
    lib_test_assert(received.type == KVM_EVENT_TEXT && received.data.text.scalar == 'a');
    lib_test_assert(kvm_keyboard_submit_record(&state, LIB_NULL, LIB_NULL, capture,
        &(kvm_keyboard_record){ .kind=KVM_KEYBOARD_CHARACTER, .utf16=0xd83du, .repeat_count=1u }));
    lib_test_assert(kvm_keyboard_submit_record(&state, LIB_NULL, LIB_NULL, capture,
        &(kvm_keyboard_record){ .kind=KVM_KEYBOARD_CHARACTER, .utf16=0xde00u, .repeat_count=1u }));
    lib_test_assert(received.type == KVM_EVENT_TEXT && received.data.text.scalar == 0x1f600u);
    lib_test_assert(kvm_keyboard_submit_record(&(kvm_keyboard_normalizer){ 0 }, LIB_NULL,
        LIB_NULL, capture, &(kvm_keyboard_record){ KVM_KEYBOARD_TRANSITION,
            0u, 0xffffu, 0u, 0u, 0u, 1, 1u }) == KVM_KEYBOARD_UNMAPPED);
    return 0;
}
