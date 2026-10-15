#ifndef TEST_CPU_INSTRUCTION_FIXTURE_H
#define TEST_CPU_INSTRUCTION_FIXTURE_H

#include "core/chips/cpu/cpu.h"
#include "core/chips/cpu/cpu_instructions.h"

/* CPU-owned instruction tests: real storage, no PC board or address aliases. */
typedef struct cpu_instruction_fixture {
    t_cpu cpu;
    t_cpuins instructions;
    core_machine_cpu_execution_context execution;
    core_machine_cpu_fault_snapshot fault;
    core_machine_cpu_fault_snapshot delivered_exception;
    lib_u32 writes;
    lib_u8 memory[524288];
} cpu_instruction_fixture;

static lib_status cpu_instruction_read(void *opaque, lib_u32 address,
    void *destination, lib_u8 bytes,
    core_machine_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch, lib_u8 cache_control)
{
    (void)cache_control;
    cpu_instruction_fixture *fixture = (cpu_instruction_fixture *)opaque;

    (void)provenance;
    (void)observe_only;
    (void)reset_fetch;
    if (address > sizeof(fixture->memory) ||
        bytes > sizeof(fixture->memory) - address) return LIB_STATUS_IO_ERROR;
    lib_memory_copy(destination, fixture->memory + address, bytes);
    return LIB_STATUS_OK;
}

static lib_status cpu_instruction_write(void *opaque, lib_u32 address,
    const void *source, lib_u8 bytes,
    core_machine_cpu_memory_access_provenance provenance, lib_u8 cache_control)
{
    (void)cache_control;
    cpu_instruction_fixture *fixture = (cpu_instruction_fixture *)opaque;

    (void)provenance;
    if (address > sizeof(fixture->memory) ||
        bytes > sizeof(fixture->memory) - address) return LIB_STATUS_IO_ERROR;
    lib_memory_copy(fixture->memory + address, source, bytes);
    ++fixture->writes;
    return LIB_STATUS_OK;
}

static lib_bool cpu_instruction_interrupt_pending(void *opaque)
{
    (void)opaque;
    return LIB_FALSE;
}

static const core_machine_cpu_bus_provider cpu_instruction_bus = {
    .read_memory = cpu_instruction_read,
    .write_memory = cpu_instruction_write,
    .interrupt_pending = cpu_instruction_interrupt_pending
};

static void cpu_instruction_fault(void *opaque,
    const core_machine_cpu_fault_snapshot *fault)
{
    ((cpu_instruction_fixture *)opaque)->fault = *fault;
}

static void cpu_instruction_delivered_exception(void *opaque,
    const core_machine_cpu_fault_snapshot *snapshot)
{
    ((cpu_instruction_fixture *)opaque)->delivered_exception = *snapshot;
}

static const core_machine_cpu_execution_diagnostic_provider cpu_instruction_diagnostics = {
    .record_delivered_exception = cpu_instruction_delivered_exception,
    .record_fault = cpu_instruction_fault
};

static void cpu_instruction_prepare_with_bus(cpu_instruction_fixture *fixture,
    core_machine_cpu_profile profile,
    const core_machine_cpu_bus_provider *bus, void *bus_context)
{
    const core_machine_instruction_timing timing = { .base_ticks = 1u };

    lib_memory_set(fixture, 0, sizeof(*fixture));
    core_machine_cpu_execution_context_initialize(&fixture->execution,
        &fixture->cpu, &fixture->instructions, bus, bus_context);
    core_machine_cpu_execution_context_bind_profiles(&fixture->execution,
        profile, X86_FPU_PROFILE_NONE, LIB_FALSE, &timing);
    core_machine_cpu_state_initialize(&fixture->execution);
    core_machine_cpu_state_reset(&fixture->execution);
    core_machine_cpu_execution_context_bind_diagnostic_provider(
        &fixture->execution, &cpu_instruction_diagnostics, fixture);
    core_machine_cpu_execution_load_segment(&fixture->execution,
        &fixture->cpu.data.cs, 0u);
    core_machine_cpu_execution_load_segment(&fixture->execution,
        &fixture->cpu.data.ds, 0u);
    core_machine_cpu_execution_load_segment(&fixture->execution,
        &fixture->cpu.data.es, 0u);
    core_machine_cpu_execution_load_segment(&fixture->execution,
        &fixture->cpu.data.ss, 0u);
    fixture->cpu.data.eip = 0u;
}

static inline void cpu_instruction_prepare(cpu_instruction_fixture *fixture,
    core_machine_cpu_profile profile)
{
    cpu_instruction_prepare_with_bus(fixture, profile,
        &cpu_instruction_bus, fixture);
}

static inline lib_status cpu_instruction_run(cpu_instruction_fixture *fixture,
    const lib_u8 *code, lib_u8 bytes, t_cpu *after)
{
    lib_memory_copy(fixture->memory, code, bytes);
    core_machine_cpu_execution_refresh(&fixture->execution);
    *after = fixture->cpu;
    return fixture->execution.stop_requested ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK;
}

/* Configure a real protected GP handler, including the VM86 ring-zero stack.
 * The caller retains its selected CPU and bus; this adds no simulated fault. */
