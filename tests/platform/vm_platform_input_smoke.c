#include "type.h"




#include "vm/platform/input.h"

typedef struct vm_platform_input_smoke_state {
    C_INT alt;
    uint16_t key_code;
    C_UINT stop_count;
} vm_platform_input_smoke_state;

static C_INT vm_platform_input_smoke_get_modifier(
    C_VOID *context, vm_platform_keyboard_modifier modifier)
{
    vm_platform_input_smoke_state *state = context;

    return modifier == VM_PLATFORM_KEYBOARD_MODIFIER_ALT ? state->alt : 0;
}

static C_VOID vm_platform_input_smoke_receive_key_press(C_VOID *context,
    uint16_t scan_code, uint16_t virtual_key)
{
    (C_VOID)virtual_key;
    ((vm_platform_input_smoke_state *)context)->key_code = scan_code;
}

static C_VOID vm_platform_input_smoke_request_stop(C_VOID *context)
{
    ((vm_platform_input_smoke_state *)context)->stop_count += 1u;
}

C_INT main(C_VOID)
{
    vm_platform_input_smoke_state state = {1};
    vm_platform_input_smoke_state second_state = {0};
    vm_platform_keyboard_sink sink = {
        vm_platform_input_smoke_get_modifier,
        vm_platform_input_smoke_receive_key_press,
        vm_platform_input_smoke_request_stop
    };
    vm_platform_keyboard_transport transport;
    vm_platform_keyboard_transport second_transport;

    vm_platform_keyboard_transport_initialize(&transport, &sink, &state);
    vm_platform_keyboard_transport_initialize(&second_transport, &sink,
                                              &second_state);
    vm_platform_keyboard_receive_key_press_for(&second_transport, 0x1234u,
        0x5678u);
    vm_platform_keyboard_request_stop_for(&second_transport);
    if (second_state.key_code != 0x1234u || second_state.stop_count != 1u) {
        return 1;
    }

    if (!vm_platform_keyboard_get_modifier_for(&transport,
            VM_PLATFORM_KEYBOARD_MODIFIER_ALT) ||
        vm_platform_keyboard_get_modifier_for(&transport,
            VM_PLATFORM_KEYBOARD_MODIFIER_SHIFT)) {
        return 1;
    }
    puts("M5:T80:S1:VM-PLATFORM-INPUT:OK");
    return 0;
}
