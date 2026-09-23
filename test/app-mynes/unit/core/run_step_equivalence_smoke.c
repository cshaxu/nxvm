#include <assert.h>

#include "core/machine.h"
#include "core/machine_interface.h"

static core_machine *make_machine(void)
{
    lib_u8 bytes[16u + 16384u];
    core_machine_options options = { 0 };
    core_machine *machine = LIB_NULL;
    static const lib_u8 program[] = { 0x58u, 0xa9u, 0x42u, 0x85u, 0x10u, 0xeau };

    lib_memory_set(bytes, 0, sizeof(bytes));
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = 1u; lib_memory_copy(bytes + 16u, program, sizeof(program));
    bytes[16u + 0x1000u] = 0xeau;
    bytes[16u + 0x3ffau] = 0u; bytes[16u + 0x3ffbu] = 0x90u;
    bytes[16u + 0x3ffcu] = 0u; bytes[16u + 0x3ffdu] = 0x80u;
    bytes[16u + 0x3ffeu] = 0u; bytes[16u + 0x3fffu] = 0x91u;
    assert(core_machine_create(&machine, bytes, sizeof(bytes), &options) == LIB_STATUS_OK);
    return machine;
}

static void compare_checkpoint(core_machine *run_machine, core_machine *step_machine)
{
    core_run_result run_result;
    core_run_result step_result;
    core_observation run_observation;
    core_observation step_observation;
    core_bus_transfer run_trace[CORE_MACHINE_TRACE_CAPACITY];
    core_bus_transfer step_trace[CORE_MACHINE_TRACE_CAPACITY];
    lib_u32 run_count = 0u;
    lib_u32 step_count = 0u;

    assert(core_machine_run(run_machine, 1u, 20u, &run_result) == LIB_STATUS_OK);
    assert(core_machine_debug_step(step_machine, 1u, 20u, &step_result) == LIB_STATUS_OK);
    assert(run_result.instructions == step_result.instructions &&
        run_result.cycles == step_result.cycles && run_result.reason == step_result.reason &&
        run_result.trap_valid == step_result.trap_valid);
    assert(core_machine_observe(run_machine, &run_observation) == LIB_STATUS_OK);
    assert(core_machine_observe(step_machine, &step_observation) == LIB_STATUS_OK);
    assert(run_observation.a == step_observation.a && run_observation.x == step_observation.x &&
        run_observation.y == step_observation.y && run_observation.s == step_observation.s &&
        run_observation.p == step_observation.p && run_observation.pc == step_observation.pc &&
        run_observation.cycles == step_observation.cycles &&
        run_observation.instructions == step_observation.instructions);
    assert(core_machine_trace_copy(run_machine, run_trace, CORE_MACHINE_TRACE_CAPACITY,
        &run_count) == LIB_STATUS_OK);
    assert(core_machine_trace_copy(step_machine, step_trace, CORE_MACHINE_TRACE_CAPACITY,
        &step_count) == LIB_STATUS_OK);
    assert(run_count == step_count);
    assert(lib_memory_compare(run_trace, step_trace, run_count * sizeof(run_trace[0])) == 0);
}

int main(void)
{
    core_machine *run_machine = make_machine();
    core_machine *step_machine = make_machine();

    compare_checkpoint(run_machine, step_machine);
    compare_checkpoint(run_machine, step_machine);
    compare_checkpoint(run_machine, step_machine);
    assert(core_machine_set_interrupt_lines(run_machine, LIB_FALSE, LIB_TRUE) == LIB_STATUS_OK);
    assert(core_machine_set_interrupt_lines(step_machine, LIB_FALSE, LIB_TRUE) == LIB_STATUS_OK);
    compare_checkpoint(run_machine, step_machine);
    core_machine_destroy(run_machine);
    core_machine_destroy(step_machine);
    return 0;
}
