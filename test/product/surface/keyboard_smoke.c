#include "product/surface/keyboard_interface.h"

static kvm_input_event events[6];
static lib_size event_count;
static lib_size event_limit = 6u;

lib_bool emulator_machine_enqueue_input(emulator_machine *machine,
    const kvm_input_event *event)
{
    (void)machine;
    if (event_count >= event_limit) return LIB_FALSE;
    events[event_count++] = *event;
    return LIB_TRUE;
}

static lib_bool chord(const char *identifier, const lib_u32 *scan,
    const kvm_key *key, const lib_u32 *flags, lib_size count)
{
    emulator_session_command_result result;
    lib_size index;

    event_count = 0u;
    event_limit = sizeof(events) / sizeof(events[0]);
    if (!product_surface_keyboard_handle_hotkey((emulator_machine *)&events,
            EMULATOR_SESSION_MACHINE_RUNNING, (const lib_u8 *)identifier, &result) ||
        event_count != count * 2u) return LIB_FALSE;
    for (index = 0u; index < count; ++index) {
        const kvm_input_event *make = &events[index];
        const kvm_input_event *release = &events[count * 2u - index - 1u];
        if (make->type != KVM_EVENT_KEY || make->data.key.scan_code != scan[index] ||
            make->data.key.key != key[index] || make->data.key.flags != flags[index] ||
            !make->data.key.pressed ||
            release->data.key.scan_code != scan[index] ||
            release->data.key.key != key[index] || release->data.key.flags != flags[index] ||
            release->data.key.pressed)
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool chord_rejects_each_prefix(const char *identifier,
    lib_size event_total)
{
    emulator_session_command_result result;
    lib_size limit;

    for (limit = 0u; limit < event_total; ++limit) {
        event_count = 0u;
        event_limit = limit;
        result = (emulator_session_command_result){0};
        if (product_surface_keyboard_handle_hotkey((emulator_machine *)&events,
                EMULATOR_SESSION_MACHINE_RUNNING, (const lib_u8 *)identifier,
                &result) || event_count != limit) {
            return LIB_FALSE;
        }
    }
    event_limit = sizeof(events) / sizeof(events[0]);
    return LIB_TRUE;
}

lib_i32 main(void)
{
    const lib_u32 cad_scan[] = {0x1du, 0x38u, 0x0153u};
    const kvm_key cad_key[] = {KVM_KEY_CONTROL, KVM_KEY_ALT, KVM_KEY_DELETE};
    const lib_u32 cad_flags[] = {0u, 0u, KVM_KEY_FLAG_EXTENDED};
    emulator_session_command_result result;

    if (!chord("send-ctrl-alt-del", cad_scan, cad_key, cad_flags, 3u)) return 1;
    event_count = 0u;
    event_limit = sizeof(events) / sizeof(events[0]);
    if (!product_surface_keyboard_handle_hotkey((emulator_machine *)&events,
            EMULATOR_SESSION_MACHINE_RUNNING, (const lib_u8 *)"send-alt-enter",
            &result) || event_count != 6u || events[0].data.key.pressed ||
        events[0].data.key.key != KVM_KEY_CONTROL || events[1].data.key.pressed ||
        events[1].data.key.key != KVM_KEY_ALT || !events[2].data.key.pressed ||
        events[2].data.key.key != KVM_KEY_ALT || !events[3].data.key.pressed ||
        events[3].data.key.key != KVM_KEY_ENTER || events[4].data.key.pressed ||
        events[4].data.key.key != KVM_KEY_ENTER || events[5].data.key.pressed ||
        events[5].data.key.key != KVM_KEY_ALT) return 2;
    if (!chord_rejects_each_prefix("send-ctrl-alt-del", 6u)) return 3;
    if (!chord_rejects_each_prefix("send-alt-enter", 6u)) return 4;
    if (!product_surface_keyboard_handle_hotkey(LIB_NULL, EMULATOR_SESSION_MACHINE_RUNNING,
            (const lib_u8 *)"release-window-mouse", &result) ||
        !result.release_window_mouse) return 5;
    return 0;
}
