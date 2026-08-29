#include "type.h"
#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct pushf_machine { core_machine *machine; } pushf_machine;

static C_INT pushf_install_gp_gate(pushf_machine *state);

static C_VOID pushf_reset(C_VOID *opaque)
{
    pushf_machine *state = (pushf_machine *)opaque;
    if (state != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider pushf_provider = {
    pushf_reset, STD_NULL
};

static C_INT pushf_prepare(pushf_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    STD_MEMSET(state, 0, sizeof(*state));
return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &pushf_provider, state, &state->machine);
}

static C_INT pushf_run(pushf_machine *state, const type_unsigned_8 *code, type_unsigned_8 bytes, t_cpu *after)
{
    core_machine_run_result result;
    return test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u) &&
        core_machine_memory_write(state->machine, 0u, code, bytes) == TYPE_STATUS_OK &&
        core_machine_run(state->machine, (core_machine_run_budget){ 1u, 0u }, &result) == TYPE_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_BUDGET &&
        ((*after = test_core_machine_fixture_capture_cpu_after_run(state->machine)), 1);
}

static C_INT pushf_run_vm86(pushf_machine *state, const type_unsigned_8 *code, type_unsigned_8 bytes,
    type_unsigned_32 eflags, type_unsigned_32 esp, C_INT fault, t_cpu *after,
    core_machine_cpu_diagnostic *diagnostic, type_status *run_status,
    core_machine_stop_reason *reason)
{
    core_machine_run_result result;
    type_status status;
    if (!test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u) ||
        core_machine_memory_write(state->machine, 0u, code, bytes) != TYPE_STATUS_OK ||
        !pushf_install_gp_gate(state))
        return 0;
    state->machine->executor_cpu.data.cr0 |= VCPU_CR0_PE;
    state->machine->executor_cpu.data.eflags = eflags;
    state->machine->executor_cpu.data.esp = esp;
    state->machine->executor_cpu.data.eip = 0u;
    state->machine->executor_cpu.data.cs.selector = 0u;
    state->machine->executor_cpu.data.cs.base = 0u;
    state->machine->executor_cpu.data.cs.limit = 0xffffu;
    state->machine->executor_cpu.data.cs.dpl = 3u;
    state->machine->executor_cpu.data.cs.flagValid = TYPE_TRUE;
    state->machine->executor_cpu.data.cs.seg.exec.defsize = TYPE_FALSE;
    state->machine->executor_cpu.data.ss.selector = 0u;
    state->machine->executor_cpu.data.ss.base = 0u;
    state->machine->executor_cpu.data.ss.limit = 0xffffu;
    state->machine->executor_cpu.data.ss.dpl = 3u;
    state->machine->executor_cpu.data.ss.flagValid = TYPE_TRUE;
    state->machine->executor_cpu.data.ss.seg.data.big = TYPE_FALSE;
    status = core_machine_run(state->machine, (core_machine_run_budget){ 1u, 0u }, &result);
    *run_status = status;
    *reason = result.reason;
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) == TYPE_STATUS_OK &&
        status == (fault ? TYPE_STATUS_FAULT : TYPE_STATUS_OK) &&
        result.reason == (fault ? CORE_MACHINE_STOP_FAULT : CORE_MACHINE_STOP_BUDGET);
}

