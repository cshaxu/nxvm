#include <assert.h>
#include <string.h>

#include "core/machine_interface.h"

int main(void)
{
    lib_u8 bytes[16u + 16384u];
    core_machine *machine = 0;
    core_machine_options options = { 0 };
    core_run_result result;
    core_observation observation;
    static const lib_u8 program[] = {
        0xa9u, 0x34u, 0x8du, 0xffu, 0x02u,
        0xa9u, 0x12u, 0x8du, 0x00u, 0x02u,
        0x6cu, 0xffu, 0x02u
    };
    memset(bytes, 0, sizeof(bytes));
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = 1u; memcpy(bytes + 16u, program, sizeof(program));
    bytes[16u + 0x3ffcu] = 0u; bytes[16u + 0x3ffdu] = 0x80u;
    assert(core_machine_create(&machine, bytes, sizeof(bytes), &options) == LIB_STATUS_OK);
    assert(core_machine_run(machine, 5u, 30u, &result) == LIB_STATUS_OK);
    assert(result.instructions == 5u && result.cycles == 17u);
    assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK && observation.pc == 0x1234u);
    core_machine_destroy(machine);

    memset(bytes, 0, sizeof(bytes));
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = 1u;
    bytes[16u] = 0x20u; bytes[17u] = 0x06u; bytes[18u] = 0x80u;
    bytes[19u] = 0xa9u; bytes[20u] = 0x55u;
    bytes[21u] = 0xeau;
    bytes[22u] = 0xa9u; bytes[23u] = 0x42u; bytes[24u] = 0x60u;
    bytes[16u + 0x3ffcu] = 0u; bytes[16u + 0x3ffdu] = 0x80u;
    assert(core_machine_create(&machine, bytes, sizeof(bytes), &options) == LIB_STATUS_OK);
    assert(core_machine_run(machine, 3u, 20u, &result) == LIB_STATUS_OK);
    assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK);
    assert(observation.pc == 0x8003u && observation.a == 0x42u);
    assert(core_machine_run(machine, 1u, 10u, &result) == LIB_STATUS_OK);
    assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK && observation.a == 0x55u);
    core_machine_destroy(machine);
    return 0;
}
