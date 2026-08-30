#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/cpu_instructions.h"
#include "core/machine/machine.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct fpu_interface_s65_machine {
    core_machine *machine;
} fpu_interface_s65_machine;

#define FPU_S65_GDT_POINTER 0x0100u
#define FPU_S65_GDT_BASE 0x0300u
#define FPU_S65_IDT_BASE 0x0400u
#define FPU_S65_CODE_BASE 0x2000u

static C_VOID fpu_interface_s65_reset(C_VOID *opaque)
{
    fpu_interface_s65_machine *state = (fpu_interface_s65_machine *)opaque;

    if (state != STD_NULL) {
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
    }
}

static const core_machine_execution_provider fpu_interface_s65_provider = {
    fpu_interface_s65_reset, STD_NULL
};

static C_INT fpu_interface_s65_prepare(core_machine_cpu_profile profile,
    core_machine_fpu_profile fpu_profile, fpu_interface_s65_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = fpu_profile
    };

    if (state == STD_NULL) {
        return 0;
    }
    STD_MEMSET(state, 0, sizeof(*state));
    return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &fpu_interface_s65_provider, state, &state->machine) &&
        test_core_machine_fixture_prepare_real_mode_execution(
                state->machine, 0u);
}

static C_INT fpu_interface_s65_run(fpu_interface_s65_machine *state,
    const type_unsigned_8 *code, STD_SIZE_T size, t_cpu *after,
    core_machine_cpu_diagnostic *diagnostic, type_status *status)
{
    core_machine_run_result result;

    if (state == STD_NULL || state->machine == STD_NULL || code == STD_NULL ||
        core_machine_memory_write(state->machine, 0u, code, size) !=
            TYPE_STATUS_OK) {
        return 0;
    }
    *status = core_machine_run(state->machine,
        (core_machine_run_budget){ 1u, 0u }, &result);
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) ==
        TYPE_STATUS_OK;
}

static C_INT fpu_interface_s65_same(const t_cpu *before, const t_cpu *after)
{
    return STD_MEMCMP(before, after, sizeof(*before)) == 0;
}

