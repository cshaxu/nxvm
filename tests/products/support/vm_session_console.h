#ifndef VM_PRODUCT_CONSOLE_H
#define VM_PRODUCT_CONSOLE_H

#include "type.h"

typedef enum vm_product_console_state {
    VM_PRODUCT_CONSOLE_READY = 1,
    VM_PRODUCT_CONSOLE_RUNNING,
    VM_PRODUCT_CONSOLE_PAUSED,
    VM_PRODUCT_CONSOLE_DEBUGGER,
    VM_PRODUCT_CONSOLE_EXITED
} vm_product_console_state;

typedef enum vm_product_console_command {
    VM_PRODUCT_CONSOLE_HELP = 1,
    VM_PRODUCT_CONSOLE_INVALID,
    VM_PRODUCT_CONSOLE_START,
    VM_PRODUCT_CONSOLE_RESET,
    VM_PRODUCT_CONSOLE_STOP,
    VM_PRODUCT_CONSOLE_RESUME,
    VM_PRODUCT_CONSOLE_DEBUG,
    VM_PRODUCT_CONSOLE_EXIT,
    VM_PRODUCT_CONSOLE_MEDIA
} vm_product_console_command;

typedef struct vm_product_console {
    vm_product_console_state state;
} vm_product_console;

typedef type_status (*vm_product_console_operation)(
    C_VOID *context, vm_product_console_command command);

C_VOID vm_product_console_initialize(vm_product_console *console);
vm_product_console_command vm_product_console_parse(const C_CHAR *line);
type_status vm_product_console_dispatch(
    vm_product_console *console,
    vm_product_console_command command,
    vm_product_console_operation operation,
    C_VOID *context);

#endif
