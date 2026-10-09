#include "lib/types/types_interface.h"
#include "ibmpc/product/keyboard_interface.h"

#include "common/machine/machine_interface.h"

lib_bool app_keyboard_deliver_input(void *context, const kvm_input_event *event)
{
    return context != NULL && event != NULL && common_machine_enqueue_input(
        (common_machine *)context, event) != 0;
}

lib_bool app_keyboard_hotkeys(kvm_hotkey_registry *registry)
{
    static const kvm_hotkey_registry hotkeys = {
        { { 'P', KVM_HOTKEY_MODIFIER_CONTROL | KVM_HOTKEY_MODIFIER_ALT,
              "pause-toggle" },
          { 'D', KVM_HOTKEY_MODIFIER_CONTROL | KVM_HOTKEY_MODIFIER_ALT,
              "send-ctrl-alt-del" },
          { 'F', KVM_HOTKEY_MODIFIER_CONTROL | KVM_HOTKEY_MODIFIER_ALT,
              "send-alt-enter" },
          { 'T', KVM_HOTKEY_MODIFIER_CONTROL | KVM_HOTKEY_MODIFIER_ALT,
              "send-alt-tab" },
          { 'M', KVM_HOTKEY_MODIFIER_CONTROL | KVM_HOTKEY_MODIFIER_ALT,
              "release-window-mouse" } }, 5u };

    if (registry == NULL) return LIB_FALSE;
    *registry = hotkeys;
    return LIB_TRUE;
}

static lib_bool app_keyboard_emit(void *context, kvm_input_sink sink, lib_u32 scan,
    lib_u32 key, lib_bool pressed)
{
    kvm_input_event event = { 0 };
    if (sink == NULL) return LIB_FALSE;
    event.type = KVM_EVENT_KEY;
    event.data.key.scan_code = scan;
    event.data.key.key = key;
    event.data.key.flags = (scan & 0x0100u) != 0u ? KVM_KEY_FLAG_EXTENDED : 0u;
    event.data.key.pressed = pressed;
    return sink(context, &event);
}

lib_bool app_keyboard_release_ctrl_alt(void *context, kvm_input_sink sink)
{
    return app_keyboard_emit(context, sink, 0x1du, KVM_KEY_CONTROL, LIB_FALSE) &&
        app_keyboard_emit(context, sink, 0x38u, KVM_KEY_ALT, LIB_FALSE);
}

lib_bool app_keyboard_submit_ctrl_alt_del(void *context, kvm_input_sink sink)
{
    return app_keyboard_emit(context, sink, 0x1du, KVM_KEY_CONTROL, LIB_TRUE) &&
        app_keyboard_emit(context, sink, 0x38u, KVM_KEY_ALT, LIB_TRUE) &&
        app_keyboard_emit(context, sink, 0x0153u, KVM_KEY_DELETE, LIB_TRUE) &&
        app_keyboard_emit(context, sink, 0x0153u, KVM_KEY_DELETE, LIB_FALSE) &&
        app_keyboard_emit(context, sink, 0x38u, KVM_KEY_ALT, LIB_FALSE) &&
        app_keyboard_emit(context, sink, 0x1du, KVM_KEY_CONTROL, LIB_FALSE);
}

static lib_bool app_keyboard_submit_alt_key(void *context, kvm_input_sink sink,
    lib_u32 scan, kvm_key key)
{
    return app_keyboard_release_ctrl_alt(context, sink) &&
        app_keyboard_emit(context, sink, 0x38u, KVM_KEY_ALT, LIB_TRUE) &&
        app_keyboard_emit(context, sink, scan, key, LIB_TRUE) &&
        app_keyboard_emit(context, sink, scan, key, LIB_FALSE) &&
        app_keyboard_emit(context, sink, 0x38u, KVM_KEY_ALT, LIB_FALSE);
}

lib_bool app_keyboard_handle_hotkey(common_machine *machine,
    common_session_machine_state state, const lib_u8 *identifier,
    common_session_command_result *out)
{
    const char *name;
    common_session_request request = COMMON_SESSION_REQUEST_NONE;
    *out = (common_session_command_result) { 0 };
    if (identifier == NULL) return LIB_FALSE;
    name = (const char *)identifier;
    if (lib_text_compare(name, "pause-toggle") == 0) {
        request = state == COMMON_SESSION_MACHINE_PAUSED ?
            COMMON_SESSION_REQUEST_RESUME : COMMON_SESSION_REQUEST_PAUSE;
        out->request = request;
        return LIB_TRUE;
    }
    if (lib_text_compare(name, "release-window-mouse") == 0) {
        out->release_window_mouse = LIB_TRUE;
        return LIB_TRUE;
    }
    if (state != COMMON_SESSION_MACHINE_RUNNING) return LIB_TRUE;
    if (lib_text_compare(name, "send-ctrl-alt-del") == 0)
        return app_keyboard_submit_ctrl_alt_del(machine,
            app_keyboard_deliver_input);
    if (lib_text_compare(name, "send-alt-enter") == 0)
        return app_keyboard_submit_alt_key(machine,
            app_keyboard_deliver_input, 0x1cu, KVM_KEY_ENTER);
    if (lib_text_compare(name, "send-alt-tab") == 0)
        return app_keyboard_submit_alt_key(machine,
            app_keyboard_deliver_input, 0x0fu, KVM_KEY_TAB);
    return LIB_TRUE;
}
