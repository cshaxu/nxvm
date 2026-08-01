#include <stdio.h>
#include <string.h>

#include "core/product/runtime/session.h"

int main(int argc, char **argv)
{
    nxvm_runtime_session *minimal = NULL;
    nxvm_runtime_session *full_pc = NULL;
    nxvm_runtime_session_config minimal_config = {
        NXVM_RUNTIME_PROFILE_NTVDM64_DOS_MINIMAL, NULL, NULL, 0
    };
    nxvm_runtime_session_config full_pc_config;

    if (argc != 3 ||
        nxvm_runtime_session_create(&minimal_config, &minimal) != NXVM_CORE_STATUS_OK ||
        strcmp(nxvm_runtime_session_profile(minimal)->name, "ntvdm64.dos_minimal") != 0 ||
        nxvm_runtime_session_reset(minimal) != NXVM_CORE_STATUS_OK) {
        nxvm_runtime_session_destroy(minimal);
        return 1;
    }
    nxvm_runtime_session_destroy(minimal);

    full_pc_config.profile = NXVM_RUNTIME_PROFILE_NXVM_FULL_PC;
    full_pc_config.fdd_image = argv[1];
    full_pc_config.hdd_image = argv[2];
    full_pc_config.boot_hdd = 0;
    if (nxvm_runtime_session_create(&full_pc_config, &full_pc) != NXVM_CORE_STATUS_OK ||
        strcmp(nxvm_runtime_session_profile(full_pc)->name, "nxvm.full_pc") != 0 ||
        nxvm_runtime_session_reset(full_pc) != NXVM_CORE_STATUS_OK) {
        nxvm_runtime_session_destroy(full_pc);
        return 1;
    }
    nxvm_runtime_session_destroy(full_pc);
    puts("M3:T5:S1:SESSION:OK");
    return 0;
}
