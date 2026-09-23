#include <assert.h>
#include <string.h>

#include "core/machine_interface.h"

static void make_nrom_fixture(lib_u8 *bytes)
{
    static const lib_u8 program[] = {
        0x78u, 0xa9u, 0x2au, 0x8du, 0x00u, 0x02u, 0xa2u, 0x00u,
        0xe8u, 0xe0u, 0x0au, 0xd0u, 0xfbu, 0x4cu, 0x0du, 0x80u
    };

    memset(bytes, 0, 16u + 16384u);
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = 1u;
    memcpy(bytes + 16u, program, sizeof(program));
    bytes[16u + 0x3ffcu] = 0x00u;
    bytes[16u + 0x3ffdu] = 0x80u;
}

int main(void)
{
    lib_u8 fixture[16u + 16384u];
    lib_u8 large_fixture[16u + 32768u];
    core_machine *machine = 0;
    core_machine_options options;
    core_observation observation;
    core_run_result result;

    make_nrom_fixture(fixture);
    options.initial_ram_byte = 0u;
    assert(core_machine_create(&machine, fixture, sizeof(fixture), &options) ==
        LIB_STATUS_OK);
    assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK);
    assert(observation.pc == 0x8000u);
    assert(observation.cycles == 7u);
    assert(core_machine_run(machine, 96u, 1000u, &result) == LIB_STATUS_OK);
    assert(!result.trap_valid);
    {
        lib_u8 value = 0u;
        assert(core_machine_peek(machine, 0x0200u, 1u, &value) == LIB_STATUS_OK);
        assert(value == 0x2au);
    }
    assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK);
    assert(observation.x == 0x0au);
    assert(observation.pc == 0x800du);
    core_machine_destroy(machine);

    fixture[0] = 0u;
    assert(core_machine_create(&machine, fixture, sizeof(fixture), &options) ==
        LIB_STATUS_INVALID_ARGUMENT);

    make_nrom_fixture(fixture);
    fixture[4] = 0u;
    assert(core_machine_create(&machine, fixture, sizeof(fixture), &options) ==
        LIB_STATUS_INVALID_ARGUMENT);
    make_nrom_fixture(fixture);
    fixture[5] = 2u;
    assert(core_machine_create(&machine, fixture, sizeof(fixture), &options) ==
        LIB_STATUS_INVALID_ARGUMENT);
    make_nrom_fixture(fixture);
    fixture[6] = 2u;
    assert(core_machine_create(&machine, fixture, sizeof(fixture), &options) ==
        LIB_STATUS_INVALID_ARGUMENT);
    make_nrom_fixture(fixture);
    fixture[7] = 1u;
    assert(core_machine_create(&machine, fixture, sizeof(fixture), &options) ==
        LIB_STATUS_INVALID_ARGUMENT);

    memset(large_fixture, 0, sizeof(large_fixture));
    large_fixture[0] = 'N'; large_fixture[1] = 'E';
    large_fixture[2] = 'S'; large_fixture[3] = 0x1au;
    large_fixture[4] = 2u;
    large_fixture[16u] = 0x11u;
    large_fixture[16u + 16384u] = 0x22u;
    large_fixture[16u + 0x7ffcu] = 0x00u;
    large_fixture[16u + 0x7ffdu] = 0x80u;
    assert(core_machine_create(&machine, large_fixture, sizeof(large_fixture),
        &options) == LIB_STATUS_OK);
    {
        lib_u8 values[2];
        assert(core_machine_peek(machine, 0x8000u, 1u, values) == LIB_STATUS_OK);
        assert(values[0] == 0x11u);
        assert(core_machine_peek(machine, 0xc000u, 1u, values) == LIB_STATUS_OK);
        assert(values[0] == 0x22u);
    }
    core_machine_destroy(machine);
    return 0;
}
