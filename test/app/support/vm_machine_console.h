#ifndef VM_APP_CONSOLE_H
#define VM_APP_CONSOLE_H

#include "type.h"

typedef enum vm_app_command_state {
    VM_APP_CONSOLE_READY = 1,
    VM_APP_CONSOLE_RUNNING,
    VM_APP_CONSOLE_PAUSED,
    VM_APP_CONSOLE_DEBUGGER,
    VM_APP_CONSOLE_EXITED
} vm_app_command_state;

typedef enum vm_app_command_command {
    VM_APP_CONSOLE_HELP = 1,
    VM_APP_CONSOLE_INVALID,
    VM_APP_CONSOLE_START,
    VM_APP_CONSOLE_RESET,
    VM_APP_CONSOLE_STOP,
    VM_APP_CONSOLE_RESUME,
    VM_APP_CONSOLE_DEBUG,
    VM_APP_CONSOLE_EXIT,
    VM_APP_CONSOLE_MEDIA
} vm_app_command_command;

typedef struct vm_app_command {
    vm_app_command_state state;
} vm_app_command;

typedef type_status (*vm_app_command_operation)(
    C_VOID *context, vm_app_command_command command);

C_VOID vm_app_command_initialize(vm_app_command *console);
vm_app_command_command vm_app_command_parse(const C_CHAR *line);
type_status vm_app_command_dispatch(
    vm_app_command *console,
    vm_app_command_command command,
    vm_app_command_operation operation,
    C_VOID *context);

#endif
