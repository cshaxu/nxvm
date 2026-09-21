#include "type.h"

#include "core/devices/machine.h"
#include "core/devices/port.h"
#include "test/integration/support/session_ini.h"
#include "core/machine/machine_private.h"

static C_INT vm_ini_cmos_seed_matches(const C_CHAR *directory,
    const C_CHAR *file_name, type_unsigned_8 index, type_unsigned_8 expected)
{
    integration_ini_session ini_session;
    type_unsigned_8 observed;

    if (integration_ini_session_open(directory, file_name, &ini_session) !=
        TYPE_STATUS_OK) return 0;
    core_machine_port_write(&ini_session.session->core_machine->executor_port,
        0x0070u, index);
    observed = (type_unsigned_8)core_machine_port_read(
        &ini_session.session->core_machine->executor_port, 0x0071u);
    integration_ini_session_close(&ini_session);
    return observed == expected;
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    if (argc != 3) return 1;
    if (!STD_STRCMP(argv[2], "compaq-deskpro-386-model-40-1200k.ini")) {
        if (!vm_ini_cmos_seed_matches(argv[1], argv[2], 0x31u, 0x04u) ||
            !vm_ini_cmos_seed_matches(argv[1], argv[2], 0x33u, 0x80u) ||
            !vm_ini_cmos_seed_matches(argv[1], argv[2], 0x2eu, 0x01u) ||
            !vm_ini_cmos_seed_matches(argv[1], argv[2], 0x2fu, 0x69u)) return 1;
    } else if (!STD_STRCMP(argv[2], "ibm-5170-model-339-1200k.ini")) {
        if (!vm_ini_cmos_seed_matches(argv[1], argv[2], 0x12u, 0x00u) ||
            !vm_ini_cmos_seed_matches(argv[1], argv[2], 0x2fu, 0x43u)) return 1;
    } else return 1;
    STD_PRINTF("M5:T533:S4:INI-CMOS-SEED:OK\n");
    return 0;
}
