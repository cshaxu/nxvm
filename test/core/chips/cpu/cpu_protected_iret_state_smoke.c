#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "core/x86/device_support_interface.h"

#include "support/cpu_instruction_fixture.h"

#define IRET_GDT_BASE 0x0300u
#define IRET_CODE_BASE 0x2000u
#define IRET_STACK 0x8000u
#define IRET_CODE_ACCESS (IRET_GDT_BASE + 13u)

typedef enum iret_negative {
    IRET_NEGATIVE_NONE,
    IRET_NEGATIVE_NONPRESENT,
    IRET_NEGATIVE_LIMIT,
    IRET_NEGATIVE_CODE_TYPE,
    IRET_NEGATIVE_CODE_DPL,
    IRET_NEGATIVE_STACK_LIMIT
} iret_negative;

typedef cpu_instruction_fixture iret_machine;

static lib_i32 iret_write(iret_machine *state, lib_u32 address,
    const void *data, lib_size bytes)
{
    if (address > sizeof(state->memory) ||
        bytes > sizeof(state->memory) - address) return 0;
    lib_memory_copy(state->memory + address, data, bytes);
    return 1;
}
static lib_status iret_read(iret_machine *state, lib_u32 address,
    void *data, lib_size bytes)
{
    if (address > sizeof(state->memory) ||
        bytes > sizeof(state->memory) - address) return LIB_STATUS_IO_ERROR;
    lib_memory_copy(data, state->memory + address, bytes);
    return LIB_STATUS_OK;
}
static lib_i32 iret_prepare(iret_machine *state, iret_negative negative,
    lib_i32 small_stack, lib_i32 conforming, lib_i32 user_cpl)
{
    lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0x40u,0,
        0xffu,0xffu,0,0,0,0x92u,0xcfu,0
    };
    t_cpu *cpu;

    if (state == LIB_NULL) return 0;
    cpu_instruction_prepare(state, CORE_MACHINE_CPU_PROFILE_80386);
    if (negative == IRET_NEGATIVE_NONPRESENT) gdt[13u] &= 0x7fu;
    if (conforming) gdt[13u] = 0x9eu;
    if (negative == IRET_NEGATIVE_CODE_TYPE) gdt[13u] = 0x92u;
    if (negative == IRET_NEGATIVE_CODE_DPL) gdt[13u] = 0xbau;
    if (user_cpl) {
        gdt[13u] = 0xfau;
        gdt[21u] = 0xf2u;
    }
    if (negative == IRET_NEGATIVE_LIMIT) {
        gdt[8u] = 0u;
        gdt[9u] = 0u;
    }
    if (!iret_write(state, IRET_GDT_BASE, gdt, sizeof(gdt))) return 0;
    cpu = &state->cpu;
    cpu->data.cr0 = VCPU_CR0_PE;
    cpu->data.gdtr.flagValid = LIB_TRUE;
    cpu->data.gdtr.sregtype = SREG_GDTR;
    cpu->data.gdtr.base = IRET_GDT_BASE;
    cpu->data.gdtr.limit = sizeof(gdt) - 1u;
    cpu->data.cs.flagValid = LIB_TRUE;
    cpu->data.cs.selector = user_cpl ? 0x000bu : 0x0008u;
    cpu->data.cs.sregtype = SREG_CODE;
    cpu->data.cs.base = IRET_CODE_BASE;
    cpu->data.cs.limit = 0xffffu;
    cpu->data.cs.dpl = user_cpl ? 3u : 0u;
    cpu->data.cs.seg.accessed = LIB_FALSE;
    cpu->data.cs.seg.executable = LIB_TRUE;
    cpu->data.cs.seg.exec.defsize = LIB_TRUE;
    cpu->data.cs.seg.exec.conform = LIB_FALSE;
    cpu->data.cs.seg.exec.readable = LIB_TRUE;
    cpu->data.ss.flagValid = LIB_TRUE;
    cpu->data.ss.selector = user_cpl ? 0x0013u : 0x0010u;
    cpu->data.ss.sregtype = SREG_STACK;
    cpu->data.ss.base = 0u;
    cpu->data.ss.limit = negative == IRET_NEGATIVE_STACK_LIMIT ? 1u :
        0xffffffffu;
    cpu->data.ss.dpl = user_cpl ? 3u : 0u;
    cpu->data.ss.seg.accessed = LIB_FALSE;
    cpu->data.ss.seg.executable = LIB_FALSE;
    cpu->data.ss.seg.data.big = small_stack ? LIB_FALSE : LIB_TRUE;
    cpu->data.ss.seg.data.expdown = LIB_FALSE;
    cpu->data.ss.seg.data.writable = LIB_TRUE;
    cpu->data.eip = 0u;
    cpu->data.esp = small_stack ? 0x00018000u : IRET_STACK;
    cpu->data.eflags = user_cpl ? 0x00000002u : 0x00000202u;
    cpu->data.flagHalt = LIB_FALSE;
    return 1;
}

