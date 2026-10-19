#ifndef TEST_CPU_INSTRUCTION_FIXTURE_H
#define TEST_CPU_INSTRUCTION_FIXTURE_H

#include "x86-core/cpu/cpu.h"
#include "x86-core/cpu/cpu_instructions.h"

/* CPU-owned instruction tests: real storage, no PC board or address aliases. */
typedef struct cpu_instruction_fixture {
    t_cpu cpu;
    t_cpuins instructions;
    x86_core_cpu_execution_context execution;
    x86_core_cpu_fault_snapshot fault;
    x86_core_cpu_fault_snapshot delivered_exception;
    lib_u32 writes;
    lib_u8 memory[524288];
} cpu_instruction_fixture;

static lib_status cpu_instruction_read(void *opaque, lib_u32 address,
    void *destination, lib_u8 bytes,
    x86_core_cpu_memory_access_provenance provenance,
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
    x86_core_cpu_memory_access_provenance provenance, lib_u8 cache_control)
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

/* This isolated instruction bus has no external wait controller. */
static inline lib_status cpu_instruction_special_cycle(void *opaque,
    x86_core_cpu_special_cycle cycle, lib_bool *acknowledged)
{
    (void)opaque;
    (void)cycle;
    *acknowledged = LIB_TRUE;
    return LIB_STATUS_OK;
}

static inline void cpu_instruction_cancel_special_cycle(void *opaque,
    x86_core_cpu_special_cycle cycle)
{
    (void)opaque;
    (void)cycle;
}

static const x86_core_cpu_bus_provider cpu_instruction_bus = {
    .read_memory = cpu_instruction_read,
    .write_memory = cpu_instruction_write,
    .interrupt_pending = cpu_instruction_interrupt_pending,
    .special_cycle = cpu_instruction_special_cycle,
    .cancel_special_cycle = cpu_instruction_cancel_special_cycle
};

static void cpu_instruction_fault(void *opaque,
    const x86_core_cpu_fault_snapshot *fault)
{
    ((cpu_instruction_fixture *)opaque)->fault = *fault;
}

static void cpu_instruction_delivered_exception(void *opaque,
    const x86_core_cpu_fault_snapshot *snapshot)
{
    ((cpu_instruction_fixture *)opaque)->delivered_exception = *snapshot;
}

static const x86_core_cpu_execution_diagnostic_provider cpu_instruction_diagnostics = {
    .record_delivered_exception = cpu_instruction_delivered_exception,
    .record_fault = cpu_instruction_fault
};

static void cpu_instruction_prepare_with_bus(cpu_instruction_fixture *fixture,
    x86_core_cpu_profile profile,
    const x86_core_cpu_bus_provider *bus, void *bus_context)
{
    const x86_core_cpu_instruction_timing timing = { .base_ticks = 1u };

    lib_memory_set(fixture, 0, sizeof(*fixture));
    x86_core_cpu_execution_context_initialize(&fixture->execution,
        &fixture->cpu, &fixture->instructions, bus, bus_context);
    x86_core_cpu_execution_context_bind_profiles(&fixture->execution,
        profile, X86_FPU_PROFILE_NONE, LIB_FALSE, &timing);
    x86_core_cpu_state_initialize(&fixture->execution);
    x86_core_cpu_state_reset(&fixture->execution);
    x86_core_cpu_execution_context_bind_diagnostic_provider(
        &fixture->execution, &cpu_instruction_diagnostics, fixture);
    x86_core_cpu_execution_load_segment(&fixture->execution,
        &fixture->cpu.data.cs, 0u);
    x86_core_cpu_execution_load_segment(&fixture->execution,
        &fixture->cpu.data.ds, 0u);
    x86_core_cpu_execution_load_segment(&fixture->execution,
        &fixture->cpu.data.es, 0u);
    x86_core_cpu_execution_load_segment(&fixture->execution,
        &fixture->cpu.data.ss, 0u);
    fixture->cpu.data.eip = 0u;
}

