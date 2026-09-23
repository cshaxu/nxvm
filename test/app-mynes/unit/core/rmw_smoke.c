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
    lib_u8 value;
    static const lib_u8 program[] = {
        0xa9u, 0xffu, 0x85u, 0x10u, 0xe6u, 0x10u, 0xc6u, 0x10u,
        0x8du, 0x00u, 0x02u, 0xeeu, 0x00u, 0x02u, 0xceu, 0x00u, 0x02u
    };
    memset(bytes, 0, sizeof(bytes));
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = 1u; memcpy(bytes + 16u, program, sizeof(program));
    bytes[16u + 0x3ffcu] = 0u; bytes[16u + 0x3ffdu] = 0x80u;
    assert(core_machine_create(&machine, bytes, sizeof(bytes), &options) == LIB_STATUS_OK);
    assert(core_machine_run(machine, 4u, 20u, &result) == LIB_STATUS_OK);
    {
        core_bus_transfer trace[CORE_MACHINE_TRACE_CAPACITY];
        lib_u32 count = 0u;
        assert(core_machine_trace_copy(machine, trace, CORE_MACHINE_TRACE_CAPACITY,
            &count) == LIB_STATUS_OK);
        assert(count == 5u);
        assert(trace[0].kind == CORE_BUS_TRANSFER_READ && trace[0].address == 0x8006u);
        assert(trace[1].kind == CORE_BUS_TRANSFER_READ && trace[1].address == 0x8007u);
        assert(trace[2].kind == CORE_BUS_TRANSFER_READ && trace[2].address == 0x0010u &&
            trace[2].value == 0u);
        assert(trace[3].kind == CORE_BUS_TRANSFER_WRITE && trace[3].address == 0x0010u &&
            trace[3].value == 0u);
        assert(trace[4].kind == CORE_BUS_TRANSFER_WRITE && trace[4].address == 0x0010u &&
            trace[4].value == 0xffu);
    }
    assert(core_machine_peek(machine, 0x0010u, 1u, &value) == LIB_STATUS_OK && value == 0xffu);
    assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK && (observation.p & 0x80u));
    assert(core_machine_run(machine, 3u, 20u, &result) == LIB_STATUS_OK);
    assert(core_machine_peek(machine, 0x0200u, 1u, &value) == LIB_STATUS_OK && value == 0xffu);
    core_machine_destroy(machine);
    return 0;
}
