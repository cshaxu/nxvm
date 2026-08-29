#include "type.h"



#include "vm/product/support/vm_session_console.h"

typedef struct console_fixture {
    C_UINT count;
    vm_product_console_command last;
} console_fixture;

static type_status record(C_VOID *context,
                               vm_product_console_command command)
{
    console_fixture *fixture = (console_fixture *)context;
    fixture->count += 1u;
    fixture->last = command;
    return TYPE_STATUS_OK;
}

C_INT main(C_VOID)
{
    vm_product_console console;
    console_fixture fixture = { 0u, VM_PRODUCT_CONSOLE_INVALID };

    vm_product_console_initialize(&console);
    if (vm_product_console_parse(" start ") != VM_PRODUCT_CONSOLE_START ||
        vm_product_console_parse("unknown") != VM_PRODUCT_CONSOLE_INVALID ||
        vm_product_console_dispatch(&console, VM_PRODUCT_CONSOLE_MEDIA,
            record, &fixture) != TYPE_STATUS_OK ||
        vm_product_console_dispatch(&console, VM_PRODUCT_CONSOLE_START,
            record, &fixture) != TYPE_STATUS_OK ||
        vm_product_console_dispatch(&console, VM_PRODUCT_CONSOLE_MEDIA,
            record, &fixture) != TYPE_STATUS_INVALID_STATE ||
        vm_product_console_dispatch(&console, VM_PRODUCT_CONSOLE_STOP,
            record, &fixture) != TYPE_STATUS_OK ||
        vm_product_console_dispatch(&console, VM_PRODUCT_CONSOLE_DEBUG,
            record, &fixture) != TYPE_STATUS_OK ||
        vm_product_console_dispatch(&console, VM_PRODUCT_CONSOLE_RESUME,
            record, &fixture) != TYPE_STATUS_OK ||
        vm_product_console_dispatch(&console, VM_PRODUCT_CONSOLE_EXIT,
            record, &fixture) != TYPE_STATUS_OK ||
        console.state != VM_PRODUCT_CONSOLE_EXITED || fixture.count != 6u) return 1;
    puts("M5:T6:S1:NXVM-CONSOLE:OK");
    return 0;
}
