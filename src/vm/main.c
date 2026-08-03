#ifndef NXVM_BUILD_SUFFIX
#define NXVM_BUILD_SUFFIX "m1t2s1"
#endif

/*
 * Author:     Xu Ha
 * Email:      cshaxu@gmail.com
 * Repository: https://github.com/cshaxu/nxvm
 * Start:      01/25/2012
 */

#include "type.h"

#include <stdlib.h>


#include "core/product/utils.h"

#include "version.h"

#include "vm/composition_console.h"

#include "vm/composition_live_machine.h"

#include "vm/product/console.h"

C_INT main(C_INT argc, C_CHAR **argv) {
    C_CHAR banner[160];
    vm_composition_live_machine *machine;

    (C_VOID)argc;
    (C_VOID)argv;
    if (!ntvdm64_version_format_nxvm_banner(banner, sizeof(banner),
        NXVM_BUILD_SUFFIX)) {
        return 1;
    }
    STD_PRINTF("%s\n", banner);
    STD_PRINTF("Built on %s at %s.\n", ntvdm64_version_build_date(),
        ntvdm64_version_build_time());
    machine = (vm_composition_live_machine *)STD_CALLOC(1u, sizeof(*machine));
    if (machine == STD_NULL) return 1;
    vm_composition_live_machine_initialize(machine);
    if (machine->core_machine == STD_NULL) {
        STD_FREE(machine);
        return 1;
    }
    vm_composition_console_target_initialize(machine->console_target, machine);
    vm_product_console_main(machine->console_context, machine->console_target);
    STD_FREE(machine);
    return 0;
}
