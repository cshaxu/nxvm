#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define IE_GDT_BASE 0x0300u
#define IE_IDT_BASE 0x0400u
#define IE_CODE_BASE 0x2000u
#define IE_STACK_BASE 0x8000u
#define IE_HANDLER_OFFSET 0x0100u
#define IE_VECTOR 0x30u

typedef struct interrupt_entry_machine {
    core_machine *machine;
} interrupt_entry_machine;

typedef enum interrupt_entry_negative {
    INTERRUPT_ENTRY_NEGATIVE_NONE,
    INTERRUPT_ENTRY_NEGATIVE_IDT_LIMIT,
    INTERRUPT_ENTRY_NEGATIVE_GATE_TYPE,
    INTERRUPT_ENTRY_NEGATIVE_GATE_DPL,
    INTERRUPT_ENTRY_NEGATIVE_GATE_NOT_PRESENT,
    INTERRUPT_ENTRY_NEGATIVE_CODE_TYPE,
    INTERRUPT_ENTRY_NEGATIVE_CODE_NOT_PRESENT,
    INTERRUPT_ENTRY_NEGATIVE_CODE_LIMIT,
    INTERRUPT_ENTRY_NEGATIVE_STACK_LIMIT
} interrupt_entry_negative;

typedef enum interrupt_entry_delivery_failure {
    INTERRUPT_ENTRY_DELIVERY_INVALID_GATE,
    INTERRUPT_ENTRY_DELIVERY_NONPRESENT_GATE,
    INTERRUPT_ENTRY_DELIVERY_TARGET_NOT_PRESENT,
    INTERRUPT_ENTRY_DELIVERY_STACK_LIMIT
} interrupt_entry_delivery_failure;

static C_VOID ie_reset(C_VOID *opaque)
{
    interrupt_entry_machine *state = (interrupt_entry_machine *)opaque;

    if (state != STD_NULL) (C_VOID)test_core_machine_fixture_reset_real_mode(
        state->machine);
}

static const core_machine_execution_provider ie_provider = {
    ie_reset, STD_NULL
};

static C_INT ie_write(interrupt_entry_machine *state, type_unsigned_32 address,
    const C_VOID *data, STD_SIZE_T bytes)
{
    return state != STD_NULL && state->machine != STD_NULL &&
        core_machine_memory_write(state->machine, address, data, bytes) ==
            TYPE_STATUS_OK;
}

static C_INT ie_read(interrupt_entry_machine *state, type_unsigned_32 address,
    C_VOID *data, STD_SIZE_T bytes)
{
    return state != STD_NULL && state->machine != STD_NULL &&
        core_machine_memory_read_physical(&state->machine->executor_memory,
            address, (type_virtual_address)data, bytes) == TYPE_STATUS_OK;
}

static C_INT ie_install_gate(interrupt_entry_machine *state, type_unsigned_8 vector,
    type_unsigned_16 selector, type_unsigned_8 gate_type)
{
    type_unsigned_8 gate[8] = {0};

    gate[0] = IE_HANDLER_OFFSET & 0xffu;
    gate[1] = IE_HANDLER_OFFSET >> 8u;
    gate[2] = selector & 0xffu;
    gate[3] = selector >> 8u;
    gate[5] = gate_type;
    return ie_write(state, IE_IDT_BASE + (type_unsigned_32)vector * 8u, gate,
        sizeof(gate));
}

