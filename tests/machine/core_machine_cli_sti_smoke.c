#include "type.h"
#include "core/machine/cpu.h"
#include "core/machine/pic.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct cli_sti_machine {
    core_machine *machine;
} cli_sti_machine;

static C_INT cli_sti_install_gp_gate(cli_sti_machine *state);

static C_VOID cli_sti_reset(C_VOID *opaque)
{
    cli_sti_machine *state = (cli_sti_machine *)opaque;

    if (state != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider cli_sti_provider = {
    cli_sti_reset, STD_NULL, STD_NULL
};

static C_INT cli_sti_prepare(core_machine_cpu_profile profile,
    cli_sti_machine *state)
{
    const core_machine_config config = { CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        profile, CORE_MACHINE_FPU_PROFILE_NONE };

    STD_MEMSET(state, 0, sizeof(*state));
    return core_machine_create(&config, &state->machine) == TYPE_STATUS_OK &&
        core_machine_bind_execution_provider(state->machine, &cli_sti_provider,
            state) == TYPE_STATUS_OK &&
        core_machine_freeze_execution_providers(state->machine) == TYPE_STATUS_OK &&
        core_machine_reset(state->machine) == TYPE_STATUS_OK;
}

static C_INT cli_sti_run(cli_sti_machine *state, const uint8_t *code,
    uint32_t count, uint32_t budget, t_cpu *after)
{
    core_machine_run_result result;

    if (!test_core_machine_fixture_prepare_real_mode_execution(state->machine,
            0u) || core_machine_memory_write(state->machine, 0u, code,
            count) != TYPE_STATUS_OK || core_machine_run(state->machine,
            (core_machine_run_budget){ budget, 0u }, &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET)
        return 0;
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return 1;
}

static C_INT cli_sti_test_real_forms(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    static const uint8_t opcodes[] = { 0xfau, 0xfbu };
    const uint32_t preserved = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF |
        VCPU_EFLAGS_DF | VCPU_EFLAGS_OF;
    uint8_t profile;
    uint8_t opcode;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile) {
        for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
            cli_sti_machine state;
            t_cpu after;
            uint32_t initial = preserved | (opcodes[opcode] == 0xfau ?
                VCPU_EFLAGS_IF : 0u);
            uint32_t expected = opcodes[opcode] == 0xfau ? preserved :
                preserved | VCPU_EFLAGS_IF;
            C_INT failed = !cli_sti_prepare(profiles[profile], &state);

            if (!failed) {
                state.machine->executor_cpu.data.eflags = initial;
                failed |= !cli_sti_run(&state, &opcodes[opcode], 1u, 1u, &after) ||
                    after.data.eip != 1u || after.data.eflags != expected;
            }
            core_machine_destroy(state.machine);
            if (failed)
                return 0;
        }
    }
    return 1;
}

static C_INT cli_sti_test_irq_shadow(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    static const uint8_t sti_nop[] = { 0xfbu, 0x90u };
    static const uint8_t cli_nop[] = { 0xfau, 0x90u };
    static const uint8_t hlt = 0xf4u;
    uint8_t profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile) {
        cli_sti_machine state;
        core_machine_pic_irq_source source;
        core_machine_run_result result;
        t_cpu after;
        uint32_t vector = 0x20u;
        uint16_t offset = 0x0100u;
        uint16_t segment = 0u;
        uint16_t frame_ip = 0u;
        type_status frame_status;
        C_INT failed = !cli_sti_prepare(profiles[profile], &state);

        if (!failed) {
            failed |= !test_core_machine_fixture_prepare_real_mode_execution(
                    state.machine, 0u) ||
                core_machine_memory_write(state.machine, vector * 4u, &offset,
                    sizeof(offset)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, vector * 4u + 2u,
                    &segment, sizeof(segment)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x0100u, &hlt,
                    sizeof(hlt)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0u, sti_nop,
                    sizeof(sti_nop)) != TYPE_STATUS_OK;
        }
        if (!failed) {
            STD_MEMSET(&source, 0, sizeof(source));
            state.machine->shared_pic_master.data.icw2 = (uint8_t)vector;
            core_machine_pic_irq_source_bind(&source,
                &state.machine->shared_pic_master, &state.machine->shared_pic_slave, 0u);
            core_machine_pic_irq_source_assert(&source);
            core_machine_pic_irq_source_deassert(&source);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){ 2u, 0u }, &result) != TYPE_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_BUDGET;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            frame_status = core_machine_memory_read_physical(
                &state.machine->executor_memory,
                after.data.ss.base + (uint16_t)after.data.esp,
                (type_virtual_address)&frame_ip, sizeof(frame_ip));
            failed |=
                after.data.eip != offset ||
                !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr, VPIC_ISR_IRQ(0u)) ||
                TYPE_GET_BIT(state.machine->shared_pic_master.data.irr, VPIC_IRR_IRQ(0u)) ||
                frame_status != TYPE_STATUS_OK || frame_ip != 2u;
        }
        if (failed) {
            core_machine_destroy(state.machine);
            return 0;
        }
        core_machine_destroy(state.machine);
    }
    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile) {
        cli_sti_machine state;
        core_machine_pic_irq_source source;
        core_machine_run_result result;
        C_INT failed = !cli_sti_prepare(profiles[profile], &state);

        if (!failed) {
            failed |= !test_core_machine_fixture_prepare_real_mode_execution(
                    state.machine, 0u) ||
                core_machine_memory_write(state.machine, 0u, cli_nop,
                    sizeof(cli_nop)) != TYPE_STATUS_OK;
        }
        if (!failed) {
            STD_MEMSET(&source, 0, sizeof(source));
            core_machine_pic_irq_source_bind(&source,
                &state.machine->shared_pic_master, &state.machine->shared_pic_slave, 0u);
            core_machine_pic_irq_source_assert(&source);
            core_machine_pic_irq_source_deassert(&source);
            failed |= core_machine_run(state.machine,
                    (core_machine_run_budget){ 2u, 0u }, &result) != TYPE_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_BUDGET ||
                !TYPE_GET_BIT(state.machine->shared_pic_master.data.irr, VPIC_IRR_IRQ(0u)) ||
                TYPE_GET_BIT(state.machine->shared_pic_master.data.isr, VPIC_ISR_IRQ(0u));
        }
        core_machine_destroy(state.machine);
        if (failed) {
            return 0;
        }
    }
    return 1;
}

