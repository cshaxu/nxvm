#ifndef NXVM_BUILD_SUFFIX
#define NXVM_BUILD_SUFFIX "m1t2s1"
#endif

/*
 * Author:     Xu Ha
 * Email:      cshaxu@gmail.com
 * Repository: https://github.com/cshaxu/nxvm
 * Start:      01/25/2012
 */

#include "core/product/utils.h"
#include "version.h"
#include "vm/composition_console.h"
#include "vm/composition_live_machine.h"
#include "vm/product/console.h"

int main(int argc, char **argv) {
    char banner[160];
    vm_composition_live_machine machine;
    nxvm_product_console_target console_target;

    (void)argc;
    (void)argv;
    if (!ntvdm64_version_format_nxvm_banner(banner, sizeof(banner),
        NXVM_BUILD_SUFFIX)) {
        return 1;
    }
    PRINTF("%s\n", banner);
    PRINTF("Built on %s at %s.\n", ntvdm64_version_build_date(),
        ntvdm64_version_build_time());
    vm_composition_console_target_initialize(&console_target, &machine);
    consoleMain(&console_target);
    return 0;
}
