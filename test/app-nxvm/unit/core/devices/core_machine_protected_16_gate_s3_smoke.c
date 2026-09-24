#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"

#include "app-nxvm/devices/cpu.h"
#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/devices/pic.h"
#include "support/core_machine_cpu_fixture.h"

#define S3_GDT_BASE 0x0300u
#define S3_IDT_BASE 0x0400u
#define S3_CODE_BASE 0x2000u
#define S3_STACK_TOP 0x8000u
#define S3_VECTOR 0x30u
#define S3_HANDLER 0x0100u

typedef struct s3_gate_machine {
    core_machine *machine;
} s3_gate_machine;

static void s3_gate_reset(void *opaque)
{
    s3_gate_machine *state = (s3_gate_machine *)opaque;

    if (state != LIB_NULL)
        (void)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider s3_gate_provider = {
    s3_gate_reset, LIB_NULL
};

static lib_i32 s3_gate_write(s3_gate_machine *state, lib_u32 address,
    const void *data, lib_size bytes)
{
    return state != LIB_NULL && state->machine != LIB_NULL &&
        core_machine_memory_write(state->machine, address, data, bytes) ==
            LIB_STATUS_OK;
}

static lib_i32 s3_gate_read(s3_gate_machine *state, lib_u32 address,
    void *data, lib_size bytes)
{
    return state != LIB_NULL && state->machine != LIB_NULL &&
        core_machine_memory_read_physical(&state->machine->executor_memory,
            address, (lib_uptr)data, bytes) == LIB_STATUS_OK;
}

static lib_i32 s3_gate_install(s3_gate_machine *state, lib_u8 vector,
    lib_u16 selector, lib_u8 type, lib_u8 dpl,
    lib_u8 present)
{
    lib_u8 gate[8u] = { 0u };

    gate[0] = S3_HANDLER & 0xffu;
    gate[1] = S3_HANDLER >> 8u;
    gate[2] = selector & 0xffu;
    gate[3] = selector >> 8u;
    gate[5] = (lib_u8)((present ? 0x80u : 0u) | (dpl << 5u) | type);
    return s3_gate_write(state, S3_IDT_BASE + (lib_u32)vector * 8u,
        gate, sizeof(gate));
}

static lib_i32 s3_gate_prepare(s3_gate_machine *state,
    core_machine_cpu_profile profile, lib_u8 user_code,
    lib_u8 gate_type, lib_u8 gate_dpl, lib_u8 present)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0,0,0x92u,0,0,
        0xffu,0xffu,0,0x20u,0,0xfau,0,0
    };
    static const lib_u8 hlt[] = { 0xf4u };
    t_cpu *cpu;

    if (state == LIB_NULL) return 0;
    lib_memory_set(state, 0, sizeof(*state));
    if (core_machine_create(&config, &state->machine) != LIB_STATUS_OK ||
        !test_core_machine_fixture_bind_freeze_reset(state->machine,
            &s3_gate_provider, state) ||
        !s3_gate_write(state, S3_GDT_BASE, gdt, sizeof(gdt)) ||
        !s3_gate_install(state, S3_VECTOR, user_code ? 0x001bu : 0x0008u,
            gate_type, gate_dpl, present) ||
        !s3_gate_write(state, S3_CODE_BASE + S3_HANDLER, hlt, sizeof(hlt))) {
        core_machine_destroy(state->machine);
        state->machine = LIB_NULL;
        return 0;
    }
    cpu = &state->machine->executor_cpu;
    cpu->data.cr0 = VCPU_CR0_PE;
    cpu->data.gdtr.flagValid = LIB_TRUE;
    cpu->data.gdtr.sregtype = SREG_GDTR;
    cpu->data.gdtr.base = S3_GDT_BASE;
    cpu->data.gdtr.limit = sizeof(gdt) - 1u;
    cpu->data.idtr.flagValid = LIB_TRUE;
    cpu->data.idtr.sregtype = SREG_IDTR;
    cpu->data.idtr.base = S3_IDT_BASE;
    cpu->data.idtr.limit = 0x01ffu;
    cpu->data.cs.flagValid = LIB_TRUE;
    cpu->data.cs.selector = user_code ? 0x001bu : 0x0008u;
    cpu->data.cs.sregtype = SREG_CODE;
    cpu->data.cs.base = S3_CODE_BASE;
    cpu->data.cs.limit = 0xffffu;
    cpu->data.cs.dpl = user_code ? 3u : 0u;
    cpu->data.cs.seg.executable = LIB_TRUE;
    cpu->data.cs.seg.exec.readable = LIB_TRUE;
    cpu->data.ss.flagValid = LIB_TRUE;
    cpu->data.ss.selector = 0x0010u;
    cpu->data.ss.sregtype = SREG_STACK;
    cpu->data.ss.base = 0u;
    cpu->data.ss.limit = 0xffffu;
    cpu->data.ss.dpl = user_code ? 3u : 0u;
    cpu->data.ss.seg.data.writable = LIB_TRUE;
    cpu->data.eax = 0xaabbccddu;
    cpu->data.ecx = 0x11223344u;
    cpu->data.edx = 0x55667788u;
    cpu->data.ebx = 0x99aabbccu;
    cpu->data.esp = S3_STACK_TOP;
    cpu->data.ebp = 0x00001234u;
    cpu->data.esi = 0x00005678u;
    cpu->data.edi = 0x00009abcu;
    cpu->data.eip = 0u;
    cpu->data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_IF | VCPU_EFLAGS_TF;
    cpu->data.flagHalt = LIB_FALSE;
    return 1;
}