static lib_i32 iret_run(iret_machine *state, lib_i32 expect_fault, t_cpu *after,
    core_machine_cpu_fault_snapshot *diagnostic)
{
    lib_u32 instructions = 16u;
    while (instructions-- != 0u && !state->execution.stop_requested && !state->cpu.data.flagHalt)
        core_machine_cpu_execution_refresh(&state->execution);
    *after = state->cpu;
    *diagnostic = state->fault;
    return expect_fault ? core_machine_cpu_is_shutdown(&state->execution) &&
        !state->execution.stop_requested && !state->fault.valid &&
        state->delivered_exception.valid &&
        state->delivered_exception.exception_mask == VCPUINS_EXCEPT_SHUTDOWN :
        !state->execution.stop_requested && state->cpu.data.flagHalt;
}
static lib_i32 iret_test_success(lib_u8 prefix, lib_i32 operand16,
    lib_i32 small_stack, lib_i32 conforming)
{
    iret_machine state;
    core_machine_cpu_fault_snapshot diagnostic;
    t_cpu after;
    lib_u8 code[] = { prefix, 0xcfu, 0xf4u };
    lib_u32 frame32[] = { prefix ? 2u : 1u, 0x0008u, 0x00000203u };
    lib_u16 frame16[] = { 2u, 0x0008u, 0x0203u };
    lib_u32 expected_esp = small_stack ?
        (operand16 ? 0x00018006u : 0x0001800cu) :
        (operand16 ? IRET_STACK + 6u : IRET_STACK + 12u);
    lib_i32 failed = !iret_prepare(&state, IRET_NEGATIVE_NONE, small_stack,
        conforming, 0);

    if (!failed) {
        failed |= !iret_write(&state, IRET_CODE_BASE, prefix ? code : code + 1u,
                prefix ? sizeof(code) : sizeof(code) - 1u) ||
            !iret_write(&state, IRET_STACK, operand16 ? (const void *)frame16 :
                (const void *)frame32, operand16 ? sizeof(frame16) : sizeof(frame32)) ||
            !iret_run(&state, 0, &after, &diagnostic) || diagnostic.valid ||
            after.data.eip != (prefix ? 3u : 2u) || after.data.esp != expected_esp ||
            after.data.cs.selector != 0x0008u || after.data.eflags != 0x00000203u;
    }
    return !failed;
}

