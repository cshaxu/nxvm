#include <assert.h>
#include <string.h>

#include "core/machine_interface.h"

#define FLAG_C 0x01u
#define FLAG_Z 0x02u
#define FLAG_D 0x08u
#define FLAG_U 0x20u
#define FLAG_I 0x04u
#define FLAG_V 0x40u
#define FLAG_N 0x80u

static lib_u8 expected_flags(lib_u8 a, lib_u8 value, lib_bool carry,
    lib_bool subtract, lib_bool decimal)
{
    lib_u16 total = (lib_u16)a + (subtract ? (lib_u8)~value : value) +
        (carry ? 1u : 0u);
    lib_u8 result = (lib_u8)total;
    lib_u8 flags = FLAG_U | FLAG_I;
    if (decimal) flags |= FLAG_D;
    if (total > 0xffu) flags |= FLAG_C;
    if (result == 0u) flags |= FLAG_Z;
    if ((result & 0x80u) != 0u) flags |= FLAG_N;
    if (((a ^ result) & ((subtract ? (lib_u8)~value : value) ^ result) & 0x80u) != 0u)
        flags |= FLAG_V;
    return flags;
}

static void run_case(lib_u8 opcode, lib_u8 a, lib_u8 value, lib_bool carry,
    lib_bool decimal)
{
    lib_u8 bytes[16u + 16384u];
    core_machine_options options = { 0 };
    core_machine *machine = LIB_NULL;
    core_run_result run;
    core_observation observation;
    lib_bool subtract = opcode == 0xe9u;
    lib_u16 total = (lib_u16)a + (subtract ? (lib_u8)~value : value) +
        (carry ? 1u : 0u);

    memset(bytes, 0, sizeof(bytes));
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = 1u;
    bytes[16u] = 0xa9u; bytes[17u] = a;
    bytes[18u] = carry ? 0x38u : 0x18u;
    bytes[19u] = decimal ? 0xf8u : 0xd8u;
    bytes[20u] = opcode; bytes[21u] = value;
    bytes[16u + 0x3ffcu] = 0u; bytes[16u + 0x3ffdu] = 0x80u;
    assert(core_machine_create(&machine, bytes, sizeof(bytes), &options) == LIB_STATUS_OK);
    assert(core_machine_run(machine, 4u, 10u, &run) == LIB_STATUS_OK);
    assert(!run.trap_valid && run.instructions == 4u && run.cycles == 8u);
    assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK);
    assert(observation.a == (lib_u8)total);
    assert(observation.p == expected_flags(a, value, carry, subtract, decimal));
    core_machine_destroy(machine);
}

int main(void)
{
    lib_u32 opcode_index;
    lib_u32 decimal;
    lib_u32 carry;
    lib_u32 a;
    lib_u32 value;
    static const lib_u8 opcodes[] = { 0x69u, 0xe9u };

    for (opcode_index = 0u; opcode_index < sizeof(opcodes); ++opcode_index)
        for (decimal = 0u; decimal < 2u; ++decimal)
            for (carry = 0u; carry < 2u; ++carry)
                for (a = 0u; a < 256u; ++a)
                    for (value = 0u; value < 256u; ++value)
                        run_case(opcodes[opcode_index], (lib_u8)a, (lib_u8)value,
                            carry != 0u, decimal != 0u);
    return 0;
}
