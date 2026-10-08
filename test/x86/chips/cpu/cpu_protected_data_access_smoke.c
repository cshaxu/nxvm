#include "support/cpu_protected_fixture.h"
#include "lib/types/file.h"

/* Segment access rules and atomicity live with CPU cached descriptors. */
static lib_bool pda_same(const t_cpu *before, const t_cpu *after,
    lib_bool eax_changes, lib_bool edi_changes)
{
    return (eax_changes || before->data.eax == after->data.eax) &&
        before->data.ecx == after->data.ecx && before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx && before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp && before->data.esi == after->data.esi &&
        (edi_changes || before->data.edi == after->data.edi) &&
        before->data.eflags == after->data.eflags &&
        lib_memory_compare(&before->data.es, &after->data.es, sizeof(before->data.es)) == 0 &&
        lib_memory_compare(&before->data.cs, &after->data.cs, sizeof(before->data.cs)) == 0 &&
        lib_memory_compare(&before->data.ss, &after->data.ss, sizeof(before->data.ss)) == 0 &&
        lib_memory_compare(&before->data.ds, &after->data.ds, sizeof(before->data.ds)) == 0 &&
        lib_memory_compare(&before->data.fs, &after->data.fs, sizeof(before->data.fs)) == 0 &&
        lib_memory_compare(&before->data.gs, &after->data.gs, sizeof(before->data.gs)) == 0;
}

static lib_bool pda_step(cpu_instruction_fixture *state, const lib_u8 *code,
    lib_u8 bytes, t_cpu *after)
{
    return cpu_protected_step(state, code, bytes, after);
}

static lib_bool pda_default_access(core_machine_cpu_profile profile)
{
    static const lib_u8 ds_read[] = {0x8au,0x06u,0x20u,0};
    static const lib_u8 ds_write[] = {0x88u,0x06u,0x21u,0};
    static const lib_u8 ss_read[] = {0x8au,0x46u,0};
    static const lib_u8 stos[] = {0xaau};
    cpu_instruction_fixture state;
    t_cpu before, after;
    lib_u8 value;

    cpu_protected_prepare_data(&state, profile); state.cpu.data.eax = 0xaabbcc44u;
    state.memory[CPU_PROTECTED_DATA_BASE + 0x20u] = 0x5au; before = state.cpu;
    if (!pda_step(&state, ds_read, sizeof(ds_read), &after) || after.data.eax != 0xaabbcc5au ||
        !pda_same(&before, &after, LIB_TRUE, LIB_FALSE)) return LIB_FALSE;
    cpu_protected_prepare_data(&state, profile); state.cpu.data.eax = 0x112233a5u; before = state.cpu;
    if (!pda_step(&state, ds_write, sizeof(ds_write), &after) ||
        state.memory[CPU_PROTECTED_DATA_BASE + 0x21u] != 0xa5u ||
        !pda_same(&before, &after, LIB_FALSE, LIB_FALSE)) return LIB_FALSE;
    cpu_protected_prepare_data(&state, profile); state.cpu.data.ebp = 0x30u;
    state.cpu.data.eax = 0x11223344u; state.memory[CPU_PROTECTED_STACK_BASE + 0x30u] = 0x6bu;
    before = state.cpu;
    if (!pda_step(&state, ss_read, sizeof(ss_read), &after) || after.data.eax != 0x1122336bu ||
        !pda_same(&before, &after, LIB_TRUE, LIB_FALSE)) return LIB_FALSE;
    cpu_protected_prepare_data(&state, profile); state.cpu.data.edi = 0x40u;
    state.cpu.data.eax = 0x5566779cu; value = 0u; state.memory[CPU_PROTECTED_ES_BASE + 0x40u] = value;
    before = state.cpu;
    return pda_step(&state, stos, sizeof(stos), &after) &&
        state.memory[CPU_PROTECTED_ES_BASE + 0x40u] == 0x9cu && after.data.edi == 0x41u &&
        pda_same(&before, &after, LIB_FALSE, LIB_TRUE);
}

static lib_bool pda_386_attributes(void)
{
    static const lib_u8 operand32[] = {0x66u,0x8bu,0x06u,0x20u,0};
    static const lib_u8 address32_ss[] = {0x67u,0x8au,0x45u,0};
    static const lib_u8 lock_read[] = {0xf0u,0x8au,0x06u,0x20u,0};
    static const lib_u8 legacy_prefix[] = {0x66u,0x8au,0x06u,0x20u,0};
    cpu_instruction_fixture state;
    t_cpu before, after;
    lib_u32 value = 0x12345678u;

    cpu_protected_prepare_data(&state, CORE_MACHINE_CPU_PROFILE_80386);
    lib_memory_copy(state.memory + CPU_PROTECTED_DATA_BASE + 0x20u, &value, sizeof(value)); before = state.cpu;
    if (!pda_step(&state, operand32, sizeof(operand32), &after) || after.data.eax != value ||
        !pda_same(&before, &after, LIB_TRUE, LIB_FALSE)) return LIB_FALSE;
    cpu_protected_prepare_data(&state, CORE_MACHINE_CPU_PROFILE_80386); state.cpu.data.ebp = 0x30u;
    state.cpu.data.eax = 0xaabbcc00u; state.memory[CPU_PROTECTED_STACK_BASE + 0x30u] = 0x4du;
    before = state.cpu;
    if (!pda_step(&state, address32_ss, sizeof(address32_ss), &after) || after.data.eax != 0xaabbcc4du ||
        !pda_same(&before, &after, LIB_TRUE, LIB_FALSE)) return LIB_FALSE;
    cpu_protected_prepare_data(&state, CORE_MACHINE_CPU_PROFILE_80386); before = state.cpu;
    if (!cpu_protected_shutdown(&state, lock_read, sizeof(lock_read), &before) ||
        !pda_same(&before, &state.cpu, LIB_FALSE, LIB_FALSE)) return LIB_FALSE;
    cpu_protected_prepare_data(&state, CORE_MACHINE_CPU_PROFILE_80286); before = state.cpu;
    return cpu_protected_shutdown(&state, legacy_prefix, sizeof(legacy_prefix), &before) &&
        pda_same(&before, &state.cpu, LIB_FALSE, LIB_FALSE);
}

