#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define PE_GDT_BASE 0x0300u
#define PE_IDT_BASE 0x0400u
#define PE_TSS_BASE 0x0600u
#define PE_KERNEL_CODE_BASE 0x2000u
#define PE_USER_CODE_BASE 0x3000u
#define PE_HANDLER_OFFSET 0x0100u
#define PE_VECTOR 0x30u

typedef struct privilege_entry_machine {
    core_machine *machine;
} privilege_entry_machine;

static C_VOID pe_reset(C_VOID *opaque)
{
    privilege_entry_machine *state = (privilege_entry_machine *)opaque;

    if (state != STD_NULL) (C_VOID)test_core_machine_fixture_reset_real_mode(
        state->machine);
}

static const core_machine_execution_provider pe_provider = {
    pe_reset, STD_NULL
};

static C_INT pe_write(privilege_entry_machine *state, type_unsigned_32 address,
    const C_VOID *data, STD_SIZE_T bytes)
{
    return state != STD_NULL && state->machine != STD_NULL &&
        core_machine_memory_write(state->machine, address, data, bytes) ==
            TYPE_STATUS_OK;
}

static C_INT pe_read(privilege_entry_machine *state, type_unsigned_32 address,
    C_VOID *data, STD_SIZE_T bytes)
{
    return state != STD_NULL && state->machine != STD_NULL &&
        core_machine_memory_read_physical(&state->machine->executor_memory,
            address, (type_virtual_address)data, bytes) == TYPE_STATUS_OK;
}

static C_INT pe_prepare(privilege_entry_machine *state, type_unsigned_8 gate_access,
    type_unsigned_8 stack_access, C_INT stack_big)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0x40u,0,
        0xffu,0xffu,0,0,0,0x92u,(type_unsigned_8)(stack_big ? 0xcfu : 0x8fu),0,
        0xffu,0xffu,0,0x30u,0,0xfau,0x40u,0,
        0xffu,0xffu,0,0,0,0xf2u,0xcfu,0,
        0x67u,0,0,0x06u,0,0x8bu,0,0
    };
    type_unsigned_8 idt[PE_VECTOR * 8u + 8u] = {0};
    type_unsigned_8 tss[10] = {0};
    static const type_unsigned_8 program[] = {0xcdu,PE_VECTOR};
    static const type_unsigned_8 handler[] = {0xf4u};
    type_unsigned_32 esp0 = 0x00009000u;
    type_unsigned_16 ss0 = 0x0010u;
    t_cpu *cpu;

    if (state == STD_NULL) return 0;
    STD_MEMSET(state, 0, sizeof(*state));
    gdt[21] = stack_access;
    idt[PE_VECTOR * 8u] = PE_HANDLER_OFFSET & 0xffu;
    idt[PE_VECTOR * 8u + 1u] = PE_HANDLER_OFFSET >> 8u;
    idt[PE_VECTOR * 8u + 2u] = 0x08u;
    idt[PE_VECTOR * 8u + 5u] = gate_access;
    STD_MEMCPY(&tss[4], &esp0, sizeof(esp0));
    STD_MEMCPY(&tss[8], &ss0, sizeof(ss0));
    if (core_machine_create(&config, &state->machine) != TYPE_STATUS_OK ||
        !test_core_machine_fixture_bind_freeze_reset(state->machine,
            &pe_provider, state) ||
        !pe_write(state, PE_GDT_BASE, gdt, sizeof(gdt)) ||
        !pe_write(state, PE_IDT_BASE, idt, sizeof(idt)) ||
        !pe_write(state, PE_TSS_BASE, tss, sizeof(tss)) ||
        !pe_write(state, PE_USER_CODE_BASE, program, sizeof(program)) ||
        !pe_write(state, PE_KERNEL_CODE_BASE + PE_HANDLER_OFFSET, handler,
            sizeof(handler))) {
        core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 0;
    }
    cpu = &state->machine->executor_cpu;
    cpu->data.cr0 = VCPU_CR0_PE;
    cpu->data.gdtr.flagValid = TYPE_TRUE;
    cpu->data.gdtr.sregtype = SREG_GDTR;
    cpu->data.gdtr.base = PE_GDT_BASE;
    cpu->data.gdtr.limit = sizeof(gdt) - 1u;
    cpu->data.idtr.flagValid = TYPE_TRUE;
    cpu->data.idtr.sregtype = SREG_IDTR;
    cpu->data.idtr.base = PE_IDT_BASE;
    cpu->data.idtr.limit = sizeof(idt) - 1u;
    cpu->data.cs.flagValid = TYPE_TRUE;
    cpu->data.cs.selector = 0x001bu;
    cpu->data.cs.sregtype = SREG_CODE;
    cpu->data.cs.base = PE_USER_CODE_BASE;
    cpu->data.cs.limit = 0xffffu;
    cpu->data.cs.dpl = 3u;
    cpu->data.cs.seg.executable = TYPE_TRUE;
    cpu->data.cs.seg.exec.defsize = TYPE_TRUE;
    cpu->data.cs.seg.exec.conform = TYPE_FALSE;
    cpu->data.cs.seg.exec.readable = TYPE_TRUE;
    cpu->data.ss.flagValid = TYPE_TRUE;
    cpu->data.ss.selector = 0x0023u;
    cpu->data.ss.sregtype = SREG_STACK;
    cpu->data.ss.base = 0u;
    cpu->data.ss.limit = 0xffffffffu;
    cpu->data.ss.dpl = 3u;
    cpu->data.ss.seg.data.big = TYPE_TRUE;
    cpu->data.ss.seg.data.expdown = TYPE_FALSE;
    cpu->data.ss.seg.data.writable = TYPE_TRUE;
    cpu->data.tr.flagValid = TYPE_TRUE;
    cpu->data.tr.selector = 0x0028u;
    cpu->data.tr.sregtype = SREG_TR;
    cpu->data.tr.base = PE_TSS_BASE;
    cpu->data.tr.limit = 0x67u;
    cpu->data.tr.dpl = 0u;
    cpu->data.tr.sys.type = VCPU_DESC_SYS_TYPE_TSS_32_BUSY;
    cpu->data.eip = 0u;
    cpu->data.esp = 0x00008800u;
    cpu->data.eflags = 0x00000302u;
    cpu->data.flagHalt = TYPE_FALSE;
    return 1;
}

