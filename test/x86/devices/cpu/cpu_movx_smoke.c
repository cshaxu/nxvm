#include "support/cpu_instruction_fixture.h"
#include <stdio.h>

typedef struct movx_form {
    lib_u8 opcode;
    lib_u32 source;
    lib_u32 result;
    lib_u8 source_bytes;
} movx_form;

static lib_i32 movx_run(cpu_instruction_fixture *state, const lib_u8 *code,
    lib_size bytes, t_cpu *after)
{
    lib_memory_copy(state->memory, code, bytes);
    core_machine_cpu_execution_refresh(&state->execution);
    *after = state->cpu;
    return !state->fault.valid && state->instructions.data.except == 0u &&
        !state->execution.stop_requested;
}

static lib_i32 movx_test_forms(void)
{
    static const movx_form forms[] = {
        {0xb6u, 0x00000080u, 0x00000080u, 1u},
        {0xb7u, 0x00008001u, 0x00008001u, 2u},
        {0xbeu, 0x00000080u, 0xffffff80u, 1u},
        {0xbfu, 0x00008001u, 0xffff8001u, 2u}
    };
    lib_u8 form_index;
    lib_i32 operand32;
    lib_i32 memory;

    for (form_index = 0u; form_index != sizeof(forms) / sizeof(forms[0]); ++form_index) {
        for (operand32 = 0; operand32 != 2; ++operand32) {
            for (memory = 0; memory != 2; ++memory) {
                lib_u8 code[6] = {0};
                lib_u8 source[2] = {
                    (lib_u8)forms[form_index].source,
                    (lib_u8)(forms[form_index].source >> 8u)
                };
                const lib_u32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF |
                    VCPU_EFLAGS_SF;
                const lib_u32 expected = operand32 ? forms[form_index].result :
                    (0xaabb0000u | (forms[form_index].result & 0xffffu));
                const lib_size code_size = (operand32 ? 1u : 0u) +
                    (memory ? 5u : 3u);
                cpu_instruction_fixture state;
                t_cpu after;
                lib_i32 failed = 0;
                lib_size index = 0u;

                cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

                if (operand32) code[index++] = 0x66u;
                code[index++] = 0x0fu;
                code[index++] = forms[form_index].opcode;
                if (memory) {
                    code[index++] = 0x0eu;
                    code[index++] = 0x00u;
                    code[index++] = 0x40u;
                } else code[index++] = 0xc8u;
                state.cpu.data.eax = forms[form_index].source;
                state.cpu.data.ecx = 0xaabbccddu;
                state.cpu.data.eflags = flags;
                if (memory) lib_memory_copy(state.memory + 0x4000u, source,
                    forms[form_index].source_bytes);
                failed |= !movx_run(&state, code, code_size, &after) ||
                    after.data.ecx != expected || after.data.eflags != flags ||
                    after.data.eip != code_size;
                if (failed) return 0;
            }
        }
    }
    return 1;
}

static lib_i32 movx_test_address_prefix(void)
{
    static const lib_u8 code[] = {0x67u,0x66u,0x0fu,0xbfu,0x0eu};
    const lib_u8 source[] = {0x01u,0x80u};
    const lib_u32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
    cpu_instruction_fixture state;
    t_cpu after;
    lib_i32 failed = 0;

    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    state.cpu.data.ecx = 0xaabbccddu;
    state.cpu.data.esi = 0x4000u;
    state.cpu.data.eflags = flags;
    lib_memory_copy(state.memory + 0x4000u, source, sizeof(source));
    failed |= !movx_run(&state, code, sizeof(code), &after) ||
        after.data.ecx != 0xffff8001u || after.data.esi != 0x4000u ||
        after.data.eflags != flags || after.data.eip != sizeof(code);
    return !failed;
}

lib_i32 main(void)
{
    if (!movx_test_forms() || !movx_test_address_prefix()) return 1;
    printf("M5:T539:S21:MOVX-CPU:OK\n");
    return 0;
}
