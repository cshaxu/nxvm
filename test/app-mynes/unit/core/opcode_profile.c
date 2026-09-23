#include <assert.h>

#include "core/machine_interface.h"

static void fixture(lib_u8 *bytes)
{
    static const lib_u8 program[] = {
        0x78u, 0xeau, 0xa9u, 0x80u, 0x85u, 0x00u, 0xa5u, 0x00u,
        0x8du, 0x00u, 0x02u, 0xa2u, 0x00u, 0xe8u, 0xcau, 0xe0u,
        0x00u, 0xd0u, 0x02u, 0xeau, 0x4cu, 0x14u, 0x80u
    };
    lib_memory_set(bytes, 0, 16u + 16384u);
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = 1u;
    lib_memory_copy(bytes + 16u, program, sizeof(program));
    bytes[16u + 0x3ffcu] = 0x00u;
    bytes[16u + 0x3ffdu] = 0x80u;
}

static core_observation one(core_machine *machine, lib_u32 cycles)
{
    core_run_result result;
    core_observation observation;
    assert(core_machine_run(machine, 1u, 10u, &result) == LIB_STATUS_OK);
    assert(result.instructions == 1u);
    assert(result.cycles == cycles);
    assert(!result.trap_valid);
    assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK);
    return observation;
}

int main(void)
{
    lib_u8 bytes[16u + 16384u];
    core_machine *machine = 0;
    core_machine_options options = { 0 };
    core_observation observation;
    lib_u8 value;

    fixture(bytes);
    assert(core_machine_create(&machine, bytes, sizeof(bytes), &options) == LIB_STATUS_OK);
    observation = one(machine, 2u); assert((observation.p & 0x04u) != 0u);
    (void)one(machine, 2u);
    observation = one(machine, 2u); assert(observation.a == 0x80u && (observation.p & 0x80u));
    (void)one(machine, 3u);
    observation = one(machine, 3u); assert(observation.a == 0x80u && (observation.p & 0x80u));
    (void)one(machine, 4u);
    assert(core_machine_peek(machine, 0x0200u, 1u, &value) == LIB_STATUS_OK && value == 0x80u);
    observation = one(machine, 2u); assert(observation.x == 0u && (observation.p & 0x02u));
    observation = one(machine, 2u); assert(observation.x == 1u && (observation.p & 0x02u) == 0u);
    observation = one(machine, 2u); assert(observation.x == 0u && (observation.p & 0x02u));
    observation = one(machine, 2u); assert((observation.p & 0x03u) == 0x03u);
    observation = one(machine, 2u); assert(observation.pc == 0x8013u);
    observation = one(machine, 2u); assert(observation.pc == 0x8014u);
    observation = one(machine, 3u); assert(observation.pc == 0x8014u);
    core_machine_destroy(machine);

    fixture(bytes);
    bytes[16u] = 0xa2u; bytes[17u] = 0x01u;
    bytes[18u] = 0xe0u; bytes[19u] = 0x00u;
    bytes[20u] = 0xd0u; bytes[21u] = 0x02u;
    bytes[22u] = 0xeau; bytes[23u] = 0xeau;
    bytes[24u] = 0x4cu; bytes[25u] = 0x08u; bytes[26u] = 0x80u;
    assert(core_machine_create(&machine, bytes, sizeof(bytes), &options) == LIB_STATUS_OK);
    (void)one(machine, 2u);
    observation = one(machine, 2u); assert((observation.p & 0x02u) == 0u);
    observation = one(machine, 3u); assert(observation.pc == 0x8008u);
    core_machine_destroy(machine);
    return 0;
}
