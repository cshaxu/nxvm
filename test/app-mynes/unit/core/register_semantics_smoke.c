#include <assert.h>
#include <string.h>

#include "core/machine_interface.h"

static core_observation step(core_machine *machine)
{
    core_run_result run;
    core_observation observation;
    assert(core_machine_run(machine, 1u, 10u, &run) == LIB_STATUS_OK);
    assert(!run.trap_valid && run.instructions == 1u && run.cycles == 2u);
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
        0xa9u, 0x80u, 0xaau, 0xe8u, 0xcau, 0xa8u, 0xc8u, 0x88u,
        0x8au, 0x98u, 0xa2u, 0u, 0x9au, 0xbau
    };
    memset(bytes, 0, sizeof(bytes));
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = 1u; memcpy(bytes + 16u, program, sizeof(program));
    bytes[16u + 0x3ffcu] = 0u; bytes[16u + 0x3ffdu] = 0x80u;
    assert(core_machine_create(&machine, bytes, sizeof(bytes), &options) == LIB_STATUS_OK);
    (void)step(machine);
    observation = step(machine); assert(observation.x == 0x80u && (observation.p & 0x82u) == 0x80u);
    observation = step(machine); assert(observation.x == 0x81u && (observation.p & 0x82u) == 0x80u);
    observation = step(machine); assert(observation.x == 0x80u && (observation.p & 0x82u) == 0x80u);
    observation = step(machine); assert(observation.y == 0x80u && (observation.p & 0x82u) == 0x80u);
    observation = step(machine); assert(observation.y == 0x81u && (observation.p & 0x82u) == 0x80u);
    observation = step(machine); assert(observation.y == 0x80u && (observation.p & 0x82u) == 0x80u);
    observation = step(machine); assert(observation.a == 0x80u && (observation.p & 0x82u) == 0x80u);
    observation = step(machine); assert(observation.a == 0x80u && (observation.p & 0x82u) == 0x80u);
    observation = step(machine); assert(observation.x == 0u && (observation.p & 0x82u) == 0x02u);
    observation = step(machine); assert(observation.s == 0u && (observation.p & 0x82u) == 0x02u);
    observation = step(machine); assert(observation.x == 0u && (observation.p & 0x82u) == 0x02u);
    core_machine_destroy(machine);
    return 0;
}
