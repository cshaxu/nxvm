#include "type.h"
#include "core/machine/cpu.h"
#include "core/machine/pic.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define LAHF_SAHF_MASK (VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | VCPU_EFLAGS_AF | \
    VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF)
#define LAHF_SAHF_IGNORED_AH_BITS 0x2au

typedef struct lahf_sahf_machine {
    core_machine *machine;
} lahf_sahf_machine;

static C_VOID lahf_sahf_reset(C_VOID *opaque)
{
    lahf_sahf_machine *state = (lahf_sahf_machine *)opaque;

    if (state != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider lahf_sahf_provider = {
    lahf_sahf_reset, STD_NULL
};

static C_INT lahf_sahf_prepare(core_machine_cpu_profile profile,
    lahf_sahf_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    STD_MEMSET(state, 0, sizeof(*state));
return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &lahf_sahf_provider, state, &state->machine) &&
        test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u);
}

static C_VOID lahf_sahf_seed(lahf_sahf_machine *state)
{
    t_cpu *cpu = &state->machine->executor_cpu;

    cpu->data.eax = 0xaabbccddU;
    cpu->data.ecx = 0x11223344u;
    cpu->data.edx = 0x55667788u;
    cpu->data.ebx = 0x99aabbccU;
    cpu->data.esp = 0x8000u;
    cpu->data.ebp = 0x120u;
    cpu->data.esi = 0x10u;
    cpu->data.edi = 0x20u;
    cpu->data.eflags = VCPU_EFLAGS_IF | VCPU_EFLAGS_DF | VCPU_EFLAGS_OF;
}

static C_INT lahf_sahf_nonparticipants_same(const t_cpu *before,
    const t_cpu *after)
{
    return after->data.ecx == before->data.ecx &&
        after->data.edx == before->data.edx &&
        after->data.ebx == before->data.ebx &&
        after->data.esp == before->data.esp &&
        after->data.ebp == before->data.ebp &&
        after->data.esi == before->data.esi &&
        after->data.edi == before->data.edi;
}

static C_INT lahf_sahf_irq_nonstack_gprs_same(const t_cpu *before,
    const t_cpu *after)
{
    return after->data.ecx == before->data.ecx &&
        after->data.edx == before->data.edx &&
        after->data.ebx == before->data.ebx &&
        after->data.ebp == before->data.ebp &&
        after->data.esi == before->data.esi &&
        after->data.edi == before->data.edi;
}

static C_INT lahf_sahf_run(lahf_sahf_machine *state, const type_unsigned_8 *code,
    type_unsigned_8 bytes, t_cpu *after, core_machine_cpu_diagnostic *diagnostic,
    type_status *status, core_machine_run_result *result)
{
    if (core_machine_memory_write(state->machine, 0u, code, bytes) !=
        TYPE_STATUS_OK)
        return 0;
    *status = core_machine_run(state->machine,
        (core_machine_run_budget){1u,0u}, result);
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) ==
        TYPE_STATUS_OK;
}

static C_INT lahf_sahf_test_default(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 lahf = 0x9fu;
    static const type_unsigned_8 sahf = 0x9eu;
    static const type_unsigned_32 transfer_values[] = {0u, LAHF_SAHF_MASK};
    type_unsigned_8 profile;
    type_unsigned_8 transfer;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        for (transfer = 0u; transfer != sizeof(transfer_values) /
            sizeof(transfer_values[0]); ++transfer) {
            lahf_sahf_machine state;
            t_cpu before;
            t_cpu after;
            core_machine_cpu_diagnostic diagnostic;
            core_machine_run_result result;
            type_status status;
            type_unsigned_32 flags = VCPU_EFLAGS_IF | VCPU_EFLAGS_DF |
                VCPU_EFLAGS_OF | transfer_values[transfer];
            C_INT failed = !lahf_sahf_prepare(profiles[profile], &state);

            if (!failed) {
                lahf_sahf_seed(&state);
                state.machine->executor_cpu.data.eflags = flags;
                before = test_core_machine_fixture_capture_cpu_after_run(
                    state.machine);
                failed |= !lahf_sahf_run(&state, &lahf, sizeof(lahf), &after,
                    &diagnostic, &status, &result) ||
                    status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
                    after.data.eip != 1u ||
                    !lahf_sahf_nonparticipants_same(&before, &after) ||
                    after.data.eax != ((before.data.eax & 0xffff00ffu) |
                    ((transfer_values[transfer] | 0x02u) << 8u)) ||
                    after.data.eflags != flags;
            }
            core_machine_destroy(state.machine);
            if (failed)
                return 0;

            failed = !lahf_sahf_prepare(profiles[profile], &state);
            if (!failed) {
                lahf_sahf_seed(&state);
                state.machine->executor_cpu.data.eax = 0xaabb00ddU |
                    ((transfer_values[transfer] |
                    LAHF_SAHF_IGNORED_AH_BITS) << 8u);
                before = test_core_machine_fixture_capture_cpu_after_run(
                    state.machine);
                failed |= !lahf_sahf_run(&state, &sahf, sizeof(sahf), &after,
                    &diagnostic, &status, &result) ||
                    status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
                    after.data.eip != 1u ||
                    !lahf_sahf_nonparticipants_same(&before, &after) ||
                    after.data.eax != before.data.eax ||
                    (after.data.eflags & LAHF_SAHF_MASK) !=
                    transfer_values[transfer] ||
                    (after.data.eflags & ~LAHF_SAHF_MASK) !=
                    (before.data.eflags & ~LAHF_SAHF_MASK);
            }
            core_machine_destroy(state.machine);
            if (failed)
                return 0;
        }
    }
    return 1;
}