static C_INT fpu_interface_s65_success(const type_unsigned_8 *code, STD_SIZE_T size,
    core_machine_cpu_profile profile, core_machine_fpu_profile fpu_profile,
    type_unsigned_32 cr0)
{
    fpu_interface_s65_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_status status;
    C_INT failed = !fpu_interface_s65_prepare(profile, fpu_profile, &state);

    if (!failed) {
        state.machine->executor_cpu.data.cr0 = cr0;
        before = state.machine->executor_cpu;
        failed |= !fpu_interface_s65_run(&state, code, size, &after,
            &diagnostic, &status) || status != TYPE_STATUS_OK ||
            diagnostic.first_fault.valid || after.data.eip != size ||
            after.data.eax != before.data.eax ||
            after.data.ebx != before.data.ebx ||
            after.data.ecx != before.data.ecx ||
            after.data.edx != before.data.edx ||
            after.data.esp != before.data.esp ||
            after.data.ebp != before.data.ebp ||
            after.data.esi != before.data.esi ||
            after.data.edi != before.data.edi ||
            after.data.eflags != before.data.eflags ||
            STD_MEMCMP(&after.data.es, &before.data.es,
                sizeof(after.data.es)) != 0 ||
            STD_MEMCMP(&after.data.cs, &before.data.cs,
                sizeof(after.data.cs)) != 0 ||
            STD_MEMCMP(&after.data.ss, &before.data.ss,
                sizeof(after.data.ss)) != 0 ||
            STD_MEMCMP(&after.data.ds, &before.data.ds,
                sizeof(after.data.ds)) != 0 ||
            STD_MEMCMP(&after.data.fs, &before.data.fs,
                sizeof(after.data.fs)) != 0 ||
            STD_MEMCMP(&after.data.gs, &before.data.gs,
                sizeof(after.data.gs)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT fpu_interface_s65_mf(C_VOID)
{
    static const type_unsigned_8 wait[] = { 0x9bu };
    static const type_unsigned_8 handler[] = { 0xf4u };
    const type_unsigned_16 handler_offset = 0x0100u;
    const type_unsigned_16 handler_segment = 0u;
    type_unsigned_16 frame[3] = { 0u, 0u, 0u };
    fpu_interface_s65_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_status status;
    C_INT failed = !fpu_interface_s65_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        CORE_MACHINE_FPU_PROFILE_80387, &state);

    if (!failed) {
        state.machine->executor_cpu.data.esp = 0x00008000u;
        failed |= core_machine_memory_write(state.machine, 0x0040u,
            &handler_offset, sizeof(handler_offset)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x0042u, &handler_segment,
                sizeof(handler_segment)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, handler_offset, handler,
                sizeof(handler)) != TYPE_STATUS_OK;
        state.machine->fpu.pending_unmasked_exception = TYPE_TRUE;
        before = state.machine->executor_cpu;
        failed |= !fpu_interface_s65_run(&state, wait, sizeof(wait), &after,
            &diagnostic, &status) || status != TYPE_STATUS_OK ||
            diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid || !TYPE_GET_BIT(
                diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_MF) || after.data.eip != handler_offset ||
            after.data.esp != ((before.data.esp & 0xffff0000u) |
                (type_unsigned_16)(before.data.esp - 6u)) ||
            !test_core_machine_fixture_read_linear(state.machine,
                after.data.ss.base + (type_unsigned_16)after.data.esp,
                TYPE_REFERENCE_OF(frame), sizeof(frame)) ||
            frame[0] != 0u || frame[1] != before.data.cs.selector ||
            frame[2] != (type_unsigned_16)before.data.eflags ||
            after.data.eax != before.data.eax || after.data.ebx != before.data.ebx ||
            after.data.ecx != before.data.ecx || after.data.edx != before.data.edx ||
            after.data.ebp != before.data.ebp || after.data.esi != before.data.esi ||
            after.data.edi != before.data.edi ||
            STD_MEMCMP(&after.data.es, &before.data.es, sizeof(after.data.es)) != 0 ||
            STD_MEMCMP(&after.data.ds, &before.data.ds, sizeof(after.data.ds)) != 0 ||
            STD_MEMCMP(&after.data.fs, &before.data.fs, sizeof(after.data.fs)) != 0 ||
            STD_MEMCMP(&after.data.gs, &before.data.gs, sizeof(after.data.gs)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT fpu_interface_s65_reject(const type_unsigned_8 *code, STD_SIZE_T size,
    core_machine_cpu_profile profile)
{
    fpu_interface_s65_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_status status;
    C_INT failed = !fpu_interface_s65_prepare(profile,
        CORE_MACHINE_FPU_PROFILE_NONE, &state);

    if (!failed) {
        failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
            state.machine);
        before = state.machine->executor_cpu;
        failed |= !fpu_interface_s65_run(&state, code, size, &after,
            &diagnostic, &status) || status != TYPE_STATUS_FAULT ||
            !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            !fpu_interface_s65_same(&before, &after);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT fpu_interface_s65_handoff(core_machine_cpu_profile cpu,
    core_machine_fpu_profile profile, type_unsigned_32 expected_min,
    type_unsigned_32 expected_max)
{
    static const type_unsigned_8 fadd_wait[] = { 0xd8u, 0xc0u, 0x9bu };
    fpu_interface_s65_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_status status;
    C_INT failed = !fpu_interface_s65_prepare(cpu, profile, &state);

    if (!failed) {
        before = state.machine->executor_cpu;
        failed |= core_machine_memory_write(state.machine, 0u, fadd_wait,
            sizeof(fadd_wait)) != TYPE_STATUS_OK;
        status = core_machine_run(state.machine, (core_machine_run_budget){ 1u, 0u },
            &(core_machine_run_result){ 0 });
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            TYPE_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            after.data.eip != sizeof(fadd_wait) - 1u ||
            after.data.eax != before.data.eax || after.data.ebx != before.data.ebx ||
            !state.machine->fpu.busy ||
            state.machine->fpu.last_escape_opcode != fadd_wait[0] ||
            state.machine->fpu.last_escape_modrm != fadd_wait[1] ||
            state.machine->fpu.operation_ticks_min != expected_min ||
            state.machine->fpu.operation_ticks_max != expected_max ||
            state.machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_NONE ||
            state.machine->transaction.kind != CORE_MACHINE_TRANSACTION_CPU_FPU_COMMAND;
        failed |= core_machine_run(state.machine, (core_machine_run_budget){ 1u, 0u },
            &(core_machine_run_result){ 0 }) != TYPE_STATUS_OK ||
            state.machine->fpu.busy;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT fpu_interface_s65_deadline(core_machine_cpu_profile cpu,
    core_machine_fpu_profile profile)
{
    static const type_unsigned_8 fadd[] = { 0xd8u, 0xc0u };
    fpu_interface_s65_machine state;
    core_machine_time_observation observation;
    type_bool advanced = TYPE_FALSE;
    C_INT failed = !fpu_interface_s65_prepare(cpu, profile, &state);

    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0u, fadd,
            sizeof(fadd)) != TYPE_STATUS_OK ||
            core_machine_run(state.machine, (core_machine_run_budget){1u, 0u},
                &(core_machine_run_result){0}) != TYPE_STATUS_OK ||
            !state.machine->fpu.busy ||
            core_machine_capture_time_observation(state.machine, &observation) !=
                TYPE_STATUS_OK || !observation.next_deadline_valid ||
            observation.next_deadline_tick <= observation.elapsed_ticks ||
            core_machine_advance_to_next_deadline(state.machine, &advanced) !=
                TYPE_STATUS_OK || !advanced || state.machine->fpu.busy;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT fpu_interface_s65_incompatible(C_VOID)
{
    static const type_unsigned_8 fninit[] = { 0xdbu, 0xe3u };
    fpu_interface_s65_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_status status;
    C_INT failed = !fpu_interface_s65_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        CORE_MACHINE_FPU_PROFILE_8087, &state);

    if (!failed) {
        failed |= !test_core_machine_fixture_preflight_real_ud_terminal(state.machine);
        before = state.machine->executor_cpu;
        failed |= !fpu_interface_s65_run(&state, fninit, sizeof(fninit), &after,
            &diagnostic, &status) || status != TYPE_STATUS_FAULT ||
            !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask,
                VCPUINS_EXCEPT_FPU_UNSUPPORTED) || !fpu_interface_s65_same(&before, &after);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT fpu_interface_s65_nm_delivery(const type_unsigned_8 *code,
    STD_SIZE_T code_size, type_unsigned_32 cr0)
{
    static const type_unsigned_8 hlt = 0xf4u;
    const type_unsigned_16 offset = 0x0100u;
    const type_unsigned_16 segment = 0u;
    fpu_interface_s65_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    t_cpu after;
    type_unsigned_16 frame_ip = 0u;
    C_INT failed = !fpu_interface_s65_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        CORE_MACHINE_FPU_PROFILE_NONE, &state);

    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0u, code,
            code_size) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 0x1cu, &offset,
            sizeof(offset)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 0x1eu, &segment,
            sizeof(segment)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, offset, &hlt,
            sizeof(hlt)) != TYPE_STATUS_OK;
    }
    if (!failed) {
        state.machine->executor_cpu.data.cr0 = cr0;
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){ 1u, 0u }, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                TYPE_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid || !TYPE_GET_BIT(
                diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_NM) || after.data.eip != offset ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
                after.data.ss.base + (type_unsigned_16)after.data.esp,
                TYPE_REFERENCE_OF(frame_ip), sizeof(frame_ip)) != TYPE_STATUS_OK ||
            frame_ip != 0u;
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){ 1u, 0u }, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= after.data.eip != offset + 1u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT fpu_interface_s65_irq(const type_unsigned_8 *instruction,
    STD_SIZE_T instruction_size)
{
    static const type_unsigned_8 hlt = 0xf4u;
    const type_unsigned_16 offset = 0x0100u;
    const type_unsigned_16 segment = 0u;
    fpu_interface_s65_machine state;
    core_machine_pic_irq_source irq;
    core_machine_run_result result;
    t_cpu before;
    t_cpu after;
    type_unsigned_8 code[8] = { 0u };
    type_unsigned_16 frame_ip = 0u;
    C_INT failed = instruction_size + 1u > sizeof(code) ||
        !fpu_interface_s65_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            CORE_MACHINE_FPU_PROFILE_NONE, &state);

    if (!failed) {
        STD_MEMCPY(code, instruction, instruction_size);
        code[instruction_size] = 0x90u;
        failed |= core_machine_memory_write(state.machine, 0u, code,
            instruction_size + 1u) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 0x80u, &offset,
            sizeof(offset)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 0x82u, &segment,
            sizeof(segment)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, offset, &hlt,
            sizeof(hlt)) != TYPE_STATUS_OK;
    }
    if (!failed) {
        state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_IF;
        before = state.machine->executor_cpu;
        STD_MEMSET(&irq, 0, sizeof(irq));
        state.machine->shared_pic_master.data.icw2 = 0x20u;
        core_machine_pic_irq_source_bind(&irq, &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&irq);
        core_machine_pic_irq_source_deassert(&irq);
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){ 2u, 0u }, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= after.data.eip != offset + 1u || frame_ip != 0u ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
                after.data.ss.base + (type_unsigned_16)after.data.esp,
                TYPE_REFERENCE_OF(frame_ip), sizeof(frame_ip)) != TYPE_STATUS_OK ||
            frame_ip != instruction_size || after.data.eax != before.data.eax ||
            after.data.ebx != before.data.ebx || after.data.ecx != before.data.ecx ||
            after.data.edx != before.data.edx || after.data.ebp != before.data.ebp ||
            after.data.esi != before.data.esi || after.data.edi != before.data.edi ||
            !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,
                VPIC_ISR_IRQ(0u)) || TYPE_GET_BIT(
                state.machine->shared_pic_master.data.irr, VPIC_IRR_IRQ(0u));
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT fpu_interface_s65_vm86(C_VOID)
{
    static const type_unsigned_8 esc[] = { 0xd8u, 0xc0u };
    fpu_interface_s65_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    t_cpu before;
    t_cpu after;
    type_status status;
    C_INT failed = !fpu_interface_s65_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        CORE_MACHINE_FPU_PROFILE_NONE, &state);

    if (!failed) {
        state.machine->executor_cpu.data.cr0 |= VCPU_CR0_PE;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_VM |
            VCPU_EFLAGS_IOPL | VCPU_EFLAGS_IF | VCPU_EFLAGS_CF;
        state.machine->executor_cpu.data.cs.selector = 0u;
        state.machine->executor_cpu.data.cs.base = 0u;
        state.machine->executor_cpu.data.cs.limit = 0xffffu;
        state.machine->executor_cpu.data.cs.dpl = 3u;
        state.machine->executor_cpu.data.cs.flagValid = TYPE_TRUE;
        state.machine->executor_cpu.data.ds.selector = 0u;
        state.machine->executor_cpu.data.ds.base = 0u;
        state.machine->executor_cpu.data.ds.limit = 0xffffu;
        state.machine->executor_cpu.data.ds.dpl = 3u;
        state.machine->executor_cpu.data.ds.flagValid = TYPE_TRUE;
        state.machine->executor_cpu.data.ss.selector = 0u;
        state.machine->executor_cpu.data.ss.base = 0u;
        state.machine->executor_cpu.data.ss.limit = 0xffffu;
        state.machine->executor_cpu.data.ss.dpl = 3u;
        state.machine->executor_cpu.data.ss.flagValid = TYPE_TRUE;
        before = state.machine->executor_cpu;
        failed |= core_machine_memory_write(state.machine, 0u, esc,
            sizeof(esc)) != TYPE_STATUS_OK;
        status = core_machine_run(state.machine,
            (core_machine_run_budget){ 1u, 0u }, &result);
        failed |= status != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                TYPE_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= after.data.eip != sizeof(esc) || after.data.eax != before.data.eax ||
            after.data.ecx != before.data.ecx || after.data.edx != before.data.edx ||
            after.data.ebx != before.data.ebx || after.data.esp != before.data.esp ||
            after.data.ebp != before.data.ebp || after.data.esi != before.data.esi ||
            after.data.edi != before.data.edi || after.data.eflags != before.data.eflags ||
            STD_MEMCMP(&after.data.es, &before.data.es, sizeof(after.data.es)) != 0 ||
            STD_MEMCMP(&after.data.cs, &before.data.cs, sizeof(after.data.cs)) != 0 ||
            STD_MEMCMP(&after.data.ss, &before.data.ss, sizeof(after.data.ss)) != 0 ||
            STD_MEMCMP(&after.data.ds, &before.data.ds, sizeof(after.data.ds)) != 0 ||
            STD_MEMCMP(&after.data.fs, &before.data.fs, sizeof(after.data.fs)) != 0 ||
            STD_MEMCMP(&after.data.gs, &before.data.gs, sizeof(after.data.gs)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT fpu_interface_s65_protected_nm(C_VOID)
{
    static const type_unsigned_8 gdt_pointer[] = { 0x1fu,0u,0u,0x03u,0u,0u };
    static const type_unsigned_8 idt_pointer[] = { 0xffu,0u,0u,0x04u,0u,0u };
    static const type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0, 0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0,0, 0xffu,0xffu,0,0x40u,0,0x92u,0,0
    };
    static const type_unsigned_8 real_code[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u, 0x0fu,0x01u,0x1eu,0x10u,0x01u,
        0xb8u,0x01u,0,0x0fu,0x01u,0xf0u, 0xb8u,0x10u,0,0x8eu,0xd8u,
        0xb8u,0x18u,0,0x8eu,0xd0u, 0xeau,0,0,0x08u,0
    };
    static const type_unsigned_8 esc[] = { 0xd8u,0xc0u };
    static const type_unsigned_8 hlt = 0xf4u;
    type_unsigned_8 idt[0x100u] = { 0u };
    fpu_interface_s65_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_unsigned_16 frame_ip = 0u;
    C_INT failed = !fpu_interface_s65_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        CORE_MACHINE_FPU_PROFILE_NONE, &state);

    idt[7u * 8u + 1u] = 0x01u;
    idt[7u * 8u + 2u] = 0x08u;
    idt[7u * 8u + 5u] = 0x86u;
    if (!failed) {
        failed |= core_machine_memory_write(state.machine, FPU_S65_GDT_POINTER,
            gdt_pointer, sizeof(gdt_pointer)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, FPU_S65_GDT_BASE, gdt,
            sizeof(gdt)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0x0110u, idt_pointer, sizeof(idt_pointer)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, FPU_S65_IDT_BASE, idt,
            sizeof(idt)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0u, real_code, sizeof(real_code)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, FPU_S65_CODE_BASE + 0x100u,
            &hlt, sizeof(hlt)) != TYPE_STATUS_OK;
    }
    if (!failed) {
        type_status boot_status = core_machine_run(state.machine,
            (core_machine_run_budget){256u,0u}, &result);
        failed |= boot_status != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
    }
    if (!failed) {
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        state.machine->executor_cpu.data.cr0 |= VCPU_CR0_EM;
        before = state.machine->executor_cpu;
        failed |= core_machine_memory_write(state.machine, FPU_S65_CODE_BASE, esc,
            sizeof(esc)) != TYPE_STATUS_OK || core_machine_run(state.machine,
            (core_machine_run_budget){64u,0u}, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
            after.data.ss.base + (type_unsigned_16)after.data.esp, TYPE_REFERENCE_OF(frame_ip),
            sizeof(frame_ip)) != TYPE_STATUS_OK;
        failed |= !diagnostic.last_delivered_exception.valid || !TYPE_GET_BIT(
            diagnostic.last_delivered_exception.exception_mask,
            VCPUINS_EXCEPT_NM) || result.executed != 0u || result.ticks != 0u ||
            after.data.eip != 0x100u || frame_ip != 0u ||
            after.data.eax != before.data.eax || after.data.ecx != before.data.ecx ||
            after.data.edx != before.data.edx || after.data.ebx != before.data.ebx ||
            after.data.ebp != before.data.ebp || after.data.esi != before.data.esi ||
            after.data.edi != before.data.edi ||
            STD_MEMCMP(&after.data.ds, &before.data.ds, sizeof(after.data.ds)) != 0 ||
            STD_MEMCMP(&after.data.es, &before.data.es, sizeof(after.data.es)) != 0 ||
            STD_MEMCMP(&after.data.fs, &before.data.fs, sizeof(after.data.fs)) != 0 ||
            STD_MEMCMP(&after.data.gs, &before.data.gs, sizeof(after.data.gs)) != 0;
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){1u,0u}, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= after.data.eip != 0x101u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

C_INT main(C_VOID)
{
    static const type_unsigned_8 wait[] = { 0x9bu };
    static const type_unsigned_8 fninit[] = { 0xdbu, 0xe3u };
    static const type_unsigned_8 escapes[][2] = {
        { 0xd8u, 0xc0u }, { 0xd9u, 0xc0u }, { 0xdau, 0xc0u },
        { 0xdbu, 0xe3u }, { 0xdcu, 0xc0u }, { 0xddu, 0xc0u },
        { 0xdeu, 0xc0u }, { 0xdfu, 0xc0u }
    };
    static const type_unsigned_8 attr_wait_66[] = { 0x66u, 0x9bu };
    static const type_unsigned_8 attr_wait_67[] = { 0x67u, 0x9bu };
    static const type_unsigned_8 attr_wait[] = { 0x66u, 0x67u, 0x9bu };
    static const type_unsigned_8 attr_esc_66[] = { 0x66u, 0xdbu, 0xe3u };
    static const type_unsigned_8 attr_esc_67[] = { 0x67u, 0xdbu, 0xe3u };
    static const type_unsigned_8 attr_esc[] = { 0x66u, 0x67u, 0xdbu, 0xe3u };
    static const type_unsigned_8 *const legacy_attributes[] = {
        attr_wait_66, attr_wait_67, attr_wait, attr_esc_66, attr_esc_67,
        attr_esc
    };
    static const type_unsigned_8 legacy_attribute_sizes[] = { 2u, 2u, 3u, 3u, 3u, 4u };
    static const type_unsigned_8 lock_forms[][5] = {
        { 0xf0u, 0x9bu }, { 0xf0u, 0x66u, 0x9bu },
        { 0xf0u, 0x67u, 0x9bu }, { 0xf0u, 0x66u, 0x67u, 0x9bu },
        { 0xf0u, 0xdbu, 0xe3u }, { 0xf0u, 0x66u, 0xdbu, 0xe3u },
        { 0xf0u, 0x67u, 0xdbu, 0xe3u },
        { 0xf0u, 0x66u, 0x67u, 0xdbu, 0xe3u }
    };
    static const type_unsigned_8 lock_sizes[] = { 2u, 3u, 3u, 4u, 3u, 4u, 4u, 5u };
    core_machine_cpu_profile profile;
    type_unsigned_8 index;
    C_INT failed = 0;

    for (profile = CORE_MACHINE_CPU_PROFILE_8086;
        profile <= CORE_MACHINE_CPU_PROFILE_80386; ++profile) {
        failed |= !fpu_interface_s65_success(wait, sizeof(wait), profile,
            CORE_MACHINE_FPU_PROFILE_NONE, 0u);
        failed |= !fpu_interface_s65_success(fninit, sizeof(fninit), profile,
            CORE_MACHINE_FPU_PROFILE_NONE, 0u);
        for (index = 0u; index != sizeof(escapes) / sizeof(escapes[0]);
            ++index) {
            failed |= !fpu_interface_s65_success(escapes[index],
                sizeof(escapes[index]), profile, CORE_MACHINE_FPU_PROFILE_NONE,
                0u);
        }
    }
    failed |= !fpu_interface_s65_success(fninit, sizeof(fninit),
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_FPU_PROFILE_8087, 0u);
    failed |= !fpu_interface_s65_handoff(CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_FPU_PROFILE_8087, 0u, 0u);
    failed |= !fpu_interface_s65_handoff(CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_FPU_PROFILE_8087, 0u, 0u);
    failed |= !fpu_interface_s65_handoff(CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_FPU_PROFILE_80287, 0u, 0u);
    failed |= !fpu_interface_s65_handoff(CORE_MACHINE_CPU_PROFILE_80386,
        CORE_MACHINE_FPU_PROFILE_80287, 0u, 0u);
    failed |= !fpu_interface_s65_handoff(CORE_MACHINE_CPU_PROFILE_80386,
        CORE_MACHINE_FPU_PROFILE_80387, 12u, 26u);
    failed |= !fpu_interface_s65_deadline(CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_FPU_PROFILE_8087);
    failed |= !fpu_interface_s65_deadline(CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_FPU_PROFILE_8087);
    failed |= !fpu_interface_s65_deadline(CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_FPU_PROFILE_80287);
    failed |= !fpu_interface_s65_deadline(CORE_MACHINE_CPU_PROFILE_80386,
        CORE_MACHINE_FPU_PROFILE_80287);
    failed |= !fpu_interface_s65_deadline(CORE_MACHINE_CPU_PROFILE_80386,
        CORE_MACHINE_FPU_PROFILE_80387);
    failed |= !fpu_interface_s65_incompatible();
    failed |= !fpu_interface_s65_success(attr_wait, sizeof(attr_wait),
        CORE_MACHINE_CPU_PROFILE_80386, CORE_MACHINE_FPU_PROFILE_NONE, 0u);
    failed |= !fpu_interface_s65_success(attr_wait_66, sizeof(attr_wait_66),
        CORE_MACHINE_CPU_PROFILE_80386, CORE_MACHINE_FPU_PROFILE_NONE, 0u);
    failed |= !fpu_interface_s65_success(attr_wait_67, sizeof(attr_wait_67),
        CORE_MACHINE_CPU_PROFILE_80386, CORE_MACHINE_FPU_PROFILE_NONE, 0u);
    failed |= !fpu_interface_s65_success(attr_esc, sizeof(attr_esc),
        CORE_MACHINE_CPU_PROFILE_80386, CORE_MACHINE_FPU_PROFILE_NONE, 0u);
    failed |= !fpu_interface_s65_success(attr_esc_66, sizeof(attr_esc_66),
        CORE_MACHINE_CPU_PROFILE_80386, CORE_MACHINE_FPU_PROFILE_NONE, 0u);
    failed |= !fpu_interface_s65_success(attr_esc_67, sizeof(attr_esc_67),
        CORE_MACHINE_CPU_PROFILE_80386, CORE_MACHINE_FPU_PROFILE_NONE, 0u);
    failed |= !fpu_interface_s65_mf();
    for (profile = CORE_MACHINE_CPU_PROFILE_8086;
        profile <= CORE_MACHINE_CPU_PROFILE_80286; ++profile) {
        for (index = 0u; index != sizeof(legacy_attributes) /
            sizeof(legacy_attributes[0]); ++index) {
            failed |= !fpu_interface_s65_reject(legacy_attributes[index],
                legacy_attribute_sizes[index], profile);
        }
    }
    for (index = 0u; index != sizeof(lock_forms) / sizeof(lock_forms[0]);
        ++index) {
        failed |= !fpu_interface_s65_reject(lock_forms[index], lock_sizes[index],
            CORE_MACHINE_CPU_PROFILE_80386);
    }
    if (!fpu_interface_s65_nm_delivery(wait, sizeof(wait),
        VCPU_CR0_TS | VCPU_CR0_MP) || !fpu_interface_s65_nm_delivery(fninit,
        sizeof(fninit), VCPU_CR0_EM) || !fpu_interface_s65_nm_delivery(fninit,
        sizeof(fninit), VCPU_CR0_TS)) {
        STD_FPRINTF(STD_STDERR, "S65 stage=nm-delivery\n");
        failed = 1;
    }
    for (index = 0u; index != sizeof(escapes) / sizeof(escapes[0]); ++index) {
        if (!fpu_interface_s65_nm_delivery(escapes[index],
            sizeof(escapes[index]), VCPU_CR0_EM)) {
            STD_FPRINTF(STD_STDERR, "S65 stage=escape-nm index=%u\n", index);
            failed = 1;
        }
        if (!fpu_interface_s65_nm_delivery(escapes[index],
            sizeof(escapes[index]), VCPU_CR0_TS)) {
            STD_FPRINTF(STD_STDERR, "S65 stage=escape-ts index=%u\n", index);
            failed = 1;
        }
    }
    if (!fpu_interface_s65_irq(wait, sizeof(wait))) {
        STD_FPRINTF(STD_STDERR, "S65 stage=wait-irq\n");
        failed = 1;
    }
    if (!fpu_interface_s65_irq(fninit, sizeof(fninit))) {
        STD_FPRINTF(STD_STDERR, "S65 stage=esc-irq\n");
        failed = 1;
    }
    if (!fpu_interface_s65_vm86()) {
        STD_FPRINTF(STD_STDERR, "S65 stage=vm86\n");
        failed = 1;
    }
    if (!fpu_interface_s65_protected_nm()) {
        STD_FPRINTF(STD_STDERR, "S65 stage=protected-nm\n");
        failed = 1;
    }
    if (failed) {
        return 1;
    }
    STD_PRINTF("M5:T316:S65:FPU-INTERFACE:OK\n");
    STD_PRINTF("M5:T437:S3:X87-CROSS-PROFILE-INTERFACE:PASS\n");
    return 0;
}