static C_INT cli_sti_prepare_protected(cli_sti_machine *state)
{
    static const uint8_t pointer[] = { 0x1fu, 0, 0, 0x03u, 0, 0 };
    static const uint8_t gdt[] = {
        0,0,0,0,0,0,0,0, 0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0,0,0x92u,0,0, 0xffu,0xffu,0,0x40u,0,0x92u,0,0
    };
    static const uint8_t bootstrap[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,0x8eu,0xc0u,0xb8u,0x18u,0x00u,0x8eu,
        0xd0u,0xbcu,0x00u,0x80u,0xeau,0x00u,0x00u,0x08u,0x00u
    };
    static const uint8_t halt[] = { 0xf4u };
    core_machine_run_result result;

    return cli_sti_prepare(CORE_MACHINE_CPU_PROFILE_80386, state) &&
        core_machine_memory_write(state->machine, 0x0100u, pointer,
            sizeof(pointer)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x0300u, gdt,
            sizeof(gdt)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0u, bootstrap,
            sizeof(bootstrap)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x2000u, halt,
            sizeof(halt)) == TYPE_STATUS_OK &&
        core_machine_run(state->machine, (core_machine_run_budget){ 96u, 0u },
            &result) == TYPE_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
}

static C_INT cli_sti_install_gp_gate(cli_sti_machine *state)
{
    static const uint8_t handler[] = { 0xf4u };
    uint8_t tss[10] = { 0 };
    static const uint8_t gdt[] = {
        0,0,0,0,0,0,0,0, 0xffu,0xffu,0,0x20u,0,0x9au,0x40u,0,
        0xffu,0xffu,0,0,0,0x92u,0xcfu,0
    };
    uint8_t gate[8] = { 0 };
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
    return core_machine_memory_write(state->machine, 0x0300u, gdt,
            sizeof(gdt)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x0500u, tss,
            sizeof(tss)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x0400u + 0x0du * 8u,
            gate, sizeof(gate)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x2100u, handler,
            sizeof(handler)) == TYPE_STATUS_OK;
}

static C_INT cli_sti_run_vm86(cli_sti_machine *state, const uint8_t *code,
    uint8_t bytes, uint32_t eflags, C_INT fault, t_cpu *after,
    core_machine_cpu_diagnostic *diagnostic)
{
    core_machine_run_result result;
    type_status status;

    if (!test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u) ||
        core_machine_memory_write(state->machine, 0u, code, bytes) != TYPE_STATUS_OK ||
        !cli_sti_install_gp_gate(state))
        return 0;
    state->machine->executor_cpu.data.cr0 |= VCPU_CR0_PE;
    state->machine->executor_cpu.data.eflags = eflags;
    state->machine->executor_cpu.data.esp = 0x8000u;
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
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) == TYPE_STATUS_OK &&
        status == (fault ? TYPE_STATUS_FAULT : TYPE_STATUS_OK) &&
        result.reason == (fault ? CORE_MACHINE_STOP_FAULT : CORE_MACHINE_STOP_BUDGET);
}