static C_INT lahf_sahf_expect_ud(core_machine_cpu_profile profile,
    const type_unsigned_8 *code, type_unsigned_8 bytes)
{
    lahf_sahf_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_status status;
    C_INT failed = !lahf_sahf_prepare(profile, &state);

    if (!failed) {
        lahf_sahf_seed(&state);
        failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
            state.machine);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !lahf_sahf_run(&state, code, bytes, &after, &diagnostic,
            &status, &result) || status != TYPE_STATUS_FAULT ||
            !diagnostic.first_fault.valid || !TYPE_GET_BIT(
            diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            after.data.eip != 0u || STD_MEMCMP(&before.data, &after.data,
            sizeof(before.data)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT lahf_sahf_test_attributes(C_VOID)
{
    static const type_unsigned_8 lahf66[] = {0x66u,0x9fu};
    static const type_unsigned_8 lahf67[] = {0x67u,0x9fu};
    static const type_unsigned_8 lahf_both[] = {0x66u,0x67u,0x9fu};
    static const type_unsigned_8 sahf66[] = {0x66u,0x9eu};
    static const type_unsigned_8 sahf67[] = {0x67u,0x9eu};
    static const type_unsigned_8 sahf_both[] = {0x66u,0x67u,0x9eu};
    static const type_unsigned_8 lock_lahf[] = {0xf0u,0x9fu};
    static const type_unsigned_8 lock_sahf[] = {0xf0u,0x9eu};
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    type_unsigned_8 profile;

    for (profile = 0u; profile != sizeof(legacy) / sizeof(legacy[0]);
        ++profile) {
        if (!lahf_sahf_expect_ud(legacy[profile], lahf66, sizeof(lahf66)) ||
            !lahf_sahf_expect_ud(legacy[profile], lahf67, sizeof(lahf67)) ||
            !lahf_sahf_expect_ud(legacy[profile], lahf_both,
            sizeof(lahf_both)) ||
            !lahf_sahf_expect_ud(legacy[profile], sahf66, sizeof(sahf66)) ||
            !lahf_sahf_expect_ud(legacy[profile], sahf67, sizeof(sahf67)) ||
            !lahf_sahf_expect_ud(legacy[profile], sahf_both,
            sizeof(sahf_both)))
            return 0;
    }
    return lahf_sahf_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, lock_lahf,
        sizeof(lock_lahf)) && lahf_sahf_expect_ud(
        CORE_MACHINE_CPU_PROFILE_80386, lock_sahf, sizeof(lock_sahf));
}

static C_INT lahf_sahf_test_386_attributes(C_VOID)
{
    static const type_unsigned_8 lahf66[] = {0x66u,0x9fu};
    static const type_unsigned_8 lahf67[] = {0x67u,0x9fu};
    static const type_unsigned_8 lahf_both[] = {0x66u,0x67u,0x9fu};
    static const type_unsigned_8 sahf66[] = {0x66u,0x9eu};
    static const type_unsigned_8 sahf67[] = {0x67u,0x9eu};
    static const type_unsigned_8 sahf_both[] = {0x66u,0x67u,0x9eu};
    const type_unsigned_8 *forms[] = {lahf66, lahf67, lahf_both, sahf66, sahf67,
        sahf_both};
    const type_unsigned_8 bytes[] = {2u,2u,3u,2u,2u,3u};
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(bytes); ++form) {
        lahf_sahf_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        type_status status;
        C_INT failed = !lahf_sahf_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed) {
            lahf_sahf_seed(&state);
            if (form < 3u)
                state.machine->executor_cpu.data.eflags |= LAHF_SAHF_MASK;
            else
                state.machine->executor_cpu.data.eax = 0xaabb00ddU;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !lahf_sahf_run(&state, forms[form], bytes[form], &after,
                &diagnostic, &status, &result) || status != TYPE_STATUS_OK ||
                diagnostic.first_fault.valid || after.data.eip != bytes[form] ||
                !lahf_sahf_nonparticipants_same(&before, &after) ||
                (form >= 3u ? after.data.eax != before.data.eax ||
                (after.data.eflags & LAHF_SAHF_MASK) != 0u ||
                (after.data.eflags & ~LAHF_SAHF_MASK) !=
                (before.data.eflags & ~LAHF_SAHF_MASK) : after.data.eax !=
                ((before.data.eax & 0xffff00ffu) |
                (((before.data.eflags & LAHF_SAHF_MASK) | 0x02u) << 8u)) ||
                after.data.eflags != before.data.eflags);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT lahf_sahf_boot_protected(lahf_sahf_machine *state)
{
    static const type_unsigned_8 pointer[] = {0x1fu,0u,0u,0x03u,0u,0u};
    static const type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0, 0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0,0,0x92u,0,0, 0xffu,0xffu,0,0,0,0x92u,0,0
    };
    static const type_unsigned_8 boot[] = {
        0x0fu,0x01u,0x16u,0,1u, 0xb8u,1u,0,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0,0x8eu,0xd8u,0x8eu,0xc0u, 0xb8u,0x18u,0,0x8eu,
        0xd0u,0xbcu,0,0x80u, 0xeau,0,0,8u,0
    };
    static const type_unsigned_8 halt = 0xf4u;
    core_machine_run_result result;

    return core_machine_memory_write(state->machine, 0x100u, pointer,
        sizeof(pointer)) == TYPE_STATUS_OK && core_machine_memory_write(
        state->machine, 0x300u, gdt, sizeof(gdt)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0u, boot, sizeof(boot)) ==
        TYPE_STATUS_OK && core_machine_memory_write(state->machine, 0x2000u,
        &halt, sizeof(halt)) == TYPE_STATUS_OK && core_machine_run(
        state->machine, (core_machine_run_budget){96u,0u}, &result) ==
        TYPE_STATUS_OK && result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
}

static C_INT lahf_sahf_test_protected(C_VOID)
{
    static const type_unsigned_8 opcodes[] = {0x9fu,0x9eu};
    type_unsigned_8 opcode;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        lahf_sahf_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        const type_unsigned_32 flags = VCPU_EFLAGS_IF | VCPU_EFLAGS_DF |
            VCPU_EFLAGS_OF | VCPU_EFLAGS_IOPL | LAHF_SAHF_MASK;
        C_INT failed = !lahf_sahf_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed)
            failed |= !lahf_sahf_boot_protected(&state);
        if (!failed) {
            const t_cpu_data_sreg cs = state.machine->executor_cpu.data.cs;
            const t_cpu_data_sreg ds = state.machine->executor_cpu.data.ds;
            const t_cpu_data_sreg es = state.machine->executor_cpu.data.es;
            const t_cpu_data_sreg ss = state.machine->executor_cpu.data.ss;

            lahf_sahf_seed(&state);
            state.machine->executor_cpu.data.cs = cs;
            state.machine->executor_cpu.data.ds = ds;
            state.machine->executor_cpu.data.es = es;
            state.machine->executor_cpu.data.ss = ss;
            state.machine->executor_cpu.data.eflags = flags;
            if (opcodes[opcode] == 0x9eu)
                state.machine->executor_cpu.data.eax = 0xaabb00ddU;
            failed |= core_machine_memory_write(state.machine, 0x2000u,
                &opcodes[opcode], 1u) != TYPE_STATUS_OK;
            before = test_core_machine_fixture_capture_cpu_after_run(
                state.machine);
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){1u,0u}, &result) != TYPE_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_BUDGET ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= diagnostic.first_fault.valid || after.data.eip != 1u ||
                !lahf_sahf_nonparticipants_same(&before, &after) ||
                (opcodes[opcode] == 0x9eu ? after.data.eax != before.data.eax ||
                (after.data.eflags & LAHF_SAHF_MASK) != 0u ||
                (after.data.eflags & ~LAHF_SAHF_MASK) !=
                (before.data.eflags & ~LAHF_SAHF_MASK) :
                after.data.eax != ((before.data.eax & 0xffff00ffu) |
                ((LAHF_SAHF_MASK | 0x02u) << 8u)) ||
                after.data.eflags != before.data.eflags) ||
                STD_MEMCMP(&before.data.cs, &after.data.cs,
                sizeof(before.data.cs)) != 0 || STD_MEMCMP(&before.data.ds,
                &after.data.ds, sizeof(before.data.ds)) != 0 ||
                STD_MEMCMP(&before.data.es, &after.data.es,
                sizeof(before.data.es)) != 0 || STD_MEMCMP(&before.data.ss,
                &after.data.ss, sizeof(before.data.ss)) != 0;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT lahf_sahf_test_vm86(C_VOID)
{
    static const type_unsigned_8 opcodes[] = {0x9fu,0x9eu};
    type_unsigned_8 opcode;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        lahf_sahf_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        const type_unsigned_32 flags = VCPU_EFLAGS_VM | VCPU_EFLAGS_IF |
            VCPU_EFLAGS_DF | VCPU_EFLAGS_OF | VCPU_EFLAGS_IOPL |
            LAHF_SAHF_MASK;
        C_INT failed = !lahf_sahf_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed) {
            lahf_sahf_seed(&state);
            state.machine->executor_cpu.data.cr0 |= VCPU_CR0_PE;
            state.machine->executor_cpu.data.eflags = flags;
            state.machine->executor_cpu.data.cs.selector = 0u;
            state.machine->executor_cpu.data.cs.base = 0u;
            state.machine->executor_cpu.data.cs.limit = 0xffffu;
            state.machine->executor_cpu.data.cs.dpl = 3u;
            state.machine->executor_cpu.data.cs.flagValid = TYPE_TRUE;
            state.machine->executor_cpu.data.cs.seg.exec.defsize = TYPE_FALSE;
            state.machine->executor_cpu.data.ss.selector = 0u;
            state.machine->executor_cpu.data.ss.base = 0u;
            state.machine->executor_cpu.data.ss.limit = 0xffffu;
            state.machine->executor_cpu.data.ss.dpl = 3u;
            state.machine->executor_cpu.data.ss.flagValid = TYPE_TRUE;
            state.machine->executor_cpu.data.ss.seg.data.big = TYPE_FALSE;
            if (opcodes[opcode] == 0x9eu)
                state.machine->executor_cpu.data.eax = 0xaabb00ddU;
            failed |= core_machine_memory_write(state.machine, 0u,
                &opcodes[opcode], 1u) != TYPE_STATUS_OK;
            before = test_core_machine_fixture_capture_cpu_after_run(
                state.machine);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){1u,0u}, &result) != TYPE_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_BUDGET ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= diagnostic.first_fault.valid || after.data.eip != 1u ||
                !lahf_sahf_nonparticipants_same(&before, &after) ||
                (opcodes[opcode] == 0x9eu ? after.data.eax != before.data.eax ||
                (after.data.eflags & LAHF_SAHF_MASK) != 0u ||
                (after.data.eflags & ~LAHF_SAHF_MASK) !=
                (before.data.eflags & ~LAHF_SAHF_MASK) :
                after.data.eax != ((before.data.eax & 0xffff00ffu) |
                ((LAHF_SAHF_MASK | 0x02u) << 8u)) ||
                after.data.eflags != before.data.eflags);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT lahf_sahf_irq_case(C_INT sahf)
{
    static const type_unsigned_8 lahf[] = {0x9fu,0x90u};
    static const type_unsigned_8 sahf_code[] = {0x9eu,0x90u};
    static const type_unsigned_8 hlt = 0xf4u;
    lahf_sahf_machine state;
    core_machine_pic_irq_source irq;
    core_machine_run_result result;
    t_cpu before;
    t_cpu after;
    type_unsigned_16 offset = 0x100u;
    type_unsigned_16 segment = 0u;
    type_unsigned_16 frame_ip = 0u;
    const type_unsigned_8 *code = sahf ? sahf_code : lahf;
    C_INT failed = !lahf_sahf_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0u, code, 2u) !=
            TYPE_STATUS_OK || core_machine_memory_write(state.machine, 0x80u,
            &offset, sizeof(offset)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0x82u, &segment, sizeof(segment)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x100u, &hlt, sizeof(hlt)) !=
            TYPE_STATUS_OK;
    }
    if (!failed) {
        lahf_sahf_seed(&state);
        if (sahf)
            state.machine->executor_cpu.data.eax = 0xaabbffddU;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_IF;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        STD_MEMSET(&irq, 0, sizeof(irq));
        state.machine->shared_pic_master.data.icw2 = 0x20u;
        core_machine_pic_irq_source_bind(&irq, &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&irq);
        core_machine_pic_irq_source_deassert(&irq);
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){2u,0u}, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
            after.data.ss.base + (type_unsigned_16)after.data.esp,
            TYPE_REFERENCE_OF(frame_ip), sizeof(frame_ip)) != TYPE_STATUS_OK ||
            after.data.eip != 0x101u || frame_ip != 1u ||
            !lahf_sahf_irq_nonstack_gprs_same(&before, &after) ||
            (sahf ? after.data.eax != 0xaabbffddU : after.data.eax !=
            0xaabb02ddU) || after.data.eflags !=
            (sahf ? LAHF_SAHF_MASK : 0u) ||
            !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,
            VPIC_ISR_IRQ(0u)) || TYPE_GET_BIT(state.machine->shared_pic_master.data.irr,
            VPIC_IRR_IRQ(0u));
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT lahf_sahf_test_irq(C_VOID)
{
    return lahf_sahf_irq_case(0) && lahf_sahf_irq_case(1);
}

