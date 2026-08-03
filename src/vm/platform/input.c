#include "vm/platform/input.h"

void vm_platform_keyboard_transport_initialize(
    vm_platform_keyboard_transport *transport,
    const vm_platform_keyboard_sink *sink, void *context)
{
    if (transport == NULL) return;
    transport->sink = sink;
    transport->context = context;
}

int vm_platform_keyboard_get_modifier_for(
    const vm_platform_keyboard_transport *transport,
    vm_platform_keyboard_modifier modifier)
{
    return transport == NULL || transport->sink == NULL ||
        transport->sink->get_modifier == NULL ? 0 :
        transport->sink->get_modifier(transport->context, modifier);
}

void vm_platform_keyboard_apply_host_state_for(
    const vm_platform_keyboard_transport *transport,
    uint32_t asynchronous_keys, uint32_t toggle_keys)
{
    if (transport != NULL && transport->sink != NULL &&
        transport->sink->apply_host_state != NULL) {
        transport->sink->apply_host_state(transport->context, asynchronous_keys,
                                          toggle_keys);
    }
}

void vm_platform_keyboard_receive_key_press_for(
    const vm_platform_keyboard_transport *transport, uint16_t code)
{
    if (transport != NULL && transport->sink != NULL &&
        transport->sink->receive_key_press != NULL) {
        transport->sink->receive_key_press(transport->context, code);
    }
}

void vm_platform_keyboard_request_stop_for(
    const vm_platform_keyboard_transport *transport)
{
    if (transport != NULL && transport->sink != NULL &&
        transport->sink->request_stop != NULL) {
        transport->sink->request_stop(transport->context);
    }
}
