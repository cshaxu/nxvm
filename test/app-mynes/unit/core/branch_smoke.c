#include <assert.h>

#include "core/machine_interface.h"

typedef struct branch_case {
    lib_u8 opcode;
    lib_u8 setup[5];
    lib_u8 setup_size;
    lib_u8 setup_instructions;
    lib_bool taken;
} branch_case;

static void run_case(const branch_case *test)
{
    lib_u8 bytes[16u + 16384u];
    core_machine_options options = { 0 };
    core_machine *machine = LIB_NULL;
    core_run_result run;
    core_observation observation;
    lib_u16 branch_pc = (lib_u16)(0x8000u + test->setup_size);

    lib_memory_set(bytes, 0, sizeof(bytes));
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = 1u;
    lib_memory_copy(bytes + 16u, test->setup, test->setup_size);
    bytes[16u + test->setup_size] = test->opcode;
    bytes[17u + test->setup_size] = 2u;
    bytes[16u + 0x3ffcu] = 0u; bytes[16u + 0x3ffdu] = 0x80u;
    assert(core_machine_create(&machine, bytes, sizeof(bytes), &options) == LIB_STATUS_OK);
    assert(core_machine_run(machine, test->setup_instructions, 20u, &run) == LIB_STATUS_OK);
    assert(!run.trap_valid && run.instructions == test->setup_instructions);
    assert(core_machine_run(machine, 1u, 10u, &run) == LIB_STATUS_OK);
    assert(!run.trap_valid && run.instructions == 1u && run.cycles == (test->taken ? 3u : 2u));
    assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK);
    assert(observation.pc == (lib_u16)(branch_pc + (test->taken ? 4u : 2u)));
    if (test->taken) {
        core_bus_transfer trace[CORE_MACHINE_TRACE_CAPACITY];
        lib_u32 count = 0u;
        assert(core_machine_trace_copy(machine, trace, CORE_MACHINE_TRACE_CAPACITY,
            &count) == LIB_STATUS_OK);
        assert(count == 3u && trace[2].kind == CORE_BUS_TRANSFER_READ &&
            trace[2].address == (lib_u16)(branch_pc + 4u));
    }
    core_machine_destroy(machine);
}

int main(void)
{
    lib_size index;
    static const branch_case tests[] = {
        {0x10u,{0xa9u,0u},2u,1u,LIB_TRUE},  {0x10u,{0xa9u,0x80u},2u,1u,LIB_FALSE},
        {0x30u,{0xa9u,0x80u},2u,1u,LIB_TRUE},{0x30u,{0xa9u,0u},2u,1u,LIB_FALSE},
        {0x50u,{0x18u},1u,1u,LIB_TRUE},     {0x50u,{0xa9u,0x7fu,0x18u,0x69u,1u},5u,3u,LIB_FALSE},
        {0x70u,{0xa9u,0x7fu,0x18u,0x69u,1u},5u,3u,LIB_TRUE},{0x70u,{0xb8u},1u,1u,LIB_FALSE},
        {0x90u,{0x18u},1u,1u,LIB_TRUE},     {0x90u,{0x38u},1u,1u,LIB_FALSE},
        {0xb0u,{0x38u},1u,1u,LIB_TRUE},     {0xb0u,{0x18u},1u,1u,LIB_FALSE},
        {0xd0u,{0xa9u,1u},2u,1u,LIB_TRUE},  {0xd0u,{0xa9u,0u},2u,1u,LIB_FALSE},
        {0xf0u,{0xa9u,0u},2u,1u,LIB_TRUE},  {0xf0u,{0xa9u,1u},2u,1u,LIB_FALSE}
    };
    lib_u8 bytes[16u + 16384u];
    core_machine_options options = { 0 };
    core_machine *machine = LIB_NULL;
    core_run_result run;
    core_observation observation;

    for (index = 0u; index < sizeof(tests) / sizeof(tests[0]); ++index) run_case(&tests[index]);

    lib_memory_set(bytes, 0, sizeof(bytes));
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = 1u;
    bytes[16u + 0x00fdu] = 0x10u; bytes[16u + 0x00feu] = 1u;
    bytes[16u + 0x3ffcu] = 0xfdu; bytes[16u + 0x3ffdu] = 0x80u;
    assert(core_machine_create(&machine, bytes, sizeof(bytes), &options) == LIB_STATUS_OK);
    assert(core_machine_run(machine, 1u, 10u, &run) == LIB_STATUS_OK);
    assert(!run.trap_valid && run.instructions == 1u && run.cycles == 4u);
    assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK && observation.pc == 0x8100u);
    {
        core_bus_transfer trace[CORE_MACHINE_TRACE_CAPACITY];
        lib_u32 count = 0u;
        assert(core_machine_trace_copy(machine, trace, CORE_MACHINE_TRACE_CAPACITY,
            &count) == LIB_STATUS_OK);
        assert(count == 4u);
        assert(trace[0].address == 0x80fdu && trace[1].address == 0x80feu);
        assert(trace[2].address == 0x8000u && trace[2].kind == CORE_BUS_TRANSFER_READ);
        assert(trace[3].address == 0x8100u && trace[3].kind == CORE_BUS_TRANSFER_READ);
    }
    core_machine_destroy(machine);
    return 0;
}
