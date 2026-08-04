#include "type.h"

#include "vm/platform/input.h"

#ifdef _WIN32
#include <windows.h>
#endif

C_VOID vm_platform_input_flush_console_input(C_VOID)
{
#ifdef _WIN32
    HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    if (input != INVALID_HANDLE_VALUE && input != STD_NULL &&
        GetConsoleMode(input, &mode)) {
        FlushConsoleInputBuffer(input);
    }
#endif
}

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

C_VOID vm_platform_keyboard_receive_key_press_for(
    const vm_platform_keyboard_transport *transport, uint16_t scan_code,
    uint16_t virtual_key)
{
    if (transport != STD_NULL && transport->sink != STD_NULL &&
        transport->sink->receive_key_press != STD_NULL) {
        transport->sink->receive_key_press(transport->context, scan_code,
            virtual_key);
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