static C_INT ie_prepare(interrupt_entry_machine *state,
    interrupt_entry_negative negative, type_unsigned_8 gate_type)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0x40u,0,
        0xffu,0xffu,0,0,0,0x92u,0xcfu,0
    };
    type_unsigned_8 idt[0x188u] = {0};
    static const type_unsigned_8 code[] = {0xcdu,IE_VECTOR};
    static const type_unsigned_8 handler[] = {0xf4u};
    t_cpu *cpu;

    if (state == STD_NULL) return 0;
    STD_MEMSET(state, 0, sizeof(*state));
    idt[IE_VECTOR * 8u] = IE_HANDLER_OFFSET & 0xffu;
    idt[IE_VECTOR * 8u + 1u] = IE_HANDLER_OFFSET >> 8u;
    idt[IE_VECTOR * 8u + 2u] = 0x08u;
    idt[IE_VECTOR * 8u + 5u] = (type_unsigned_8)(0xe0u | gate_type);
    if (negative == INTERRUPT_ENTRY_NEGATIVE_GATE_TYPE)
        idt[IE_VECTOR * 8u + 5u] = 0x80u;
    if (negative == INTERRUPT_ENTRY_NEGATIVE_GATE_DPL)
        idt[IE_VECTOR * 8u + 5u] = (type_unsigned_8)(0x80u | gate_type);
    if (negative == INTERRUPT_ENTRY_NEGATIVE_GATE_DPL) {
        idt[IE_VECTOR * 8u + 2u] = 0x0bu;
        gdt[13] = 0xfau;
    }
    if (negative == INTERRUPT_ENTRY_NEGATIVE_GATE_NOT_PRESENT)
        idt[IE_VECTOR * 8u + 5u] &= 0x7fu;
    if (negative == INTERRUPT_ENTRY_NEGATIVE_CODE_NOT_PRESENT)
        gdt[13] &= 0x7fu;
    if (negative == INTERRUPT_ENTRY_NEGATIVE_CODE_TYPE)
        gdt[13] = 0x92u;
    if (negative == INTERRUPT_ENTRY_NEGATIVE_CODE_LIMIT)
        gdt[8] = 0u;
    if (!test_core_machine_fixture_create_bind_freeze_reset(&config,
            &ie_provider, state, &state->machine) ||
        !ie_write(state, IE_GDT_BASE, gdt, sizeof(gdt)) ||
        !ie_write(state, IE_IDT_BASE, idt, sizeof(idt)) ||
        !ie_write(state, IE_CODE_BASE, code, sizeof(code)) ||
        !ie_write(state, IE_CODE_BASE + IE_HANDLER_OFFSET, handler,
            sizeof(handler))) {
        core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 0;
    }
    cpu = &state->machine->executor_cpu;
    cpu->data.cr0 = VCPU_CR0_PE;
    cpu->data.gdtr.flagValid = TYPE_TRUE;
    cpu->data.gdtr.sregtype = SREG_GDTR;
    cpu->data.gdtr.base = IE_GDT_BASE;
    cpu->data.gdtr.limit = sizeof(gdt) - 1u;
    cpu->data.idtr.flagValid = TYPE_TRUE;
    cpu->data.idtr.sregtype = SREG_IDTR;
    cpu->data.idtr.base = IE_IDT_BASE;
    cpu->data.idtr.limit = negative == INTERRUPT_ENTRY_NEGATIVE_IDT_LIMIT ?
        7u : sizeof(idt) - 1u;
    cpu->data.cs.flagValid = TYPE_TRUE;
    cpu->data.cs.selector = negative == INTERRUPT_ENTRY_NEGATIVE_GATE_DPL ?
        0x000bu : 0x0008u;
    cpu->data.cs.sregtype = SREG_CODE;
    cpu->data.cs.base = IE_CODE_BASE;
    cpu->data.cs.limit = negative == INTERRUPT_ENTRY_NEGATIVE_CODE_LIMIT ?
        0u : 0xffffu;
    cpu->data.cs.dpl = negative == INTERRUPT_ENTRY_NEGATIVE_GATE_DPL ? 3u : 0u;
    cpu->data.cs.seg.accessed = TYPE_FALSE;
    cpu->data.cs.seg.executable = TYPE_TRUE;
    cpu->data.cs.seg.exec.defsize = TYPE_TRUE;
    cpu->data.cs.seg.exec.conform = TYPE_FALSE;
    cpu->data.cs.seg.exec.readable = TYPE_TRUE;
    cpu->data.ss.flagValid = TYPE_TRUE;
    cpu->data.ss.selector = 0x0010u;
    cpu->data.ss.sregtype = SREG_STACK;
    cpu->data.ss.base = 0u;
    cpu->data.ss.limit = negative == INTERRUPT_ENTRY_NEGATIVE_STACK_LIMIT ?
        0x0010u : 0xffffffffu;
    cpu->data.ss.dpl = 0u;
    cpu->data.ss.seg.accessed = TYPE_FALSE;
    cpu->data.ss.seg.executable = TYPE_FALSE;
    cpu->data.ss.seg.data.big = TYPE_TRUE;
    cpu->data.ss.seg.data.expdown = TYPE_FALSE;
    cpu->data.ss.seg.data.writable = TYPE_TRUE;
    cpu->data.eip = 0u;
    cpu->data.esp = IE_STACK_BASE;
    cpu->data.eflags = 0x00000302u;
    cpu->data.flagHalt = TYPE_FALSE;
    return 1;
}

