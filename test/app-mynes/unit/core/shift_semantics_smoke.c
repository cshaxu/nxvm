#include <assert.h>

#include "core/machine_interface.h"

typedef struct shift_case {
    lib_u8 opcode;
    lib_u8 input;
    lib_bool carry;
    lib_u8 output;
    lib_u8 flags;
} shift_case;

static void run_memory_case(const shift_case *test)
{
    lib_u8 bytes[16u + 16384u];
    core_machine_options options = { 0 };
    core_machine *machine = LIB_NULL;
    core_run_result run;
    core_observation observation;
    lib_u8 value;
    lib_memory_set(bytes, 0, sizeof(bytes));
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = 1u;
    bytes[16u] = 0xa9u; bytes[17u] = test->input;
    bytes[18u] = 0x85u; bytes[19u] = 0x10u;
    bytes[20u] = test->carry ? 0x38u : 0x18u;
    bytes[21u] = test->opcode; bytes[22u] = 0x10u;
    bytes[16u + 0x3ffcu] = 0u; bytes[16u + 0x3ffdu] = 0x80u;
    assert(core_machine_create(&machine, bytes, sizeof(bytes), &options) == LIB_STATUS_OK);
    assert(core_machine_run(machine, 4u, 20u, &run) == LIB_STATUS_OK);
    assert(!run.trap_valid && run.instructions == 4u && run.cycles == 12u);
    assert(core_machine_peek(machine, 0x10u, 1u, &value) == LIB_STATUS_OK && value == test->output);
    assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK);
    assert((observation.p & 0xc3u) == test->flags);
    core_machine_destroy(machine);
}

static void run_accumulator_case(const shift_case *test)
{
    lib_u8 bytes[16u + 16384u];
    core_machine_options options = { 0 };
    core_machine *machine = LIB_NULL;
    core_run_result run;
    core_observation observation;
    lib_memory_set(bytes, 0, sizeof(bytes));
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = 1u;
    bytes[16u] = 0xa9u; bytes[17u] = test->input;
    bytes[18u] = test->carry ? 0x38u : 0x18u;
    bytes[19u] = test->opcode;
    bytes[16u + 0x3ffcu] = 0u; bytes[16u + 0x3ffdu] = 0x80u;
    assert(core_machine_create(&machine, bytes, sizeof(bytes), &options) == LIB_STATUS_OK);
    assert(core_machine_run(machine, 3u, 10u, &run) == LIB_STATUS_OK);
    assert(!run.trap_valid && run.instructions == 3u && run.cycles == 6u);
    assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK);
    assert(observation.a == test->output && (observation.p & 0xc3u) == test->flags);
    core_machine_destroy(machine);
}

int main(void)
{
    lib_size index;
    static const shift_case memory_cases[] = {
        {0x06u, 0x80u, LIB_FALSE, 0u, 0x03u},
        {0x46u, 0x01u, LIB_FALSE, 0u, 0x03u},
        {0x26u, 0x80u, LIB_TRUE, 1u, 0x01u},
        {0x66u, 0x01u, LIB_TRUE, 0x80u, 0x81u}
    };
    static const shift_case accumulator_cases[] = {
        {0x0au, 0x80u, LIB_FALSE, 0u, 0x03u},
        {0x4au, 0x01u, LIB_FALSE, 0u, 0x03u},
        {0x2au, 0x80u, LIB_TRUE, 1u, 0x01u},
        {0x6au, 0x01u, LIB_TRUE, 0x80u, 0x81u}
    };
    for (index = 0u; index < sizeof(memory_cases) / sizeof(memory_cases[0]); ++index)
        run_memory_case(&memory_cases[index]);
    for (index = 0u; index < sizeof(accumulator_cases) / sizeof(accumulator_cases[0]); ++index)
        run_accumulator_case(&accumulator_cases[index]);
    return 0;
}