static C_INT cli_sti_test_protected_success(C_VOID)
{
    static const uint8_t opcodes[] = { 0xfau, 0xfbu };
    uint8_t pass;

    for (pass = 0u; pass != 2u; ++pass) {
        uint8_t opcode;

        for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
            cli_sti_machine state;
            core_machine_run_result result;
            t_cpu after;
            uint8_t cpl = pass ? 3u : 0u;
            uint32_t iopl = pass ? VCPU_EFLAGS_IOPL : 0u;
            uint32_t initial = VCPU_EFLAGS_CF | iopl |
                (opcodes[opcode] == 0xfau ? VCPU_EFLAGS_IF : 0u);
            uint32_t expected = VCPU_EFLAGS_CF | iopl |
                (opcodes[opcode] == 0xfbu ? VCPU_EFLAGS_IF : 0u);
            C_INT failed = !cli_sti_prepare_protected(&state);

            if (!failed) {
                state.machine->executor_cpu.data.cs.dpl = cpl;
                state.machine->executor_cpu.data.eflags = initial;
                failed |= core_machine_memory_write(state.machine, 0x2000u,
                        &opcodes[opcode], 1u) != TYPE_STATUS_OK;
                test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
                failed |= core_machine_run(state.machine,
                        (core_machine_run_budget){ 1u, 0u }, &result) != TYPE_STATUS_OK ||
                    result.reason != CORE_MACHINE_STOP_BUDGET;
                after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
                failed |= after.data.eip != 1u ||
                    (after.data.eflags & (VCPU_EFLAGS_CF | VCPU_EFLAGS_IF |
                        VCPU_EFLAGS_IOPL)) != expected;
            }
            core_machine_destroy(state.machine);
            if (failed)
                return 0;
        }
    }
    return 1;
}

static C_INT cli_sti_test_protected_reject(C_VOID)
{
    static const uint8_t opcodes[] = { 0xfau, 0xfbu };
    uint8_t opcode;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        cli_sti_machine state;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu after;
        type_status status;
        uint32_t frame[4] = { 0u, 0u, 0u, 0u };
        const uint32_t flags = VCPU_EFLAGS_CF;
        C_INT failed = !cli_sti_prepare_protected(&state);

        if (!failed) {
            failed |= !cli_sti_install_gp_gate(&state);
        }
        if (!failed) {
            state.machine->executor_cpu.data.cs.dpl = 3u;
            state.machine->executor_cpu.data.eflags = flags;
            failed |= core_machine_memory_write(state.machine, 0x2000u,
                    &opcodes[opcode], 1u) != TYPE_STATUS_OK;
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            status = core_machine_run(state.machine,
                (core_machine_run_budget){ 1u, 0u }, &result);
            failed |= status != TYPE_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_BUDGET ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= diagnostic.first_fault.valid ||
                !diagnostic.last_delivered_exception.valid ||
                diagnostic.delivered_exception_count != 1u || !TYPE_GET_BIT(
                    diagnostic.last_delivered_exception.exception_mask,
                    VCPUINS_EXCEPT_GP) ||
                diagnostic.last_delivered_exception.exception_code != 0u ||
                after.data.cs.selector != 0x0008u || after.data.eip != 0x00000100u ||
                core_machine_memory_read_physical(&state.machine->executor_memory,
                    after.data.ss.base + after.data.esp, (type_virtual_address)frame,
                    sizeof(frame)) != TYPE_STATUS_OK || frame[0] != 0u ||
                frame[1] != 0u || frame[2] != 0x0008u || frame[3] != flags;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT cli_sti_test_vm86(C_VOID)
{
    static const uint8_t opcodes[] = { 0xfau, 0xfbu };
    uint8_t opcode;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        uint8_t pass;

        for (pass = 0u; pass != 2u; ++pass) {
            cli_sti_machine state;
            core_machine_cpu_diagnostic diagnostic;
            t_cpu after;
            const uint32_t flags = VCPU_EFLAGS_VM | VCPU_EFLAGS_CF |
                (pass ? 0u : VCPU_EFLAGS_IOPL);
            const uint32_t expected = pass ? flags :
                (opcodes[opcode] == 0xfau ? flags & ~VCPU_EFLAGS_IF :
                    flags | VCPU_EFLAGS_IF);
            C_INT failed = !cli_sti_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

            if (!failed) {
                failed |= !cli_sti_run_vm86(&state, &opcodes[opcode], 1u,
                    flags, pass, &after, &diagnostic);
                if (pass) {
                    failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                        diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_GP) ||
                        after.data.eip != 0u || after.data.eflags != flags;
                } else {
                    failed |= diagnostic.first_fault.valid || after.data.eip != 1u ||
                        after.data.eflags != expected;
                }
            }
            core_machine_destroy(state.machine);
            if (failed)
                return 0;
        }
    }
    return 1;
}

C_INT main(C_VOID)
{
    if (!cli_sti_test_real_forms()) {
        STD_PRINTF("CLI-STI stage=real failed\n");
        return 1;
    }
    if (!cli_sti_test_irq_shadow()) {
        STD_PRINTF("CLI-STI stage=pic failed\n");
        return 1;
    }
    if (!cli_sti_test_protected_success())
        return 1;
    if (!cli_sti_test_protected_reject())
        return 1;
    if (!cli_sti_test_vm86())
        return 1;
    STD_PRINTF("M5:T316:S22:CLI-STI:OK\n");
    return 0;
}
