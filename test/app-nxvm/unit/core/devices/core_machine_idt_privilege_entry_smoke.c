#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"

#include "app-nxvm/devices/cpu.h"
#include "app-nxvm/devices/machine_interface.h"
#include "support/core_machine_cpu_fixture.h"

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

static void pe_reset(void *opaque)
{
    privilege_entry_machine *state = (privilege_entry_machine *)opaque;

    if (state != LIB_NULL) (void)test_core_machine_fixture_reset_real_mode(
        state->machine);
}

static const core_machine_execution_provider pe_provider = {
    pe_reset, LIB_NULL
};

static lib_i32 pe_write(privilege_entry_machine *state, lib_u32 address,
    const void *data, lib_size bytes)
{
    return state != LIB_NULL && state->machine != LIB_NULL &&
        core_machine_memory_write(state->machine, address, data, bytes) ==
            LIB_STATUS_OK;
}

static lib_i32 pe_read(privilege_entry_machine *state, lib_u32 address,
    void *data, lib_size bytes)
{
    return state != LIB_NULL && state->machine != LIB_NULL &&
        core_machine_memory_read_physical(&state->machine->executor_memory,
            address, (lib_uptr)data, bytes) == LIB_STATUS_OK;
}

static lib_i32 pe_prepare(privilege_entry_machine *state, lib_u8 gate_access,
    lib_u8 stack_access, lib_i32 stack_big)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0x40u,0,
        0xffu,0xffu,0,0,0,0x92u,(lib_u8)(stack_big ? 0xcfu : 0x8fu),0,
        0xffu,0xffu,0,0x30u,0,0xfau,0x40u,0,
        0xffu,0xffu,0,0,0,0xf2u,0xcfu,0,
        0x67u,0,0,0x06u,0,0x8bu,0,0
    };
    lib_u8 idt[PE_VECTOR * 8u + 8u] = {0};
    lib_u8 tss[10] = {0};
    static const lib_u8 program[] = {0xcdu,PE_VECTOR};
    static const lib_u8 handler[] = {0xf4u};
    lib_u32 esp0 = 0x00009000u;
    lib_u16 ss0 = 0x0010u;
    t_cpu *cpu;

    if (state == LIB_NULL) return 0;
    lib_memory_set(state, 0, sizeof(*state));
    gdt[21] = stack_access;
    idt[PE_VECTOR * 8u] = PE_HANDLER_OFFSET & 0xffu;
    idt[PE_VECTOR * 8u + 1u] = PE_HANDLER_OFFSET >> 8u;
    idt[PE_VECTOR * 8u + 2u] = 0x08u;
    idt[PE_VECTOR * 8u + 5u] = gate_access;
    lib_memory_copy(&tss[4], &esp0, sizeof(esp0));
    lib_memory_copy(&tss[8], &ss0, sizeof(ss0));
    if (core_machine_create(&config, &state->machine) != LIB_STATUS_OK ||
        !test_core_machine_fixture_bind_freeze_reset(state->machine,
            &pe_provider, state) ||
        !pe_write(state, PE_GDT_BASE, gdt, sizeof(gdt)) ||
        !pe_write(state, PE_IDT_BASE, idt, sizeof(idt)) ||
        !pe_write(state, PE_TSS_BASE, tss, sizeof(tss)) ||
        !pe_write(state, PE_USER_CODE_BASE, program, sizeof(program)) ||
        !pe_write(state, PE_KERNEL_CODE_BASE + PE_HANDLER_OFFSET, handler,
            sizeof(handler))) {
        core_machine_destroy(state->machine);
        state->machine = LIB_NULL;
        return 0;
    }
    cpu = &state->machine->executor_cpu;
    cpu->data.cr0 = VCPU_CR0_PE;
    cpu->data.gdtr.flagValid = LIB_TRUE;
    cpu->data.gdtr.sregtype = SREG_GDTR;
    cpu->data.gdtr.base = PE_GDT_BASE;
    cpu->data.gdtr.limit = sizeof(gdt) - 1u;
    cpu->data.idtr.flagValid = LIB_TRUE;
    cpu->data.idtr.sregtype = SREG_IDTR;
    cpu->data.idtr.base = PE_IDT_BASE;
    cpu->data.idtr.limit = sizeof(idt) - 1u;
    cpu->data.cs.flagValid = LIB_TRUE;
    cpu->data.cs.selector = 0x001bu;
    cpu->data.cs.sregtype = SREG_CODE;
    cpu->data.cs.base = PE_USER_CODE_BASE;
    cpu->data.cs.limit = 0xffffu;
    cpu->data.cs.dpl = 3u;
    cpu->data.cs.seg.executable = LIB_TRUE;
    cpu->data.cs.seg.exec.defsize = LIB_TRUE;
    cpu->data.cs.seg.exec.conform = LIB_FALSE;
    cpu->data.cs.seg.exec.readable = LIB_TRUE;
    cpu->data.ss.flagValid = LIB_TRUE;
    cpu->data.ss.selector = 0x0023u;
    cpu->data.ss.sregtype = SREG_STACK;
    cpu->data.ss.base = 0u;
    cpu->data.ss.limit = 0xffffffffu;
    cpu->data.ss.dpl = 3u;
    cpu->data.ss.seg.data.big = LIB_TRUE;
    cpu->data.ss.seg.data.expdown = LIB_FALSE;
    cpu->data.ss.seg.data.writable = LIB_TRUE;
    cpu->data.tr.flagValid = LIB_TRUE;
    cpu->data.tr.selector = 0x0028u;
    cpu->data.tr.sregtype = SREG_TR;
    cpu->data.tr.base = PE_TSS_BASE;
    cpu->data.tr.limit = 0x67u;
    cpu->data.tr.dpl = 0u;
    cpu->data.tr.sys.type = VCPU_DESC_SYS_TYPE_TSS_32_BUSY;
    cpu->data.eip = 0u;
    cpu->data.esp = 0x00008800u;
    cpu->data.eflags = 0x00000302u;
    cpu->data.flagHalt = LIB_FALSE;
    return 1;
}