static C_INT ie_run(interrupt_entry_machine *state, C_INT expect_fault,
    t_cpu *out_cpu, core_machine_cpu_diagnostic *out_diagnostic)
{
    const core_machine_run_budget budget = {32u, 0u};
    core_machine_run_result result;
    type_status status;

    status = core_machine_run(state->machine, budget, &result);
    if (core_machine_get_cpu_diagnostic(state->machine, out_diagnostic) !=
        TYPE_STATUS_OK) return 0;
    *out_cpu = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return status == (expect_fault ? TYPE_STATUS_FAULT : TYPE_STATUS_OK) &&
        result.reason == (expect_fault ? CORE_MACHINE_STOP_FAULT :
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT);
}

static C_INT ie_run_external(interrupt_entry_machine *state, C_INT expect_fault,
    t_cpu *out_cpu, core_machine_cpu_diagnostic *out_diagnostic)
{
    const core_machine_run_budget budget = {32u, 0u};
    core_machine_run_result result;
    type_status status;

    status = core_machine_run(state->machine, budget, &result);
    if (core_machine_get_cpu_diagnostic(state->machine, out_diagnostic) !=
        TYPE_STATUS_OK) return 0;
    *out_cpu = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return status == (expect_fault ? TYPE_STATUS_FAULT : TYPE_STATUS_OK) &&
        result.reason == (expect_fault ? CORE_MACHINE_STOP_FAULT :
            CORE_MACHINE_STOP_BUDGET);
}

static C_INT ie_run_budget(interrupt_entry_machine *state, C_INT expect_fault,
    t_cpu *out_cpu, core_machine_cpu_diagnostic *out_diagnostic)
{
    return ie_run_external(state, expect_fault, out_cpu, out_diagnostic);
}

static C_INT ie_fault_is(const core_machine_cpu_diagnostic *diagnostic,
    type_unsigned_32 mask, type_unsigned_32 code)
{
    return diagnostic->first_fault.valid && TYPE_GET_BIT(
        diagnostic->first_fault.exception_mask, mask) &&
        diagnostic->first_fault.exception_code == code;
}

