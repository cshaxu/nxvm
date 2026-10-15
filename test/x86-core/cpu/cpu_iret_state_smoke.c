#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

static lib_i32 iret_prepare(cpu_instruction_fixture *state,
    x86_core_cpu_profile profile)
{
    cpu_instruction_prepare(state, profile);
    return 1;
}
static lib_i32 iret_entry(cpu_instruction_fixture *state, lib_u32 eip)
{
    state->cpu.data.eip = eip;
    return 1;
}
static lib_status iret_write(cpu_instruction_fixture *state,
    lib_u32 address, const void *data, lib_size bytes)
{
    if (address > sizeof(state->memory) ||
        bytes > sizeof(state->memory) - address) return LIB_STATUS_IO_ERROR;
    lib_memory_copy(state->memory + address, data, bytes);
    return LIB_STATUS_OK;
}
static lib_status iret_read(cpu_instruction_fixture *state,
    lib_u32 address, void *data, lib_size bytes)
{
    if (address > sizeof(state->memory) ||
        bytes > sizeof(state->memory) - address) return LIB_STATUS_IO_ERROR;
    lib_memory_copy(data, state->memory + address, bytes);
    return LIB_STATUS_OK;
}
static lib_status iret_run(cpu_instruction_fixture *state, lib_u32 instructions)
{
    while (instructions-- != 0u && !state->execution.stop_requested &&
            !state->cpu.data.flagHalt)
        x86_core_cpu_execution_refresh(&state->execution);
    return state->execution.stop_requested ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK;
}

static lib_i32 iret_sregs_same(const t_cpu *before, const t_cpu *after)
{
    return lib_memory_compare(&before->data.cs, &after->data.cs,
            sizeof(before->data.cs)) == 0 &&
        lib_memory_compare(&before->data.es, &after->data.es,
            sizeof(before->data.es)) == 0 &&
        lib_memory_compare(&before->data.ss, &after->data.ss,
            sizeof(before->data.ss)) == 0 &&
        lib_memory_compare(&before->data.ds, &after->data.ds,
            sizeof(before->data.ds)) == 0 &&
        lib_memory_compare(&before->data.fs, &after->data.fs,
            sizeof(before->data.fs)) == 0 &&
        lib_memory_compare(&before->data.gs, &after->data.gs,
            sizeof(before->data.gs)) == 0;
}

static void iret_seed(cpu_instruction_fixture *state, lib_u32 flags)
{
    t_cpu *cpu = &state->cpu;

    cpu->data.eax = 0xaabbccddu;
    cpu->data.ecx = 0x11223344u;
    cpu->data.edx = 0x55667788u;
    cpu->data.ebx = 0x99aabbccu;
    cpu->data.esp = 0x00018000u;
    cpu->data.ebp = 0x00000120u;
    cpu->data.esi = 0x00000010u;
    cpu->data.edi = 0x00000020u;
    cpu->data.eflags = flags;
}

static lib_u32 iret_real_flags_load(
    x86_core_cpu_profile profile, lib_u32 flags)
{
    const lib_u16 known_mask = profile < X86_CORE_CPU_PROFILE_80286 ?
        0x0fd5u : 0x7fd5u;

    return (flags & known_mask) | 0x02u;
}

