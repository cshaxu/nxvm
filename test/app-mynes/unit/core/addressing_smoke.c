#include <assert.h>

#include "core/machine_interface.h"

static void run_one(core_machine *machine, lib_u32 cycles)
{
    core_run_result result;
    assert(core_machine_run(machine, 1u, 10u, &result) == LIB_STATUS_OK);
    assert(result.instructions == 1u && result.cycles == cycles);
}

int main(void)
{
    lib_u8 bytes[16u + 16384u];
    core_machine *machine = 0;
    core_machine_options options = { 0 };
    core_observation observation;
    static const lib_u8 program[] = {
        0xa9u, 0x5au, 0x85u, 0x11u, 0xa2u, 0x01u, 0xb5u, 0x10u,
        0xa9u, 0x6bu, 0x8du, 0x01u, 0x02u, 0xbdu, 0x00u, 0x02u,
        0xbdu, 0xffu, 0x80u
    };

    lib_memory_set(bytes, 0, sizeof(bytes));
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = 1u;
    lib_memory_copy(bytes + 16u, program, sizeof(program));
    bytes[16u + 0x100u] = 0x77u;
    bytes[16u + 0x3ffcu] = 0x00u; bytes[16u + 0x3ffdu] = 0x80u;
    assert(core_machine_create(&machine, bytes, sizeof(bytes), &options) == LIB_STATUS_OK);
    run_one(machine, 2u); run_one(machine, 3u); run_one(machine, 2u);
    run_one(machine, 4u);
    assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK && observation.a == 0x5au);
    run_one(machine, 2u); run_one(machine, 4u); run_one(machine, 4u);
    assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK && observation.a == 0x6bu);
    run_one(machine, 5u);
    assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK && observation.a == 0x77u);
    {
        core_bus_transfer trace[CORE_MACHINE_TRACE_CAPACITY];
        lib_u32 count = 0u;
        assert(core_machine_trace_copy(machine, trace, CORE_MACHINE_TRACE_CAPACITY,
            &count) == LIB_STATUS_OK);
        assert(count == 5u);
        assert(trace[0].address == 0x8010u && trace[1].address == 0x8011u &&
            trace[2].address == 0x8012u);
        assert(trace[3].address == 0x8000u && trace[3].kind == CORE_BUS_TRANSFER_READ);
        assert(trace[4].address == 0x8100u && trace[4].kind == CORE_BUS_TRANSFER_READ &&
            trace[4].value == 0x77u);
    }
    core_machine_destroy(machine);

    lib_memory_set(bytes, 0, sizeof(bytes));
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = 1u;
    bytes[16u] = 0xa0u; bytes[17u] = 0x01u;
    bytes[18u] = 0xa9u; bytes[19u] = 0x55u;
    bytes[20u] = 0x85u; bytes[21u] = 0x11u;
    bytes[22u] = 0xa2u; bytes[23u] = 0x01u;
    bytes[24u] = 0xb4u; bytes[25u] = 0x10u;
    bytes[26u] = 0xa9u; bytes[27u] = 0x66u;
    bytes[28u] = 0x85u; bytes[29u] = 0x11u;
    bytes[30u] = 0xa0u; bytes[31u] = 0x01u;
    bytes[32u] = 0xb6u; bytes[33u] = 0x10u;
    bytes[16u + 0x3ffcu] = 0x00u; bytes[16u + 0x3ffdu] = 0x80u;
    assert(core_machine_create(&machine, bytes, sizeof(bytes), &options) == LIB_STATUS_OK);
    run_one(machine, 2u); run_one(machine, 2u); run_one(machine, 3u); run_one(machine, 2u);
    run_one(machine, 4u);
    assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK && observation.y == 0x55u);
    run_one(machine, 2u); run_one(machine, 3u); run_one(machine, 2u); run_one(machine, 4u);
    assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK && observation.x == 0x66u);
    core_machine_destroy(machine);

    lib_memory_set(bytes, 0, sizeof(bytes));
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = 1u;
    bytes[16u] = 0xa9u; bytes[17u] = 0x44u;
    bytes[18u] = 0x8du; bytes[19u] = 0x00u; bytes[20u] = 0x02u;
    bytes[21u] = 0xa9u; bytes[22u] = 0x00u;
    bytes[23u] = 0x85u; bytes[24u] = 0x20u;
    bytes[25u] = 0xa9u; bytes[26u] = 0x02u;
    bytes[27u] = 0x85u; bytes[28u] = 0x21u;
    bytes[29u] = 0xa2u; bytes[30u] = 0x00u;
    bytes[31u] = 0xa1u; bytes[32u] = 0x20u;
    bytes[33u] = 0xa0u; bytes[34u] = 0x01u;
    bytes[35u] = 0xa9u; bytes[36u] = 0x55u;
    bytes[37u] = 0x8du; bytes[38u] = 0x01u; bytes[39u] = 0x02u;
    bytes[40u] = 0xb1u; bytes[41u] = 0x20u;
    bytes[16u + 0x3ffcu] = 0x00u; bytes[16u + 0x3ffdu] = 0x80u;
    assert(core_machine_create(&machine, bytes, sizeof(bytes), &options) == LIB_STATUS_OK);
    run_one(machine, 2u); run_one(machine, 4u); run_one(machine, 2u);
    run_one(machine, 3u); run_one(machine, 2u); run_one(machine, 3u); run_one(machine, 2u);
    run_one(machine, 6u);
    {
        core_bus_transfer trace[CORE_MACHINE_TRACE_CAPACITY];
        lib_u32 count = 0u;
        assert(core_machine_trace_copy(machine, trace, CORE_MACHINE_TRACE_CAPACITY,
            &count) == LIB_STATUS_OK);
        assert(count == 6u);
        assert(trace[0].address == 0x800fu && trace[1].address == 0x8010u);
        assert(trace[2].address == 0x0020u && trace[2].kind == CORE_BUS_TRANSFER_READ);
        assert(trace[3].address == 0x0020u && trace[3].kind == CORE_BUS_TRANSFER_READ);
        assert(trace[4].address == 0x0021u && trace[4].kind == CORE_BUS_TRANSFER_READ);
        assert(trace[5].address == 0x0200u && trace[5].kind == CORE_BUS_TRANSFER_READ &&
            trace[5].value == 0x44u);
    }
    assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK && observation.a == 0x44u);
    run_one(machine, 2u); run_one(machine, 2u); run_one(machine, 4u); run_one(machine, 5u);
    assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK && observation.a == 0x55u);
    core_machine_destroy(machine);

    /* Zero-page indexed forms use the unindexed zero-page address for their
     * fourth transfer, then perform the effective write. */
    lib_memory_set(bytes, 0, sizeof(bytes));
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = 1u;
    bytes[16u] = 0xa2u; bytes[17u] = 0x01u;
    bytes[18u] = 0xa9u; bytes[19u] = 0x55u;
    bytes[20u] = 0x95u; bytes[21u] = 0x10u;
    bytes[16u + 0x3ffcu] = 0x00u; bytes[16u + 0x3ffdu] = 0x80u;
    assert(core_machine_create(&machine, bytes, sizeof(bytes), &options) == LIB_STATUS_OK);
    run_one(machine, 2u); run_one(machine, 2u); run_one(machine, 4u);
    {
        core_bus_transfer trace[CORE_MACHINE_TRACE_CAPACITY];
        lib_u32 count = 0u;
        assert(core_machine_trace_copy(machine, trace, CORE_MACHINE_TRACE_CAPACITY,
            &count) == LIB_STATUS_OK);
        assert(count == 4u);
        assert(trace[0].address == 0x8004u && trace[0].kind == CORE_BUS_TRANSFER_READ);
        assert(trace[1].address == 0x8005u && trace[1].kind == CORE_BUS_TRANSFER_READ);
        assert(trace[2].address == 0x0010u && trace[2].kind == CORE_BUS_TRANSFER_READ);
        assert(trace[3].address == 0x0011u && trace[3].kind == CORE_BUS_TRANSFER_WRITE &&
            trace[3].value == 0x55u);
    }
    core_machine_destroy(machine);

    /* STA absolute,X consumes its fifth cycle as a read even when indexing
     * stays within the page.  The final write must follow that discard read. */
    lib_memory_set(bytes, 0, sizeof(bytes));
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = 1u;
    bytes[16u] = 0xa9u; bytes[17u] = 0x55u;
    bytes[18u] = 0xa2u; bytes[19u] = 0x01u;
    bytes[20u] = 0x9du; bytes[21u] = 0x00u; bytes[22u] = 0x02u;
    bytes[16u + 0x3ffcu] = 0x00u; bytes[16u + 0x3ffdu] = 0x80u;
    assert(core_machine_create(&machine, bytes, sizeof(bytes), &options) == LIB_STATUS_OK);
    run_one(machine, 2u); run_one(machine, 2u); run_one(machine, 5u);
    {
        core_bus_transfer trace[CORE_MACHINE_TRACE_CAPACITY];
        lib_u32 count = 0u;
        assert(core_machine_trace_copy(machine, trace, CORE_MACHINE_TRACE_CAPACITY,
            &count) == LIB_STATUS_OK);
        assert(count == 5u);
        assert(trace[0].address == 0x8004u && trace[0].kind == CORE_BUS_TRANSFER_READ);
        assert(trace[1].address == 0x8005u && trace[1].kind == CORE_BUS_TRANSFER_READ);
        assert(trace[2].address == 0x8006u && trace[2].kind == CORE_BUS_TRANSFER_READ);
        assert(trace[3].address == 0x0201u && trace[3].kind == CORE_BUS_TRANSFER_READ);
        assert(trace[4].address == 0x0201u && trace[4].kind == CORE_BUS_TRANSFER_WRITE &&
            trace[4].value == 0x55u);
    }
    core_machine_destroy(machine);
    return 0;
}
