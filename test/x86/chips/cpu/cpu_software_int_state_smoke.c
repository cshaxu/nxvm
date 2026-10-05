#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

typedef struct software_int_form {
    lib_u8 vector;
    lib_u8 opcode[2];
    lib_u8 bytes;
    lib_i32 requires_overflow;
} software_int_form;

static void software_int_state_seed(cpu_instruction_fixture *state, lib_u32 flags)
{
    state->cpu.data.eax = 0xaabbccddu;
    state->cpu.data.ecx = 0x11223344u;
    state->cpu.data.edx = 0x55667788u;
    state->cpu.data.ebx = 0x99aabbccu;
    state->cpu.data.esp = 0x8000u;
    state->cpu.data.ebp = 0x120u;
    state->cpu.data.esi = 0x10u;
    state->cpu.data.edi = 0x20u;
    state->cpu.data.eflags = flags;
}

/* T337_REAL_UD_TERMINAL_CPU_OWNER: these negatives limit IDTR before any exception-frame
 * write. Only the CPU owner may seed that private architectural state. */
static lib_bool software_int_state_reject(cpu_instruction_fixture *state,
    core_machine_cpu_profile profile, const lib_u8 *code, lib_u8 bytes)
{
    t_cpu before, after;
    lib_u8 stack_before[16], stack_after[16];
    cpu_instruction_prepare(state, profile);
    software_int_state_seed(state, VCPU_EFLAGS_OF | VCPU_EFLAGS_IF | VCPU_EFLAGS_CF);
    state->cpu.data.idtr.limit = 0x17u;
    before = state->cpu;
    lib_memory_copy(stack_before, state->memory + 0x7ff0u, sizeof(stack_before));
    const lib_status status = cpu_instruction_run(state, code, bytes, &after);
    lib_memory_copy(stack_after, state->memory + 0x7ff0u, sizeof(stack_after));
    return status == LIB_STATUS_INTERNAL_ERROR && state->fault.valid &&
        (state->fault.exception_mask & VCPUINS_EXCEPT_UD) != 0u &&
        lib_memory_compare(&before, &after, sizeof(before)) == 0 &&
        lib_memory_compare(stack_before, stack_after, sizeof(stack_before)) == 0;
}

static lib_i32 software_int_s50_test_rejections(void)
{
    static const software_int_form forms[] = {
        { 0x03u, { 0xccu, 0u }, 1u, 0 },
        { 0x31u, { 0xcdu, 0x31u }, 2u, 0 },
        { 0x04u, { 0xceu, 0u }, 1u, 1 }
    };
    static const lib_u8 prefixes[][2] = {
        { 0x66u, 0u },
        { 0x67u, 0u },
        { 0x66u, 0x67u }
    };
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_8088,
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    lib_u8 profile;

    for (profile = 0u; profile != sizeof(legacy) / sizeof(legacy[0]);
        ++profile) {
        lib_u8 prefix;

        for (prefix = 0u; prefix != sizeof(prefixes) / sizeof(prefixes[0]);
            ++prefix) {
            lib_u8 form;

            for (form = 0u; form != sizeof(forms) / sizeof(forms[0]); ++form) {
                cpu_instruction_fixture state;
                const lib_u8 prefix_bytes = prefix == 2u ? 2u : 1u;
                lib_u8 code[5] = {0};
                lib_memory_copy(code, prefixes[prefix], prefix_bytes);
                lib_memory_copy(code + prefix_bytes, forms[form].opcode, forms[form].bytes);
                if (!software_int_state_reject(&state, legacy[profile], code,
                        prefix_bytes + forms[form].bytes)) return 0;
            }
        }
    }
    for (profile = 0u; profile != 4u; ++profile) {
        lib_u8 form;

        for (form = 0u; form != sizeof(forms) / sizeof(forms[0]); ++form) {
            cpu_instruction_fixture state;
            const lib_u8 prefix_bytes = profile == 0u ? 0u : profile == 3u ? 2u : 1u;
            lib_u8 code[5] = {0xf0u,0,0,0,0};
            if (profile != 0u) lib_memory_copy(code + 1u, prefixes[profile - 1u], prefix_bytes);
            lib_memory_copy(code + 1u + prefix_bytes, forms[form].opcode, forms[form].bytes);
            if (!software_int_state_reject(&state, CORE_MACHINE_CPU_PROFILE_80386,
                    code, 1u + prefix_bytes + forms[form].bytes)) return 0;
        }
    }
    return 1;
}