static lib_i32 iret_test_failure(iret_negative negative, lib_u32 mask,
    lib_u32 code)
{
    iret_machine state;
    core_machine_cpu_fault_snapshot diagnostic;
    t_cpu before;
    t_cpu after;
    lib_u8 program[] = {0xcfu,0xf4u};
    lib_u32 frame[] = { negative == IRET_NEGATIVE_LIMIT ? 1u : 1u,
        0x0008u, 0x00000203u };
    lib_u8 access_before = 0u;
    lib_u8 access_after = 0u;
    lib_i32 failed = !iret_prepare(&state, negative, 0, 0, 0);

    (void)mask;
    (void)code;

    if (!failed) {
        before = state.cpu;
        failed |= !iret_write(&state, IRET_CODE_BASE, program, sizeof(program)) ||
            !iret_write(&state, IRET_STACK, frame, sizeof(frame)) ||
            iret_read(&state,
                IRET_CODE_ACCESS, (void *)CORE_MACHINE_REFERENCE_OF(access_before), 1u) != LIB_STATUS_OK ||
            !iret_run(&state, 1, &after, &diagnostic) ||
            iret_read(&state,
                IRET_CODE_ACCESS, (void *)CORE_MACHINE_REFERENCE_OF(access_after), 1u) != LIB_STATUS_OK ||
            after.data.eip != before.data.eip || after.data.esp != before.data.esp ||
            after.data.eflags != before.data.eflags ||
            lib_memory_compare(&after.data.cs, &before.data.cs, sizeof(before.data.cs)) != 0 ||
            lib_memory_compare(&after.data.ss, &before.data.ss, sizeof(before.data.ss)) != 0 ||
            access_after != access_before;
    }
    return !failed;
}

static lib_i32 iret_test_user_flags(void)
{
    core_machine_cpu_fault_snapshot diagnostic;
    iret_machine state;
    t_cpu after;
    lib_u8 program[] = {0xcfu,0x90u};
    lib_u32 frame[] = {1u,0x000bu,0x00013203u};
    lib_i32 failed = !iret_prepare(&state, IRET_NEGATIVE_NONE, 0, 0, 1);

    if (!failed) {
        failed |= !iret_write(&state, IRET_CODE_BASE, program, sizeof(program)) ||
            !iret_write(&state, IRET_STACK, frame, sizeof(frame));
        if (!failed) core_machine_cpu_execution_refresh(&state.execution);
        failed |= state.execution.stop_requested;
        diagnostic = state.fault;
        after = state.cpu;
        failed |= diagnostic.valid || after.data.eip != 1u ||
            after.data.esp != IRET_STACK + 12u || after.data.cs.selector != 0x000bu ||
            after.data.eflags != 0x00010003u;
    }
    return !failed;
}

