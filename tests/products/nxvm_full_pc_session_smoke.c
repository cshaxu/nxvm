#include <stdio.h>
#include <string.h>

#include "vm/composition_session.h"

int main(int argc, char **argv)
{
    vm_composition_full_pc_session *full_pc = NULL;
    vm_composition_full_pc_session_config full_pc_config;

    full_pc_config.fdd_image = argv[1];
    full_pc_config.hdd_image = argv[2];
    full_pc_config.boot_hdd = 0;
    if (argc != 3 ||
        vm_composition_full_pc_session_create(&full_pc_config, &full_pc) != NTVDM64_STATUS_OK ||
        STD_STRCMP(vm_composition_full_pc_session_profile(full_pc)->name, "nxvm.full_pc") != 0 ||
        vm_composition_full_pc_session_reset(full_pc) != NTVDM64_STATUS_OK) {
        vm_composition_full_pc_session_destroy(full_pc);
        return 1;
    }
    vm_composition_full_pc_session_destroy(full_pc);
    puts("M5:T13:S8:VM-SESSION:OK");
    return 0;
}
