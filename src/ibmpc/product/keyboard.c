#include "lib/types/types_interface.h"

#include "ibmpc/product/keyboard.h"

static void vm_app_keyboard_clear_result(common_session_command_result *result)
{
    if (result != LIB_NULL) *result = (common_session_command_result){0};
}

static lib_bool vm_app_keyboard_text_equal(const lib_u8 *left, const char *right)
{
    while (*left != '\0' && *right != '\0') {
        if (*left++ != (lib_u8)*right++) return LIB_FALSE;
    }
    return *left == '\0' && *right == '\0';
}

static void vm_app_keyboard_submit_chord(common_machine *machine, lib_bool cad)
{
    const lib_u16 scan[] = { cad ? 0x1du : 0x38u, cad ? 0x38u : 0x1cu, 0x153u };
    const kvm_key key[] = { cad ? KVM_KEY_CONTROL : KVM_KEY_ALT,
        cad ? KVM_KEY_ALT : KVM_KEY_ENTER, KVM_KEY_DELETE };
    lib_u32 count = cad ? 3u : 2u;
    lib_u32 index;

    if (machine == LIB_NULL) return;
    for (index = 0u; index < count; ++index) {
        kvm_input_event input = {0};
        input.type = KVM_EVENT_KEY;
        input.data.key.scan_code = scan[index];
        input.data.key.key = key[index];
        input.data.key.pressed = LIB_TRUE;
        (void)common_machine_enqueue_input(machine, &input);
    }
    for (index = count; index-- != 0u;) {
        kvm_input_event input = {0};
        input.type = KVM_EVENT_KEY;
        input.data.key.scan_code = scan[index];
        input.data.key.key = key[index];
        input.data.key.pressed = LIB_FALSE;
        (void)common_machine_enqueue_input(machine, &input);
    }
}

void vm_app_keyboard_register_hotkeys(kvm_hotkey_registry *registry)
{
    if (registry == LIB_NULL) return;
    kvm_hotkey_registry_initialize(registry);
    (void)kvm_hotkey_registry_register(registry, 'P',
        KVM_HOTKEY_MODIFIER_CONTROL | KVM_HOTKEY_MODIFIER_ALT, "pause");
    (void)kvm_hotkey_registry_register(registry, 'D',
        KVM_HOTKEY_MODIFIER_CONTROL | KVM_HOTKEY_MODIFIER_ALT, "cad");
    (void)kvm_hotkey_registry_register(registry, 'F',
        KVM_HOTKEY_MODIFIER_CONTROL | KVM_HOTKEY_MODIFIER_ALT, "alt-enter");
    (void)kvm_hotkey_registry_register(registry, 'M',
        KVM_HOTKEY_MODIFIER_CONTROL | KVM_HOTKEY_MODIFIER_ALT, "release-mouse");
}

lib_bool vm_app_keyboard_handle_hotkey(common_machine *machine,
    common_session_machine_state state, const lib_u8 *identifier,
    common_session_command_result *result)
{
    vm_app_keyboard_clear_result(result);
    if (identifier == LIB_NULL || result == LIB_NULL) return LIB_FALSE;
    if (vm_app_keyboard_text_equal(identifier, "pause")) {
        result->request = state == COMMON_SESSION_MACHINE_RUNNING ?
            COMMON_SESSION_REQUEST_PAUSE : COMMON_SESSION_REQUEST_RESUME;
        return LIB_TRUE;
    }
    if (vm_app_keyboard_text_equal(identifier, "release-mouse")) {
        result->release_window_mouse = LIB_TRUE;
        return LIB_TRUE;
    }
    if (vm_app_keyboard_text_equal(identifier, "cad") ||
        vm_app_keyboard_text_equal(identifier, "alt-enter")) {
        vm_app_keyboard_submit_chord(machine,
            vm_app_keyboard_text_equal(identifier, "cad"));
        return LIB_TRUE;
    }
    return LIB_FALSE;
}
