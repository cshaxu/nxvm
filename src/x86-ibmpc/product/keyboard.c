#include "lib/types/types_interface.h"
#include "x86-ibmpc/product/keyboard_interface.h"

#include "emulator/machine/machine_interface.h"

static const x86_ibmpc_product_hotkey x86_ibmpc_product_keyboard_builtin[] = {
    {{"Ctrl+Alt+P", "pause or resume"},
        {'P', BASE_KEY_MODIFIER_CONTROL | BASE_KEY_MODIFIER_ALT, "pause-toggle"}},
    {{"Ctrl+Alt+D", "send Ctrl+Alt+Del to the guest"},
        {'D', BASE_KEY_MODIFIER_CONTROL | BASE_KEY_MODIFIER_ALT, "send-ctrl-alt-del"}},
    {{"Ctrl+Alt+F", "send Alt+Enter to the guest"},
        {'F', BASE_KEY_MODIFIER_CONTROL | BASE_KEY_MODIFIER_ALT, "send-alt-enter"}},
    {{"Ctrl+Alt+T", "send Alt+Tab to the guest"},
        {'T', BASE_KEY_MODIFIER_CONTROL | BASE_KEY_MODIFIER_ALT, "send-alt-tab"}},
    {{"Ctrl+Alt+M", "release captured mouse"},
        {'M', BASE_KEY_MODIFIER_CONTROL | BASE_KEY_MODIFIER_ALT, "release-window-mouse"}}
};

