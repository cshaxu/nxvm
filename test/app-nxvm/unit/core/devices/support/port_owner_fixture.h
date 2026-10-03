#ifndef TEST_NXVM_PORT_OWNER_FIXTURE_H
#define TEST_NXVM_PORT_OWNER_FIXTURE_H
#include "x86/core/machine.h"

/* Adapt port-only unit fixtures to the single Core-owned registration path. */
static inline core_machine *test_port_owner_open(const t_port *port)
{
    core_machine *machine = (core_machine *)lib_allocate_zero(1u, sizeof(*machine));

    if (machine != LIB_NULL) {
        machine->lifecycle = CORE_MACHINE_INITIALIZED;
        machine->executor_port = *port;
    }
    return machine;
}

static inline void test_port_owner_close(t_port *port, core_machine *machine)
{
    *port = machine->executor_port;
    lib_release(machine);
}

#endif
