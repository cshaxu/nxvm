#include "type.h"



#include "vm/app/support/vm_machine_console.h"

typedef struct console_fixture {
    C_UINT count;
    vm_app_command_command last;
} console_fixture;

static type_status record(C_VOID *context,
                               vm_app_command_command command)
{
    console_fixture *fixture = (console_fixture *)context;
    fixture->count += 1u;
    fixture->last = command;
    return TYPE_STATUS_OK;
}

C_INT main(C_VOID)
{
    vm_app_command console;
    console_fixture fixture = { 0u, VM_APP_CONSOLE_INVALID };

    vm_app_command_initialize(&console);
    if (vm_app_command_parse(" start ") != VM_APP_CONSOLE_START ||
        vm_app_command_parse("unknown") != VM_APP_CONSOLE_INVALID ||
        vm_app_command_dispatch(&console, VM_APP_CONSOLE_MEDIA,
            record, &fixture) != TYPE_STATUS_OK ||
        vm_app_command_dispatch(&console, VM_APP_CONSOLE_START,
            record, &fixture) != TYPE_STATUS_OK ||
        vm_app_command_dispatch(&console, VM_APP_CONSOLE_MEDIA,
            record, &fixture) != TYPE_STATUS_INVALID_STATE ||
        vm_app_command_dispatch(&console, VM_APP_CONSOLE_STOP,
            record, &fixture) != TYPE_STATUS_OK ||
        vm_app_command_dispatch(&console, VM_APP_CONSOLE_DEBUG,
            record, &fixture) != TYPE_STATUS_OK ||
        vm_app_command_dispatch(&console, VM_APP_CONSOLE_RESUME,
            record, &fixture) != TYPE_STATUS_OK ||
        vm_app_command_dispatch(&console, VM_APP_CONSOLE_EXIT,
            record, &fixture) != TYPE_STATUS_OK ||
        console.state != VM_APP_CONSOLE_EXITED || fixture.count != 6u) return 1;
    puts("M5:T6:S1:NXVM-CONSOLE:OK");
    return 0;
}