static C_INT pushf_test_vm86(C_VOID)
{
    static const type_unsigned_8 pushf[] = { 0x9cu };
    static const type_unsigned_8 popf[] = { 0x9du };
    type_unsigned_8 pass;
    for (pass = 0u; pass != 2u; ++pass) {
        const type_unsigned_32 flags = VCPU_EFLAGS_VM | (pass ? 0u : VCPU_EFLAGS_IOPL) | VCPU_EFLAGS_CF;
        pushf_machine state;
        t_cpu after = {0};
        core_machine_cpu_diagnostic diagnostic;
        type_status status;
        core_machine_stop_reason reason;
        C_INT failed = !pushf_prepare(&state);
        if (!failed) {
            state.machine->executor_cpu.data.esp = 0x8000u;
            state.machine->executor_cpu.data.eflags = flags;
            failed |= !pushf_run_vm86(&state, pushf, sizeof(pushf), flags, 0x8000u,
                0, &after, &diagnostic, &status, &reason);
            if (pass)
                failed |= diagnostic.first_fault.valid ||
                    !diagnostic.last_delivered_exception.valid || !TYPE_GET_BIT(
                        diagnostic.last_delivered_exception.exception_mask, VCPUINS_EXCEPT_GP) ||
                    after.data.cs.selector != 0x0008u || after.data.ss.selector != 0x0010u ||
                    after.data.eip != 0x00000100u || TYPE_GET_BIT(after.data.eflags,
                        VCPU_EFLAGS_VM);
            else
                failed |= diagnostic.first_fault.valid || after.data.esp != 0x7ffeu || after.data.eip != 1u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    for (pass = 0u; pass != 2u; ++pass) {
        const type_unsigned_32 flags = VCPU_EFLAGS_VM | (pass ? 0u : VCPU_EFLAGS_IOPL) |
            VCPU_EFLAGS_CF;
        const type_unsigned_16 image = VCPU_EFLAGS_ZF | VCPU_EFLAGS_IF;
        pushf_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_status status;
        core_machine_stop_reason reason;
        C_INT failed = !pushf_prepare(&state);
        if (!failed) {
            state.machine->executor_cpu.data.esp = 0x8000u;
            state.machine->executor_cpu.data.eflags = flags;
            failed |= core_machine_memory_write(state.machine, 0x8000u, &image, sizeof(image)) != TYPE_STATUS_OK ||
                !pushf_run_vm86(&state, popf, sizeof(popf), flags, 0x8000u,
                    0, &after, &diagnostic, &status, &reason);
            if (pass)
                failed |= diagnostic.first_fault.valid ||
                    !diagnostic.last_delivered_exception.valid || !TYPE_GET_BIT(
                        diagnostic.last_delivered_exception.exception_mask, VCPUINS_EXCEPT_GP) ||
                    after.data.cs.selector != 0x0008u || after.data.ss.selector != 0x0010u ||
                    after.data.eip != 0x00000100u || TYPE_GET_BIT(after.data.eflags,
                        VCPU_EFLAGS_VM);
            else
                failed |= diagnostic.first_fault.valid || after.data.esp != 0x8002u || after.data.eip != 1u ||
                    (after.data.eflags & (VCPU_EFLAGS_ZF | VCPU_EFLAGS_IF)) != image;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        static const type_unsigned_8 pushfd[] = { 0x66u, 0x9cu };
        const type_unsigned_32 flags = VCPU_EFLAGS_VM | VCPU_EFLAGS_IOPL |
            VCPU_EFLAGS_RF | VCPU_EFLAGS_CF;
        const type_unsigned_32 expected = (flags & ~(VCPU_EFLAGS_VM | VCPU_EFLAGS_RF |
            VCPU_EFLAGS_RESERVED)) | 0x02u;
        type_unsigned_32 image = 0u;
        pushf_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_status status;
        core_machine_stop_reason reason;
        C_INT failed = !pushf_prepare(&state);
        if (!failed) {
            failed |= !pushf_run_vm86(&state, pushfd, sizeof(pushfd), flags, 0x8000u,
                0, &after, &diagnostic, &status, &reason) || diagnostic.first_fault.valid ||
                after.data.eip != 2u || after.data.esp != 0x7ffcu ||
                core_machine_memory_read(state.machine, 0x7ffcu, &image, sizeof(image)) != TYPE_STATUS_OK ||
                image != expected;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT pushf_prepare_protected(pushf_machine *state)
{
    static const type_unsigned_8 pointer[] = { 0x1fu, 0, 0, 0x03u, 0, 0 };
    static const type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0, 0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0,0, 0xffu,0xffu,0,0x40u,0,0x92u,0,0
    };
    static const type_unsigned_8 bootstrap[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,0x8eu,0xc0u,0xb8u,0x18u,0x00u,0x8eu,
        0xd0u,0xbcu,0x00u,0x80u,0xeau,0x00u,0x00u,0x08u,0x00u
    };
    static const type_unsigned_8 halt[] = { 0xf4u };
    core_machine_run_result result;
    return pushf_prepare(state) &&
        core_machine_memory_write(state->machine, 0x0100u, pointer, sizeof(pointer)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x0300u, gdt, sizeof(gdt)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0u, bootstrap, sizeof(bootstrap)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x2000u, halt, sizeof(halt)) == TYPE_STATUS_OK &&
        core_machine_run(state->machine, (core_machine_run_budget){ 96u, 0u }, &result) == TYPE_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
}

static C_INT pushf_install_gp_gate(pushf_machine *state)
{
    static const type_unsigned_8 handler[] = { 0xf4u };
    type_unsigned_8 tss[10] = { 0 };
    static const type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0, 0xffu,0xffu,0,0x20u,0,0x9au,0x40u,0,
        0xffu,0xffu,0,0,0,0x92u,0xcfu,0
    };
    type_unsigned_8 gate[8] = { 0 };
    t_cpu *cpu = &state->machine->executor_cpu;
    gate[0] = 0x00u;
    gate[1] = 0x01u;
    gate[2] = 0x08u;
    gate[5] = 0x8eu;
    tss[4] = 0x00u;
    tss[5] = 0x90u;
    tss[8] = 0x10u;
    cpu->data.gdtr.flagValid = TYPE_TRUE;
    cpu->data.gdtr.sregtype = SREG_GDTR;
    cpu->data.gdtr.base = 0x0300u;
    cpu->data.gdtr.limit = 31u;
    cpu->data.idtr.flagValid = TYPE_TRUE;
    cpu->data.idtr.sregtype = SREG_IDTR;
    cpu->data.idtr.base = 0x0400u;
    cpu->data.idtr.limit = 0x006fu;
    cpu->data.cs.flagValid = TYPE_TRUE;
    cpu->data.cs.selector = 0x0008u;
    cpu->data.cs.sregtype = SREG_CODE;
    cpu->data.cs.base = 0x2000u;
    cpu->data.cs.limit = 0xffffu;
    cpu->data.cs.dpl = 0u;
    cpu->data.cs.seg.executable = TYPE_TRUE;
    cpu->data.cs.seg.exec.defsize = TYPE_TRUE;
    cpu->data.cs.seg.exec.readable = TYPE_TRUE;
    cpu->data.ss.flagValid = TYPE_TRUE;
    cpu->data.ss.selector = 0x0010u;
    cpu->data.ss.sregtype = SREG_STACK;
    cpu->data.ss.base = 0u;
    cpu->data.ss.limit = 0xffffffffu;
    cpu->data.ss.dpl = 0u;
    cpu->data.ss.seg.data.big = TYPE_TRUE;
    cpu->data.ss.seg.data.writable = TYPE_TRUE;
    cpu->data.tr.flagValid = TYPE_TRUE;
    cpu->data.tr.selector = 0x0028u;
    cpu->data.tr.sregtype = SREG_TR;
    cpu->data.tr.base = 0x0500u;
    cpu->data.tr.limit = 0x67u;
    cpu->data.tr.dpl = 0u;
    cpu->data.tr.sys.type = VCPU_DESC_SYS_TYPE_TSS_32_BUSY;
    cpu->data.esp = 0x8000u;
    return core_machine_memory_write(state->machine, 0x0300u, gdt, sizeof(gdt)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x0500u, tss, sizeof(tss)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x0400u + 0x0du * 8u,
        gate, sizeof(gate)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x2100u, handler,
            sizeof(handler)) == TYPE_STATUS_OK;
}

static C_INT pushf_test_protected_iopl(C_VOID)
{
    static const type_unsigned_8 popf[] = { 0x9du };
    type_unsigned_8 pass;
    for (pass = 0u; pass != 3u; ++pass) {
        const type_unsigned_8 cpl = pass == 0u ? 0u : 3u;
        const type_unsigned_32 iopl = pass == 2u ? VCPU_EFLAGS_IOPL : 0u;
        const type_unsigned_32 initial = VCPU_EFLAGS_CF | iopl;
        const type_unsigned_32 image = VCPU_EFLAGS_IF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_IOPL;
        const type_unsigned_32 expected_if = pass == 1u ? 0u : VCPU_EFLAGS_IF;
        const type_unsigned_32 expected_iopl = pass == 0u ? VCPU_EFLAGS_IOPL : iopl;
        pushf_machine state;
        t_cpu after;
        core_machine_run_result result;
        C_INT failed = !pushf_prepare_protected(&state);
        if (!failed) {
            state.machine->executor_cpu.data.cs.dpl = cpl;
            state.machine->executor_cpu.data.eflags = initial;
            failed |= core_machine_memory_write(state.machine, 0xc000u, &image, 2u) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x2000u, popf, sizeof(popf)) != TYPE_STATUS_OK;
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            failed |= core_machine_run(state.machine, (core_machine_run_budget){ 1u, 0u }, &result) != TYPE_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_BUDGET;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= (after.data.eflags & VCPU_EFLAGS_IF) != expected_if ||
                (after.data.eflags & VCPU_EFLAGS_IOPL) != expected_iopl ||
                (after.data.eflags & VCPU_EFLAGS_ZF) != VCPU_EFLAGS_ZF || after.data.eip != 1u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT pushf_test_stack_faults(C_VOID)
{
    static const type_unsigned_8 pushfw[] = { 0x9cu };
    static const type_unsigned_8 popfd[] = { 0x66u, 0x9du };
    type_unsigned_8 pass;
    for (pass = 0u; pass != 2u; ++pass) {
        const type_unsigned_8 *code = pass ? popfd : pushfw;
        const type_unsigned_8 bytes = pass ? sizeof(popfd) : sizeof(pushfw);
        const type_unsigned_32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_IF;
        type_unsigned_32 image = VCPU_EFLAGS_ZF | VCPU_EFLAGS_IF;
        type_unsigned_32 after_image = 0u;
        pushf_machine state;
        t_cpu after;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        type_status status;
        C_INT failed = !pushf_prepare_protected(&state);
        if (!failed) {
            state.machine->executor_cpu.data.esp = 0x8000u;
            state.machine->executor_cpu.data.eflags = flags;
            if (pass)
                state.machine->executor_cpu.data.ss.limit = 0x7fffu;
            else
                state.machine->executor_cpu.data.ss.seg.data.writable = TYPE_FALSE;
            failed |= core_machine_memory_write(state.machine, 0xc000u, &image, sizeof(image)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x2000u, code, bytes) != TYPE_STATUS_OK;
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            status = core_machine_run(state.machine, (core_machine_run_budget){ 1u, 0u }, &result);
            failed |= status != TYPE_STATUS_FAULT ||
                result.reason != CORE_MACHINE_STOP_FAULT ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_memory_read_physical(&state.machine->executor_memory, 0xc000u,
                TYPE_REFERENCE_OF(after_image), sizeof(after_image)) != TYPE_STATUS_OK;
            failed |= !diagnostic.first_fault.valid || after.data.eip != 0u ||
                after.data.esp != 0x8000u || after.data.eflags != flags || after_image != image;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}


C_INT main(C_VOID)
{
    static const type_unsigned_8 pushfw[] = { 0x9cu };
    static const type_unsigned_8 pushfd[] = { 0x66u, 0x9cu };
    static const type_unsigned_8 popfw[] = { 0x9du };
    static const type_unsigned_8 popfd[] = { 0x66u, 0x9du };
    const type_unsigned_32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | VCPU_EFLAGS_IF |
        VCPU_EFLAGS_DF | VCPU_EFLAGS_OF;
    const type_unsigned_8 *code[] = { pushfw, pushfd, popfw, popfd };
    const type_unsigned_8 bytes[] = { 1u, 2u, 1u, 2u };
    type_unsigned_8 form;
    for (form = 0u; form != 4u; ++form) {
        pushf_machine state;
        t_cpu after = {0};
        type_unsigned_32 image = form < 2u ? 0u : VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_IF;
        const type_unsigned_32 pushfw_image = (flags & ~VCPU_EFLAGS_RESERVED) | 0x02u;
        const type_unsigned_32 pushfd_image = (flags & ~(VCPU_EFLAGS_RESERVED |
            VCPU_EFLAGS_VM | VCPU_EFLAGS_RF)) | 0x02u;
        C_INT failed = !pushf_prepare(&state);
        if (!failed) {
            state.machine->executor_cpu.data.esp = 0x8000u;
            state.machine->executor_cpu.data.eflags = flags;
            if (form >= 2u)
                failed |= core_machine_memory_write(state.machine, 0x8000u, &image,
                    form == 2u ? 2u : 4u) != TYPE_STATUS_OK;
            failed |= !pushf_run(&state, code[form], bytes[form], &after);
            if (form < 2u)
                failed |= core_machine_memory_read(state.machine, after.data.esp, &image,
                    form == 0u ? 2u : 4u) != TYPE_STATUS_OK;
            failed |= after.data.eip != bytes[form] ||
                (form == 0u && ((image & 0xffffu) != (pushfw_image & 0xffffu))) ||
                (form == 0u && after.data.esp != 0x7ffeu) ||
                (form == 1u && image != pushfd_image) ||
                (form == 1u && after.data.esp != 0x7ffcu) ||
                (form == 2u && after.data.esp != 0x8002u) ||
                (form == 2u && (after.data.eflags & 0xffff0000u) != (flags & 0xffff0000u)) ||
                (form == 3u && after.data.esp != 0x8004u) ||
                (form == 3u && (after.data.eflags & (VCPU_EFLAGS_VM | VCPU_EFLAGS_RF |
                (VCPU_EFLAGS_RESERVED & ~0x02u))) !=
                    (flags & (VCPU_EFLAGS_VM | VCPU_EFLAGS_RF |
                    (VCPU_EFLAGS_RESERVED & ~0x02u)))) ||
                (form >= 2u && (after.data.eflags & (VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_IF)) != image);
        }
        core_machine_destroy(state.machine);
        if (failed) return 1;
    }
    if (!pushf_test_protected_iopl() || !pushf_test_stack_faults() || !pushf_test_vm86()) return 1;
    STD_PRINTF("M5:T316:S21:PUSHF-POPF:OK\n");
    return 0;
}
