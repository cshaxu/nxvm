#ifndef TEST_NXVM_KBC_PORT_OWNER_FIXTURE_H
#define TEST_NXVM_KBC_PORT_OWNER_FIXTURE_H
#include "port_owner_fixture.h"

/* Port-only protocol tests still use KBC's one production Core route batch. */
static inline lib_status test_kbc_initialize(t_kbc *kbc, t_port *port)
{
    core_machine *machine = test_port_owner_open(port);
    lib_status status;

    if (machine == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    status = core_machine_kbc_initialize(kbc, machine);
    test_port_owner_close(port, machine);
    return status;
}

#endif
