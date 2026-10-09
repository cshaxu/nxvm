#ifndef TEST_X86_CORE_CONSTRUCTION_FIXTURE_H
#define TEST_X86_CORE_CONSTRUCTION_FIXTURE_H
#include "core/x86/machine.h"
#include "../board_construction_fixture.h"

typedef struct test_core_allocation {
    core_machine_memory_test_allocation *memory;
    core_machine_port_test_allocation *port;
} test_core_allocation;

static lib_status test_core_construct_with_allocation(
    const core_machine_executor_config *config, void *owner,
    core_machine **out_machine)
{
    test_core_allocation *allocation = owner;
    return core_machine_neutral_create_with_test_allocation(config,
        allocation->memory, allocation->port, out_machine);
}

static lib_status test_core_machine_create_with_allocation(
    const core_machine_config *config, core_machine **out_machine,
    core_machine_memory_test_allocation *memory_allocation,
    core_machine_port_test_allocation *port_allocation,
    core_machine_board_state **out_board)
{
    test_core_allocation allocation = {memory_allocation, port_allocation};
    return test_board_construct(config, test_core_construct_with_allocation,
        &allocation, out_machine, out_board);
}
#endif
