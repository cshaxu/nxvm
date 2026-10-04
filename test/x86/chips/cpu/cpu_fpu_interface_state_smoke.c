#include "support/cpu_instruction_fixture.h"
#include <stdio.h>

/* Full CPU storage rollback remains with the CPU, not a Core receiver. */
static lib_i32 fpu_state_reject(const lib_u8 *code, lib_u8 bytes,
    core_machine_cpu_profile profile, x86_fpu_profile fpu_profile,
    lib_u32 exception)
{
    cpu_instruction_fixture state;
    t_cpu before, after;
    x86_fpu *fpu = LIB_NULL;
    lib_i32 failed;

    cpu_instruction_prepare(&state, profile);
    failed = x86_fpu_create(fpu_profile, &fpu) != LIB_STATUS_OK;
    if (!failed) {
        core_machine_cpu_execution_context_bind_fpu(&state.execution, fpu);
        state.cpu.data.idtr.limit = 0x17u;
        before = state.cpu;
        failed |= cpu_instruction_run(&state, code, bytes, &after) !=
            LIB_STATUS_INTERNAL_ERROR || !state.fault.valid ||
            (state.fault.exception_mask & exception) == 0u ||
            lib_memory_compare(&before, &after, sizeof(before)) != 0;
    }
    x86_fpu_destroy(fpu);
    return failed;
}

static void fpu_state_command(void *owner, lib_u8 opcode, lib_u8 modrm)
{
    (void)owner; (void)opcode; (void)modrm;
}

static lib_i32 fpu_state_success(const lib_u8 *code, lib_u8 bytes,
    core_machine_cpu_profile profile, x86_fpu_profile fpu_profile)
{
    const core_machine_cpu_bus_provider bus = {
        .read_memory = cpu_instruction_read, .write_memory = cpu_instruction_write,
        .interrupt_pending = cpu_instruction_interrupt_pending,
        .extension_command = fpu_state_command
    };
    cpu_instruction_fixture state;
    t_cpu before, after;
    x86_fpu *fpu = LIB_NULL;
    lib_i32 failed;

    cpu_instruction_prepare_with_bus(&state, profile, &bus, &state);
    failed = x86_fpu_create(fpu_profile, &fpu) != LIB_STATUS_OK;
    if (!failed) {
        core_machine_cpu_execution_context_bind_fpu(&state.execution, fpu);
        state.cpu.data.cr0 = 0u;
        before = state.cpu;
        failed |= cpu_instruction_run(&state, code, bytes, &after) != LIB_STATUS_OK ||
            state.fault.valid || after.data.eip != bytes ||
        after.data.eax != before.data.eax ||
        after.data.ebx != before.data.ebx ||
        after.data.ecx != before.data.ecx ||
        after.data.edx != before.data.edx ||
        after.data.esp != before.data.esp ||
        after.data.ebp != before.data.ebp ||
        after.data.esi != before.data.esi ||
        after.data.edi != before.data.edi ||
        after.data.eflags != before.data.eflags ||
        lib_memory_compare(&after.data.es, &before.data.es, sizeof(after.data.es)) != 0 ||
        lib_memory_compare(&after.data.cs, &before.data.cs, sizeof(after.data.cs)) != 0 ||
        lib_memory_compare(&after.data.ss, &before.data.ss, sizeof(after.data.ss)) != 0 ||
        lib_memory_compare(&after.data.ds, &before.data.ds, sizeof(after.data.ds)) != 0 ||
        lib_memory_compare(&after.data.fs, &before.data.fs, sizeof(after.data.fs)) != 0 ||
        lib_memory_compare(&after.data.gs, &before.data.gs, sizeof(after.data.gs)) != 0;
    }
    x86_fpu_destroy(fpu);
    return failed;
}

static lib_i32 fpu_state_vm86(void)
{
    const lib_u8 esc[] = {0xd8u, 0xc0u};
    cpu_instruction_fixture state;
    t_cpu before, after;
    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    state.cpu.data.cr0 |= VCPU_CR0_PE;
    state.cpu.data.eflags = CORE_MACHINE_DEBUG_EFLAGS_VM | VCPU_EFLAGS_IOPL |
        CORE_MACHINE_DEBUG_EFLAGS_IF | CORE_MACHINE_DEBUG_EFLAGS_CF;
    state.cpu.data.cs.selector = 0u; state.cpu.data.cs.base = 0u;
    state.cpu.data.cs.limit = 0xffffu; state.cpu.data.cs.dpl = 3u;
    state.cpu.data.cs.flagValid = LIB_TRUE;
    state.cpu.data.ds.selector = 0u; state.cpu.data.ds.base = 0u;
    state.cpu.data.ds.limit = 0xffffu; state.cpu.data.ds.dpl = 3u;
    state.cpu.data.ds.flagValid = LIB_TRUE;
    state.cpu.data.ss.selector = 0u; state.cpu.data.ss.base = 0u;
    state.cpu.data.ss.limit = 0xffffu; state.cpu.data.ss.dpl = 3u;
    state.cpu.data.ss.flagValid = LIB_TRUE;
    before = state.cpu;
    if (cpu_instruction_run(&state, esc, sizeof(esc), &after) != LIB_STATUS_OK ||
        state.fault.valid || after.data.eip != sizeof(esc) ||
        after.data.eax != before.data.eax || after.data.ecx != before.data.ecx ||
        after.data.edx != before.data.edx || after.data.ebx != before.data.ebx ||
        after.data.esp != before.data.esp || after.data.ebp != before.data.ebp ||
        after.data.esi != before.data.esi || after.data.edi != before.data.edi ||
        after.data.eflags != before.data.eflags) return 1;
    return lib_memory_compare(&after.data.es, &before.data.es, sizeof(after.data.es)) != 0 ||
        lib_memory_compare(&after.data.cs, &before.data.cs, sizeof(after.data.cs)) != 0 ||
        lib_memory_compare(&after.data.ss, &before.data.ss, sizeof(after.data.ss)) != 0 ||
        lib_memory_compare(&after.data.ds, &before.data.ds, sizeof(after.data.ds)) != 0 ||
        lib_memory_compare(&after.data.fs, &before.data.fs, sizeof(after.data.fs)) != 0 ||
        lib_memory_compare(&after.data.gs, &before.data.gs, sizeof(after.data.gs)) != 0;
}

