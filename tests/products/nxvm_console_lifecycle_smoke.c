#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>

#ifdef _WIN32
#include <io.h>
#define NXVM_DUP _dup
#define NXVM_DUP2 _dup2
#define NXVM_CLOSE _close
#define NXVM_FILENO _fileno
#else
#include <unistd.h>
#define NXVM_DUP dup
#define NXVM_DUP2 dup2
#define NXVM_CLOSE close
#define NXVM_FILENO fileno
#endif

#include "vm/composition_console.h"
#include "vm/composition_live_machine.h"
#include "vm/product/console.h"

int main(void)
{
    vm_composition_live_machine machine = {0};
    FILE *input;
    int saved_stdin;

    input = tmpfile();
    if (input == NULL ||
        fputs("help\ninfo\nexit\n", input) < 0 ||
        fflush(input) != 0 ||
        fseek(input, 0L, SEEK_SET) != 0) {
        if (input != NULL) fclose(input);
        return 1;
    }
    saved_stdin = NXVM_DUP(NXVM_FILENO(stdin));
    if (saved_stdin < 0 ||
        NXVM_DUP2(NXVM_FILENO(input), NXVM_FILENO(stdin)) < 0) {
        if (saved_stdin >= 0) NXVM_CLOSE(saved_stdin);
        fclose(input);
        return 1;
    }

    vm_composition_live_machine_initialize(&machine);
    if (machine.core_machine == NULL) {
        NXVM_DUP2(saved_stdin, NXVM_FILENO(stdin));
        NXVM_CLOSE(saved_stdin);
        fclose(input);
        return 1;
    }
    vm_composition_console_target_initialize(machine.console_target, &machine);
    vm_product_console_main(machine.console_context, machine.console_target);

    NXVM_DUP2(saved_stdin, NXVM_FILENO(stdin));
    NXVM_CLOSE(saved_stdin);
    fclose(input);
    if (machine.core_machine != NULL) return 1;
    puts("M5:T96:S1:CONSOLE-LIFECYCLE:OK");
    return 0;
}
