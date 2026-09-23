#include "type.h"

#include "app-nxvm/product/keyboard.h"
#include "app-nxvm/machine/machine_interface.h"

static C_VOID vm_app_keyboard_clear_result(common_session_command_result *result)
{
    if (result != LIB_NULL) *result = (common_session_command_result){0};
}

static C_VOID vm_app_keyboard_submit_chord(vm_machine *machine, C_INT cad)
{
    const type_unsigned_16 scan[] = { cad ? 0x1du : 0x38u, 0x38u,
        cad ? 0x153u : 0u };
    const type_unsigned_16 key[] = { cad ? 0x11u : 0x12u, 0x12u,
        cad ? 0x2eu : 0u };
    type_unsigned_32 count = cad ? 3u : 2u;
    type_unsigned_32 index;

    if (machine == STD_NULL) return;
    for (index = 0u; index < count; ++index) {
        vm_machine_input input = {0};
        input.kind = VM_MACHINE_INPUT_KEY_EVENT;
        input.data.key_event.scan_code = scan[index];
        input.data.key_event.virtual_key = key[index];
        input.data.key_event.pressed = TYPE_TRUE;
        (C_VOID)vm_machine_submit_input(machine, &input);
    }
    for (index = count; index-- != 0u;) {
        vm_machine_input input = {0};
        input.kind = VM_MACHINE_INPUT_KEY_EVENT;
        input.data.key_event.scan_code = scan[index];
        input.data.key_event.virtual_key = key[index];
        input.data.key_event.pressed = TYPE_FALSE;
        (C_VOID)vm_machine_submit_input(machine, &input);
    }
}

C_VOID vm_app_keyboard_register_hotkeys(kvm_hotkey_registry *registry)
{
    if (registry == STD_NULL) return;
    kvm_hotkey_registry_initialize(registry);
    (C_VOID)kvm_hotkey_registry_register(registry, 'P',
        KVM_HOTKEY_MODIFIER_CONTROL | KVM_HOTKEY_MODIFIER_ALT, "pause");
    (C_VOID)kvm_hotkey_registry_register(registry, 'D',
        KVM_HOTKEY_MODIFIER_CONTROL | KVM_HOTKEY_MODIFIER_ALT, "cad");
    (C_VOID)kvm_hotkey_registry_register(registry, 'F',
        KVM_HOTKEY_MODIFIER_CONTROL | KVM_HOTKEY_MODIFIER_ALT, "alt-enter");
    (C_VOID)kvm_hotkey_registry_register(registry, 'M',
        KVM_HOTKEY_MODIFIER_CONTROL | KVM_HOTKEY_MODIFIER_ALT, "release-mouse");
}

lib_bool vm_app_keyboard_handle_hotkey(vm_machine *machine,
    common_session_machine_state state, const C_CHAR *identifier,
    common_session_command_result *result)
{
    vm_app_keyboard_clear_result(result);
    if (identifier == STD_NULL || result == LIB_NULL) return LIB_FALSE;
    if (!STD_STRCMP(identifier, "pause")) {
        result->request = state == COMMON_SESSION_MACHINE_RUNNING ?
            COMMON_SESSION_REQUEST_PAUSE : COMMON_SESSION_REQUEST_RESUME;
        return LIB_TRUE;
    }
    if (!STD_STRCMP(identifier, "release-mouse")) {
        result->release_window_mouse = LIB_TRUE;
        return LIB_TRUE;
    }
    if (!STD_STRCMP(identifier, "cad") || !STD_STRCMP(identifier, "alt-enter")) {
        vm_app_keyboard_submit_chord(machine, !STD_STRCMP(identifier, "cad"));
        return LIB_TRUE;
    }
    return LIB_FALSE;
}