int main(void)
{
    static const lib_u8 attributes[][4] = {
        {0x66u,0x9bu}, {0x67u,0x9bu}, {0x66u,0x67u,0x9bu},
        {0x66u,0xdbu,0xe3u}, {0x67u,0xdbu,0xe3u}, {0x66u,0x67u,0xdbu,0xe3u}
    };
    static const lib_u8 attribute_sizes[] = {2u,2u,3u,3u,3u,4u};
    static const lib_u8 locks[][5] = {
        {0xf0u,0x9bu}, {0xf0u,0x66u,0x9bu}, {0xf0u,0x67u,0x9bu},
        {0xf0u,0x66u,0x67u,0x9bu}, {0xf0u,0xdbu,0xe3u},
        {0xf0u,0x66u,0xdbu,0xe3u}, {0xf0u,0x67u,0xdbu,0xe3u},
        {0xf0u,0x66u,0x67u,0xdbu,0xe3u}
    };
    static const lib_u8 lock_sizes[] = {2u,3u,3u,4u,3u,4u,4u,5u};
    static const lib_u8 fninit[] = {0xdbu,0xe3u};
    static const lib_u8 wait[] = {0x9bu};
    static const lib_u8 escapes[][2] = {
        {0xd8u,0xc0u}, {0xd9u,0xc0u}, {0xdau,0xc0u}, {0xdbu,0xe3u},
        {0xdcu,0xc0u}, {0xddu,0xc0u}, {0xdeu,0xc0u}, {0xdfu,0xc0u}
    };
    core_machine_cpu_profile profile;
    lib_size index;
    lib_i32 failed = 0;
    for (profile = CORE_MACHINE_CPU_PROFILE_8086;
         profile <= CORE_MACHINE_CPU_PROFILE_80386; ++profile) {
        failed |= fpu_state_success(wait, sizeof(wait), profile, X86_FPU_PROFILE_NONE);
        failed |= fpu_state_success(fninit, sizeof(fninit), profile, X86_FPU_PROFILE_NONE);
        for (index = 0u; index < sizeof(escapes) / sizeof(escapes[0]); ++index)
            failed |= fpu_state_success(escapes[index], sizeof(escapes[index]),
                profile, X86_FPU_PROFILE_NONE);
    }
    failed |= fpu_state_success(fninit, sizeof(fninit), CORE_MACHINE_CPU_PROFILE_8086,
        X86_FPU_PROFILE_8087);
    for (index = 0u; index < sizeof(attribute_sizes); ++index)
        failed |= fpu_state_success(attributes[index], attribute_sizes[index],
            CORE_MACHINE_CPU_PROFILE_80386, X86_FPU_PROFILE_NONE);
    for (profile = CORE_MACHINE_CPU_PROFILE_8086;
         profile <= CORE_MACHINE_CPU_PROFILE_80286; ++profile) {
        for (index = 0u; index < sizeof(attribute_sizes); ++index)
            failed |= fpu_state_reject(attributes[index], attribute_sizes[index],
                profile, X86_FPU_PROFILE_NONE, VCPUINS_EXCEPT_UD);
    }
    for (index = 0u; index < sizeof(lock_sizes); ++index)
        failed |= fpu_state_reject(locks[index], lock_sizes[index],
            CORE_MACHINE_CPU_PROFILE_80386, X86_FPU_PROFILE_NONE, VCPUINS_EXCEPT_UD);
    failed |= fpu_state_reject(fninit, sizeof(fninit), CORE_MACHINE_CPU_PROFILE_80386,
        X86_FPU_PROFILE_8087, VCPUINS_EXCEPT_FPU_UNSUPPORTED);
    failed |= fpu_state_vm86();
    if (failed) return 1;
    puts("S65 CPU full rollback/cache state: PASS");
    return 0;
}