static lib_i32 pe_run(privilege_entry_machine *state, lib_i32 expect_fault,
    t_cpu *out_cpu, core_machine_cpu_diagnostic *out_diagnostic)
{
    const core_machine_run_budget budget = {32u, 0u};
    core_machine_run_result result;
    lib_status status = core_machine_run(state->machine, budget, &result);

    if (core_machine_get_cpu_diagnostic(state->machine, out_diagnostic) !=
        LIB_STATUS_OK) return 0;
    *out_cpu = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return status == (expect_fault ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK) &&
        result.reason == (expect_fault ? CORE_MACHINE_STOP_FAULT :
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT);
}

static lib_i32 pe_fault_is(const core_machine_cpu_diagnostic *diagnostic,
    lib_u32 mask, lib_u32 code)
{
    return diagnostic->first_fault.valid && CORE_MACHINE_BIT_IS_SET(
        diagnostic->first_fault.exception_mask, mask) &&
        diagnostic->first_fault.exception_code == code;
}

static lib_i32 pe_test_success(lib_u8 gate_access, lib_i32 expect_if)
{
    privilege_entry_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu after;
    lib_u32 frame[5] = {0u,0u,0u,0u,0u};
    lib_u8 cs_access = 0u;
    lib_u8 ss_access = 0u;
    lib_i32 failed = !pe_prepare(&state, gate_access, 0x92u, 1);

    if (!failed) {
        failed |= !pe_run(&state, 0, &after, &diagnostic) ||
            diagnostic.first_fault.valid || after.data.cs.selector != 0x0008u ||
            after.data.cs.dpl != 0u || after.data.eip != PE_HANDLER_OFFSET + 1u ||
            after.data.ss.selector != 0x0010u || after.data.ss.dpl != 0u ||
            after.data.esp != 0x00008fecu ||
            CORE_MACHINE_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_TF) ||
            (CORE_MACHINE_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_IF) != expect_if) ||
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

static lib_i32 pe_test_16bit_target_stack(void)
{
    privilege_entry_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu after;
    lib_u32 frame[5] = {0u,0u,0u,0u,0u};
    lib_i32 failed = !pe_prepare(&state, 0xeeu, 0x92u, 0);

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

static lib_i32 pe_test_external_bypasses_software_dpl(void)
{
    privilege_entry_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_pic_irq_source source;
    t_cpu after;
    static const lib_u8 program[] = {0x90u};
    lib_i32 failed = !pe_prepare(&state, 0x8eu, 0x92u, 1);

    if (!failed) {
        lib_memory_set(&source, 0, sizeof(source));
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
            CORE_MACHINE_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_IF) ||
            !CORE_MACHINE_BIT_IS_SET(state.machine->shared_pic_master.data.isr, 1u) ||
            CORE_MACHINE_BIT_IS_SET(state.machine->shared_pic_master.data.irr, 1u);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 pe_test_software_dpl_atomic(void)
{
    privilege_entry_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    lib_u8 cs_before = 0u, cs_after = 0u;
    lib_u8 ss_before = 0u, ss_after = 0u;
    lib_i32 failed = !pe_prepare(&state, 0x8eu, 0x92u, 1);

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

static lib_i32 pe_test_stack_failure_atomic(void)
{
    privilege_entry_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    lib_u8 cs_before = 0u, cs_after = 0u;
    lib_u8 ss_before = 0u, ss_after = 0u;
    lib_i32 failed = !pe_prepare(&state, 0xeeu, 0x12u, 1);

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

static lib_i32 pe_test_code_failure_atomic(void)
{
    privilege_entry_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    lib_u8 code_access = 0x1au;
    lib_u8 cs_before = 0u, cs_after = 0u;
    lib_u8 ss_before = 0u, ss_after = 0u;
    lib_i32 failed = !pe_prepare(&state, 0xeeu, 0x92u, 1);

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
    lib_i32 failed = !pe_test_success(0xeeu, 0) || !pe_test_success(0xefu, 1) ||
        !pe_test_16bit_target_stack() || !pe_test_external_bypasses_software_dpl() ||
        !pe_test_software_dpl_atomic() || !pe_test_stack_failure_atomic() ||
        !pe_test_code_failure_atomic();

    if (failed) return 1;
    printf("M5:T307:IDT-PRIVILEGE-ENTRY:OK\n");
    return 0;
}
