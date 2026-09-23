#include <assert.h>

#include "core/machine_interface.h"

int main(void)
{
    lib_u8 bytes[16u + 16384u];
    core_machine_options options = { .initial_ram_byte = 0xa5u };
    core_machine *machine = LIB_NULL;
    core_observation observation;
    core_run_result run;
    lib_u8 value;
    static const lib_u8 program[] = {
        0xa9u, 0x55u, 0x85u, 0x00u, 0xa2u, 0x80u, 0x9au, 0xa0u, 0x33u
    };

    lib_memory_set(bytes, 0, sizeof(bytes));
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = 1u; lib_memory_copy(bytes + 16u, program, sizeof(program));
    bytes[16u + 0x3ffcu] = 0u; bytes[16u + 0x3ffdu] = 0x80u;
    assert(core_machine_create(&machine, bytes, sizeof(bytes), &options) == LIB_STATUS_OK);
    assert(core_machine_peek(machine, 0u, 1u, &value) == LIB_STATUS_OK && value == 0xa5u);
    assert(core_machine_run(machine, 5u, 20u, &run) == LIB_STATUS_OK && run.instructions == 5u);
    assert(core_machine_reset(machine, CORE_RESET_WARM) == LIB_STATUS_OK);
    {
        core_bus_transfer trace[CORE_MACHINE_TRACE_CAPACITY];
        lib_u32 count = 0u;
        assert(core_machine_trace_copy(machine, trace, CORE_MACHINE_TRACE_CAPACITY,
            &count) == LIB_STATUS_OK);
        assert(count == 7u);
        assert(trace[0].kind == CORE_BUS_TRANSFER_READ && trace[1].kind == CORE_BUS_TRANSFER_READ);
        assert(trace[2].address == 0x0180u && trace[3].address == 0x017fu &&
            trace[4].address == 0x017eu);
        assert(trace[5].address == 0xfffcu && trace[6].address == 0xfffdu);
    }
    assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK);
    assert(observation.a == 0x55u && observation.x == 0x80u && observation.y == 0x33u);
    assert(observation.s == 0x7du && observation.p == 0x24u && observation.pc == 0x8000u);
    assert(observation.cycles == 25u && observation.instructions == 5u);
    assert(core_machine_peek(machine, 0u, 1u, &value) == LIB_STATUS_OK && value == 0x55u);
    assert(core_machine_reset(machine, CORE_RESET_POWER) == LIB_STATUS_OK);
    {
        core_bus_transfer trace[CORE_MACHINE_TRACE_CAPACITY];
        lib_u32 count = 0u;
        assert(core_machine_trace_copy(machine, trace, CORE_MACHINE_TRACE_CAPACITY,
            &count) == LIB_STATUS_OK);
        assert(count == 7u);
        assert(trace[2].address == 0x0100u && trace[3].address == 0x01ffu &&
            trace[4].address == 0x01feu);
        assert(trace[5].address == 0xfffcu && trace[6].address == 0xfffdu);
    }
    assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK);
    assert(observation.a == 0u && observation.x == 0u && observation.y == 0u);
    assert(observation.s == 0xfdu && observation.p == 0x24u && observation.pc == 0x8000u);
    assert(core_machine_peek(machine, 0u, 1u, &value) == LIB_STATUS_OK && value == 0xa5u);
    core_machine_destroy(machine);
    return 0;
}