static inline void cpu_instruction_prepare_protected_exception(
    cpu_instruction_fixture *fixture, lib_u8 mode)
{
    const lib_bool vm86 = mode == 4u;
    const lib_u8 cpl = vm86 ? 3u : mode;
    const lib_u8 target_cpl = vm86 ? 0u : cpl;
    const lib_u8 descriptor[] = {0xffu, 0xffu, 0u, 0u, 0u,
        (lib_u8)(0x9bu | (target_cpl << 5u)), 0u, 0u};
    const lib_u8 stack[] = {0xffu, 0xffu, 0u, 0u, 0u, 0x93u, 0u, 0u};
    const lib_u8 gate[] = {0u, 3u, (lib_u8)(8u | target_cpl), 0u, 0u, 0x8eu, 0u, 0u};

    fixture->cpu.data.cr0 = VCPU_CR0_PE;
    fixture->cpu.data.cs.selector = (lib_u16)(8u | cpl);
    fixture->cpu.data.cs.dpl = cpl;
    fixture->cpu.data.ss.selector = (lib_u16)(0x10u | cpl);
    fixture->cpu.data.ss.dpl = cpl;
    fixture->cpu.data.esp = 0x800u;
    fixture->cpu.data.eflags = 2u | (vm86 ? VCPU_EFLAGS_VM : 0u);
    fixture->cpu.data.gdtr = (t_cpu_data_sreg){.flagValid = LIB_TRUE,
        .sregtype = SREG_GDTR, .base = 0x200u, .limit = 23u};
    fixture->cpu.data.idtr = (t_cpu_data_sreg){.flagValid = LIB_TRUE,
        .sregtype = SREG_IDTR, .base = 0x400u, .limit = 0x87u};
    lib_memory_copy(fixture->memory + 0x208u, descriptor, sizeof(descriptor));
    lib_memory_copy(fixture->memory + 0x210u, stack, sizeof(stack));
    lib_memory_copy(fixture->memory + 0x468u, gate, sizeof(gate));
    if (vm86) {
        const lib_u32 ring0_sp = 0x1000u;
        const lib_u16 ring0_ss = 0x10u;
        fixture->cpu.data.tr = (t_cpu_data_sreg){.flagValid = LIB_TRUE,
            .sregtype = SREG_TR, .selector = 0x18u, .base = 0x600u, .limit = 0x67u};
        fixture->cpu.data.tr.sys.type = 0xbu;
        lib_memory_copy(fixture->memory + 0x604u, &ring0_sp, sizeof(ring0_sp));
        lib_memory_copy(fixture->memory + 0x608u, &ring0_ss, sizeof(ring0_ss));
    }
}

/* Real UD/GP proofs use valid handlers; early strict diagnostics do not invent
 * hardware exceptions. Missing tables are separate shutdown tests. */
static inline lib_bool cpu_instruction_expect_real_fault(cpu_instruction_fixture *state,
    const lib_u8 *code, lib_u8 bytes, lib_u8 id)
{
    t_cpu before, after;
    const lib_u16 vector[] = {0x0100u, 0u};
    lib_u16 frame[3] = {0};
    const core_machine_cpu_profile profile = state->execution.cpu_profile;
    const lib_u32 exception = UINT32_C(1) << id;
    const lib_u32 writes = state->writes;

    before = state->cpu;
    if (profile < CORE_MACHINE_CPU_PROFILE_80186)
        return cpu_instruction_run(state, code, bytes, &after) == LIB_STATUS_INTERNAL_ERROR &&
            state->fault.valid && state->fault.exception_mask == exception &&
            state->writes == writes &&
            lib_memory_compare(&before, &after, sizeof(before)) == 0;
    lib_memory_copy(state->memory + id * 4u, vector, sizeof(vector));
    if (cpu_instruction_run(state, code, bytes, &after) != LIB_STATUS_OK ||
        state->fault.valid || !state->delivered_exception.valid ||
        state->writes != writes + 3u ||
        state->delivered_exception.exception_mask != exception ||
        state->delivered_exception.exception_code != 0u ||
        state->delivered_exception.point.eip != before.data.eip ||
        state->delivered_exception.point.cs != before.data.cs.selector ||
        state->delivered_exception.eflags != before.data.eflags)
        return LIB_FALSE;
    lib_memory_copy(frame, state->memory + after.data.ss.base + (lib_u16)after.data.esp,
        sizeof(frame));
    if (frame[0] != before.data.ip || frame[1] != before.data.cs.selector ||
        frame[2] != (lib_u16)(before.data.flags | 2u |
            (profile < CORE_MACHINE_CPU_PROFILE_80286 ? 0xf000u : 0u)))
        return LIB_FALSE;
    before.data.eip = 0x0100u;
    before.data.cs.flagValid = LIB_TRUE;
    before.data.cs.selector = 0u;
    before.data.cs.base = 0u;
    before.data.cs.limit = 0xffffu;
    before.data.cs.dpl = 0u;
    before.data.cs.seg.accessed = LIB_TRUE;
    before.data.cs.seg.executable = LIB_TRUE;
    before.data.cs.seg.exec.defsize = LIB_FALSE;
    before.data.cs.seg.exec.conform = LIB_FALSE;
    before.data.cs.seg.exec.readable = LIB_TRUE;
    before.data.sp = (lib_u16)(before.data.sp - 6u);
    before.data.eflags &= ~(VCPU_EFLAGS_IF | VCPU_EFLAGS_TF | VCPU_EFLAGS_RF);
    return lib_memory_compare(&before, &after, sizeof(before)) == 0;
}

#endif