static lib_bool iret_test_flags_matrix(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    lib_u8 profile, wide, outer, cpl, old_iopl, new_iopl, bits;
    lib_u32 count = 0u, failures = 0u;

    for (profile = 0u; profile < 2u; ++profile)
    for (wide = 0u; wide <= profile; ++wide)
    for (outer = 0u; outer < 2u; ++outer)
    for (cpl = 0u; cpl < (outer ? 3u : 4u); ++cpl)
    for (old_iopl = 0u; old_iopl < 4u; ++old_iopl)
    for (new_iopl = 0u; new_iopl < 4u; ++new_iopl)
    for (bits = 0u; bits < (profile ? 16u : 4u); ++bits) {
        iret_machine state;
        const lib_u8 target_cpl = outer ? 3u : cpl;
        const lib_u8 code[] = {0x66u,0xcfu};
        const lib_u8 length = profile && !wide ? 2u : 1u;
        const lib_u32 oldflags = 2u | ((lib_u32)old_iopl << 12u) |
            ((bits & 1u) ? VCPU_EFLAGS_IF : 0u) |
            ((bits & 4u) ? VCPU_EFLAGS_RF : 0u);
        const lib_u32 newflags = 3u | ((lib_u32)new_iopl << 12u) |
            ((bits & 2u) ? VCPU_EFLAGS_IF : 0u) |
            ((bits & 8u) ? VCPU_EFLAGS_RF : 0u);
        lib_u32 frame32[] = {length,8u | target_cpl,newflags,0x9000u,
            0x10u | target_cpl};
        lib_u16 frame16[] = {length,(lib_u16)(8u | target_cpl),
            (lib_u16)newflags,0x9000u,(lib_u16)(0x10u | target_cpl)};
        lib_u32 preserve = wide ? 0u : 0xffff0000u;
        lib_u32 expected;

        if (cpl) preserve |= VCPU_EFLAGS_IOPL;
        if (cpl > old_iopl) preserve |= VCPU_EFLAGS_IF;
        expected = (oldflags & preserve) | (newflags & ~preserve);
        if (!profile) expected &= 0x7fd5u;
        expected |= 2u;
        if (!iret_prepare(&state, IRET_NEGATIVE_NONE, 0, 0, 0)) return LIB_FALSE;
        state.execution.cpu_profile = profiles[profile];
        state.cpu.data.cs.dpl = cpl;
        state.cpu.data.cs.selector = (lib_u16)(8u | cpl);
        state.cpu.data.ss.dpl = cpl;
        state.cpu.data.eflags = oldflags;
        state.memory[IRET_GDT_BASE + 13u] = (lib_u8)(0x9au | (target_cpl << 5u));
        state.memory[IRET_GDT_BASE + 21u] = (lib_u8)(0x92u | (target_cpl << 5u));
        state.cpu.data.ss.seg.data.big = LIB_FALSE;
        if (!iret_write(&state, IRET_CODE_BASE, length == 2u ? code : code + 1u,
                length) || !iret_write(&state, IRET_STACK,
                wide ? (const void *)frame32 : (const void *)frame16,
                (wide ? 4u : 2u) * (outer ? 5u : 3u))) return LIB_FALSE;
        core_machine_cpu_execution_refresh(&state.execution);
        ++count;
        if (state.execution.stop_requested || state.fault.valid ||
            state.delivered_exception.valid || state.cpu.data.eip != length ||
            state.cpu.data.cs.dpl != target_cpl || state.cpu.data.eflags != expected ||
            state.cpu.data.esp != (outer ? 0x9000u : IRET_STACK +
                (wide ? 12u : 6u))) {
            if (failures < 4u) lib_c_printf("IRET flags profile=%u wide=%u outer=%u cpl=%u old=%x new=%x got=%x expected=%x ip=%x sp=%x fault=%x\n",
                profile, wide, outer, cpl, oldflags, newflags,
                state.cpu.data.eflags, expected, state.cpu.data.eip,
                state.cpu.data.esp, state.fault.exception_mask);
            ++failures;
        }
    }
    lib_c_printf("IRET flags cases=%u failures=%u\n", (unsigned)count,
        (unsigned)failures);
    return failures == 0u;
}

lib_i32 main(void)
{
    if (!iret_test_flags_matrix()) return 1;
    if (!iret_test_success(0u, 0, 0, 0) || !iret_test_success(0x66u, 1, 0, 0) ||
        !iret_test_success(0x67u, 0, 0, 0) || !iret_test_success(0x66u, 1, 1, 0) ||
        !iret_test_success(0u, 0, 0, 1) ||
        !iret_test_user_flags() ||
        !iret_test_failure(IRET_NEGATIVE_NONPRESENT, VCPUINS_EXCEPT_DF, 0u) ||
        !iret_test_failure(IRET_NEGATIVE_LIMIT, VCPUINS_EXCEPT_DF, 0u) ||
        !iret_test_failure(IRET_NEGATIVE_CODE_TYPE, VCPUINS_EXCEPT_DF, 0u) ||
        !iret_test_failure(IRET_NEGATIVE_CODE_DPL, VCPUINS_EXCEPT_DF, 0u) ||
        !iret_test_failure(IRET_NEGATIVE_STACK_LIMIT, VCPUINS_EXCEPT_DF, 0u)) return 1;
    lib_c_printf("PROTECTED-IRET-CPU-STATE:OK\n");
    lib_c_printf("PROTECTED-IRET:OK\n");
    return 0;
}
