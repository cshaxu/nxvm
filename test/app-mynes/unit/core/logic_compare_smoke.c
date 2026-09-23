#include <assert.h>

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
    static const lib_u8 program[] = {
        0xa9u, 0xf0u, 0x09u, 0x0fu, 0x29u, 0x0fu, 0x49u, 0x0fu,
        0xa9u, 0x80u, 0xc9u, 0x80u, 0xc9u, 0x81u, 0xa2u, 0u,
        0xe0u, 1u, 0xa0u, 1u, 0xc0u, 1u, 0xa9u, 0x40u, 0x85u,
        0x10u, 0xa9u, 0xc0u, 0x24u, 0x10u
    };
    lib_memory_set(bytes, 0, sizeof(bytes));
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = 1u; lib_memory_copy(bytes + 16u, program, sizeof(program));
    bytes[16u + 0x3ffcu] = 0u; bytes[16u + 0x3ffdu] = 0x80u;
    assert(core_machine_create(&machine, bytes, sizeof(bytes), &options) == LIB_STATUS_OK);
    (void)step(machine, 2u);
    observation = step(machine, 2u); assert(observation.a == 0xffu && (observation.p & 0x82u) == 0x80u);
    observation = step(machine, 2u); assert(observation.a == 0x0fu && (observation.p & 0x82u) == 0u);
    observation = step(machine, 2u); assert(observation.a == 0u && (observation.p & 0x82u) == 0x02u);
    (void)step(machine, 2u);
    observation = step(machine, 2u); assert((observation.p & 0x83u) == 0x03u);
    observation = step(machine, 2u); assert((observation.p & 0x83u) == 0x80u);
    (void)step(machine, 2u);
    observation = step(machine, 2u); assert((observation.p & 0x83u) == 0x80u);
    (void)step(machine, 2u);
    observation = step(machine, 2u); assert((observation.p & 0x83u) == 0x03u);
    (void)step(machine, 2u); (void)step(machine, 3u); (void)step(machine, 2u);
    observation = step(machine, 3u); assert((observation.p & 0xc3u) == 0x41u);
    core_machine_destroy(machine);
    return 0;
}