static lib_i32 iret_real_case(x86_core_cpu_profile profile,
    const lib_u8 *prefix, lib_u8 prefix_bytes, lib_u8 wrap_stack)
{
    static const lib_u8 hlt = 0xf4u;
    const lib_u32 flags = X86_CORE_CPU_DEBUG_EFLAGS_CF | X86_CORE_CPU_DEBUG_EFLAGS_PF |
        X86_CORE_CPU_DEBUG_EFLAGS_IF | X86_CORE_CPU_DEBUG_EFLAGS_DF | VCPU_EFLAGS_IOPL | X86_CORE_CPU_DEBUG_EFLAGS_NT |
        0x8002u;
    const lib_u32 expected_flags = iret_real_flags_load(profile, flags);
    const lib_i32 wide = prefix_bytes != 0u && prefix[0] == 0x66u;
    const lib_u16 return_ip = wrap_stack ? 0x0200u : 0x0100u;
    const lib_u32 code_offset = wrap_stack ? 0x0100u : 0u;
    cpu_instruction_fixture state;
    x86_core_cpu_fault_snapshot diagnostic;
    t_cpu before;
    t_cpu after;
    lib_u8 code[3] = { 0u, 0u, 0u };
    lib_u16 frame16[] = { return_ip, 0x0000u, (lib_u16)flags };
    lib_u32 frame32[] = { return_ip, 0x00000000u, flags };
    lib_u8 unselected_before[12] = {
        0xd1u, 0xd2u, 0xd3u, 0xd4u, 0xd5u, 0xd6u,
        0xd7u, 0xd8u, 0xd9u, 0xdau, 0xdbu, 0xdcu
    };
    lib_u8 unselected_after[12] = { 0u };
    lib_i32 failed = !iret_prepare(&state, profile);

    if (!failed) {
        lib_memory_copy(code, prefix, prefix_bytes);
        code[prefix_bytes] = 0xcfu;
        failed = !iret_entry(&state, code_offset);
        failed |= iret_write(&state, code_offset, code,
            prefix_bytes + 1u) != LIB_STATUS_OK;
        failed |= iret_write(&state, return_ip, &hlt,
            sizeof(hlt)) != LIB_STATUS_OK;
        if (wrap_stack) {
            const lib_u32 stack_base = state.cpu.data.ss.base;

            failed |= wide || iret_write(&state,
                stack_base + 0xfffeu, frame16, sizeof(frame16[0u])) != LIB_STATUS_OK;
            failed |= iret_write(&state, stack_base,
                frame16 + 1u, sizeof(frame16[0u])) != LIB_STATUS_OK;
            failed |= iret_write(&state, stack_base + 2u,
                frame16 + 2u, sizeof(frame16[0u])) != LIB_STATUS_OK;
        } else {
            failed |= iret_write(&state, 0x8000u,
                wide ? (const void *)frame32 : (const void *)frame16,
                wide ? sizeof(frame32) : sizeof(frame16)) != LIB_STATUS_OK;
        }
        failed |= iret_write(&state, 0x18000u,
            unselected_before, sizeof(unselected_before)) != LIB_STATUS_OK;
    }
    if (!failed) {
        iret_seed(&state, flags);
        if (wrap_stack) state.cpu.data.sp = 0xfffeu;
        before = state.cpu;
        failed |= iret_run(&state, 2u) != LIB_STATUS_OK;
        after = state.cpu;
        diagnostic = state.fault;
        failed |= diagnostic.valid;
        failed |= after.data.eip != return_ip + 1u;
        failed |= !after.data.flagHalt;
        failed |= wrap_stack ? after.data.sp != 0x0004u :
            after.data.esp != before.data.esp + (wide ? 12u : 6u);
        failed |= after.data.cs.selector != 0u || after.data.cs.base != 0u;
        failed |= after.data.cs.limit != before.data.cs.limit;
        failed |= after.data.cs.flagValid != before.data.cs.flagValid;
        failed |= after.data.cs.sregtype != before.data.cs.sregtype;
        failed |= (after.data.eflags & iret_real_flags_load(profile,
            0xffffu)) != expected_flags;
        failed |= after.data.eax != before.data.eax;
        failed |= after.data.ecx != before.data.ecx;
        failed |= after.data.edx != before.data.edx;
        failed |= after.data.ebx != before.data.ebx;
        failed |= after.data.ebp != before.data.ebp;
        failed |= after.data.esi != before.data.esi;
        failed |= after.data.edi != before.data.edi;
        failed |= !iret_sregs_same(&before, &after);
        failed |= iret_read(&state, 0x18000u,
            (void *)unselected_after,
            sizeof(unselected_after)) != LIB_STATUS_OK;
        failed |= lib_memory_compare(unselected_before, unselected_after,
            sizeof(unselected_before)) != 0;
    }
    return !failed;
}

static lib_i32 iret_test_real(void)
{
    static const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_8086,
        X86_CORE_CPU_PROFILE_8088,
        X86_CORE_CPU_PROFILE_80186,
        X86_CORE_CPU_PROFILE_80286,
        X86_CORE_CPU_PROFILE_80386,
        X86_CORE_CPU_PROFILE_80486SX
    };
    static const lib_u8 prefixes[][2] = {
        { 0x66u, 0u },
        { 0x67u, 0u },
        { 0x66u, 0x67u }
    };
    lib_u8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        if (!iret_real_case(profiles[profile], (const lib_u8[]){ 0u },
                0u, LIB_FALSE))
            return 0;
    }
    for (profile = 0u; profile != 4u; ++profile) {
        if (!iret_real_case(profiles[profile], (const lib_u8[]){ 0u },
                0u, LIB_TRUE))
            return 0;
    }
    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        if (profiles[profile] < X86_CORE_CPU_PROFILE_80386) continue;
        for (lib_size prefix = 0u; prefix < sizeof(prefixes) / sizeof(prefixes[0]);
            ++prefix) {
            lib_u8 bytes = prefix == 2u ? 2u : 1u;

            if (!iret_real_case(profiles[profile], prefixes[prefix], bytes,
                LIB_FALSE)) return 0;
        }
    }
    return 1;
}

