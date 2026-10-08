#include "ibmpc/product/keyboard.c"

static kvm_input_event events[6];
static lib_size event_count;
static lib_size event_limit = 6u;

lib_bool common_machine_enqueue_input(common_machine *machine,
    const kvm_input_event *event)
{
    (void)machine;
    if (event_count >= event_limit) return LIB_FALSE;
    events[event_count++] = *event;
    return LIB_TRUE;
}

static lib_bool chord(const char *identifier, const lib_u16 *scan,
    const kvm_key *key, lib_size count)
{
    common_session_command_result result;
    lib_size index;

    event_count = 0u;
    if (!vm_app_keyboard_handle_hotkey((common_machine *)&events,
            COMMON_SESSION_MACHINE_RUNNING, (const lib_u8 *)identifier, &result) ||
        event_count != count * 2u) return LIB_FALSE;
    for (index = 0u; index < count; ++index) {
        const kvm_input_event *make = &events[index];
        const kvm_input_event *release = &events[count * 2u - index - 1u];
        if (make->type != KVM_EVENT_KEY || make->data.key.scan_code != scan[index] ||
            make->data.key.key != key[index] || !make->data.key.pressed ||
            release->data.key.scan_code != scan[index] ||
            release->data.key.key != key[index] || release->data.key.pressed)
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

lib_i32 main(void)
{
    const lib_u16 cad_scan[] = {0x1du, 0x38u, 0x153u};
    const kvm_key cad_key[] = {KVM_KEY_CONTROL, KVM_KEY_ALT, KVM_KEY_DELETE};
    const lib_u16 enter_scan[] = {0x38u, 0x1cu};
    const kvm_key enter_key[] = {KVM_KEY_ALT, KVM_KEY_ENTER};
    common_session_command_result result;

    if (!chord("cad", cad_scan, cad_key, 3u) ||
        !chord("alt-enter", enter_scan, enter_key, 2u)) return 1;
    event_count = 0u;
    event_limit = 5u;
    if (vm_app_keyboard_handle_hotkey((common_machine *)&events,
            COMMON_SESSION_MACHINE_RUNNING, (const lib_u8 *)"cad", &result) ||
        event_count != 5u) return 1;
    event_limit = 6u;
    if (!vm_app_keyboard_handle_hotkey(LIB_NULL, COMMON_SESSION_MACHINE_RUNNING,
            (const lib_u8 *)"pause", &result) ||
        result.request != COMMON_SESSION_REQUEST_PAUSE) return 1;
    if (!vm_app_keyboard_handle_hotkey(LIB_NULL, COMMON_SESSION_MACHINE_PAUSED,
            (const lib_u8 *)"pause", &result) ||
        result.request != COMMON_SESSION_REQUEST_RESUME) return 1;
    if (!vm_app_keyboard_handle_hotkey(LIB_NULL, COMMON_SESSION_MACHINE_RUNNING,
            (const lib_u8 *)"release-mouse", &result) ||
        !result.release_window_mouse) return 1;
    return 0;
}