static lib_status x86_ibmpc_product_keyboard_register(kvm_hotkey_registry *registry,
    const kvm_hotkey_registration *binding)
{
    const char *identifier = (const char *)binding->identifier;

    if (identifier[0] == '\0' ||
        lib_memory_find(identifier, '\0', sizeof(binding->identifier)) == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    for (lib_u32 index = 0u; index < registry->count; ++index)
        if (lib_text_compare(identifier, (const char *)registry->entries[index].identifier) == 0)
            return LIB_STATUS_INVALID_STATE;
    return kvm_hotkey_registry_register(registry, binding->key, binding->modifiers, identifier);
}

lib_status x86_ibmpc_product_keyboard_configure(x86_ibmpc_product_command_context *command)
{
    const lib_size builtins = sizeof(x86_ibmpc_product_keyboard_builtin) /
        sizeof(x86_ibmpc_product_keyboard_builtin[0]);
    const x86_ibmpc_product_command_extensions *extensions = &command->extensions;

    if (extensions->hotkey_count > KVM_HOTKEY_CAPACITY - builtins)
        return LIB_STATUS_LIMIT_EXCEEDED;
    if (extensions->hotkey_count != 0u && extensions->hotkeys == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;

    kvm_hotkey_registry_initialize(&command->hotkey_registry);
    for (lib_size index = 0u; index < builtins + extensions->hotkey_count; ++index) {
        const x86_ibmpc_product_hotkey *hotkey = index < builtins ?
            &x86_ibmpc_product_keyboard_builtin[index] : &extensions->hotkeys[index - builtins];

        if (hotkey->registration.key != 0u) {
            lib_status status;

            if (index >= builtins && extensions->submit_hotkey == LIB_NULL)
                return LIB_STATUS_INVALID_ARGUMENT;
            status = x86_ibmpc_product_keyboard_register(&command->hotkey_registry,
                &hotkey->registration);
            if (status != LIB_STATUS_OK) return status;
        }
        command->hotkey_help[index] = hotkey->help;
    }
    command->monitor.hotkeys = (emulator_product_help_map){
        command->hotkey_help, builtins + extensions->hotkey_count};
    return LIB_STATUS_OK;
}

lib_bool x86_ibmpc_product_keyboard_deliver_input(void *context, const kvm_input_event *event)
{
    lib_status status = emulator_machine_enqueue_input((emulator_machine *)context, event);
    return status == LIB_STATUS_OK || status == LIB_STATUS_INVALID_STATE;
}

static lib_bool x86_ibmpc_product_keyboard_emit(void *context, kvm_input_sink sink, lib_u32 scan,
    lib_u32 key, lib_bool pressed)
{
    kvm_input_event event = { 0 };
    if (sink == LIB_NULL) return LIB_FALSE;
    event.type = KVM_EVENT_KEY;
    event.data.key.scan_code = scan & 0xffu;
    event.data.key.key = key;
    event.data.key.flags = (scan & 0x0100u) != 0u ? BASE_KEY_FLAG_EXTENDED : 0u;
    event.data.key.pressed = pressed;
    return sink(context, &event);
}

lib_bool x86_ibmpc_product_keyboard_release_ctrl_alt(void *context, kvm_input_sink sink)
{
    return x86_ibmpc_product_keyboard_emit(context, sink, 0x1du, BASE_KEY_CONTROL, LIB_FALSE) &&
        x86_ibmpc_product_keyboard_emit(context, sink, 0x38u, BASE_KEY_ALT, LIB_FALSE);
}

lib_bool x86_ibmpc_product_keyboard_submit_ctrl_alt_del(void *context, kvm_input_sink sink)
{
    return x86_ibmpc_product_keyboard_emit(context, sink, 0x1du, BASE_KEY_CONTROL, LIB_TRUE) &&
        x86_ibmpc_product_keyboard_emit(context, sink, 0x38u, BASE_KEY_ALT, LIB_TRUE) &&
        x86_ibmpc_product_keyboard_emit(context, sink, 0x0153u, BASE_KEY_DELETE, LIB_TRUE) &&
        x86_ibmpc_product_keyboard_emit(context, sink, 0x0153u, BASE_KEY_DELETE, LIB_FALSE) &&
        x86_ibmpc_product_keyboard_emit(context, sink, 0x38u, BASE_KEY_ALT, LIB_FALSE) &&
        x86_ibmpc_product_keyboard_emit(context, sink, 0x1du, BASE_KEY_CONTROL, LIB_FALSE);
}

static lib_bool x86_ibmpc_product_keyboard_submit_alt_key(void *context, kvm_input_sink sink,
    lib_u32 scan, base_key key)
{
    return x86_ibmpc_product_keyboard_release_ctrl_alt(context, sink) &&
        x86_ibmpc_product_keyboard_emit(context, sink, 0x38u, BASE_KEY_ALT, LIB_TRUE) &&
        x86_ibmpc_product_keyboard_emit(context, sink, scan, key, LIB_TRUE) &&
        x86_ibmpc_product_keyboard_emit(context, sink, scan, key, LIB_FALSE) &&
        x86_ibmpc_product_keyboard_emit(context, sink, 0x38u, BASE_KEY_ALT, LIB_FALSE);
}

lib_bool x86_ibmpc_product_keyboard_handle_hotkey(emulator_machine *machine,
    emulator_session_machine_state state, const lib_u8 *identifier,
    emulator_session_command_result *out)
{
    const char *name;
    *out = (emulator_session_command_result) { 0 };
    if (identifier == LIB_NULL) return LIB_FALSE;
    name = (const char *)identifier;
    if (lib_text_compare(name, "pause-toggle") == 0) return LIB_FALSE;
    if (lib_text_compare(name, "release-window-mouse") == 0) {
        out->release_window_mouse = LIB_TRUE;
        return LIB_TRUE;
    }
    if (state != EMULATOR_SESSION_MACHINE_RUNNING) return LIB_TRUE;
    if (lib_text_compare(name, "send-ctrl-alt-del") == 0)
        return x86_ibmpc_product_keyboard_submit_ctrl_alt_del(machine,
            x86_ibmpc_product_keyboard_deliver_input);
    if (lib_text_compare(name, "send-alt-enter") == 0)
        return x86_ibmpc_product_keyboard_submit_alt_key(machine,
            x86_ibmpc_product_keyboard_deliver_input, 0x1cu, BASE_KEY_ENTER);
    if (lib_text_compare(name, "send-alt-tab") == 0)
        return x86_ibmpc_product_keyboard_submit_alt_key(machine,
            x86_ibmpc_product_keyboard_deliver_input, 0x0fu, BASE_KEY_TAB);
    return LIB_TRUE;
}
