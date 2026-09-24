#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/port.h"
#include "test/app-nxvm/integration/support/session_ini.h"
#include "app-nxvm/machine/machine_private.h"

static lib_i32 vm_ini_cmos_seed_matches(const char *directory,
    const char *file_name, lib_u8 index, lib_u8 expected)
{
    integration_ini_session ini_session;
    lib_u8 observed;

    if (integration_ini_session_open(directory, file_name, &ini_session) !=
        LIB_STATUS_OK) return 0;
    core_machine_port_write(&ini_session.session->core_machine->executor_port,
        0x0070u, index);
    observed = (lib_u8)core_machine_port_read(
        &ini_session.session->core_machine->executor_port, 0x0071u);
    integration_ini_session_close(&ini_session);
    return observed == expected;
}

lib_i32 main(lib_i32 argc, char **argv)
{
    if (argc != 3) return 1;
    if (!lib_c_strcmp(argv[2], "compaq-deskpro-386-model-40-1200k/NXVM.ini")) {
        if (!vm_ini_cmos_seed_matches(argv[1], argv[2], 0x31u, 0x04u) ||
            !vm_ini_cmos_seed_matches(argv[1], argv[2], 0x33u, 0x80u) ||
            !vm_ini_cmos_seed_matches(argv[1], argv[2], 0x2eu, 0x01u) ||
            !vm_ini_cmos_seed_matches(argv[1], argv[2], 0x2fu, 0x69u)) return 1;
    } else if (!lib_c_strcmp(argv[2], "ibm-5170-model-339-1200k/NXVM.ini")) {
        if (!vm_ini_cmos_seed_matches(argv[1], argv[2], 0x12u, 0x00u) ||
            !vm_ini_cmos_seed_matches(argv[1], argv[2], 0x2fu, 0x43u)) return 1;
    } else return 1;
    printf("M5:T533:S4:INI-CMOS-SEED:OK\n");
    return 0;
}
