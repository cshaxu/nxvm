#include <assert.h>
#include <string.h>

#include "core/machine_interface.h"

static void step(core_machine *machine)
{
    core_run_result result;
    assert(core_machine_run(machine, 1u, 10u, &result) == LIB_STATUS_OK);
    assert(result.instructions == 1u && result.cycles == 2u);
}

static void assert_implied_trace(core_machine *machine)
{
    core_bus_transfer trace[CORE_MACHINE_TRACE_CAPACITY];
    lib_u32 count = 0u;

    assert(core_machine_trace_copy(machine, trace, CORE_MACHINE_TRACE_CAPACITY,
        &count) == LIB_STATUS_OK);
    assert(count == 2u);
    assert(trace[0].kind == CORE_BUS_TRANSFER_READ && trace[0].address == 0x8000u &&
        trace[0].value == 0x18u);
    assert(trace[1].kind == CORE_BUS_TRANSFER_READ && trace[1].address == 0x8001u &&
        trace[1].value == 0xa9u);
}

int main(void)
{
    lib_u8 bytes[16u + 16384u];
    core_machine *machine = 0;
    core_machine_options options = { 0 };
    core_observation observation;
    static const lib_u8 program[] = {
        0x18u, 0xa9u, 0x7fu, 0x69u, 0x01u,
        0x38u, 0xa9u, 0x00u, 0xe9u, 0x01u,
        0xf8u, 0x18u, 0xa9u, 0x09u, 0x69u, 0x01u
    };
    memset(bytes, 0, sizeof(bytes));
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = 1u; memcpy(bytes + 16u, program, sizeof(program));
    bytes[16u + 0x3ffcu] = 0x00u; bytes[16u + 0x3ffdu] = 0x80u;
    assert(core_machine_create(&machine, bytes, sizeof(bytes), &options) == LIB_STATUS_OK);
    step(machine); assert_implied_trace(machine); step(machine); step(machine);
    assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK);
    assert(observation.a == 0x80u && (observation.p & 0xc0u) == 0xc0u);
    step(machine); step(machine); step(machine);
    assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK);
    assert(observation.a == 0xffu && (observation.p & 0x81u) == 0x80u);
    step(machine); step(machine); step(machine); step(machine);
    assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK);
    assert(observation.a == 0x0au && (observation.p & 0x08u) != 0u);
    core_machine_destroy(machine);
    return 0;
}
