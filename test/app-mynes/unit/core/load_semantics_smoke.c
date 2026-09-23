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
    static const lib_u8 program[] = {
        0xa9u,0x80u, 0xa2u,0u, 0xa0u,0x80u,
        0xa9u,0u, 0x85u,0x10u, 0xa2u,1u, 0xb5u,0x0fu,
        0xa0u,1u, 0xb6u,0x0fu,
        0xa9u,0x80u, 0x8du,0x00u,0x02u, 0xa2u,1u, 0xbcu,0xffu,0x01u,
        0xa0u,1u, 0xbeu,0xffu,0x01u,
        0xa9u,0u, 0x85u,0x20u, 0xa9u,2u, 0x85u,0x21u,
        0xa2u,0u, 0xa1u,0x20u, 0xa0u,0u, 0xb1u,0x20u
    };
    memset(bytes, 0, sizeof(bytes));
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = 1u; memcpy(bytes + 16u, program, sizeof(program));
    bytes[16u + 0x3ffcu] = 0u; bytes[16u + 0x3ffdu] = 0x80u;
    assert(core_machine_create(&machine, bytes, sizeof(bytes), &options) == LIB_STATUS_OK);
    observation = step(machine, 2u); assert(observation.a == 0x80u && (observation.p & 0x82u) == 0x80u);
    observation = step(machine, 2u); assert(observation.x == 0u && (observation.p & 0x82u) == 0x02u);
    observation = step(machine, 2u); assert(observation.y == 0x80u && (observation.p & 0x82u) == 0x80u);
    (void)step(machine,2u); (void)step(machine,3u); (void)step(machine,2u);
    observation = step(machine,4u); assert(observation.a == 0u && (observation.p & 0x82u) == 0x02u);
    (void)step(machine,2u);
    observation = step(machine,4u); assert(observation.x == 0u && (observation.p & 0x82u) == 0x02u);
    (void)step(machine,2u); (void)step(machine,4u); (void)step(machine,2u);
    observation = step(machine,5u); assert(observation.y == 0x80u && (observation.p & 0x82u) == 0x80u);
    (void)step(machine,2u);
    observation = step(machine,5u); assert(observation.x == 0x80u && (observation.p & 0x82u) == 0x80u);
    (void)step(machine,2u); (void)step(machine,3u); (void)step(machine,2u); (void)step(machine,3u);
    (void)step(machine,2u);
    observation = step(machine,6u); assert(observation.a == 0x80u && (observation.p & 0x82u) == 0x80u);
    (void)step(machine,2u);
    observation = step(machine,5u); assert(observation.a == 0x80u && (observation.p & 0x82u) == 0x80u);
    core_machine_destroy(machine);
    return 0;
}
