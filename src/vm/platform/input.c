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

C_VOID vm_platform_keyboard_receive_key_event_for(
    const vm_platform_keyboard_transport *transport, uint16_t scan_code,
    uint16_t virtual_key, C_INT pressed)
{
    if (transport != STD_NULL && transport->sink != STD_NULL &&
        transport->sink->receive_key_event != STD_NULL) {
        transport->sink->receive_key_event(transport->context, scan_code,
            virtual_key, pressed);
    }
}

C_VOID vm_platform_mouse_transport_initialize(
    vm_platform_mouse_transport *transport, const vm_platform_mouse_sink *sink,
    C_VOID *context)
{
    if (transport == STD_NULL) return;
    transport->sink = sink;
    transport->context = context;
}

C_VOID vm_platform_mouse_receive_relative_event_for(
    const vm_platform_mouse_transport *transport, int16_t delta_x,
    int16_t delta_y, uint8_t buttons)
{
    if (transport != STD_NULL && transport->sink != STD_NULL &&
        transport->sink->receive_relative_event != STD_NULL) {
        transport->sink->receive_relative_event(transport->context, delta_x,
            delta_y, buttons);
    }
}
