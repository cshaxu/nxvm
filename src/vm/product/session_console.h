#ifndef NXVM_PRODUCT_NXVM_CONSOLE_H
#define NXVM_PRODUCT_NXVM_CONSOLE_H

#include "type.h"

typedef enum nxvm_product_nxvm_console_state {
    NXVM_PRODUCT_NXVM_CONSOLE_READY = 1,
    NXVM_PRODUCT_NXVM_CONSOLE_RUNNING,
    NXVM_PRODUCT_NXVM_CONSOLE_PAUSED,
    NXVM_PRODUCT_NXVM_CONSOLE_DEBUGGER,
    NXVM_PRODUCT_NXVM_CONSOLE_EXITED
} nxvm_product_nxvm_console_state;

typedef enum nxvm_product_nxvm_console_command {
    NXVM_PRODUCT_NXVM_CONSOLE_HELP = 1,
    NXVM_PRODUCT_NXVM_CONSOLE_INVALID,
    NXVM_PRODUCT_NXVM_CONSOLE_START,
    NXVM_PRODUCT_NXVM_CONSOLE_RESET,
    NXVM_PRODUCT_NXVM_CONSOLE_STOP,
    NXVM_PRODUCT_NXVM_CONSOLE_RESUME,
    NXVM_PRODUCT_NXVM_CONSOLE_DEBUG,
    NXVM_PRODUCT_NXVM_CONSOLE_EXIT,
    NXVM_PRODUCT_NXVM_CONSOLE_MEDIA
} nxvm_product_nxvm_console_command;

typedef struct nxvm_product_nxvm_console {
    nxvm_product_nxvm_console_state state;
} nxvm_product_nxvm_console;

typedef ntvdm64_status (*nxvm_product_nxvm_console_operation)(
    void *context, nxvm_product_nxvm_console_command command);

void nxvm_product_nxvm_console_initialize(nxvm_product_nxvm_console *console);
nxvm_product_nxvm_console_command nxvm_product_nxvm_console_parse(const char *line);
ntvdm64_status nxvm_product_nxvm_console_dispatch(
    nxvm_product_nxvm_console *console,
    nxvm_product_nxvm_console_command command,
    nxvm_product_nxvm_console_operation operation,
    void *context);

#endif
