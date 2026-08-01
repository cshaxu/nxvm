#include "vm/platform/input.h"

#include <string.h>

static vm_platform_keyboard_sink vmPlatformKeyboardSink;
static void *vmPlatformKeyboardContext;

void vm_platform_keyboard_bind(const vm_platform_keyboard_sink *sink,
                               void *context)
{
    if (sink == NULL) {
        memset(&vmPlatformKeyboardSink, 0, sizeof(vmPlatformKeyboardSink));
        vmPlatformKeyboardContext = NULL;
        return;
    }

    vmPlatformKeyboardSink = *sink;
    vmPlatformKeyboardContext = context;
}

int vm_platform_keyboard_get_modifier(vm_platform_keyboard_modifier modifier)
{
    if (vmPlatformKeyboardSink.get_modifier == NULL) {
        return 0;
    }
    return vmPlatformKeyboardSink.get_modifier(vmPlatformKeyboardContext, modifier);
}

void vm_platform_keyboard_apply_host_state(uint32_t asynchronous_keys,
                                           uint32_t toggle_keys)
{
    if (vmPlatformKeyboardSink.apply_host_state != NULL) {
        vmPlatformKeyboardSink.apply_host_state(vmPlatformKeyboardContext,
                                                asynchronous_keys, toggle_keys);
    }
}

void vm_platform_keyboard_receive_key_press(uint16_t code)
{
    if (vmPlatformKeyboardSink.receive_key_press != NULL) {
        vmPlatformKeyboardSink.receive_key_press(vmPlatformKeyboardContext, code);
    }
}

void vm_platform_keyboard_request_stop(void)
{
    if (vmPlatformKeyboardSink.request_stop != NULL) {
        vmPlatformKeyboardSink.request_stop(vmPlatformKeyboardContext);
    }
}
