#include <assert.h>
#include <string.h>

#include "core/machine_interface.h"

static core_observation step(core_machine *machine, lib_u32 cycles)
{
    core_run_result run;
    core_observation observation;
    assert(core_machine_run(machine, 1u, 10u, &run) == LIB_STATUS_OK);
    assert(!run.trap_valid && run.instructions == 1u && run.cycles == cycles);
    assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK);
    return observation;
}

int main(void)
{
    lib_u8 bytes[16u + 16384u];
    core_machine_options options = { 0 };
    core_machine *machine = LIB_NULL;
    core_observation observation;
    lib_u8 values[2];
    static const lib_u8 program[] = {
        0x38u, 0xa9u,0xffu, 0x85u,0x10u, 0xe6u,0x10u, 0xc6u,0x10u,
        0xa2u,1u, 0xa9u,0xffu, 0x8du,0x00u,0x02u,
        0xfeu,0xffu,0x01u, 0xdeu,0xffu,0x01u
    };
    memset(bytes, 0, sizeof(bytes));
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = 1u; memcpy(bytes + 16u, program, sizeof(program));
    bytes[16u + 0x3ffcu] = 0u; bytes[16u + 0x3ffdu] = 0x80u;
    assert(core_machine_create(&machine, bytes, sizeof(bytes), &options) == LIB_STATUS_OK);
    (void)step(machine, 2u); (void)step(machine, 2u); (void)step(machine, 3u);
    observation = step(machine, 5u); assert((observation.p & 0x83u) == 0x03u);
    assert(core_machine_peek(machine, 0x10u, 1u, values) == LIB_STATUS_OK && values[0] == 0u);
    observation = step(machine, 5u); assert((observation.p & 0x83u) == 0x81u);
    assert(core_machine_peek(machine, 0x10u, 1u, values) == LIB_STATUS_OK && values[0] == 0xffu);
    (void)step(machine, 2u); (void)step(machine, 2u); (void)step(machine, 4u);
    observation = step(machine, 7u); assert((observation.p & 0x83u) == 0x03u);
    {
        core_bus_transfer trace[CORE_MACHINE_TRACE_CAPACITY];
        lib_u32 count = 0u;
        assert(core_machine_trace_copy(machine, trace, CORE_MACHINE_TRACE_CAPACITY,
            &count) == LIB_STATUS_OK);
        assert(count == 7u);
        assert(trace[0].address == 0x8010u && trace[1].address == 0x8011u &&
            trace[2].address == 0x8012u);
        assert(trace[3].address == 0x0100u && trace[3].kind == CORE_BUS_TRANSFER_READ);
        assert(trace[4].address == 0x0200u && trace[4].kind == CORE_BUS_TRANSFER_READ &&
            trace[4].value == 0xffu);
        assert(trace[5].address == 0x0200u && trace[5].kind == CORE_BUS_TRANSFER_WRITE &&
            trace[5].value == 0xffu);
        assert(trace[6].address == 0x0200u && trace[6].kind == CORE_BUS_TRANSFER_WRITE &&
            trace[6].value == 0u);
    }
    assert(core_machine_peek(machine, 0x0200u, 1u, values) == LIB_STATUS_OK && values[0] == 0u);
    observation = step(machine, 7u); assert((observation.p & 0x83u) == 0x81u);
    assert(core_machine_peek(machine, 0x0200u, 1u, values) == LIB_STATUS_OK && values[0] == 0xffu);
    core_machine_destroy(machine);
    return 0;
}
