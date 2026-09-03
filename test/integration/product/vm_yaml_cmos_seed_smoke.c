#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/port.h"
#include "test/integration/support/session_yaml.h"
#include "vm/composition/session/session_private.h"

static C_INT vm_yaml_cmos_seed_matches(const C_CHAR *directory,
    const C_CHAR *file_name, type_unsigned_8 index, type_unsigned_8 expected)
{
    integration_yaml_session yaml_session;
    type_unsigned_8 observed;

    if (integration_yaml_session_open(directory, file_name, &yaml_session) !=
        TYPE_STATUS_OK) return 0;
    core_machine_port_write(&yaml_session.session->core_machine->executor_port,
        0x0070u, index);
    observed = (type_unsigned_8)core_machine_port_read(
        &yaml_session.session->core_machine->executor_port, 0x0071u);
    integration_yaml_session_close(&yaml_session);
    return observed == expected;
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    if (argc != 2 ||
        !vm_yaml_cmos_seed_matches(argv[1],
            "compaq-deskpro-386-model-40.yaml", 0x31u, 0x04u) ||
        !vm_yaml_cmos_seed_matches(argv[1],
            "compaq-deskpro-386-model-40.yaml", 0x33u, 0x80u) ||
        !vm_yaml_cmos_seed_matches(argv[1],
            "ibm-5170-model-339.yaml", 0x12u, 0x00u) ||
        !vm_yaml_cmos_seed_matches(argv[1],
            "ibm-5170-model-339.yaml", 0x2fu, 0x43u)) return 1;
    STD_PRINTF("M5:T515:INTEGRATION:YAML-CMOS-SEED:OK\n");
    return 0;
}
