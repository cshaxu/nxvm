#include "vm/product/session_console.h"

#include <ctype.h>
#include <string.h>

static int nxvm_product_nxvm_console_word(const char *line, const char *word)
{
    size_t index;

    if (line == NULL || word == NULL) return 0;
    while (*line != '\0' && isspace((unsigned char)*line)) ++line;
    for (index = 0u; word[index] != '\0'; ++index) {
        if (toupper((unsigned char)line[index]) != word[index]) return 0;
    }
    return line[index] == '\0' || isspace((unsigned char)line[index]);
}

static int nxvm_product_nxvm_console_allowed(
    nxvm_product_nxvm_console_state state,
    nxvm_product_nxvm_console_command command)
{
    if (command == NXVM_PRODUCT_NXVM_CONSOLE_HELP ||
        command == NXVM_PRODUCT_NXVM_CONSOLE_INVALID ||
        command == NXVM_PRODUCT_NXVM_CONSOLE_EXIT) return 1;
    if (command == NXVM_PRODUCT_NXVM_CONSOLE_MEDIA ||
        command == NXVM_PRODUCT_NXVM_CONSOLE_RESET) {
        return state == NXVM_PRODUCT_NXVM_CONSOLE_READY ||
               state == NXVM_PRODUCT_NXVM_CONSOLE_PAUSED;
    }
    if (command == NXVM_PRODUCT_NXVM_CONSOLE_START) {
        return state == NXVM_PRODUCT_NXVM_CONSOLE_READY ||
               state == NXVM_PRODUCT_NXVM_CONSOLE_PAUSED;
    }
    if (command == NXVM_PRODUCT_NXVM_CONSOLE_STOP) {
        return state == NXVM_PRODUCT_NXVM_CONSOLE_RUNNING;
    }
    if (command == NXVM_PRODUCT_NXVM_CONSOLE_RESUME) {
        return state == NXVM_PRODUCT_NXVM_CONSOLE_PAUSED ||
               state == NXVM_PRODUCT_NXVM_CONSOLE_DEBUGGER;
    }
    return command == NXVM_PRODUCT_NXVM_CONSOLE_DEBUG &&
           state == NXVM_PRODUCT_NXVM_CONSOLE_PAUSED;
}

void nxvm_product_nxvm_console_initialize(nxvm_product_nxvm_console *console)
{
    if (console != NULL) console->state = NXVM_PRODUCT_NXVM_CONSOLE_READY;
}

nxvm_product_nxvm_console_command nxvm_product_nxvm_console_parse(const char *line)
{
    if (nxvm_product_nxvm_console_word(line, "HELP")) return NXVM_PRODUCT_NXVM_CONSOLE_HELP;
    if (nxvm_product_nxvm_console_word(line, "START")) return NXVM_PRODUCT_NXVM_CONSOLE_START;
    if (nxvm_product_nxvm_console_word(line, "RESET")) return NXVM_PRODUCT_NXVM_CONSOLE_RESET;
    if (nxvm_product_nxvm_console_word(line, "STOP")) return NXVM_PRODUCT_NXVM_CONSOLE_STOP;
    if (nxvm_product_nxvm_console_word(line, "RESUME")) return NXVM_PRODUCT_NXVM_CONSOLE_RESUME;
    if (nxvm_product_nxvm_console_word(line, "DEBUG")) return NXVM_PRODUCT_NXVM_CONSOLE_DEBUG;
    if (nxvm_product_nxvm_console_word(line, "EXIT")) return NXVM_PRODUCT_NXVM_CONSOLE_EXIT;
    if (nxvm_product_nxvm_console_word(line, "DEVICE") ||
        nxvm_product_nxvm_console_word(line, "SET BOOT")) return NXVM_PRODUCT_NXVM_CONSOLE_MEDIA;
    return NXVM_PRODUCT_NXVM_CONSOLE_INVALID;
}

ntvdm64_status nxvm_product_nxvm_console_dispatch(
    nxvm_product_nxvm_console *console,
    nxvm_product_nxvm_console_command command,
    nxvm_product_nxvm_console_operation operation,
    void *context)
{
    ntvdm64_status status;

    if (console == NULL || operation == NULL ||
        console->state == NXVM_PRODUCT_NXVM_CONSOLE_EXITED) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    if (command == NXVM_PRODUCT_NXVM_CONSOLE_INVALID) return NTVDM64_STATUS_UNSUPPORTED;
    if (!nxvm_product_nxvm_console_allowed(console->state, command)) {
        return NTVDM64_STATUS_INVALID_STATE;
    }
    status = operation(context, command);
    if (status != NTVDM64_STATUS_OK) return status;
    if (command == NXVM_PRODUCT_NXVM_CONSOLE_START ||
        command == NXVM_PRODUCT_NXVM_CONSOLE_RESUME) {
        console->state = NXVM_PRODUCT_NXVM_CONSOLE_RUNNING;
    } else if (command == NXVM_PRODUCT_NXVM_CONSOLE_STOP ||
               command == NXVM_PRODUCT_NXVM_CONSOLE_RESET) {
        console->state = NXVM_PRODUCT_NXVM_CONSOLE_PAUSED;
    } else if (command == NXVM_PRODUCT_NXVM_CONSOLE_DEBUG) {
        console->state = NXVM_PRODUCT_NXVM_CONSOLE_DEBUGGER;
    } else if (command == NXVM_PRODUCT_NXVM_CONSOLE_EXIT) {
        console->state = NXVM_PRODUCT_NXVM_CONSOLE_EXITED;
    }
    return NTVDM64_STATUS_OK;
}