static C_INT ie_test_success(type_unsigned_8 gate_type, C_INT expect_if)
{
    interrupt_entry_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu after;
    type_unsigned_32 frame[3] = {0u, 0u, 0u};
    type_unsigned_8 code_access = 0u;
    C_INT failed = !ie_prepare(&state, INTERRUPT_ENTRY_NEGATIVE_NONE, gate_type);

    if (!failed) {
        failed |= !ie_run(&state, 0, &after, &diagnostic) ||
            diagnostic.first_fault.valid || after.data.cs.selector != 0x0008u ||
            after.data.eip != IE_HANDLER_OFFSET + 1u ||
            after.data.esp != IE_STACK_BASE - 12u ||
            TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_TF) ||
            (TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_IF) != expect_if) ||
            !ie_read(&state, IE_STACK_BASE - 12u, frame, sizeof(frame)) ||
            frame[0] != 2u || frame[1] != 0x0008u || frame[2] != 0x00000302u ||
            !ie_read(&state, IE_GDT_BASE + 13u, &code_access,
                sizeof(code_access)) || code_access != 0x9bu;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT ie_test_prefix_keeps_gate_width(C_VOID)
{
    interrupt_entry_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu after;
    type_unsigned_32 frame[3] = {0u, 0u, 0u};
    static const type_unsigned_8 code[] = {0x66u,0xcdu,IE_VECTOR};
    C_INT failed = !ie_prepare(&state, INTERRUPT_ENTRY_NEGATIVE_NONE,
        VCPU_DESC_SYS_TYPE_INTGATE_32);

    if (!failed) {
        failed |= !ie_write(&state, IE_CODE_BASE, code, sizeof(code)) ||
            !ie_run(&state, 0, &after, &diagnostic) ||
            after.data.esp != IE_STACK_BASE - 12u ||
            !ie_read(&state, IE_STACK_BASE - 12u, frame, sizeof(frame)) ||
            frame[0] != 3u || frame[1] != 0x0008u || frame[2] != 0x00000302u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT ie_test_failure(interrupt_entry_negative negative, type_unsigned_32 mask,
    type_unsigned_32 code)
{
    interrupt_entry_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_unsigned_8 access_before = 0u;
    type_unsigned_8 access_after = 0u;
    C_INT failed = !ie_prepare(&state, negative, VCPU_DESC_SYS_TYPE_INTGATE_32);

    if (!failed) {
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !ie_read(&state, IE_GDT_BASE + 13u, &access_before,
            sizeof(access_before)) || !ie_run(&state, 1, &after, &diagnostic) ||
            !ie_fault_is(&diagnostic, mask, code) ||
            !ie_read(&state, IE_GDT_BASE + 13u, &access_after,
                sizeof(access_after)) || after.data.cs.selector != before.data.cs.selector ||
            after.data.cs.base != before.data.cs.base ||
            after.data.cs.limit != before.data.cs.limit ||
            after.data.esp != before.data.esp || after.data.eflags != before.data.eflags ||
            after.data.eip != before.data.eip || access_after != access_before;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT ie_prepare_user_code(interrupt_entry_machine *state)
{
    type_unsigned_8 code_access = 0xfau;
    type_unsigned_8 selector[] = {0x0bu, 0u};

    if (!ie_write(state, IE_GDT_BASE + 13u, &code_access,
            sizeof(code_access)) || !ie_write(state,
            IE_IDT_BASE + IE_VECTOR * 8u + 2u, selector, sizeof(selector)))
        return 0;
    state->machine->executor_cpu.data.cs.selector = 0x000bu;
    state->machine->executor_cpu.data.cs.dpl = 3u;
    return 1;
}

static C_INT ie_test_software_frontends(C_VOID)
{
    static const type_unsigned_8 int3[] = {0xccu};
    static const type_unsigned_8 into[] = {0xceu};
    static const type_unsigned_8 into_clear[] = {0xceu,0xf4u};
    const type_unsigned_8 *programs[] = {int3, into};
    const type_unsigned_8 vectors[] = {0x03u, 0x04u};
    const type_unsigned_32 returns[] = {1u, 1u};
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(programs) / sizeof(programs[0]); ++index) {
        interrupt_entry_machine state;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu after;
        type_unsigned_32 frame[3] = {0u, 0u, 0u};
        type_unsigned_32 flags = 0x00000302u;
        C_INT failed = !ie_prepare(&state, INTERRUPT_ENTRY_NEGATIVE_NONE,
            VCPU_DESC_SYS_TYPE_TRAPGATE_32);

        if (!failed) {
            if (vectors[index] == 0x04u) flags |= VCPU_EFLAGS_OF;
            state.machine->executor_cpu.data.eflags = flags;
            failed |= !ie_install_gate(&state, vectors[index], 0x0008u,
                    (type_unsigned_8)(0xe0u | VCPU_DESC_SYS_TYPE_TRAPGATE_32)) ||
                !ie_write(&state, IE_CODE_BASE, programs[index],
                    index == 0u ? sizeof(int3) : sizeof(into)) ||
                !ie_run(&state, 0, &after, &diagnostic) ||
                diagnostic.first_fault.valid || after.data.esp != IE_STACK_BASE - 12u ||
                TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_TF) ||
                !TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_IF) ||
                !ie_read(&state, IE_STACK_BASE - 12u, frame, sizeof(frame)) ||
                frame[0] != returns[index] || frame[1] != 0x0008u ||
                frame[2] != flags;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        interrupt_entry_machine state;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu after;
        C_INT failed = !ie_prepare(&state, INTERRUPT_ENTRY_NEGATIVE_NONE,
            VCPU_DESC_SYS_TYPE_INTGATE_32);

        if (!failed) {
            state.machine->executor_cpu.data.eflags = 0x00000202u;
            failed |= !ie_install_gate(&state, 0x04u, 0x0008u,
                    (type_unsigned_8)(0xe0u | VCPU_DESC_SYS_TYPE_INTGATE_32)) ||
                !ie_write(&state, IE_CODE_BASE, into_clear, sizeof(into_clear)) ||
                !ie_run(&state, 0, &after, &diagnostic) ||
                diagnostic.first_fault.valid || after.data.eip != sizeof(into_clear) ||
                after.data.esp != IE_STACK_BASE || after.data.eflags != 0x00000202u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT ie_test_external_origin(C_INT nmi, C_INT reject)
{
    interrupt_entry_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_pic_irq_source source;
    t_cpu before;
    t_cpu after;
    static const type_unsigned_8 code[] = {0x90u};
    static const type_unsigned_8 handler[] = {0xebu,0xfeu};
    type_unsigned_8 vector = nmi ? 0x02u : IE_VECTOR;
    C_INT failed = !ie_prepare(&state, INTERRUPT_ENTRY_NEGATIVE_NONE,
        VCPU_DESC_SYS_TYPE_INTGATE_32);

    if (!failed) {
        state.machine->executor_cpu.data.eflags = 0x00000202u;
        failed |= !ie_prepare_user_code(&state) ||
            !ie_install_gate(&state, vector, 0x000bu,
                reject ? 0x80u : (type_unsigned_8)(0x80u | VCPU_DESC_SYS_TYPE_INTGATE_32)) ||
            !ie_write(&state, IE_CODE_BASE, code, sizeof(code)) ||
            !ie_write(&state, IE_CODE_BASE + IE_HANDLER_OFFSET, handler,
                sizeof(handler));
        if (!failed && !nmi) {
            STD_MEMSET(&source, 0, sizeof(source));
            state.machine->shared_pic_master.data.icw2 = IE_VECTOR;
            core_machine_pic_irq_source_bind(&source,
                &state.machine->shared_pic_master, &state.machine->shared_pic_slave, 0u);
            core_machine_pic_irq_source_assert(&source);
            core_machine_pic_irq_source_deassert(&source);
        } else if (!failed) {
            state.machine->executor_cpu.data.flagNMI = TYPE_TRUE;
        }
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !ie_run_external(&state, reject, &after, &diagnostic);
        if (!failed && !reject) {
            failed |= after.data.cs.selector != 0x000bu ||
                after.data.esp != IE_STACK_BASE - 12u ||
                (!TYPE_GET_BIT(state.machine->shared_pic_master.data.isr, 1u) && !nmi) ||
                TYPE_GET_BIT(state.machine->shared_pic_master.data.irr, 1u) ||
                state.machine->executor_cpu.data.flagNMI;
        } else if (!failed) {
            failed |= after.data.cs.selector != before.data.cs.selector ||
                after.data.esp != before.data.esp || after.data.eflags != before.data.eflags ||
                (nmi ? !state.machine->executor_cpu.data.flagNMI :
                    (!TYPE_GET_BIT(state.machine->shared_pic_master.data.isr, 1u) ||
                     TYPE_GET_BIT(state.machine->shared_pic_master.data.irr, 1u)));
        }
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT ie_delivered_is(const core_machine_cpu_diagnostic *diagnostic,
    type_unsigned_32 mask, type_unsigned_32 code)
{
    return !diagnostic->first_fault.valid &&
        diagnostic->last_delivered_exception.valid &&
        diagnostic->delivered_exception_count == 1u && TYPE_GET_BIT(
            diagnostic->last_delivered_exception.exception_mask, mask) &&
        diagnostic->last_delivered_exception.exception_code == code;
}

static C_INT ie_delivery_state_equal(const t_cpu *before, const t_cpu *after)
{
    return before->data.eip == after->data.eip &&
        before->data.esp == after->data.esp &&
        before->data.eflags == after->data.eflags &&
        STD_MEMCMP(&before->data.cs, &after->data.cs,
            sizeof(before->data.cs)) == 0 &&
        STD_MEMCMP(&before->data.ss, &after->data.ss,
            sizeof(before->data.ss)) == 0;
}

static C_INT ie_test_fault_delivery(type_unsigned_32 mask, type_unsigned_8 vector,
    type_unsigned_32 code, C_INT user_source)
{
    interrupt_entry_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu after;
    type_unsigned_32 frame[4] = {0u, 0u, 0u, 0u};
    type_unsigned_8 access_before = 0u;
    type_unsigned_8 access_after = 0u;
    static const type_unsigned_8 gp_code[] = {0x0fu,0x01u,0xf0u};
    static const type_unsigned_8 np_code[] = {0xb8u,0x18u,0,0,0,0x8eu,0xd8u};
    static const type_unsigned_8 ss_code[] = {0xb8u,0x18u,0,0,0,0x8eu,0xd0u};
    static const type_unsigned_8 loop[] = {0xebu,0xfeu};
    static const type_unsigned_8 halt[] = {0xf4u};
    type_unsigned_8 ss_descriptor[] = {0xffu,0xffu,0,0,0,0x12u,0xcfu,0};
    const type_unsigned_8 *program = mask == VCPUINS_EXCEPT_GP ? gp_code :
        (mask == VCPUINS_EXCEPT_NP ? np_code : ss_code);
    STD_SIZE_T bytes = mask == VCPUINS_EXCEPT_GP ? sizeof(gp_code) :
        (mask == VCPUINS_EXCEPT_NP ? sizeof(np_code) : sizeof(ss_code));
    type_unsigned_16 selector = user_source ? 0x000bu : 0x0008u;
    C_INT failed = !ie_prepare(&state, INTERRUPT_ENTRY_NEGATIVE_NONE,
        VCPU_DESC_SYS_TYPE_INTGATE_32);

    if (!failed) {
        if (user_source) failed |= !ie_prepare_user_code(&state);
        if (!user_source) state.machine->executor_cpu.data.eflags = 0x00000202u;
        if (mask == VCPUINS_EXCEPT_SS || mask == VCPUINS_EXCEPT_NP) {
            state.machine->executor_cpu.data.gdtr.limit = 0x001fu;
            failed |= !ie_write(&state, IE_GDT_BASE + 24u, ss_descriptor,
                sizeof(ss_descriptor));
        }
        failed |= !ie_install_gate(&state, vector, selector,
                (type_unsigned_8)(0x80u | VCPU_DESC_SYS_TYPE_INTGATE_32)) ||
            !ie_write(&state, IE_CODE_BASE, program, bytes) ||
            !ie_write(&state, IE_CODE_BASE + IE_HANDLER_OFFSET,
                user_source ? loop : halt, user_source ? sizeof(loop) :
                sizeof(halt)) || !ie_read(&state, IE_GDT_BASE + 13u,
                &access_before, sizeof(access_before)) || !(user_source ?
                ie_run_budget(&state, 0, &after, &diagnostic) : ie_run(&state,
                0, &after, &diagnostic)) || !ie_delivered_is(&diagnostic, mask,
                code) || after.data.cs.selector != selector ||
            after.data.esp != IE_STACK_BASE - 16u ||
            !ie_read(&state, IE_STACK_BASE - 16u, frame, sizeof(frame)) ||
            frame[0] != code || frame[1] != diagnostic.last_delivered_exception.point.eip ||
            frame[2] != selector || frame[3] != (user_source ? 0x00000302u :
                0x00000202u) ||
            !ie_read(&state, IE_GDT_BASE + 13u, &access_after,
                sizeof(access_after)) || access_after != (type_unsigned_8)(access_before | 1u);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT ie_test_t305_fault_delivery(C_VOID)
{
    interrupt_entry_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu after;
    type_unsigned_32 frame[4] = {0u,0u,0u,0u};
    static const type_unsigned_8 code[] = {0xcdu,IE_VECTOR};
    C_INT failed = !ie_prepare(&state, INTERRUPT_ENTRY_NEGATIVE_NONE,
        VCPU_DESC_SYS_TYPE_INTGATE_32);

    if (!failed) {
        state.machine->executor_cpu.data.eflags = 0x00000202u;
        failed |= !ie_install_gate(&state, IE_VECTOR, 0x0008u, 0x80u) ||
            !ie_install_gate(&state, 0x0du, 0x0008u,
                (type_unsigned_8)(0x80u | VCPU_DESC_SYS_TYPE_INTGATE_32)) ||
            !ie_write(&state, IE_CODE_BASE, code, sizeof(code)) ||
            !ie_write(&state, IE_CODE_BASE + IE_HANDLER_OFFSET,
                (const type_unsigned_8[]){0xf4u}, 1u) || !ie_run(&state, 0, &after,
                &diagnostic) || !ie_delivered_is(&diagnostic,
                VCPUINS_EXCEPT_GP, IE_VECTOR * 8u + 2u) ||
            after.data.cs.selector != 0x0008u ||
            after.data.esp != IE_STACK_BASE - 16u ||
            !ie_read(&state, IE_STACK_BASE - 16u, frame, sizeof(frame)) ||
            frame[0] != IE_VECTOR * 8u + 2u || frame[1] != 0u ||
            frame[2] != 0x0008u || frame[3] != 0x00000202u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT ie_test_fault_delivery_failure(
    interrupt_entry_delivery_failure failure)
{
    interrupt_entry_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_unsigned_8 access_before = 0u;
    type_unsigned_8 access_after = 0u;
    type_unsigned_32 stack_before[4] = {0u,0u,0u,0u};
    type_unsigned_32 stack_after[4] = {0u,0u,0u,0u};
    type_unsigned_8 not_present_access = 0x7au;
    static const type_unsigned_8 code[] = {0x0fu,0x01u,0xf0u};
    type_unsigned_8 gate_access = (type_unsigned_8)(0x80u | VCPU_DESC_SYS_TYPE_INTGATE_32);
    C_INT failed = !ie_prepare(&state, INTERRUPT_ENTRY_NEGATIVE_NONE,
        VCPU_DESC_SYS_TYPE_INTGATE_32);

    if (!failed) {
        if (failure == INTERRUPT_ENTRY_DELIVERY_INVALID_GATE) gate_access = 0x80u;
        if (failure == INTERRUPT_ENTRY_DELIVERY_NONPRESENT_GATE)
            gate_access = VCPU_DESC_SYS_TYPE_INTGATE_32;
        failed |= !ie_prepare_user_code(&state) ||
            !ie_install_gate(&state, 0x0du, 0x000bu, gate_access) ||
            !ie_write(&state, IE_CODE_BASE, code, sizeof(code));
        if (!failed && failure == INTERRUPT_ENTRY_DELIVERY_TARGET_NOT_PRESENT) {
            failed |= !ie_write(&state, IE_GDT_BASE + 13u, &not_present_access,
                sizeof(not_present_access));
        }
        if (!failed && failure == INTERRUPT_ENTRY_DELIVERY_STACK_LIMIT)
            state.machine->executor_cpu.data.ss.limit = IE_STACK_BASE - 2u;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !ie_read(&state, IE_GDT_BASE + 13u, &access_before,
            sizeof(access_before)) || !ie_read(&state, IE_STACK_BASE - 16u,
            stack_before, sizeof(stack_before)) || !ie_run_budget(&state, 1, &after,
            &diagnostic) || !ie_fault_is(&diagnostic, VCPUINS_EXCEPT_DF, 0u) ||
            diagnostic.last_delivered_exception.valid ||
            diagnostic.delivered_exception_count != 0u ||
            !ie_read(&state, IE_GDT_BASE + 13u, &access_after,
                sizeof(access_after)) || !ie_read(&state, IE_STACK_BASE - 16u,
                stack_after, sizeof(stack_after)) ||
            !ie_delivery_state_equal(&before, &after) ||
            access_after != access_before || STD_MEMCMP(stack_before, stack_after,
                sizeof(stack_before)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

int main(void)
{
    C_INT failed = !ie_test_success(VCPU_DESC_SYS_TYPE_INTGATE_32, 0) ||
        !ie_test_success(VCPU_DESC_SYS_TYPE_TRAPGATE_32, 1) ||
        !ie_test_prefix_keeps_gate_width() ||
        !ie_test_failure(INTERRUPT_ENTRY_NEGATIVE_IDT_LIMIT, VCPUINS_EXCEPT_DF,
            0u) ||
        !ie_test_failure(INTERRUPT_ENTRY_NEGATIVE_GATE_TYPE, VCPUINS_EXCEPT_DF,
            0u) ||
        !ie_test_failure(INTERRUPT_ENTRY_NEGATIVE_GATE_DPL, VCPUINS_EXCEPT_DF,
            0u) ||
        !ie_test_failure(INTERRUPT_ENTRY_NEGATIVE_GATE_NOT_PRESENT,
            VCPUINS_EXCEPT_DF, 0u) ||
        !ie_test_failure(INTERRUPT_ENTRY_NEGATIVE_CODE_TYPE, VCPUINS_EXCEPT_DF,
            0u) ||
        !ie_test_failure(INTERRUPT_ENTRY_NEGATIVE_CODE_NOT_PRESENT,
            VCPUINS_EXCEPT_DF, 0u) ||
        !ie_test_failure(INTERRUPT_ENTRY_NEGATIVE_CODE_LIMIT, VCPUINS_EXCEPT_DF,
            0u) ||
        !ie_test_failure(INTERRUPT_ENTRY_NEGATIVE_STACK_LIMIT,
            VCPUINS_EXCEPT_DF, 0u) ||
        !ie_test_software_frontends() ||
        !ie_test_external_origin(0, 0) || !ie_test_external_origin(1, 0) ||
        !ie_test_external_origin(0, 1) || !ie_test_external_origin(1, 1);

    failed |= !ie_test_fault_delivery(VCPUINS_EXCEPT_GP, 0x0du, 0u, 1);
    failed |= !ie_test_fault_delivery(VCPUINS_EXCEPT_NP, 0x0bu, 0x0018u, 0);
    failed |= !ie_test_fault_delivery(VCPUINS_EXCEPT_SS, 0x0cu, 0x0018u, 0);
    failed |= !ie_test_t305_fault_delivery();
    failed |= !ie_test_fault_delivery_failure(
        INTERRUPT_ENTRY_DELIVERY_INVALID_GATE);
    failed |= !ie_test_fault_delivery_failure(
        INTERRUPT_ENTRY_DELIVERY_NONPRESENT_GATE);
    failed |= !ie_test_fault_delivery_failure(
        INTERRUPT_ENTRY_DELIVERY_TARGET_NOT_PRESENT);
    failed |= !ie_test_fault_delivery_failure(
        INTERRUPT_ENTRY_DELIVERY_STACK_LIMIT);

    if (failed) return 1;
    STD_PRINTF("M5:T305:INTERRUPT-ENTRY:OK\n");
    STD_PRINTF("M5:T308:S2:SAME-CPL-ERROR-DELIVERY:OK\n");
    return 0;
}
