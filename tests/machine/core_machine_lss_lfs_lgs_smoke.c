#include "type.h"
#include "core/machine/cpu.h"
#include "core/machine/pic.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct lfg_machine {
    core_machine *machine;
} lfg_machine;

static C_VOID lfg_reset(C_VOID *opaque)
{
    lfg_machine *state = (lfg_machine *)opaque;

    if (state != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider lfg_provider = {
    lfg_reset, STD_NULL
};

static C_INT lfg_prepare(core_machine_cpu_profile profile, lfg_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    STD_MEMSET(state, 0, sizeof(*state));
return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &lfg_provider, state, &state->machine);
}

static C_INT lfg_run_prepared(lfg_machine *state, const type_unsigned_8 *code,
    type_unsigned_8 bytes, t_cpu *after, core_machine_cpu_diagnostic *diagnostic,
    type_status *status)
{
    core_machine_run_result result;

    if (core_machine_memory_write(state->machine, 0u, code, bytes) !=
            TYPE_STATUS_OK)
        return 0;
    *status = core_machine_run(state->machine,
        (core_machine_run_budget){ 1u, 0u }, &result);
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) ==
        TYPE_STATUS_OK;
}
static C_INT lfg_test_real(C_VOID)
{
    static const type_unsigned_8 op[] = { 0xb2u, 0xb4u, 0xb5u };
    type_unsigned_8 i;
    type_unsigned_8 z;

    for (i = 0u; i < 3u; ++i) {
        for (z = 0u; z < 2u; ++z) {
            lfg_machine s;
            t_cpu a = {0};
            core_machine_cpu_diagnostic d = {0};
            type_status st = TYPE_STATUS_INVALID_STATE;
            type_unsigned_8 c[] = { 0x0fu, op[i], 0x06u, 0, 0x10u, 0 };
            type_unsigned_8 p16[] = { 0x44u, 0x33u, 0x34u, 0x12u };
            type_unsigned_8 p32[] = { 0x44u, 0x33u, 0x22u, 0x11u, 0x34u, 0x12u };
            C_INT f = !lfg_prepare(CORE_MACHINE_CPU_PROFILE_80386, &s);

            if (z) {
                c[0] = 0x66u;
                c[1] = 0x0fu;
                c[2] = op[i];
                c[3] = 0x06u;
                c[4] = 0u;
                c[5] = 0x10u;
            }
            if (!f) {
                f |= !test_core_machine_fixture_prepare_real_mode_execution(
                    s.machine, 0u);
                s.machine->executor_cpu.data.eflags =
                    VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
                f |= core_machine_memory_write(s.machine, 0x1000u, z ? p32 :
                        p16, z ? 6u : 4u) != TYPE_STATUS_OK ||
                    !lfg_run_prepared(&s, c, z ? 6u : 5u, &a, &d, &st) ||
                    st != TYPE_STATUS_OK || d.first_fault.valid ||
                    a.data.eip != (z ? 6u : 5u) ||
                    a.data.eflags != (VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF) ||
                    (z ? a.data.eax : a.data.ax) !=
                        (z ? 0x11223344u : 0x3344u);
                if (i == 0u)
                    f |= a.data.ss.selector != 0x1234u;
                else if (i == 1u)
                    f |= a.data.fs.selector != 0x1234u;
                else
                    f |= a.data.gs.selector != 0x1234u;
            }
            if (f) {
                STD_PRINTF("LFG real op=%02x size=%u status=%d eip=%08x eax=%08x ss=%04x fs=%04x gs=%04x flags=%08x first=%u mask=%08x\n",
                    op[i], z, st, a.data.eip, a.data.eax, a.data.ss.selector,
                    a.data.fs.selector, a.data.gs.selector, a.data.eflags,
                    d.first_fault.valid, d.first_fault.exception_mask);
                core_machine_destroy(s.machine);
                return 0;
            }
            core_machine_destroy(s.machine);
        }
    }
    return 1;
}
static C_INT lfg_test_reg_direct_80386(C_VOID)
{
    static const type_unsigned_8 op[] = { 0xb2u, 0xb4u, 0xb5u };
    type_unsigned_8 i;

    for (i = 0u; i < 3u; ++i) {
        lfg_machine s;
        t_cpu a;
        t_cpu b;
        core_machine_cpu_diagnostic d;
        type_status st;
        type_unsigned_8 c[] = { 0x0fu, op[i], 0xc0u };
        C_INT f = !lfg_prepare(CORE_MACHINE_CPU_PROFILE_80386, &s);

        if (!f) {
            f |= !test_core_machine_fixture_prepare_real_mode_execution(
                s.machine, 0u);
            s.machine->executor_cpu.data.ss.selector = 0x0018u;
            s.machine->executor_cpu.data.fs.selector = 0x1111u;
            s.machine->executor_cpu.data.gs.selector = 0x2222u;
            f |= !test_core_machine_fixture_preflight_real_ud_terminal(s.machine);
            b = test_core_machine_fixture_capture_cpu_after_run(s.machine);
            f |= !lfg_run_prepared(&s, c, 3u, &a, &d, &st) ||
                st != TYPE_STATUS_FAULT || !d.first_fault.valid ||
                !TYPE_GET_BIT(d.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                a.data.eip != b.data.eip || a.data.eax != b.data.eax ||
                a.data.eflags != b.data.eflags ||
                (i == 0u ? a.data.ss.selector : i == 1u ?
                    a.data.fs.selector : a.data.gs.selector) !=
                    (i == 0u ? b.data.ss.selector : i == 1u ?
                        b.data.fs.selector : b.data.gs.selector);
        }
        core_machine_destroy(s.machine);
        if (f)
            return 0;
    }
    return 1;
}

static C_INT lfg_test_80286_memory(C_VOID)
{
    static const type_unsigned_8 op[] = { 0xb2u, 0xb4u, 0xb5u };
    type_unsigned_8 i;
    type_unsigned_8 z;

    for (i = 0u; i < 3u; ++i) {
        for (z = 0u; z < 2u; ++z) {
            lfg_machine s;
            t_cpu a;
            t_cpu b;
            core_machine_cpu_diagnostic d;
            type_status st;
            type_unsigned_8 c[] = { 0x0fu, op[i], 0x06u, 0, 0x10u, 0 };
            C_INT f = !lfg_prepare(CORE_MACHINE_CPU_PROFILE_80286, &s);

            if (z) {
                c[0] = 0x66u;
                c[1] = 0x0fu;
                c[2] = op[i];
                c[3] = 0x06u;
                c[4] = 0u;
                c[5] = 0x10u;
            }
            if (!f) {
                f |= !test_core_machine_fixture_prepare_real_mode_execution(
                    s.machine, 0u);
                s.machine->executor_cpu.data.ss.selector = 0x0018u;
                s.machine->executor_cpu.data.fs.selector = 0x1111u;
                s.machine->executor_cpu.data.gs.selector = 0x2222u;
                f |= !test_core_machine_fixture_preflight_real_ud_terminal(s.machine);
                b = test_core_machine_fixture_capture_cpu_after_run(s.machine);
                f |= !lfg_run_prepared(&s, c, z ? 6u : 5u, &a, &d, &st) ||
                    st != TYPE_STATUS_FAULT || !d.first_fault.valid ||
                    !TYPE_GET_BIT(d.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                    a.data.eip != b.data.eip || a.data.eax != b.data.eax ||
                    a.data.eflags != b.data.eflags ||
                    a.data.fs.selector != b.data.fs.selector ||
                    a.data.gs.selector != b.data.gs.selector ||
                    (i == 0u ? a.data.ss.selector : i == 1u ?
                        a.data.fs.selector : a.data.gs.selector) !=
                        (i == 0u ? b.data.ss.selector : i == 1u ?
                            b.data.fs.selector : b.data.gs.selector);
            }
            core_machine_destroy(s.machine);
            if (f)
                return 0;
        }
    }
    return 1;
}
static C_INT lfg_prepare_protected(lfg_machine *state)
{
    static const type_unsigned_8 pointer[] = { 0x1fu, 0, 0, 0x03u, 0, 0 };
    static const type_unsigned_8 gdt[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0xffu, 0xffu, 0, 0x20u, 0, 0x9au, 0, 0,
        0xffu, 0xffu, 0, 0, 0, 0x92u, 0, 0, 0xffu, 0xffu, 0, 0x40u, 0, 0x92u,
        0, 0
    };
    static const type_unsigned_8 bootstrap[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u, 0xb8u, 0x01u, 0x00u, 0x0fu, 0x01u,
        0xf0u, 0xb8u, 0x10u, 0x00u, 0x8eu, 0xd8u, 0x8eu, 0xc0u, 0xb8u, 0x18u,
        0x00u, 0x8eu, 0xd0u, 0xbcu, 0x00u, 0x80u, 0xeau, 0x00u, 0x00u, 0x08u,
        0x00u
    };
    static const type_unsigned_8 hlt[] = { 0xf4u };
    core_machine_run_result result;

    return lfg_prepare(CORE_MACHINE_CPU_PROFILE_80386, state) &&
        core_machine_memory_write(state->machine, 0x0100u, pointer,
            sizeof(pointer)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x0300u, gdt,
            sizeof(gdt)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0u, bootstrap,
            sizeof(bootstrap)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x2000u, hlt,
            sizeof(hlt)) == TYPE_STATUS_OK &&
        core_machine_run(state->machine, (core_machine_run_budget){ 96u, 0u },
            &result) == TYPE_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
}
static C_INT lfg_test_protected(C_VOID)
{
    static const type_unsigned_8 opcodes[] = { 0xb2u, 0xb4u, 0xb5u };
    static const type_unsigned_8 pointer16[] = { 0x44u, 0x33u, 0x10u, 0x00u };
    static const type_unsigned_8 pointer32[] = { 0x44u, 0x33u, 0x22u, 0x11u, 0x10u, 0x00u };
    type_unsigned_8 opcode;
    type_unsigned_8 operand32;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        for (operand32 = 0u; operand32 != 2u; ++operand32) {
            lfg_machine state;
            core_machine_run_result result;
            t_cpu after;
            type_unsigned_8 code[] = { 0x0fu, opcodes[opcode], 0x06u, 0x00u, 0x10u, 0u };
            const type_unsigned_8 *pointer = operand32 ? pointer32 : pointer16;
            type_unsigned_8 code_bytes = operand32 ? 6u : 5u;
            type_unsigned_8 pointer_bytes = operand32 ? 6u : 4u;
            type_unsigned_32 expected_offset = operand32 ? 0x11223344u : 0x3344u;
            C_INT failed = !lfg_prepare_protected(&state);

            if (!failed && operand32) {
                code[0] = 0x66u;
                code[1] = 0x0fu;
                code[2] = opcodes[opcode];
                code[3] = 0x06u;
                code[4] = 0x00u;
                code[5] = 0x10u;
            }
            if (!failed) {
                state.machine->executor_cpu.data.ss.selector = 0x0018u;
                state.machine->executor_cpu.data.fs.selector = 0x1111u;
                state.machine->executor_cpu.data.gs.selector = 0x2222u;
                state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
                failed |= core_machine_memory_write(state.machine, 0x1000u, pointer,
                        pointer_bytes) != TYPE_STATUS_OK ||
                    core_machine_memory_write(state.machine, 0x2000u, code,
                        code_bytes) != TYPE_STATUS_OK;
                test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
                failed |= core_machine_run(state.machine,
                        (core_machine_run_budget){ 1u, 0u }, &result) != TYPE_STATUS_OK ||
                    result.reason != CORE_MACHINE_STOP_BUDGET;
                after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
                failed |= after.data.eip != code_bytes ||
                    (operand32 ? after.data.eax : after.data.ax) != expected_offset ||
                    after.data.eflags != (VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF) ||
                    (opcode == 0u ? after.data.ss.selector : opcode == 1u ?
                        after.data.fs.selector : after.data.gs.selector) != 0x0010u;
            }
            core_machine_destroy(state.machine);
            if (failed)
                return 0;
        }
    }
    return 1;
}
static C_INT lfg_test_source_fault_atomicity(C_VOID)
{
    static const type_unsigned_8 opcodes[] = { 0xb2u, 0xb4u, 0xb5u };
    type_unsigned_8 opcode;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        lfg_machine state;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu before = {0};
        t_cpu after = {0};
        type_unsigned_8 code[] = { 0x0fu, opcodes[opcode], 0x06u, 0x00u, 0x10u };
        C_INT failed = !lfg_prepare_protected(&state);

        if (!failed) {
            state.machine->executor_cpu.data.ds.limit = 0x1001u;
            state.machine->executor_cpu.data.eax = 0x55557777u;
            state.machine->executor_cpu.data.ss.selector = 0x0018u;
            state.machine->executor_cpu.data.fs.selector = 0x1111u;
            state.machine->executor_cpu.data.gs.selector = 0x2222u;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
            failed |= core_machine_memory_write(state.machine, 0x2000u, code,
                    sizeof(code)) != TYPE_STATUS_OK;
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_run(state.machine,
                    (core_machine_run_budget){ 1u, 0u }, &result) != TYPE_STATUS_FAULT ||
                result.reason != CORE_MACHINE_STOP_FAULT ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                    diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
                after.data.eip != before.data.eip || after.data.eax != before.data.eax ||
                after.data.eflags != before.data.eflags ||
                (opcode == 0u ? after.data.ss.selector : opcode == 1u ?
                    after.data.fs.selector : after.data.gs.selector) !=
                    (opcode == 0u ? before.data.ss.selector : opcode == 1u ?
                        before.data.fs.selector : before.data.gs.selector);
        }
        if (failed)
            STD_PRINTF("LFG source-fault op=%02x reason=%d first=%u mask=%08x eip=%08x/%08x eax=%08x/%08x flags=%08x/%08x\n",
                opcodes[opcode], result.reason, diagnostic.first_fault.valid,
                diagnostic.first_fault.exception_mask, before.data.eip, after.data.eip,
                before.data.eax, after.data.eax, before.data.eflags, after.data.eflags);
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}
static C_INT lfg_test_irq_shadow(C_VOID)
{
    static const type_unsigned_8 opcodes[] = { 0xb2u, 0xb4u, 0xb5u };
    static const type_unsigned_8 pointer[] = { 0x44u, 0x33u, 0x00u, 0x00u };
    static const type_unsigned_8 hlt = 0xf4u;
    type_unsigned_8 opcode;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        lfg_machine state;
        core_machine_pic_irq_source source;
        core_machine_run_result result;
        t_cpu after = {0};
        type_unsigned_8 code[] = { 0x0fu, opcodes[opcode], 0x06u, 0x00u, 0x10u, 0x90u };
        type_unsigned_16 vector_offset = 0x0100u;
        type_unsigned_16 vector_segment = 0u;
        type_unsigned_16 frame_ip = 0u;
        C_INT failed = !lfg_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            failed |= !test_core_machine_fixture_prepare_real_mode_execution(
                    state.machine, 0u) ||
                core_machine_memory_write(state.machine, 0x1000u, pointer,
                    sizeof(pointer)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0u, code,
                    sizeof(code)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x20u * 4u,
                    &vector_offset, sizeof(vector_offset)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x20u * 4u + 2u,
                    &vector_segment, sizeof(vector_segment)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x0100u, &hlt,
                    sizeof(hlt)) != TYPE_STATUS_OK;
        }
        if (!failed) {
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_IF;
            STD_MEMSET(&source, 0, sizeof(source));
            state.machine->shared_pic_master.data.icw2 = 0x20u;
            core_machine_pic_irq_source_bind(&source,
                &state.machine->shared_pic_master, &state.machine->shared_pic_slave, 0u);
            core_machine_pic_irq_source_assert(&source);
            core_machine_pic_irq_source_deassert(&source);
            failed |= core_machine_run(state.machine,
                    (core_machine_run_budget){ 2u, 0u }, &result) != TYPE_STATUS_OK ||
                result.reason != (opcode == 0u ? CORE_MACHINE_STOP_BUDGET :
                    CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT);
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                    after.data.ss.base + (type_unsigned_16)after.data.esp,
                    (type_virtual_address)&frame_ip, sizeof(frame_ip)) != TYPE_STATUS_OK ||
                after.data.eip != (opcode == 0u ? 0x0100u : 0x0101u) || !TYPE_GET_BIT(
                    state.machine->shared_pic_master.data.isr, VPIC_ISR_IRQ(0u)) ||
                TYPE_GET_BIT(state.machine->shared_pic_master.data.irr,
                    VPIC_IRR_IRQ(0u)) || frame_ip != (opcode == 0u ? 6u : 5u);
        }
        if (failed)
            STD_PRINTF("LFG irq op=%02x eip=%08x esp=%08x irr=%02x isr=%02x frame=%04x\n",
                opcodes[opcode], after.data.eip, after.data.esp,
                state.machine->shared_pic_master.data.irr,
                state.machine->shared_pic_master.data.isr, frame_ip);
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}
C_INT main(C_VOID)
{
    if (!lfg_test_real()) {
        STD_PRINTF("LFG stage=real\n");
        return 1;
    }
    if (!lfg_test_reg_direct_80386()) {
        STD_PRINTF("LFG stage=regdirect\n");
        return 1;
    }
    if (!lfg_test_80286_memory()) {
        STD_PRINTF("LFG stage=80286\n");
        return 1;
    }
    if (!lfg_test_protected()) {
        STD_PRINTF("LFG stage=protected\n");
        return 1;
    }
    if (!lfg_test_source_fault_atomicity())
        return 1;
    if (!lfg_test_irq_shadow())
        return 1;
    STD_PRINTF("M5:T316:S24:LSS-LFS-LGS:OK\n");
    return 0;
}
