#include "lib/ux/internal/linux_input.h"

int main(void)
{
    ux_event event;

    if (!ux_linux_key_to_event(UX_LINUX_KEY_F1, &event) ||
        event.type != UX_EVENT_KEY || event.data.key.scan_code != 0u ||
        event.data.key.virtual_key != UX_KEY_F1 ||
        !event.data.key.pressed ||
        !ux_linux_key_to_event(UX_LINUX_KEY_F12, &event) ||
        event.data.key.virtual_key != UX_KEY_F12 ||
        !ux_linux_key_to_event(UX_LINUX_KEY_PAGE_DOWN, &event) ||
        event.data.key.virtual_key != UX_KEY_PAGE_DOWN ||
        !ux_linux_key_to_event(UX_LINUX_KEY_ENTER, &event) ||
        event.data.key.scan_code != 0u || event.data.key.virtual_key != UX_KEY_ENTER ||
        ux_linux_key_to_event((ux_linux_key)999, &event)) return 1;
    puts("M5:T523:S4:UX-LINUX-INPUT:OK");
    return 0;
}