static C_INT pe_run(privilege_entry_machine *state, C_INT expect_fault,
    t_cpu *out_cpu, core_machine_cpu_diagnostic *out_diagnostic)
{
    const core_machine_run_budget budget = {32u, 0u};
    core_machine_run_result result;
    type_status status = core_machine_run(state->machine, budget, &result);

    if (core_machine_get_cpu_diagnostic(state->machine, out_diagnostic) !=
        TYPE_STATUS_OK) return 0;
    *out_cpu = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return status == (expect_fault ? TYPE_STATUS_FAULT : TYPE_STATUS_OK) &&
        result.reason == (expect_fault ? CORE_MACHINE_STOP_FAULT :
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT);
}

static C_INT pe_fault_is(const core_machine_cpu_diagnostic *diagnostic,
    type_unsigned_32 mask, type_unsigned_32 code)
{
    return diagnostic->first_fault.valid && TYPE_GET_BIT(
        diagnostic->first_fault.exception_mask, mask) &&
        diagnostic->first_fault.exception_code == code;
}

static C_INT pe_test_success(type_unsigned_8 gate_access, C_INT expect_if)
{
    privilege_entry_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu after;
    type_unsigned_32 frame[5] = {0u,0u,0u,0u,0u};
    type_unsigned_8 cs_access = 0u;
    type_unsigned_8 ss_access = 0u;
    C_INT failed = !pe_prepare(&state, gate_access, 0x92u, 1);

    if (!failed) {
        failed |= !pe_run(&state, 0, &after, &diagnostic) ||
            diagnostic.first_fault.valid || after.data.cs.selector != 0x0008u ||
            after.data.cs.dpl != 0u || after.data.eip != PE_HANDLER_OFFSET + 1u ||
            after.data.ss.selector != 0x0010u || after.data.ss.dpl != 0u ||
            after.data.esp != 0x00008fecu ||
            TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_TF) ||
            (TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_IF) != expect_if) ||
            !pe_read(&state, 0x00008fecu, frame, sizeof(frame)) ||
            frame[0] != 2u || frame[1] != 0x0000001bu ||
            frame[2] != 0x00000302u || frame[3] != 0x00008800u ||
            frame[4] != 0x00000023u ||
            !pe_read(&state, PE_GDT_BASE + 13u, &cs_access, sizeof(cs_access)) ||
            !pe_read(&state, PE_GDT_BASE + 21u, &ss_access, sizeof(ss_access)) ||
            cs_access != 0x9bu || ss_access != 0x93u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT pe_test_16bit_target_stack(C_VOID)
{
    privilege_entry_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu after;
    type_unsigned_32 frame[5] = {0u,0u,0u,0u,0u};
    C_INT failed = !pe_prepare(&state, 0xeeu, 0x92u, 0);

    if (!failed) {
        failed |= !pe_run(&state, 0, &after, &diagnostic) ||
            diagnostic.first_fault.valid || after.data.ss.seg.data.big ||
            after.data.esp != 0x00008fecu ||
            !pe_read(&state, 0x00008fecu, frame, sizeof(frame)) ||
            frame[0] != 2u || frame[1] != 0x0000001bu ||
            frame[2] != 0x00000302u || frame[3] != 0x00008800u ||
            frame[4] != 0x00000023u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT pe_test_external_bypasses_software_dpl(C_VOID)
{
    privilege_entry_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_pic_irq_source source;
    t_cpu after;
    static const type_unsigned_8 program[] = {0x90u};
    C_INT failed = !pe_prepare(&state, 0x8eu, 0x92u, 1);

    if (!failed) {
        STD_MEMSET(&source, 0, sizeof(source));
        state.machine->executor_cpu.data.eflags = 0x00000202u;
        state.machine->shared_pic_master.data.icw2 = PE_VECTOR;
        core_machine_pic_irq_source_bind(&source,
            &state.machine->shared_pic_master, &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&source);
        core_machine_pic_irq_source_deassert(&source);
        failed |= !pe_write(&state, PE_USER_CODE_BASE, program, sizeof(program)) ||
            !pe_run(&state, 0, &after, &diagnostic) || diagnostic.first_fault.valid ||
            after.data.cs.selector != 0x0008u || after.data.cs.dpl != 0u ||
            after.data.ss.selector != 0x0010u || after.data.esp != 0x00008fecu ||
            TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_IF) ||
            !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr, 1u) ||
            TYPE_GET_BIT(state.machine->shared_pic_master.data.irr, 1u);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT pe_test_software_dpl_atomic(C_VOID)
{
    privilege_entry_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_unsigned_8 cs_before = 0u, cs_after = 0u;
    type_unsigned_8 ss_before = 0u, ss_after = 0u;
    C_INT failed = !pe_prepare(&state, 0x8eu, 0x92u, 1);

    if (!failed) {
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !pe_read(&state, PE_GDT_BASE + 13u, &cs_before,
                sizeof(cs_before)) || !pe_read(&state, PE_GDT_BASE + 21u,
                &ss_before, sizeof(ss_before)) || !pe_run(&state, 1, &after,
                &diagnostic) || !pe_fault_is(&diagnostic, VCPUINS_EXCEPT_DF,
                0u) || !pe_read(&state, PE_GDT_BASE + 13u,
                &cs_after, sizeof(cs_after)) || !pe_read(&state, PE_GDT_BASE + 21u,
                &ss_after, sizeof(ss_after)) || after.data.eip != before.data.eip ||
            after.data.esp != before.data.esp || after.data.eflags != before.data.eflags ||
            after.data.cs.selector != before.data.cs.selector ||
            after.data.ss.selector != before.data.ss.selector || cs_after != cs_before ||
            ss_after != ss_before;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT pe_test_stack_failure_atomic(C_VOID)
{
    privilege_entry_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_unsigned_8 cs_before = 0u, cs_after = 0u;
    type_unsigned_8 ss_before = 0u, ss_after = 0u;
    C_INT failed = !pe_prepare(&state, 0xeeu, 0x12u, 1);

    if (!failed) {
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !pe_read(&state, PE_GDT_BASE + 13u, &cs_before,
                sizeof(cs_before)) || !pe_read(&state, PE_GDT_BASE + 21u,
                &ss_before, sizeof(ss_before)) || !pe_run(&state, 1, &after,
                &diagnostic) || !pe_fault_is(&diagnostic, VCPUINS_EXCEPT_DF,
                0u) || !pe_read(&state, PE_GDT_BASE + 13u, &cs_after,
                sizeof(cs_after)) || !pe_read(&state, PE_GDT_BASE + 21u,
                &ss_after, sizeof(ss_after)) || after.data.eip != before.data.eip ||
            after.data.esp != before.data.esp || after.data.eflags != before.data.eflags ||
            after.data.cs.selector != before.data.cs.selector ||
            after.data.ss.selector != before.data.ss.selector || cs_after != cs_before ||
            ss_after != ss_before;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT pe_test_code_failure_atomic(C_VOID)
{
    privilege_entry_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_unsigned_8 code_access = 0x1au;
    type_unsigned_8 cs_before = 0u, cs_after = 0u;
    type_unsigned_8 ss_before = 0u, ss_after = 0u;
    C_INT failed = !pe_prepare(&state, 0xeeu, 0x92u, 1);

    if (!failed) {
        failed |= !pe_write(&state, PE_GDT_BASE + 13u, &code_access,
                sizeof(code_access));
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !pe_read(&state, PE_GDT_BASE + 13u, &cs_before,
                sizeof(cs_before)) || !pe_read(&state, PE_GDT_BASE + 21u,
                &ss_before, sizeof(ss_before)) || !pe_run(&state, 1, &after,
                &diagnostic) || !pe_fault_is(&diagnostic, VCPUINS_EXCEPT_DF,
                0u) || !pe_read(&state, PE_GDT_BASE + 13u, &cs_after,
                sizeof(cs_after)) || !pe_read(&state, PE_GDT_BASE + 21u,
                &ss_after, sizeof(ss_after)) || after.data.eip != before.data.eip ||
            after.data.esp != before.data.esp || after.data.eflags != before.data.eflags ||
            after.data.cs.selector != before.data.cs.selector ||
            after.data.ss.selector != before.data.ss.selector || cs_after != cs_before ||
            ss_after != ss_before;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

int main(void)
{
    C_INT failed = !pe_test_success(0xeeu, 0) || !pe_test_success(0xefu, 1) ||
        !pe_test_16bit_target_stack() || !pe_test_external_bypasses_software_dpl() ||
        !pe_test_software_dpl_atomic() || !pe_test_stack_failure_atomic() ||
        !pe_test_code_failure_atomic();

    if (failed) return 1;
    STD_PRINTF("M5:T307:IDT-PRIVILEGE-ENTRY:OK\n");
    return 0;
}