/* Compaq's 386 POST carries CF through a 32-bit checksum rotation by using
 * SAHF/LAHF around RCL.  Keep that real-mode sequence at the sole FLAGS owner
 * boundary: RCL changes only CF/OF, and LAHF must expose the resulting CF. */
static C_INT lahf_sahf_test_386_checksum_sequence(C_VOID)
{
    static const type_unsigned_8 code[] = {0x9eu,0x66u,0xd1u,0xd3u,0x9fu};
    lahf_sahf_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    t_cpu after;
    C_INT failed = !lahf_sahf_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0u, code,
            sizeof(code)) != TYPE_STATUS_OK;
    }
    if (!failed) {
        lahf_sahf_seed(&state);
        state.machine->executor_cpu.data.eax = 0x00000300u;
        state.machine->executor_cpu.data.ebx = 0x80000000u;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_IF;
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){3u,0u}, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                TYPE_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= diagnostic.first_fault.valid || after.data.eip != sizeof(code) ||
            after.data.eax != 0x00000300u || after.data.ebx != 1u ||
            after.data.eflags != (VCPU_EFLAGS_IF | VCPU_EFLAGS_CF |
                VCPU_EFLAGS_OF);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

C_INT main(C_VOID)
{
    if (!lahf_sahf_test_default()) {
        STD_PRINTF("LAHF-SAHF stage=default\n");
        return 1;
    }
    if (!lahf_sahf_test_attributes()) {
        STD_PRINTF("LAHF-SAHF stage=attributes\n");
        return 1;
    }
    if (!lahf_sahf_test_386_attributes()) {
        STD_PRINTF("LAHF-SAHF stage=386-attributes\n");
        return 1;
    }
    if (!lahf_sahf_test_protected()) {
        STD_PRINTF("LAHF-SAHF stage=protected\n");
        return 1;
    }
    if (!lahf_sahf_test_vm86()) {
        STD_PRINTF("LAHF-SAHF stage=vm86\n");
        return 1;
    }
    if (!lahf_sahf_test_irq()) {
        STD_PRINTF("LAHF-SAHF stage=irq\n");
        return 1;
    }
    if (!lahf_sahf_test_386_checksum_sequence()) {
        STD_PRINTF("LAHF-SAHF stage=386-checksum-sequence\n");
        return 1;
    }
    STD_PRINTF("M5:T316:S39:LAHF-SAHF:OK\n");
    STD_PRINTF("M5:T401:S36:LAHF-SAHF-PROFILES:OK\n");
    return 0;
}