static lib_i32 iret_test_80286_stack_boundary(void)
{
    static const lib_u8 code[] = { 0xcfu };
    cpu_instruction_fixture state;
    lib_i32 failed = !iret_prepare(&state, X86_CORE_CPU_PROFILE_80286);

    if (!failed) {
        failed = !iret_entry(&state, 0u);
        failed |= iret_write(&state, 0u, code,
            sizeof(code)) != LIB_STATUS_OK;
    }
    if (!failed) {
        iret_seed(&state, X86_CORE_CPU_DEBUG_EFLAGS_CF | X86_CORE_CPU_DEBUG_EFLAGS_IF);
        state.cpu.data.sp = 0xffffu;
        failed |= !cpu_instruction_expect_real_fault(&state, code,
            sizeof(code), 13u);
    }
    return !failed;
}

static lib_i32 iret_expect_ud(x86_core_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes)
{
    cpu_instruction_fixture state;
    lib_i32 failed = !iret_prepare(&state, profile);

    if (!failed) {
        failed = !iret_entry(&state, 0u);
        failed |= iret_write(&state, 0u, code, bytes) !=
            LIB_STATUS_OK;
    }
    if (!failed) {
        iret_seed(&state, X86_CORE_CPU_DEBUG_EFLAGS_CF | X86_CORE_CPU_DEBUG_EFLAGS_IF);
        state.cpu.data.idtr.limit = 0x03ffu;
        failed |= !cpu_instruction_expect_real_fault(&state, code, bytes, 6u);
    }
    return !failed;
}

static lib_i32 iret_test_rejections(void)
{
    static const x86_core_cpu_profile legacy[] = {
        X86_CORE_CPU_PROFILE_8086,
        X86_CORE_CPU_PROFILE_8088,
        X86_CORE_CPU_PROFILE_80186,
        X86_CORE_CPU_PROFILE_80286
    };
    static const lib_u8 attributes[][3] = {
        { 0x66u, 0xcfu, 0u },
        { 0x67u, 0xcfu, 0u },
        { 0x66u, 0x67u, 0xcfu }
    };
    static const lib_u8 lock_forms[][4] = {
        { 0xf0u, 0xcfu, 0u, 0u },
        { 0xf0u, 0x66u, 0xcfu, 0u },
        { 0xf0u, 0x67u, 0xcfu, 0u },
        { 0xf0u, 0x66u, 0x67u, 0xcfu }
    };
    static const x86_core_cpu_profile modern[] = {
        X86_CORE_CPU_PROFILE_80386, X86_CORE_CPU_PROFILE_80486SX
    };
    lib_u8 profile;

    for (profile = 0u; profile != sizeof(legacy) / sizeof(legacy[0]);
        ++profile) {
        lib_u8 attribute;

        for (attribute = 0u;
            attribute != sizeof(attributes) / sizeof(attributes[0]); ++attribute) {
            lib_u8 bytes = attribute == 2u ? 3u : 2u;

            if (!iret_expect_ud(legacy[profile], attributes[attribute], bytes))
                return 0;
        }
    }
    for (profile = 0u; profile != sizeof(modern) / sizeof(modern[0]); ++profile) {
        for (lib_size form = 0u; form < sizeof(lock_forms) / sizeof(lock_forms[0]);
            ++form) {
            lib_u8 bytes = form == 3u ? 4u : form == 0u ? 2u : 3u;

            if (!iret_expect_ud(modern[profile], lock_forms[form], bytes)) return 0;
        }
    }
    return 1;
}


lib_i32 main(void)
{
    if (!iret_test_real() || !iret_test_80286_stack_boundary() ||
        !iret_test_rejections()) return 1;
    lib_c_printf("IRET-CPU-STATE:OK\n");
    return 0;
}
