#include <assert.h>

#include "core/machine_interface.h"

static void run_steps(core_machine *machine, lib_u32 instructions, lib_u32 cycles)
{
    core_run_result run;
    assert(core_machine_run(machine, instructions, 30u, &run) == LIB_STATUS_OK);
    assert(!run.trap_valid && run.instructions == instructions && run.cycles == cycles);
}

int main(void)
{
    lib_u8 bytes[16u + 16384u];
    core_machine_options options = { 0 };
    core_machine *machine = LIB_NULL;
    core_observation observation;
    lib_u8 stack[3];
    static const lib_u8 main_program[] = {
        0x18u, 0x08u, 0x38u, 0x28u, 0xa9u, 0x42u, 0x48u,
        0xa9u, 0u, 0x68u, 0x20u, 0x20u, 0x80u, 0x00u, 0xeau,
        0xa9u, 0x55u
    };

    lib_memory_set(bytes, 0, sizeof(bytes));
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = 1u; lib_memory_copy(bytes + 16u, main_program, sizeof(main_program));
    bytes[16u + 0x20u] = 0xa9u; bytes[16u + 0x21u] = 0x99u; bytes[16u + 0x22u] = 0x60u;
    bytes[16u + 0x30u] = 0xa9u; bytes[16u + 0x31u] = 0x77u; bytes[16u + 0x32u] = 0x40u;
    bytes[16u + 0x3ffcu] = 0u; bytes[16u + 0x3ffdu] = 0x80u;
    bytes[16u + 0x3ffeu] = 0x30u; bytes[16u + 0x3fffu] = 0x80u;
    assert(core_machine_create(&machine, bytes, sizeof(bytes), &options) == LIB_STATUS_OK);

    run_steps(machine, 8u, 22u);
    {
        core_bus_transfer trace[CORE_MACHINE_TRACE_CAPACITY];
        lib_u32 count = 0u;
        assert(core_machine_trace_copy(machine, trace, CORE_MACHINE_TRACE_CAPACITY,
            &count) == LIB_STATUS_OK);
        assert(count == 4u);
        assert(trace[0].address == 0x8009u && trace[1].address == 0x800au);
        assert(trace[2].address == 0x01fcu && trace[3].address == 0x01fdu &&
            trace[3].value == 0x42u);
    }
    assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK);
    assert(observation.a == 0x42u && observation.p == 0x24u && observation.s == 0xfdu);
    run_steps(machine, 1u, 6u);
    {
        core_bus_transfer trace[CORE_MACHINE_TRACE_CAPACITY];
        lib_u32 count = 0u;
        assert(core_machine_trace_copy(machine, trace, CORE_MACHINE_TRACE_CAPACITY,
            &count) == LIB_STATUS_OK);
        assert(count == 6u);
        assert(trace[0].address == 0x800au && trace[1].address == 0x800bu);
        assert(trace[2].address == 0x01fdu && trace[2].kind == CORE_BUS_TRANSFER_READ);
        assert(trace[3].address == 0x01fdu && trace[3].kind == CORE_BUS_TRANSFER_WRITE &&
            trace[3].value == 0x80u);
        assert(trace[4].address == 0x01fcu && trace[4].kind == CORE_BUS_TRANSFER_WRITE &&
            trace[4].value == 0x0cu);
        assert(trace[5].address == 0x800cu && trace[5].kind == CORE_BUS_TRANSFER_READ);
    }
    assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK);
    assert(observation.pc == 0x8020u && observation.s == 0xfbu);
    assert(core_machine_peek(machine, 0x01fcu, 2u, stack) == LIB_STATUS_OK);
    assert(stack[0] == 0x0cu && stack[1] == 0x80u);
    run_steps(machine, 2u, 8u);
    {
        core_bus_transfer trace[CORE_MACHINE_TRACE_CAPACITY];
        lib_u32 count = 0u;
        assert(core_machine_trace_copy(machine, trace, CORE_MACHINE_TRACE_CAPACITY,
            &count) == LIB_STATUS_OK);
        assert(count == 6u);
        assert(trace[0].address == 0x8022u && trace[1].address == 0x8023u);
        assert(trace[2].address == 0x01fbu && trace[3].address == 0x01fcu &&
            trace[4].address == 0x01fdu);
        assert(trace[5].address == 0x800cu && trace[5].kind == CORE_BUS_TRANSFER_READ);
    }
    assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK);
    assert(observation.pc == 0x800du && observation.a == 0x99u && observation.s == 0xfdu);
    run_steps(machine, 1u, 7u);
    {
        core_bus_transfer trace[CORE_MACHINE_TRACE_CAPACITY];
        lib_u32 count = 0u;
        assert(core_machine_trace_copy(machine, trace, CORE_MACHINE_TRACE_CAPACITY,
            &count) == LIB_STATUS_OK);
        assert(count == 7u);
        assert(trace[0].address == 0x800du && trace[1].address == 0x800eu);
        assert(trace[2].address == 0x01fdu && trace[3].address == 0x01fcu &&
            trace[4].address == 0x01fbu);
        assert(trace[5].address == 0xfffeu && trace[6].address == 0xffffu);
    }
    assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK);
    assert(observation.pc == 0x8030u && observation.s == 0xfau && (observation.p & 0x04u) != 0u);
    assert(core_machine_peek(machine, 0x01fbu, 3u, stack) == LIB_STATUS_OK);
    assert(stack[0] == 0xb4u && stack[1] == 0x0fu && stack[2] == 0x80u);
    run_steps(machine, 2u, 8u);
    assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK);
    assert(observation.pc == 0x800fu && observation.a == 0x77u && observation.p == 0xa4u);
    run_steps(machine, 1u, 2u);
    assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK && observation.a == 0x55u);
    core_machine_destroy(machine);
    return 0;
}