static lib_bool software_int_state_transfer(core_machine_cpu_profile profile,
    const software_int_form *form, const lib_u8 *prefix, lib_u8 prefix_bytes)
{
    cpu_instruction_fixture state;
    t_cpu before;
    const lib_u8 width = prefix_bytes != 0u && prefix[0] == 0x66u ? 4u : 2u;
    const lib_u16 offset = 0x100u, segment = 0u;
    cpu_instruction_prepare(&state, profile);
    software_int_state_seed(&state, VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | VCPU_EFLAGS_IF |
        VCPU_EFLAGS_TF | VCPU_EFLAGS_DF | VCPU_EFLAGS_OF | 0x8000u);
    lib_memory_copy(state.memory, prefix, prefix_bytes);
    lib_memory_copy(state.memory + prefix_bytes, form->opcode, form->bytes);
    lib_memory_copy(state.memory + form->vector * 4u, &offset, sizeof(offset));
    lib_memory_copy(state.memory + form->vector * 4u + 2u, &segment, sizeof(segment));
    state.memory[offset] = 0xf4u;
    before = state.cpu;
    core_machine_cpu_execution_refresh(&state.execution);
    core_machine_cpu_execution_refresh(&state.execution);
    const t_cpu *after = &state.cpu;
    return !state.execution.stop_requested && !state.fault.valid && after->data.flagHalt &&
        after->data.eip == 0x101u && after->data.esp == before.data.esp - 3u * width &&
        after->data.eax == before.data.eax && after->data.ecx == before.data.ecx &&
        after->data.edx == before.data.edx && after->data.ebx == before.data.ebx &&
        after->data.ebp == before.data.ebp && after->data.esi == before.data.esi &&
        after->data.edi == before.data.edi &&
        lib_memory_compare(&after->data.es, &before.data.es, sizeof(before.data.es)) == 0 &&
        lib_memory_compare(&after->data.cs, &before.data.cs, sizeof(before.data.cs)) == 0 &&
        lib_memory_compare(&after->data.ss, &before.data.ss, sizeof(before.data.ss)) == 0 &&
        lib_memory_compare(&after->data.ds, &before.data.ds, sizeof(before.data.ds)) == 0 &&
        lib_memory_compare(&after->data.fs, &before.data.fs, sizeof(before.data.fs)) == 0 &&
        lib_memory_compare(&after->data.gs, &before.data.gs, sizeof(before.data.gs)) == 0;
}

static lib_bool software_int_state_into_clear(core_machine_cpu_profile profile,
    const lib_u8 *prefix, lib_u8 prefix_bytes)
{
    cpu_instruction_fixture state;
    t_cpu before;
    cpu_instruction_prepare(&state, profile);
    software_int_state_seed(&state, VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | VCPU_EFLAGS_IF | VCPU_EFLAGS_DF);
    lib_memory_copy(state.memory, prefix, prefix_bytes);
    state.memory[prefix_bytes] = 0xceu;
    state.memory[prefix_bytes + 1u] = 0xf4u;
    before = state.cpu;
    core_machine_cpu_execution_refresh(&state.execution);
    core_machine_cpu_execution_refresh(&state.execution);
    before.data.eip = prefix_bytes + 2u;
    before.data.flagHalt = LIB_TRUE;
    return !state.execution.stop_requested && !state.fault.valid &&
        lib_memory_compare(&before, &state.cpu, sizeof(before)) == 0;
}

static lib_i32 software_int_state_real_forms(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_8088,
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    static const software_int_form forms[] = {
        { 0x03u, { 0xccu, 0u }, 1u, 0 },
        { 0x31u, { 0xcdu, 0x31u }, 2u, 0 },
        { 0x04u, { 0xceu, 0u }, 1u, 1 }
    };
    static const lib_u8 no_prefix[] = { 0u };
    static const lib_u8 prefixes[][2] = {
        { 0x66u, 0u },
        { 0x67u, 0u },
        { 0x66u, 0x67u }
    };
    lib_u8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        lib_u8 form;

        for (form = 0u; form != sizeof(forms) / sizeof(forms[0]); ++form) {
            if (!software_int_state_transfer(profiles[profile], &forms[form],
                    no_prefix, 0u))
                return 0;
        }
        if (!software_int_state_into_clear(profiles[profile], no_prefix, 0u))
            return 0;
    }
    for (profile = 0u; profile != sizeof(prefixes) / sizeof(prefixes[0]);
        ++profile) {
        lib_u8 form;
        lib_u8 prefix_bytes = profile == 2u ? 2u : 1u;

        for (form = 0u; form != sizeof(forms) / sizeof(forms[0]); ++form) {
            if (!software_int_state_transfer(CORE_MACHINE_CPU_PROFILE_80386,
                    &forms[form], prefixes[profile], prefix_bytes))
                return 0;
        }
        if (!software_int_state_into_clear(CORE_MACHINE_CPU_PROFILE_80386,
                prefixes[profile], prefix_bytes))
            return 0;
    }
    return 1;
}

int main(void)
{
    if (!software_int_state_real_forms() || !software_int_s50_test_rejections()) return 1;
    lib_c_printf("%s\n", "M5:T316:S50:SOFTWARE-INT:CPU-ROLLBACK:OK");
    return 0;
}