static lib_i32 s3_gate_gprs_same(const t_cpu *before, const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi && before->data.edi == after->data.edi;
}

static lib_i32 s3_gate_non_target_sregs_same(const t_cpu *before,
    const t_cpu *after)
{
    return lib_memory_compare(&before->data.es, &after->data.es,
            sizeof(before->data.es)) == 0 && lib_memory_compare(&before->data.ss,
            &after->data.ss, sizeof(before->data.ss)) == 0 && lib_memory_compare(
            &before->data.ds, &after->data.ds, sizeof(before->data.ds)) == 0 &&
        lib_memory_compare(&before->data.fs, &after->data.fs, sizeof(before->data.fs)) ==
            0 && lib_memory_compare(&before->data.gs, &after->data.gs,
            sizeof(before->data.gs)) == 0;
}

static lib_i32 s3_gate_success(core_machine_cpu_profile profile,
    lib_u8 gate_type, const lib_u8 *code, lib_size code_size,
    lib_u16 return_ip)
{
    s3_gate_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    lib_u16 frame[3u] = { 0u,0u,0u };
    lib_u8 code_access = 0u;
    t_cpu before;
    lib_i32 expect_if = gate_type == VCPU_DESC_SYS_TYPE_TRAPGATE_16;
    lib_i32 failed = !s3_gate_prepare(&state, profile, LIB_FALSE, gate_type, 0u,
        LIB_TRUE);

    if (!failed) {
        failed |= !s3_gate_write(&state, S3_CODE_BASE, code, code_size);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_run(state.machine, (core_machine_run_budget){32u,0u},
                &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != LIB_STATUS_OK ||
            diagnostic.first_fault.valid || state.machine->executor_cpu.data.eip !=
                S3_HANDLER + 1u || state.machine->executor_cpu.data.esp !=
                S3_STACK_TOP - 6u || !s3_gate_gprs_same(&before,
                &state.machine->executor_cpu) || !s3_gate_non_target_sregs_same(
                &before, &state.machine->executor_cpu) ||
            !CORE_MACHINE_BIT_IS_SET(state.machine->executor_cpu.data.eflags, VCPU_EFLAGS_CF) ||
            CORE_MACHINE_BIT_IS_SET(state.machine->executor_cpu.data.eflags, VCPU_EFLAGS_TF) ||
            (CORE_MACHINE_BIT_IS_SET(state.machine->executor_cpu.data.eflags, VCPU_EFLAGS_IF) !=
                expect_if) || !s3_gate_read(&state, S3_STACK_TOP - 6u, frame,
                sizeof(frame)) || frame[0] != return_ip || frame[1] != 0x0008u ||
            frame[2] != (VCPU_EFLAGS_CF | VCPU_EFLAGS_IF | VCPU_EFLAGS_TF) ||
            !s3_gate_read(&state, S3_GDT_BASE + 13u, &code_access,
                sizeof(code_access)) || code_access != 0x9bu;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 s3_gate_attributes(void)
{
    static const lib_u8 operand[] = { 0x66u,0xcdu,S3_VECTOR };
    static const lib_u8 address[] = { 0x67u,0xcdu,S3_VECTOR };
    static const lib_u8 combined[] = { 0x66u,0x67u,0xcdu,S3_VECTOR };

    return s3_gate_success(CORE_MACHINE_CPU_PROFILE_80386,
        VCPU_DESC_SYS_TYPE_INTGATE_16, operand, sizeof(operand), 3u) &&
        s3_gate_success(CORE_MACHINE_CPU_PROFILE_80386,
            VCPU_DESC_SYS_TYPE_TRAPGATE_16, address, sizeof(address), 3u) &&
        s3_gate_success(CORE_MACHINE_CPU_PROFILE_80386,
            VCPU_DESC_SYS_TYPE_INTGATE_16, combined, sizeof(combined), 4u);
}

static lib_i32 s3_gate_cpu_same(const t_cpu *before, const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ecx == after->data.ecx && before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx && before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp && before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi && before->data.eip == after->data.eip &&
        before->data.eflags == after->data.eflags && lib_memory_compare(&before->data.es,
            &after->data.es, sizeof(before->data.es)) == 0 && lib_memory_compare(
            &before->data.cs, &after->data.cs, sizeof(before->data.cs)) == 0 &&
        lib_memory_compare(&before->data.ss, &after->data.ss, sizeof(before->data.ss)) == 0 &&
        lib_memory_compare(&before->data.ds, &after->data.ds, sizeof(before->data.ds)) == 0 &&
        lib_memory_compare(&before->data.fs, &after->data.fs, sizeof(before->data.fs)) == 0 &&
        lib_memory_compare(&before->data.gs, &after->data.gs, sizeof(before->data.gs)) == 0;
}

static lib_i32 s3_gate_expect_ud(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_size code_size)
{
    s3_gate_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    lib_i32 failed = !s3_gate_prepare(&state, profile, LIB_FALSE,
        VCPU_DESC_SYS_TYPE_INTGATE_16, 0u, LIB_TRUE);

    if (!failed) {
        failed |= !s3_gate_write(&state, S3_CODE_BASE, code, code_size);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_run(state.machine, (core_machine_run_budget){8u,0u},
                &result) != LIB_STATUS_INTERNAL_ERROR || result.reason != CORE_MACHINE_STOP_FAULT ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != LIB_STATUS_OK ||
            !diagnostic.first_fault.valid || diagnostic.first_fault.exception_mask !=
                VCPUINS_EXCEPT_UD;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !s3_gate_cpu_same(&before, &after);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 s3_gate_rejections(void)
{
    static const lib_u8 prefix66[] = { 0x66u,0xcdu,S3_VECTOR };
    static const lib_u8 prefix67[] = { 0x67u,0xcdu,S3_VECTOR };
    static const lib_u8 combined[] = { 0x66u,0x67u,0xcdu,S3_VECTOR };
    static const lib_u8 lock[] = { 0xf0u,0xcdu,S3_VECTOR };

    return s3_gate_expect_ud(CORE_MACHINE_CPU_PROFILE_80286, prefix66,
        sizeof(prefix66)) && s3_gate_expect_ud(CORE_MACHINE_CPU_PROFILE_80286,
        prefix67, sizeof(prefix67)) && s3_gate_expect_ud(
        CORE_MACHINE_CPU_PROFILE_80286, combined, sizeof(combined)) &&
        s3_gate_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, lock, sizeof(lock));
}

static lib_i32 s3_gate_entry_rejected(core_machine_cpu_profile profile,
    lib_u8 user_code, lib_u8 gate_type, lib_u8 gate_dpl,
    lib_u8 present)
{
    static const lib_u8 code[] = { 0xcdu,S3_VECTOR };
    s3_gate_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    lib_i32 failed = !s3_gate_prepare(&state, profile, user_code, gate_type,
        gate_dpl, present);

    if (!failed) {
        failed |= !s3_gate_write(&state, S3_CODE_BASE, code, sizeof(code));
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_run(state.machine, (core_machine_run_budget){16u,0u},
                &result) != LIB_STATUS_INTERNAL_ERROR || result.reason != CORE_MACHINE_STOP_FAULT ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != LIB_STATUS_OK ||
            !diagnostic.first_fault.valid;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !s3_gate_cpu_same(&before, &after);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 s3_gate_error_frame(void)
{
    static const lib_u8 fault[] = { 0x0fu,0x01u,0xf0u };
    s3_gate_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    lib_u16 frame[4u] = { 0u,0u,0u,0u };
    lib_i32 failed = !s3_gate_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386,
        LIB_FALSE, VCPU_DESC_SYS_TYPE_INTGATE_16, 0u, LIB_TRUE);

    if (!failed) {
        failed |= !s3_gate_install(&state, 0x0du, 0x0008u,
            VCPU_DESC_SYS_TYPE_INTGATE_16, 0u, LIB_TRUE) || !s3_gate_write(
            &state, S3_CODE_BASE, fault, sizeof(fault)) || core_machine_run(
            state.machine, (core_machine_run_budget){32u,0u}, &result) !=
            LIB_STATUS_OK || result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != LIB_STATUS_OK ||
            !diagnostic.last_delivered_exception.valid ||
            diagnostic.last_delivered_exception.exception_mask != VCPUINS_EXCEPT_GP ||
            state.machine->executor_cpu.data.eip != S3_HANDLER + 1u ||
            state.machine->executor_cpu.data.esp != S3_STACK_TOP - 8u ||
            !s3_gate_read(&state, S3_STACK_TOP - 8u, frame, sizeof(frame)) ||
            frame[0] != 0x000au || frame[1] !=
                diagnostic.last_delivered_exception.point.eip || frame[2] != 0x0008u || frame[3] !=
                (VCPU_EFLAGS_CF | VCPU_EFLAGS_IF | VCPU_EFLAGS_TF);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 s3_gate_external_irq(void)
{
    static const lib_u8 nop[] = { 0x90u };
    s3_gate_machine state;
    core_machine_pic_irq_source source;
    core_machine_run_result result;
    lib_u16 frame[3u] = { 0u,0u,0u };
    lib_i32 failed = !s3_gate_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386,
        LIB_FALSE, VCPU_DESC_SYS_TYPE_INTGATE_16, 0u, LIB_TRUE);

    lib_memory_set(&source, 0, sizeof(source));
    if (!failed) {
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_IF;
        state.machine->shared_pic_master.data.icw2 = S3_VECTOR;
        core_machine_pic_irq_source_bind(&source, &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&source);
        core_machine_pic_irq_source_deassert(&source);
        failed |= !s3_gate_write(&state, S3_CODE_BASE, nop, sizeof(nop)) ||
            core_machine_run(state.machine, (core_machine_run_budget){16u,0u},
                &result) != LIB_STATUS_OK || result.reason !=
                CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            state.machine->executor_cpu.data.eip != S3_HANDLER + 1u ||
            state.machine->executor_cpu.data.esp != S3_STACK_TOP - 6u ||
            CORE_MACHINE_BIT_IS_SET(state.machine->executor_cpu.data.eflags, VCPU_EFLAGS_IF) ||
            CORE_MACHINE_BIT_IS_SET(state.machine->executor_cpu.data.eflags, VCPU_EFLAGS_TF) ||
            !CORE_MACHINE_BIT_IS_SET(state.machine->shared_pic_master.data.isr,
                VPIC_ISR_IRQ(0u)) || CORE_MACHINE_BIT_IS_SET(state.machine->shared_pic_master.data.irr,
                VPIC_IRR_IRQ(0u)) || !s3_gate_read(&state, S3_STACK_TOP - 6u,
                frame, sizeof(frame)) || frame[0] != 1u || frame[1] != 0x0008u ||
            frame[2] != (VCPU_EFLAGS_CF | VCPU_EFLAGS_IF);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

lib_i32 main(void)
{
    static const lib_u8 code[] = { 0xcdu,S3_VECTOR };
    lib_i32 failed = !s3_gate_success(CORE_MACHINE_CPU_PROFILE_80286,
        VCPU_DESC_SYS_TYPE_INTGATE_16, code, sizeof(code), 2u) ||
        !s3_gate_success(CORE_MACHINE_CPU_PROFILE_80286,
            VCPU_DESC_SYS_TYPE_TRAPGATE_16, code, sizeof(code), 2u) ||
        !s3_gate_success(CORE_MACHINE_CPU_PROFILE_80386,
            VCPU_DESC_SYS_TYPE_INTGATE_16, code, sizeof(code), 2u) ||
        !s3_gate_success(CORE_MACHINE_CPU_PROFILE_80386,
            VCPU_DESC_SYS_TYPE_TRAPGATE_16, code, sizeof(code), 2u) ||
        !s3_gate_attributes() || !s3_gate_rejections() ||
        !s3_gate_entry_rejected(CORE_MACHINE_CPU_PROFILE_80286, LIB_TRUE,
            VCPU_DESC_SYS_TYPE_INTGATE_16, 0u, LIB_TRUE) ||
        !s3_gate_entry_rejected(CORE_MACHINE_CPU_PROFILE_80386, LIB_FALSE,
            0u, 0u, LIB_TRUE) || !s3_gate_entry_rejected(
            CORE_MACHINE_CPU_PROFILE_80386, LIB_FALSE,
            VCPU_DESC_SYS_TYPE_INTGATE_16, 0u, LIB_FALSE) ||
        !s3_gate_error_frame() || !s3_gate_external_irq();

    if (failed) return 1;
    printf("M5:T323:S3:PROTECTED-16-GATE:OK\n");
    return 0;
}
