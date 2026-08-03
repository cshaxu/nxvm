#include "type.h"

#include <stdio.h>


#include "vm/product/session_console.h"

typedef struct console_fixture {
    C_UINT count;
    vm_product_console_command last;
} console_fixture;

static ntvdm64_status record(C_VOID *context,
                               vm_product_console_command command)
{
    console_fixture *fixture = (console_fixture *)context;
    fixture->count += 1u;
    fixture->last = command;
    return NTVDM64_STATUS_OK;
}

C_INT main(C_VOID)
{
    vm_product_console console;
    console_fixture fixture = { 0u, VM_PRODUCT_CONSOLE_INVALID };

    vm_product_console_initialize(&console);
    if (vm_product_console_parse(" start ") != VM_PRODUCT_CONSOLE_START ||
        vm_product_console_parse("unknown") != VM_PRODUCT_CONSOLE_INVALID ||
        vm_product_console_dispatch(&console, VM_PRODUCT_CONSOLE_MEDIA,
            record, &fixture) != NTVDM64_STATUS_OK ||
        vm_product_console_dispatch(&console, VM_PRODUCT_CONSOLE_START,
            record, &fixture) != NTVDM64_STATUS_OK ||
        vm_product_console_dispatch(&console, VM_PRODUCT_CONSOLE_MEDIA,
            record, &fixture) != NTVDM64_STATUS_INVALID_STATE ||
        vm_product_console_dispatch(&console, VM_PRODUCT_CONSOLE_STOP,
            record, &fixture) != NTVDM64_STATUS_OK ||
        vm_product_console_dispatch(&console, VM_PRODUCT_CONSOLE_DEBUG,
            record, &fixture) != NTVDM64_STATUS_OK ||
        vm_product_console_dispatch(&console, VM_PRODUCT_CONSOLE_RESUME,
            record, &fixture) != NTVDM64_STATUS_OK ||
        vm_product_console_dispatch(&console, VM_PRODUCT_CONSOLE_EXIT,
            record, &fixture) != NTVDM64_STATUS_OK ||
        console.state != VM_PRODUCT_CONSOLE_EXITED || fixture.count != 6u) return 1;
    puts("M5:T6:S1:NXVM-CONSOLE:OK");
    return 0;
}
