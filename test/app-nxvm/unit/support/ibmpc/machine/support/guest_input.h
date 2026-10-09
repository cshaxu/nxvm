#ifndef NXVM_TEST_GUEST_INPUT_H
#define NXVM_TEST_GUEST_INPUT_H
#include "lib/types/types_interface.h"
#include "core/machine/machine_private.h"


typedef enum core_machine_guest_input_kind {
    CORE_MACHINE_GUEST_INPUT_KEY,
    CORE_MACHINE_GUEST_INPUT_RELATIVE_MOUSE
} core_machine_guest_input_kind;

typedef struct core_machine_guest_input_event {
    core_machine_guest_input_kind kind;
    union {
        struct {
            lib_u16 scan_code;
            lib_u16 virtual_key;
            lib_i32 pressed;
        } key;
        struct {
            lib_i16 delta_x;
            lib_i16 delta_y;
            lib_u8 buttons;
        } relative_mouse;
    } data;
} core_machine_guest_input_event;

/* Legacy fixture records only. Production receives KVM events through the
 * Common driver; uncomposed synchronous tests use that same delivery body. */
static inline lib_status vm_test_submit_host_input(vm_machine *session,
    const core_machine_guest_input_event *event)
{
    kvm_input_event input = {0};

    if (session == LIB_NULL || !session->active) return LIB_STATUS_INVALID_STATE;
    if (event == LIB_NULL || (event->kind != CORE_MACHINE_GUEST_INPUT_KEY &&
        event->kind != CORE_MACHINE_GUEST_INPUT_RELATIVE_MOUSE))
        return LIB_STATUS_INVALID_ARGUMENT;
    if (event->kind == CORE_MACHINE_GUEST_INPUT_KEY) {
        input.type = KVM_EVENT_KEY;
        input.data.key.scan_code = event->data.key.scan_code;
        input.data.key.key = event->data.key.virtual_key;
        input.data.key.pressed = event->data.key.pressed;
    } else {
        input.type = KVM_EVENT_MOUSE;
        input.data.mouse.delta_x = event->data.relative_mouse.delta_x;
        input.data.mouse.delta_y = event->data.relative_mouse.delta_y;
        input.data.mouse.buttons = event->data.relative_mouse.buttons;
    }
    if (session->executor != LIB_NULL)
        return emulator_machine_enqueue_input(session->executor, &input) ?
            LIB_STATUS_OK : LIB_STATUS_INVALID_STATE;
    (void)vm_machine_deliver_emulator_input(session, &input);
    return LIB_STATUS_OK;
}

#endif
