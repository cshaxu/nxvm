#include "type.h"

#include "vm/platform/input.h"

C_VOID vm_platform_keyboard_transport_initialize(
    vm_platform_keyboard_transport *transport,
    const vm_platform_keyboard_sink *sink, C_VOID *context)
{
    if (transport == STD_NULL) return;
    transport->sink = sink;
    transport->context = context;
}

C_INT vm_platform_keyboard_get_modifier_for(
    const vm_platform_keyboard_transport *transport,
    vm_platform_keyboard_modifier modifier)
{
    return transport == STD_NULL || transport->sink == STD_NULL ||
        transport->sink->get_modifier == STD_NULL ? 0 :
        transport->sink->get_modifier(transport->context, modifier);
}

C_VOID vm_platform_keyboard_apply_host_state_for(
    const vm_platform_keyboard_transport *transport,
    uint32_t asynchronous_keys, uint32_t toggle_keys)
{
    if (transport != STD_NULL && transport->sink != STD_NULL &&
        transport->sink->apply_host_state != STD_NULL) {
        transport->sink->apply_host_state(transport->context, asynchronous_keys,
                                          toggle_keys);
    }
}

C_VOID vm_platform_keyboard_receive_key_press_for(
    const vm_platform_keyboard_transport *transport, uint16_t code)
{
    if (transport != STD_NULL && transport->sink != STD_NULL &&
        transport->sink->receive_key_press != STD_NULL) {
        transport->sink->receive_key_press(transport->context, code);
    }
}

C_VOID vm_platform_keyboard_request_stop_for(
    const vm_platform_keyboard_transport *transport)
{
    if (transport != STD_NULL && transport->sink != STD_NULL &&
        transport->sink->request_stop != STD_NULL) {
        transport->sink->request_stop(transport->context);
    }
}
