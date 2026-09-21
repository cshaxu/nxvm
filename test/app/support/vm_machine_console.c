#include "type.h"

#include "app/support/vm_machine_console.h"




static C_INT vm_app_command_word(const C_CHAR *line, const C_CHAR *word)
{
    STD_SIZE_T index;

    if (line == STD_NULL || word == STD_NULL) return 0;
    while (*line != '\0' && STD_ISSPACE((C_UCHAR)*line)) ++line;
    for (index = 0u; word[index] != '\0'; ++index) {
        if (STD_TOUPPER((C_UCHAR)line[index]) != word[index]) return 0;
    }
    return line[index] == '\0' || STD_ISSPACE((C_UCHAR)line[index]);
}

static C_INT vm_app_command_allowed(
    vm_app_command_state state,
    vm_app_command_command command)
{
    if (command == VM_APP_CONSOLE_HELP ||
        command == VM_APP_CONSOLE_INVALID ||
        command == VM_APP_CONSOLE_EXIT) return 1;
    if (command == VM_APP_CONSOLE_MEDIA ||
        command == VM_APP_CONSOLE_RESET) {
        return state == VM_APP_CONSOLE_READY ||
               state == VM_APP_CONSOLE_PAUSED;
    }
    if (command == VM_APP_CONSOLE_START) {
        return state == VM_APP_CONSOLE_READY ||
               state == VM_APP_CONSOLE_PAUSED;
    }
    if (command == VM_APP_CONSOLE_STOP) {
        return state == VM_APP_CONSOLE_RUNNING;
    }
    if (command == VM_APP_CONSOLE_RESUME) {
        return state == VM_APP_CONSOLE_PAUSED ||
               state == VM_APP_CONSOLE_DEBUGGER;
    }
    return command == VM_APP_CONSOLE_DEBUG &&
           state == VM_APP_CONSOLE_PAUSED;
}

C_VOID vm_app_command_initialize(vm_app_command *console)
{
    if (console != STD_NULL) console->state = VM_APP_CONSOLE_READY;
}

vm_app_command_command vm_app_command_parse(const C_CHAR *line)
{
    if (vm_app_command_word(line, "HELP")) return VM_APP_CONSOLE_HELP;
    if (vm_app_command_word(line, "START")) return VM_APP_CONSOLE_START;
    if (vm_app_command_word(line, "RESET")) return VM_APP_CONSOLE_RESET;
    if (vm_app_command_word(line, "STOP")) return VM_APP_CONSOLE_STOP;
    if (vm_app_command_word(line, "RESUME")) return VM_APP_CONSOLE_RESUME;
    if (vm_app_command_word(line, "DEBUG")) return VM_APP_CONSOLE_DEBUG;
    if (vm_app_command_word(line, "EXIT")) return VM_APP_CONSOLE_EXIT;
    if (vm_app_command_word(line, "FLOPPY")) return VM_APP_CONSOLE_MEDIA;
    return VM_APP_CONSOLE_INVALID;
}

type_status vm_app_command_dispatch(
    vm_app_command *console,
    vm_app_command_command command,
    vm_app_command_operation operation,
    C_VOID *context)
{
    type_status status;

    if (console == STD_NULL || operation == STD_NULL ||
        console->state == VM_APP_CONSOLE_EXITED) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (command == VM_APP_CONSOLE_INVALID) return TYPE_STATUS_UNSUPPORTED;
    if (!vm_app_command_allowed(console->state, command)) {
        return TYPE_STATUS_INVALID_STATE;
    }
    status = operation(context, command);
    if (status != TYPE_STATUS_OK) return status;
    if (command == VM_APP_CONSOLE_START ||
        command == VM_APP_CONSOLE_RESUME) {
        console->state = VM_APP_CONSOLE_RUNNING;
    } else if (command == VM_APP_CONSOLE_STOP ||
               command == VM_APP_CONSOLE_RESET) {
        console->state = VM_APP_CONSOLE_PAUSED;
    } else if (command == VM_APP_CONSOLE_DEBUG) {
        console->state = VM_APP_CONSOLE_DEBUGGER;
    } else if (command == VM_APP_CONSOLE_EXIT) {
        console->state = VM_APP_CONSOLE_EXITED;
    }
    return TYPE_STATUS_OK;
}
