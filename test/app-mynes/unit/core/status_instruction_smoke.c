#include <assert.h>

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
        0x38u, 0x18u, 0x58u, 0x78u, 0xa9u,0x7fu, 0x18u,0x69u,1u,
        0xb8u, 0xf8u, 0xd8u
    };
    lib_memory_set(bytes, 0, sizeof(bytes));
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = 1u; lib_memory_copy(bytes + 16u, program, sizeof(program));
    bytes[16u + 0x3ffcu] = 0u; bytes[16u + 0x3ffdu] = 0x80u;
    assert(core_machine_create(&machine, bytes, sizeof(bytes), &options) == LIB_STATUS_OK);
    observation = step(machine); assert((observation.p & 0x01u) != 0u);
    observation = step(machine); assert((observation.p & 0x01u) == 0u);
    observation = step(machine); assert((observation.p & 0x04u) == 0u);
    observation = step(machine); assert((observation.p & 0x04u) != 0u);
    (void)step(machine); (void)step(machine);
    observation = step(machine); assert((observation.p & 0xc0u) == 0xc0u);
    observation = step(machine); assert((observation.p & 0xc0u) == 0x80u);
    observation = step(machine); assert((observation.p & 0x08u) != 0u);
    observation = step(machine); assert((observation.p & 0x08u) == 0u);
    core_machine_destroy(machine);
    return 0;
}