static inline void cpu_instruction_prepare(cpu_instruction_fixture *fixture,
    x86_core_cpu_profile profile)
{
    cpu_instruction_prepare_with_bus(fixture, profile,
        &cpu_instruction_bus, fixture);
}

static inline lib_status cpu_instruction_run(cpu_instruction_fixture *fixture,
    const lib_u8 *code, lib_u8 bytes, t_cpu *after)
{
    lib_memory_copy(fixture->memory, code, bytes);
    x86_core_cpu_execution_refresh(&fixture->execution);
    *after = fixture->cpu;
    return fixture->execution.stop_requested ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK;
}

static inline lib_bool cpu_instruction_segments_same(const t_cpu *before,
    const t_cpu *after)
{
    return lib_memory_compare(&before->data.es, &after->data.es,
            sizeof(before->data.es)) == 0 &&
        lib_memory_compare(&before->data.cs, &after->data.cs,
            sizeof(before->data.cs)) == 0 &&
        lib_memory_compare(&before->data.ss, &after->data.ss,
            sizeof(before->data.ss)) == 0 &&
        lib_memory_compare(&before->data.ds, &after->data.ds,
            sizeof(before->data.ds)) == 0 &&
        lib_memory_compare(&before->data.fs, &after->data.fs,
            sizeof(before->data.fs)) == 0 &&
        lib_memory_compare(&before->data.gs, &after->data.gs,
            sizeof(before->data.gs)) == 0;
}

/* Real guest bootstrap shared by protected stack instruction scenarios. */
static inline lib_bool cpu_instruction_boot_protected(cpu_instruction_fixture *state)
{
    static const lib_u8 pointer[] = {0x1fu, 0u, 0u, 0x03u, 0u, 0u};
    static const lib_u8 gdt[] = {
        0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
        0xffu, 0xffu, 0u, 0x20u, 0u, 0x9au, 0u, 0u,
        0xffu, 0xffu, 0u, 0x30u, 0u, 0x92u, 0u, 0u,
        0xffu, 0xffu, 0u, 0x40u, 0u, 0x92u, 0u, 0u
    };
    static const lib_u8 bootstrap[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u, 0xb8u, 0x01u, 0x00u,
        0x0fu, 0x01u, 0xf0u, 0xb8u, 0x10u, 0x00u, 0x8eu, 0xd8u,
        0x8eu, 0xc0u, 0xb8u, 0x18u, 0x00u, 0x8eu, 0xd0u, 0xbcu,
        0x00u, 0x80u, 0xeau, 0x00u, 0x00u, 0x08u, 0x00u
    };
    lib_u8 instruction;

    if (cpu_instruction_write(state, 0x0100u, pointer, sizeof(pointer),
            X86_CORE_CPU_MEMORY_ACCESS_DATA, 0u) != LIB_STATUS_OK ||
        cpu_instruction_write(state, 0x0300u, gdt, sizeof(gdt),
            X86_CORE_CPU_MEMORY_ACCESS_DATA, 0u) != LIB_STATUS_OK ||
        cpu_instruction_write(state, 0u, bootstrap, sizeof(bootstrap),
            X86_CORE_CPU_MEMORY_ACCESS_DATA, 0u) != LIB_STATUS_OK)
        return LIB_FALSE;
    for (instruction = 0u; instruction != 10u; ++instruction) {
        x86_core_cpu_execution_refresh(&state->execution);
        if (state->execution.stop_requested || state->fault.valid) return LIB_FALSE;
    }
    return state->cpu.data.cs.selector == 8u && state->cpu.data.eip == 0u;
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
    const x86_core_cpu_profile profile = state->execution.cpu_profile;
    const lib_u32 exception = UINT32_C(1) << id;
    const lib_u32 writes = state->writes;

    before = state->cpu;
    if (profile < X86_CORE_CPU_PROFILE_80186)
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
            (profile < X86_CORE_CPU_PROFILE_80286 ? 0xf000u : 0u)))
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
