#include <stdio.h>

#include "vm/platform/presentation_mailbox.h"

int main(void)
{
    vm_platform_presentation_mailbox first;
    vm_platform_presentation_mailbox second;
    core_platform_display_frame published = {0};
    core_platform_display_frame captured = {0};

    vm_platform_presentation_mailbox_initialize(&first);
    vm_platform_presentation_mailbox_initialize(&second);
    published.columns = 80u;
    published.rows = 25u;
    published.characters[0] = 'A';
    published.generation = 1u;
    vm_platform_presentation_mailbox_publish(&first, &published);
    published.characters[0] = 'B';
    published.generation = 2u;
    vm_platform_presentation_mailbox_publish(&second, &published);
    vm_platform_presentation_mailbox_capture(&first, &captured);
    if (captured.characters[0] != 'A' || captured.generation != 1u) return 1;
    vm_platform_presentation_mailbox_capture(&second, &captured);
    if (captured.characters[0] != 'B' || captured.generation != 2u) return 1;
    puts("M5:T80:S4:PRESENTATION-MAILBOX:OK");
    return 0;
}
