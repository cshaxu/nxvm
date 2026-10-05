#include "support/cpu_protected_fixture.h"
#include "lib/types/file.h"

/* Far transfer owns descriptor lookup and cached segment state. */
static lib_bool pft_same(const t_cpu *before, const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ecx == after->data.ecx && before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx && before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp && before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi && before->data.eip == after->data.eip &&
        before->data.eflags == after->data.eflags &&
        lib_memory_compare(&before->data.es, &after->data.es, sizeof(before->data.es)) == 0 &&
        lib_memory_compare(&before->data.cs, &after->data.cs, sizeof(before->data.cs)) == 0 &&
        lib_memory_compare(&before->data.ss, &after->data.ss, sizeof(before->data.ss)) == 0 &&
        lib_memory_compare(&before->data.ds, &after->data.ds, sizeof(before->data.ds)) == 0 &&
        lib_memory_compare(&before->data.fs, &after->data.fs, sizeof(before->data.fs)) == 0 &&
        lib_memory_compare(&before->data.gs, &after->data.gs, sizeof(before->data.gs)) == 0;
}

static lib_bool pft_run_forms(core_machine_cpu_profile profile)
{
    static const lib_u8 jmp[] = {0xeau,0,0,0x18u,0};
    static const lib_u8 call[] = {0x9au,0,0,0x18u,0};
    static const lib_u8 indirect_jmp[] = {0xffu,0x2eu,0,1};
    static const lib_u8 indirect_call[] = {0xffu,0x1eu,0,1};
    static const lib_u8 pointer[] = {0,0,0x18u,0};
    const lib_u8 *const forms[] = {jmp, call, indirect_jmp, indirect_call};
    const lib_u8 sizes[] = {sizeof(jmp), sizeof(call), sizeof(indirect_jmp), sizeof(indirect_call)};
    cpu_instruction_fixture state;
    t_cpu before, after;
    lib_u16 frame[2];

    for (lib_size index = 0u; index != 4u; ++index) {
        cpu_protected_prepare_far(&state, profile);
        state.memory[0x4000u] = 0xf4u;
        if (index >= 2u) lib_memory_copy(state.memory + CPU_PROTECTED_DATA_BASE + 0x100u,
            pointer, sizeof(pointer));
        before = state.cpu;
        if (!cpu_protected_step(&state, forms[index], sizes[index], &after) ||
            after.data.cs.selector != 0x18u || after.data.cs.base != 0x4000u ||
            after.data.eip != 0u || after.data.eflags != before.data.eflags) return LIB_FALSE;
        if (index == 1u || index == 3u) {
            lib_memory_copy(frame, state.memory + CPU_PROTECTED_DATA_BASE + 0x7ffcu,
                sizeof(frame));
            if (after.data.esp != 0x7ffcu || frame[0] != sizes[index] || frame[1] != 0x08u)
                return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_bool pft_run_386_forms(void)
{
    static const lib_u8 jmp32[] = {0x66u,0xeau,0,1,0,0,0x18u,0};
    static const lib_u8 call32[] = {0x66u,0x9au,0,1,0,0,0x18u,0};
    static const lib_u8 pointer16[] = {0,1,0x18u,0};
    static const lib_u8 pointer32[] = {0,1,0,0,0x18u,0};
    static const lib_u8 jmp67[] = {0x67u,0xffu,0x2du,0,1,0,0};
    static const lib_u8 jmp3267[] = {0x66u,0x67u,0xffu,0x2du,0,1,0,0};
    static const lib_u8 call3267[] = {0x66u,0x67u,0xffu,0x1du,0,1,0,0};
    static const lib_u8 lock_jmp[] = {0xf0u,0xeau,0,0,0x18u,0};
    static const lib_u8 lock_call[] = {0xf0u,0x9au,0,0,0x18u,0};
    static const lib_u8 old_jmp[] = {0x66u,0xeau,0,0,0,0,0x18u,0};
    static const lib_u8 old_call[] = {0x66u,0x9au,0,0,0,0,0x18u,0};
    cpu_instruction_fixture state;
    t_cpu before, after;

    cpu_protected_prepare_far(&state, CORE_MACHINE_CPU_PROFILE_80386);
    state.memory[0x4100u] = 0xf4u;
    if (!cpu_protected_step(&state, jmp32, sizeof(jmp32), &after) || after.data.eip != 0x100u)
        return LIB_FALSE;
    cpu_protected_prepare_far(&state, CORE_MACHINE_CPU_PROFILE_80386);
    state.memory[0x4100u] = 0xf4u;
    if (!cpu_protected_step(&state, call32, sizeof(call32), &after) || after.data.eip != 0x100u ||
        after.data.esp != 0x7ff8u) return LIB_FALSE;
    cpu_protected_prepare_far(&state, CORE_MACHINE_CPU_PROFILE_80386);
    state.memory[0x4100u] = 0xf4u;
    lib_memory_copy(state.memory + CPU_PROTECTED_DATA_BASE + 0x100u, pointer16, sizeof(pointer16));
    if (!cpu_protected_step(&state, jmp67, sizeof(jmp67), &after) || after.data.eip != 0x100u)
        return LIB_FALSE;
    cpu_protected_prepare_far(&state, CORE_MACHINE_CPU_PROFILE_80386);
    state.memory[0x4100u] = 0xf4u;
    lib_memory_copy(state.memory + CPU_PROTECTED_DATA_BASE + 0x100u, pointer32, sizeof(pointer32));
    if (!cpu_protected_step(&state, jmp3267, sizeof(jmp3267), &after) || after.data.eip != 0x100u)
        return LIB_FALSE;
    cpu_protected_prepare_far(&state, CORE_MACHINE_CPU_PROFILE_80386);
    state.memory[0x4100u] = 0xf4u;
    lib_memory_copy(state.memory + CPU_PROTECTED_DATA_BASE + 0x100u, pointer32, sizeof(pointer32));
    if (!cpu_protected_step(&state, call3267, sizeof(call3267), &after) || after.data.eip != 0x100u ||
        after.data.esp != 0x7ff8u) return LIB_FALSE;
    cpu_protected_prepare_far(&state, CORE_MACHINE_CPU_PROFILE_80386);
    before = state.cpu;
    if (!cpu_protected_fault(&state, lock_jmp, sizeof(lock_jmp), VCPUINS_EXCEPT_UD, &before) ||
        !pft_same(&before, &state.cpu)) return LIB_FALSE;
    cpu_protected_prepare_far(&state, CORE_MACHINE_CPU_PROFILE_80386);
    before = state.cpu;
    if (!cpu_protected_fault(&state, lock_call, sizeof(lock_call), VCPUINS_EXCEPT_UD, &before) ||
        !pft_same(&before, &state.cpu)) return LIB_FALSE;
    cpu_protected_prepare_far(&state, CORE_MACHINE_CPU_PROFILE_80286);
    before = state.cpu;
    if (!cpu_protected_fault(&state, old_jmp, sizeof(old_jmp), VCPUINS_EXCEPT_UD, &before) ||
        !pft_same(&before, &state.cpu)) return LIB_FALSE;
    cpu_protected_prepare_far(&state, CORE_MACHINE_CPU_PROFILE_80286);
    before = state.cpu;
    return cpu_protected_fault(&state, old_call, sizeof(old_call), VCPUINS_EXCEPT_UD, &before) &&
        pft_same(&before, &state.cpu);
}

static lib_bool pft_descriptor_and_rollback(void)
{
    static const lib_u8 rejected[][5] = {{0x9au,0,0,0x28u,0},{0xeau,0,0,0x1bu,0},
        {0x9au,0,0,0x30u,0},{0xeau,0,0,0x38u,0}};
    static const lib_u8 conform_jmp[] = {0xeau,0,0x10u,0x20u,0};
    static const lib_u8 conform_call[] = {0x9au,0,0x10u,0x20u,0};
    static const lib_u8 jmp_limit[] = {0xeau,1,0,0x18u,0};
    static const lib_u8 call_stack[] = {0x9au,0,0,0x18u,0};
    cpu_instruction_fixture state;
    t_cpu before, after;
    lib_u32 sentinel = 0x11223344u;

    for (lib_size index = 0u; index < 4u; ++index) {
        cpu_protected_prepare_far(&state, CORE_MACHINE_CPU_PROFILE_80386); before = state.cpu;
        if (!cpu_protected_fault(&state, rejected[index], sizeof(rejected[index]),
            VCPUINS_EXCEPT_DF, &before) || !pft_same(&before, &state.cpu)) return LIB_FALSE;
    }
    cpu_protected_prepare_far(&state, CORE_MACHINE_CPU_PROFILE_80386); state.memory[0x6000u] = 0xf4u;
    if (!cpu_protected_step(&state, conform_jmp, sizeof(conform_jmp), &after) ||
        after.data.cs.selector != 0x20u || !after.data.cs.seg.exec.conform) return LIB_FALSE;
    cpu_protected_prepare_far(&state, CORE_MACHINE_CPU_PROFILE_80386); state.memory[0x6000u] = 0xf4u;
    if (!cpu_protected_step(&state, conform_call, sizeof(conform_call), &after) ||
        after.data.cs.selector != 0x20u || !after.data.cs.seg.exec.conform || after.data.esp != 0x7ffcu)
        return LIB_FALSE;
    cpu_protected_prepare_far(&state, CORE_MACHINE_CPU_PROFILE_80386);
    state.memory[CPU_PROTECTED_GDT_BASE + 0x18u] = state.memory[CPU_PROTECTED_GDT_BASE + 0x19u] = 0u;
    before = state.cpu;
    if (!cpu_protected_fault(&state, jmp_limit, sizeof(jmp_limit), VCPUINS_EXCEPT_DF, &before) ||
        !pft_same(&before, &state.cpu)) return LIB_FALSE;
    cpu_protected_prepare_far(&state, CORE_MACHINE_CPU_PROFILE_80386); state.cpu.data.ss.limit = 1u;
    lib_memory_copy(state.memory + CPU_PROTECTED_DATA_BASE + 0x7ffcu, &sentinel, sizeof(sentinel)); before = state.cpu;
    if (!cpu_protected_fault(&state, call_stack, sizeof(call_stack), VCPUINS_EXCEPT_DF, &before) ||
        !pft_same(&before, &state.cpu)) return LIB_FALSE;
    lib_memory_copy(&sentinel, state.memory + CPU_PROTECTED_DATA_BASE + 0x7ffcu, sizeof(sentinel));
    return sentinel == 0x11223344u;
}

int main(void)
{
    if (!pft_run_forms(CORE_MACHINE_CPU_PROFILE_80286) || !pft_run_forms(CORE_MACHINE_CPU_PROFILE_80386) ||
        !pft_run_386_forms() || !pft_descriptor_and_rollback()) {
        lib_c_fprintf(lib_c_stderr, "%s", "M5:T539:S53:CPU-PROTECTED-FAR:FAIL\n"); return 1;
    }
    lib_c_printf("%s\n", "M5:T323:S1:PROTECTED-FAR:OK"); lib_c_printf("%s\n", "M5:T539:S53:CPU-PROTECTED-FAR:OK"); return 0;
}
