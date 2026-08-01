#include <stdint.h>
#include <stdio.h>

#include "vm/platform/input.h"

typedef struct vm_platform_input_smoke_state {
    int alt;
    uint32_t asynchronous_keys;
    uint32_t toggle_keys;
    uint16_t key_code;
    unsigned stop_count;
} vm_platform_input_smoke_state;

static int vm_platform_input_smoke_get_modifier(
    void *context, vm_platform_keyboard_modifier modifier)
{
    vm_platform_input_smoke_state *state = context;

    return modifier == VM_PLATFORM_KEYBOARD_MODIFIER_ALT ? state->alt : 0;
}

static void vm_platform_input_smoke_apply_host_state(
    void *context, uint32_t asynchronous_keys, uint32_t toggle_keys)
{
    vm_platform_input_smoke_state *state = context;

    state->asynchronous_keys = asynchronous_keys;
    state->toggle_keys = toggle_keys;
}

static void vm_platform_input_smoke_receive_key_press(void *context, uint16_t code)
{
    ((vm_platform_input_smoke_state *)context)->key_code = code;
}

static void vm_platform_input_smoke_request_stop(void *context)
{
    ((vm_platform_input_smoke_state *)context)->stop_count += 1u;
}

int main(void)
{
    vm_platform_input_smoke_state state = {1};
    vm_platform_keyboard_sink sink = {
        vm_platform_input_smoke_get_modifier,
        vm_platform_input_smoke_apply_host_state,
        vm_platform_input_smoke_receive_key_press,
        vm_platform_input_smoke_request_stop
    };

    vm_platform_keyboard_bind(&sink, &state);
    if (!vm_platform_keyboard_get_modifier(VM_PLATFORM_KEYBOARD_MODIFIER_ALT) ||
        vm_platform_keyboard_get_modifier(VM_PLATFORM_KEYBOARD_MODIFIER_SHIFT)) {
        return 1;
    }
    vm_platform_keyboard_apply_host_state(0x12u, 0x34u);
    vm_platform_keyboard_receive_key_press(0x5678u);
    vm_platform_keyboard_request_stop();
    if (state.asynchronous_keys != 0x12u || state.toggle_keys != 0x34u ||
        state.key_code != 0x5678u || state.stop_count != 1u) {
        return 1;
    }
    vm_platform_keyboard_bind(NULL, NULL);
    if (vm_platform_keyboard_get_modifier(VM_PLATFORM_KEYBOARD_MODIFIER_ALT)) {
        return 1;
    }
    puts("M5:T14:S3:VM-PLATFORM-INPUT:OK");
    return 0;
}