static lib_bool pda_fault_atomicity(void)
{
    static const lib_u8 read_ds[] = {0x8au,0x06u,0x20u,0};
    static const lib_u8 read_expand[] = {0x8au,0x06u,0x1fu,0};
    static const lib_u8 write_ds[] = {0x88u,0x06u,0x20u,0};
    static const lib_u8 read_ss[] = {0x8au,0x46u,0};
    static const lib_u8 stos[] = {0xaau};
    cpu_instruction_fixture state;
    t_cpu before;
    lib_u8 sentinel = 0x5au;

    cpu_protected_prepare_data(&state, CORE_MACHINE_CPU_PROFILE_80386);
    state.cpu.data.ds.selector = 0u; state.cpu.data.ds.flagValid = LIB_FALSE; before = state.cpu;
    if (!cpu_protected_shutdown(&state, read_ds, sizeof(read_ds), &before) ||
        !pda_same(&before, &state.cpu, LIB_FALSE, LIB_FALSE)) return LIB_FALSE;
    cpu_protected_prepare_data(&state, CORE_MACHINE_CPU_PROFILE_80386);
    state.cpu.data.ds.seg.data.writable = LIB_FALSE;
    state.memory[CPU_PROTECTED_DATA_BASE + 0x20u] = sentinel; before = state.cpu;
    if (!cpu_protected_shutdown(&state, write_ds, sizeof(write_ds), &before) ||
        state.memory[CPU_PROTECTED_DATA_BASE + 0x20u] != sentinel || !pda_same(&before, &state.cpu, LIB_FALSE, LIB_FALSE)) return LIB_FALSE;
    cpu_protected_prepare_data(&state, CORE_MACHINE_CPU_PROFILE_80386); state.cpu.data.ds.limit = 0x1fu; before = state.cpu;
    if (!cpu_protected_shutdown(&state, read_ds, sizeof(read_ds), &before) ||
        !pda_same(&before, &state.cpu, LIB_FALSE, LIB_FALSE)) return LIB_FALSE;
    cpu_protected_prepare_data(&state, CORE_MACHINE_CPU_PROFILE_80386); state.cpu.data.ds.limit = 0x1fu;
    state.cpu.data.ds.seg.data.expdown = LIB_TRUE; before = state.cpu;
    if (!cpu_protected_shutdown(&state, read_expand, sizeof(read_expand), &before) ||
        !pda_same(&before, &state.cpu, LIB_FALSE, LIB_FALSE)) return LIB_FALSE;
    cpu_protected_prepare_data(&state, CORE_MACHINE_CPU_PROFILE_80386); state.cpu.data.ss.limit = 0x1fu;
    state.cpu.data.ebp = 0x20u; before = state.cpu;
    if (!cpu_protected_shutdown(&state, read_ss, sizeof(read_ss), &before) ||
        !pda_same(&before, &state.cpu, LIB_FALSE, LIB_FALSE)) return LIB_FALSE;
    cpu_protected_prepare_data(&state, CORE_MACHINE_CPU_PROFILE_80386); state.cpu.data.es.limit = 0x1fu;
    state.cpu.data.edi = 0x20u; before = state.cpu;
    return cpu_protected_shutdown(&state, stos, sizeof(stos), &before) &&
        pda_same(&before, &state.cpu, LIB_FALSE, LIB_FALSE);
}

static lib_bool pda_expand_down_success(void)
{
    static const lib_u8 read_ds[] = {0x8au,0x06u,0x20u,0};
    cpu_instruction_fixture state;
    t_cpu before, after;

    cpu_protected_prepare_data(&state, CORE_MACHINE_CPU_PROFILE_80386);
    state.cpu.data.ds.limit = 0x1fu; state.cpu.data.ds.seg.data.expdown = LIB_TRUE;
    state.cpu.data.eax = 0xaabbcc00u; state.memory[CPU_PROTECTED_DATA_BASE + 0x20u] = 0x3cu;
    before = state.cpu;
    return pda_step(&state, read_ds, sizeof(read_ds), &after) && after.data.eax == 0xaabbcc3cu &&
        pda_same(&before, &after, LIB_TRUE, LIB_FALSE);
}

int main(void)
{
    if (!pda_default_access(CORE_MACHINE_CPU_PROFILE_80286) ||
        !pda_default_access(CORE_MACHINE_CPU_PROFILE_80386) || !pda_386_attributes() ||
        !pda_fault_atomicity() || !pda_expand_down_success()) {
        lib_c_fprintf(lib_c_stderr, "%s", "CPU-PROTECTED-DATA:FAIL\n"); return 1;
    }
    lib_c_printf("%s\n", "PROTECTED-DATA-ACCESS:OK"); lib_c_printf("%s\n", "CPU-PROTECTED-DATA:OK"); return 0;
}
