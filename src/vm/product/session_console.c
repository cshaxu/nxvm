#include "vm/product/session_console.h"

#include <ctype.h>
#include <string.h>

static int vm_product_console_word(const char *line, const char *word)
{
    size_t index;

    if (line == NULL || word == NULL) return 0;
    while (*line != '\0' && isspace((unsigned char)*line)) ++line;
    for (index = 0u; word[index] != '\0'; ++index) {
        if (toupper((unsigned char)line[index]) != word[index]) return 0;
    }
    return line[index] == '\0' || isspace((unsigned char)line[index]);
}

static int vm_product_console_allowed(
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

void vm_product_console_initialize(vm_product_console *console)
{
    if (console != NULL) console->state = VM_PRODUCT_CONSOLE_READY;
}

vm_product_console_command vm_product_console_parse(const char *line)
{
    if (vm_product_console_word(line, "HELP")) return VM_PRODUCT_CONSOLE_HELP;
    if (vm_product_console_word(line, "START")) return VM_PRODUCT_CONSOLE_START;
    if (vm_product_console_word(line, "RESET")) return VM_PRODUCT_CONSOLE_RESET;
    if (vm_product_console_word(line, "STOP")) return VM_PRODUCT_CONSOLE_STOP;
    if (vm_product_console_word(line, "RESUME")) return VM_PRODUCT_CONSOLE_RESUME;
    if (vm_product_console_word(line, "DEBUG")) return VM_PRODUCT_CONSOLE_DEBUG;
    if (vm_product_console_word(line, "EXIT")) return VM_PRODUCT_CONSOLE_EXIT;
    if (vm_product_console_word(line, "DEVICE") ||
        vm_product_console_word(line, "SET BOOT")) return VM_PRODUCT_CONSOLE_MEDIA;
    return VM_PRODUCT_CONSOLE_INVALID;
}

ntvdm64_status vm_product_console_dispatch(
    vm_product_console *console,
    vm_product_console_command command,
    vm_product_console_operation operation,
    void *context)
{
    ntvdm64_status status;

    if (console == NULL || operation == NULL ||
        console->state == VM_PRODUCT_CONSOLE_EXITED) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    if (command == VM_PRODUCT_CONSOLE_INVALID) return NTVDM64_STATUS_UNSUPPORTED;
    if (!vm_product_console_allowed(console->state, command)) {
        return NTVDM64_STATUS_INVALID_STATE;
    }
    status = operation(context, command);
    if (status != NTVDM64_STATUS_OK) return status;
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
    return NTVDM64_STATUS_OK;
}
