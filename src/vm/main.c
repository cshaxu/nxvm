#ifndef NXVM_BUILD_SUFFIX
#define NXVM_BUILD_SUFFIX "m1t2s1"
#endif

/*
 * Author:     Xu Ha
 * Email:      cshaxu@gmail.com
 * Repository: https://github.com/cshaxu/nxvm
 * Start:      01/25/2012
 */

#include <stdlib.h>

#include "core/product/utils.h"
#include "version.h"
#include "vm/composition_console.h"
#include "vm/composition_live_machine.h"
#include "vm/product/console.h"

int main(int argc, char **argv) {
    char banner[160];
    vm_composition_live_machine *machine;

    (void)argc;
    (void)argv;
    if (!ntvdm64_version_format_nxvm_banner(banner, sizeof(banner),
        NXVM_BUILD_SUFFIX)) {
        return 1;
    }
    PRINTF("%s\n", banner);
    PRINTF("Built on %s at %s.\n", ntvdm64_version_build_date(),
        ntvdm64_version_build_time());
    machine = (vm_composition_live_machine *)calloc(1u, sizeof(*machine));
    if (machine == NULL) return 1;
    vm_composition_live_machine_initialize(machine);
    if (machine->core_machine == NULL) {
        free(machine);
        return 1;
    }
    vm_composition_console_target_initialize(machine->console_target, machine);
    vm_product_console_main(machine->console_context, machine->console_target);
    free(machine);
    return 0;
}
