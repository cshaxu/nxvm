#include "type.h"

#include "vm/product/support/vm_session_console.h"




static C_INT vm_product_console_word(const C_CHAR *line, const C_CHAR *word)
{
    STD_SIZE_T index;

    if (line == STD_NULL || word == STD_NULL) return 0;
    while (*line != '\0' && STD_ISSPACE((C_UCHAR)*line)) ++line;
    for (index = 0u; word[index] != '\0'; ++index) {
        if (STD_TOUPPER((C_UCHAR)line[index]) != word[index]) return 0;
    }
    return line[index] == '\0' || STD_ISSPACE((C_UCHAR)line[index]);
}

static C_INT vm_product_console_allowed(
    vm_product_console_state state,
    vm_product_console_command command)
{
    if (command == VM_PRODUCT_CONSOLE_HELP ||
        command == VM_PRODUCT_CONSOLE_INVALID ||
        command == VM_PRODUCT_CONSOLE_EXIT) return 1;
    if (command == VM_PRODUCT_CONSOLE_MEDIA ||
        command == VM_PRODUCT_CONSOLE_RESET) {
        return state == VM_PRODUCT_CONSOLE_READY ||
               state == VM_PRODUCT_CONSOLE_PAUSED;
    }
    if (command == VM_PRODUCT_CONSOLE_START) {
        return state == VM_PRODUCT_CONSOLE_READY ||
               state == VM_PRODUCT_CONSOLE_PAUSED;
    }
    if (command == VM_PRODUCT_CONSOLE_STOP) {
        return state == VM_PRODUCT_CONSOLE_RUNNING;
    }
    if (command == VM_PRODUCT_CONSOLE_RESUME) {
        return state == VM_PRODUCT_CONSOLE_PAUSED ||
               state == VM_PRODUCT_CONSOLE_DEBUGGER;
    }
    return command == VM_PRODUCT_CONSOLE_DEBUG &&
           state == VM_PRODUCT_CONSOLE_PAUSED;
}

C_VOID vm_product_console_initialize(vm_product_console *console)
{
    if (console != STD_NULL) console->state = VM_PRODUCT_CONSOLE_READY;
}

vm_product_console_command vm_product_console_parse(const C_CHAR *line)
{
    if (vm_product_console_word(line, "HELP")) return VM_PRODUCT_CONSOLE_HELP;
    if (vm_product_console_word(line, "START")) return VM_PRODUCT_CONSOLE_START;
    if (vm_product_console_word(line, "RESET")) return VM_PRODUCT_CONSOLE_RESET;
    if (vm_product_console_word(line, "STOP")) return VM_PRODUCT_CONSOLE_STOP;
    if (vm_product_console_word(line, "RESUME")) return VM_PRODUCT_CONSOLE_RESUME;
    if (vm_product_console_word(line, "DEBUG")) return VM_PRODUCT_CONSOLE_DEBUG;
    if (vm_product_console_word(line, "EXIT")) return VM_PRODUCT_CONSOLE_EXIT;
    if (vm_product_console_word(line, "FLOPPY")) return VM_PRODUCT_CONSOLE_MEDIA;
    return VM_PRODUCT_CONSOLE_INVALID;
}

type_status vm_product_console_dispatch(
    vm_product_console *console,
    vm_product_console_command command,
    vm_product_console_operation operation,
    C_VOID *context)
{
    type_status status;

    if (console == STD_NULL || operation == STD_NULL ||
        console->state == VM_PRODUCT_CONSOLE_EXITED) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (command == VM_PRODUCT_CONSOLE_INVALID) return TYPE_STATUS_UNSUPPORTED;
    if (!vm_product_console_allowed(console->state, command)) {
        return TYPE_STATUS_INVALID_STATE;
    }
    status = operation(context, command);
    if (status != TYPE_STATUS_OK) return status;
    if (command == VM_PRODUCT_CONSOLE_START ||
        command == VM_PRODUCT_CONSOLE_RESUME) {
        console->state = VM_PRODUCT_CONSOLE_RUNNING;
    } else if (command == VM_PRODUCT_CONSOLE_STOP ||
               command == VM_PRODUCT_CONSOLE_RESET) {
        console->state = VM_PRODUCT_CONSOLE_PAUSED;
    } else if (command == VM_PRODUCT_CONSOLE_DEBUG) {
        console->state = VM_PRODUCT_CONSOLE_DEBUGGER;
    } else if (command == VM_PRODUCT_CONSOLE_EXIT) {
        console->state = VM_PRODUCT_CONSOLE_EXITED;
    }
    return TYPE_STATUS_OK;
}
