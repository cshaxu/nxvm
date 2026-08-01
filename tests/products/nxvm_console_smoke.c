#include <stdio.h>

#include "vm/product/session_console.h"

typedef struct console_fixture {
    unsigned count;
    nxvm_product_nxvm_console_command last;
} console_fixture;

static nxvm_core_status record(void *context,
                               nxvm_product_nxvm_console_command command)
{
    console_fixture *fixture = (console_fixture *)context;
    fixture->count += 1u;
    fixture->last = command;
    return NXVM_CORE_STATUS_OK;
}

int main(void)
{
    nxvm_product_nxvm_console console;
    console_fixture fixture = { 0u, NXVM_PRODUCT_NXVM_CONSOLE_INVALID };

    nxvm_product_nxvm_console_initialize(&console);
    if (nxvm_product_nxvm_console_parse(" start ") != NXVM_PRODUCT_NXVM_CONSOLE_START ||
        nxvm_product_nxvm_console_parse("unknown") != NXVM_PRODUCT_NXVM_CONSOLE_INVALID ||
        nxvm_product_nxvm_console_dispatch(&console, NXVM_PRODUCT_NXVM_CONSOLE_MEDIA,
            record, &fixture) != NXVM_CORE_STATUS_OK ||
        nxvm_product_nxvm_console_dispatch(&console, NXVM_PRODUCT_NXVM_CONSOLE_START,
            record, &fixture) != NXVM_CORE_STATUS_OK ||
        nxvm_product_nxvm_console_dispatch(&console, NXVM_PRODUCT_NXVM_CONSOLE_MEDIA,
            record, &fixture) != NXVM_CORE_STATUS_INVALID_STATE ||
        nxvm_product_nxvm_console_dispatch(&console, NXVM_PRODUCT_NXVM_CONSOLE_STOP,
            record, &fixture) != NXVM_CORE_STATUS_OK ||
        nxvm_product_nxvm_console_dispatch(&console, NXVM_PRODUCT_NXVM_CONSOLE_DEBUG,
            record, &fixture) != NXVM_CORE_STATUS_OK ||
        nxvm_product_nxvm_console_dispatch(&console, NXVM_PRODUCT_NXVM_CONSOLE_RESUME,
            record, &fixture) != NXVM_CORE_STATUS_OK ||
        nxvm_product_nxvm_console_dispatch(&console, NXVM_PRODUCT_NXVM_CONSOLE_EXIT,
            record, &fixture) != NXVM_CORE_STATUS_OK ||
        console.state != NXVM_PRODUCT_NXVM_CONSOLE_EXITED || fixture.count != 6u) return 1;
    puts("M5:T6:S1:NXVM-CONSOLE:OK");
    return 0;
}
