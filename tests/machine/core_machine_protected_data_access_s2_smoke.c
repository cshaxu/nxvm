#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/cpu_instructions.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define PDA_GDT_POINTER 0x0100u
#define PDA_GDT_ADDRESS 0x0300u
#define PDA_CODE_ADDRESS 0x2000u
#define PDA_DATA_ADDRESS 0x3000u
#define PDA_STACK_ADDRESS 0x4000u
#define PDA_ES_ADDRESS 0x5000u

typedef struct pda_machine {
    core_machine *machine;
} pda_machine;

static C_VOID pda_reset(C_VOID *opaque)
{
    pda_machine *state = (pda_machine *)opaque;

    if (state != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider pda_provider = {
    pda_reset, STD_NULL
};

static C_INT pda_write(pda_machine *state, type_unsigned_32 address,
    const C_VOID *bytes, STD_SIZE_T byte_count)
{
    return state != STD_NULL && state->machine != STD_NULL &&
        core_machine_memory_write(state->machine, address, bytes, byte_count) ==
            TYPE_STATUS_OK;
}

static C_INT pda_read(pda_machine *state, type_unsigned_32 address,
    C_VOID *bytes, STD_SIZE_T byte_count)
{
    return state != STD_NULL && state->machine != STD_NULL &&
        core_machine_memory_read_physical(&state->machine->executor_memory,
            address, (type_virtual_address)bytes, byte_count) == TYPE_STATUS_OK;
}

static C_INT pda_prepare(pda_machine *state, core_machine_cpu_profile profile)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    static const type_unsigned_8 gdt_pointer[] = { 0x1fu,0u,0u,0x03u,0u,0u };
    static const type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0,0,
        0xffu,0xffu,0,0x40u,0,0x92u,0,0
    };
    static const type_unsigned_8 bootstrap[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,
        0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,0x8eu,0xc0u,
        0xb8u,0x18u,0x00u,0x8eu,0xd0u,0xbcu,0x00u,0x80u,
        0xeau,0x00u,0x00u,0x08u,0x00u
    };
    static const type_unsigned_8 halt[] = { 0xf4u };
    core_machine_run_result result;

    if (state == STD_NULL) return 0;
    STD_MEMSET(state, 0, sizeof(*state));
    if (core_machine_create(&config, &state->machine) != TYPE_STATUS_OK ||
        !test_core_machine_fixture_bind_freeze_reset(state->machine,
            &pda_provider, state) ||
        !pda_write(state, PDA_GDT_POINTER, gdt_pointer, sizeof(gdt_pointer)) ||
        !pda_write(state, PDA_GDT_ADDRESS, gdt, sizeof(gdt)) ||
        !pda_write(state, 0u, bootstrap, sizeof(bootstrap)) ||
        !pda_write(state, PDA_CODE_ADDRESS, halt, sizeof(halt)) ||
        core_machine_run(state->machine, (core_machine_run_budget){96u,0u},
            &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
        core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 0;
    }
    return 1;
}

static C_INT pda_same_cpu(const t_cpu *before, const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi &&
        before->data.eip == after->data.eip &&
        before->data.eflags == after->data.eflags &&
        STD_MEMCMP(&before->data.es, &after->data.es, sizeof(before->data.es)) == 0 &&
        STD_MEMCMP(&before->data.cs, &after->data.cs, sizeof(before->data.cs)) == 0 &&
        STD_MEMCMP(&before->data.ss, &after->data.ss, sizeof(before->data.ss)) == 0 &&
        STD_MEMCMP(&before->data.ds, &after->data.ds, sizeof(before->data.ds)) == 0 &&
        STD_MEMCMP(&before->data.fs, &after->data.fs, sizeof(before->data.fs)) == 0 &&
        STD_MEMCMP(&before->data.gs, &after->data.gs, sizeof(before->data.gs)) == 0;
}

static C_INT pda_nonresult_state_same(const t_cpu *before, const t_cpu *after,
    type_bool eax_changes, type_bool edi_changes)
{
    return (eax_changes || before->data.eax == after->data.eax) &&
        before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi &&
        (edi_changes || before->data.edi == after->data.edi) &&
        before->data.eflags == after->data.eflags &&
        STD_MEMCMP(&before->data.es, &after->data.es, sizeof(before->data.es)) == 0 &&
        STD_MEMCMP(&before->data.cs, &after->data.cs, sizeof(before->data.cs)) == 0 &&
        STD_MEMCMP(&before->data.ss, &after->data.ss, sizeof(before->data.ss)) == 0 &&
        STD_MEMCMP(&before->data.ds, &after->data.ds, sizeof(before->data.ds)) == 0 &&
        STD_MEMCMP(&before->data.fs, &after->data.fs, sizeof(before->data.fs)) == 0 &&
        STD_MEMCMP(&before->data.gs, &after->data.gs, sizeof(before->data.gs)) == 0;
}

static C_INT pda_start(pda_machine *state, const type_unsigned_8 *code,
    STD_SIZE_T code_size, t_cpu *out_before)
{
    if (!pda_write(state, PDA_CODE_ADDRESS, code, code_size)) return 0;
    test_core_machine_fixture_resume_after_halt_at(state->machine, 0u);
    *out_before = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return 1;
}

static C_INT pda_run_halt(pda_machine *state, t_cpu *out_after)
{
    core_machine_run_result result;

    if (core_machine_run(state->machine, (core_machine_run_budget){32u,0u},
        &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) return 0;
    *out_after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return 1;
}

static C_INT pda_expect_fault(pda_machine *state, type_unsigned_32 expected_mask,
    t_cpu *out_after)
{
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;

    if (core_machine_run(state->machine, (core_machine_run_budget){16u,0u},
        &result) != TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT ||
        core_machine_get_cpu_diagnostic(state->machine, &diagnostic) != TYPE_STATUS_OK ||
        !diagnostic.first_fault.valid ||
        diagnostic.first_fault.exception_mask != expected_mask) return 0;
    *out_after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return 1;
}

static C_INT pda_test_default_access(core_machine_cpu_profile profile)
{
    static const type_unsigned_8 ds_read[] = { 0x8au,0x06u,0x20u,0,0xf4u };
    static const type_unsigned_8 ds_write[] = { 0x88u,0x06u,0x21u,0,0xf4u };
    static const type_unsigned_8 ss_read[] = { 0x8au,0x46u,0,0xf4u };
    static const type_unsigned_8 stos[] = { 0xaau,0xf4u };
    pda_machine state;
    t_cpu before;
    t_cpu after;
    type_unsigned_8 byte = 0x5au;
    C_INT failed = !pda_prepare(&state, profile);

    if (!failed) {
        state.machine->executor_cpu.data.eax = 0xaabbcc44u;
        failed |= !pda_write(&state, PDA_DATA_ADDRESS + 0x20u, &byte, sizeof(byte)) ||
            !pda_start(&state, ds_read, sizeof(ds_read), &before) ||
            !pda_run_halt(&state, &after) || after.data.eax != 0xaabbcc5au ||
            after.data.eip != sizeof(ds_read) ||
            !pda_nonresult_state_same(&before, &after, TYPE_TRUE, TYPE_FALSE);
    }
    if (!failed) {
        byte = 0u;
        state.machine->executor_cpu.data.eax = 0x112233a5u;
        failed |= !pda_write(&state, PDA_DATA_ADDRESS + 0x21u, &byte, sizeof(byte)) ||
            !pda_start(&state, ds_write, sizeof(ds_write), &before) ||
            !pda_run_halt(&state, &after) || !pda_read(&state,
                PDA_DATA_ADDRESS + 0x21u, &byte, sizeof(byte)) || byte != 0xa5u ||
            after.data.eip != sizeof(ds_write) ||
            !pda_nonresult_state_same(&before, &after, TYPE_FALSE, TYPE_FALSE);
    }
    if (!failed) {
        byte = 0x6bu;
        state.machine->executor_cpu.data.ebp = 0x30u;
        state.machine->executor_cpu.data.eax = 0x11223344u;
        failed |= !pda_write(&state, PDA_STACK_ADDRESS + 0x30u, &byte, sizeof(byte)) ||
            !pda_start(&state, ss_read, sizeof(ss_read), &before) ||
            !pda_run_halt(&state, &after) || after.data.eax != 0x1122336bu ||
            after.data.eip != sizeof(ss_read) ||
            !pda_nonresult_state_same(&before, &after, TYPE_TRUE, TYPE_FALSE);
    }
    if (!failed) {
        byte = 0u;
        state.machine->executor_cpu.data.es.base = PDA_ES_ADDRESS;
        state.machine->executor_cpu.data.edi = 0x40u;
        state.machine->executor_cpu.data.eax = 0x5566779cu;
        failed |= !pda_write(&state, PDA_ES_ADDRESS + 0x40u, &byte, sizeof(byte)) ||
            !pda_start(&state, stos, sizeof(stos), &before) ||
            !pda_run_halt(&state, &after) || !pda_read(&state,
                PDA_ES_ADDRESS + 0x40u, &byte, sizeof(byte)) || byte != 0x9cu ||
            after.data.edi != 0x41u ||
            !pda_nonresult_state_same(&before, &after, TYPE_FALSE, TYPE_TRUE);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT pda_test_386_attributes(C_VOID)
{
    static const type_unsigned_8 operand32[] = { 0x66u,0x8bu,0x06u,0x20u,0,0xf4u };
    static const type_unsigned_8 address32_ss[] = { 0x67u,0x8au,0x45u,0,0xf4u };
    static const type_unsigned_8 lock_read[] = { 0xf0u,0x8au,0x06u,0x20u,0 };
    static const type_unsigned_8 legacy_prefix[] = { 0x66u,0x8au,0x06u,0x20u,0 };
    pda_machine state;
    t_cpu before;
    t_cpu after;
    type_unsigned_32 value = 0x12345678u;
    type_unsigned_8 byte = 0x4du;
    C_INT failed = !pda_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        failed |= !pda_write(&state, PDA_DATA_ADDRESS + 0x20u, &value, sizeof(value)) ||
            !pda_start(&state, operand32, sizeof(operand32), &before) ||
            !pda_run_halt(&state, &after) || after.data.eax != value ||
            after.data.eip != sizeof(operand32) ||
            !pda_nonresult_state_same(&before, &after, TYPE_TRUE, TYPE_FALSE);
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;
    failed = !pda_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    if (!failed) {
        state.machine->executor_cpu.data.ebp = 0x30u;
        state.machine->executor_cpu.data.eax = 0xaabbcc00u;
        failed |= !pda_write(&state, PDA_STACK_ADDRESS + 0x30u, &byte, sizeof(byte)) ||
            !pda_start(&state, address32_ss, sizeof(address32_ss), &before) ||
            !pda_run_halt(&state, &after) || after.data.eax != 0xaabbcc4du ||
            after.data.eip != sizeof(address32_ss) ||
            !pda_nonresult_state_same(&before, &after, TYPE_TRUE, TYPE_FALSE);
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;
    failed = !pda_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    if (!failed) {
        failed |= !pda_start(&state, lock_read, sizeof(lock_read), &before) ||
            !pda_expect_fault(&state, VCPUINS_EXCEPT_UD, &after) ||
            !pda_same_cpu(&before, &after);
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;
    failed = !pda_prepare(&state, CORE_MACHINE_CPU_PROFILE_80286);
    if (!failed) {
        failed |= !pda_start(&state, legacy_prefix, sizeof(legacy_prefix), &before) ||
            !pda_expect_fault(&state, VCPUINS_EXCEPT_UD, &after) ||
            !pda_same_cpu(&before, &after);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT pda_test_fault_atomicity(C_VOID)
{
    static const type_unsigned_8 read_ds[] = { 0x8au,0x06u,0x20u,0 };
    static const type_unsigned_8 read_ds_expand_down_invalid[] = {
        0x8au,0x06u,0x1fu,0
    };
    static const type_unsigned_8 write_ds[] = { 0x88u,0x06u,0x20u,0 };
    static const type_unsigned_8 read_ss[] = { 0x8au,0x46u,0,0 };
    static const type_unsigned_8 stos[] = { 0xaau };
    pda_machine state;
    t_cpu before;
    t_cpu after;
    type_unsigned_8 sentinel = 0x5au;
    C_INT failed = 0;

    if (!pda_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386)) return 0;
    state.machine->executor_cpu.data.ds.selector = 0u;
    state.machine->executor_cpu.data.ds.flagValid = TYPE_FALSE;
    failed |= !pda_start(&state, read_ds, sizeof(read_ds), &before) ||
        !pda_expect_fault(&state, VCPUINS_EXCEPT_DF, &after) ||
        !pda_same_cpu(&before, &after);
    core_machine_destroy(state.machine);
    if (failed || !pda_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386)) return 0;
    state.machine->executor_cpu.data.ds.seg.data.writable = TYPE_FALSE;
    failed |= !pda_write(&state, PDA_DATA_ADDRESS + 0x20u, &sentinel, sizeof(sentinel)) ||
        !pda_start(&state, write_ds, sizeof(write_ds), &before) ||
        !pda_expect_fault(&state, VCPUINS_EXCEPT_DF, &after) ||
        !pda_same_cpu(&before, &after) ||
        !pda_read(&state, PDA_DATA_ADDRESS + 0x20u, &sentinel, sizeof(sentinel)) ||
        sentinel != 0x5au;
    core_machine_destroy(state.machine);
    if (failed || !pda_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386)) return 0;
    state.machine->executor_cpu.data.ds.limit = 0x1fu;
    failed |= !pda_start(&state, read_ds, sizeof(read_ds), &before) ||
        !pda_expect_fault(&state, VCPUINS_EXCEPT_DF, &after) ||
        !pda_same_cpu(&before, &after);
    core_machine_destroy(state.machine);
    if (failed || !pda_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386)) return 0;
    state.machine->executor_cpu.data.ds.limit = 0x1fu;
    state.machine->executor_cpu.data.ds.seg.data.expdown = TYPE_TRUE;
    state.machine->executor_cpu.data.ds.seg.data.big = TYPE_FALSE;
    failed |= !pda_start(&state, read_ds_expand_down_invalid,
        sizeof(read_ds_expand_down_invalid), &before) ||
        !pda_expect_fault(&state, VCPUINS_EXCEPT_DF, &after) ||
        !pda_same_cpu(&before, &after);
    core_machine_destroy(state.machine);
    if (failed || !pda_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386)) return 0;
    state.machine->executor_cpu.data.ss.limit = 0x1fu;
    state.machine->executor_cpu.data.ebp = 0x20u;
    failed |= !pda_start(&state, read_ss, sizeof(read_ss), &before) ||
        !pda_expect_fault(&state, VCPUINS_EXCEPT_DF, &after) ||
        !pda_same_cpu(&before, &after);
    core_machine_destroy(state.machine);
    if (failed || !pda_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386)) return 0;
    state.machine->executor_cpu.data.es.limit = 0x1fu;
    state.machine->executor_cpu.data.edi = 0x20u;
    failed |= !pda_start(&state, stos, sizeof(stos), &before) ||
        !pda_expect_fault(&state, VCPUINS_EXCEPT_DF, &after) ||
        !pda_same_cpu(&before, &after);
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT pda_test_expand_down_success(C_VOID)
{
    static const type_unsigned_8 read_ds[] = { 0x8au,0x06u,0x20u,0,0xf4u };
    pda_machine state;
    t_cpu before;
    t_cpu after;
    type_unsigned_8 byte = 0x3cu;
    C_INT failed = !pda_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        state.machine->executor_cpu.data.ds.limit = 0x1fu;
        state.machine->executor_cpu.data.ds.seg.data.expdown = TYPE_TRUE;
        state.machine->executor_cpu.data.ds.seg.data.big = TYPE_FALSE;
        state.machine->executor_cpu.data.eax = 0xaabbcc00u;
        failed |= !pda_write(&state, PDA_DATA_ADDRESS + 0x20u, &byte, sizeof(byte)) ||
            !pda_start(&state, read_ds, sizeof(read_ds), &before) ||
            !pda_run_halt(&state, &after) || after.data.eax != 0xaabbcc3cu ||
            !pda_nonresult_state_same(&before, &after, TYPE_TRUE, TYPE_FALSE);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT pda_test_irq_no_shadow(C_VOID)
{
    static const type_unsigned_8 code[] = {
        0x8au,0x06u,0x20u,0,0x90u
    };
    static const type_unsigned_8 handler[] = { 0xf4u };
    static const type_unsigned_8 gate[] = { 0,1,8,0,0,0x86u,0,0 };
    pda_machine state;
    core_machine_pic_irq_source source;
    core_machine_run_result result;
    t_cpu after;
    type_unsigned_8 byte = 0x5au;
    type_unsigned_16 frame_ip = 0u;
    C_INT failed = !pda_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        failed |= !pda_write(&state, PDA_DATA_ADDRESS + 0x20u, &byte, sizeof(byte)) ||
            !pda_write(&state, PDA_CODE_ADDRESS + 0x100u, handler,
                sizeof(handler)) || !pda_write(&state, 0x0600u + 0x100u, gate,
                sizeof(gate)) || !pda_write(&state, PDA_CODE_ADDRESS, code,
                sizeof(code));
        state.machine->executor_cpu.data.idtr.flagValid = TYPE_TRUE;
        state.machine->executor_cpu.data.idtr.sregtype = SREG_IDTR;
        state.machine->executor_cpu.data.idtr.base = 0x0600u;
        state.machine->executor_cpu.data.idtr.limit = 0x0107u;
        state.machine->executor_cpu.data.eax = 0xaabbcc44u;
        state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_IF;
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        STD_MEMSET(&source, 0, sizeof(source));
        state.machine->shared_pic_master.data.icw2 = 0x20u;
        core_machine_pic_irq_source_bind(&source, &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&source);
        core_machine_pic_irq_source_deassert(&source);
        failed |= core_machine_run(state.machine, (core_machine_run_budget){2u,0u},
            &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !pda_read(&state, PDA_STACK_ADDRESS +
            (type_unsigned_16)after.data.esp, &frame_ip, sizeof(frame_ip)) ||
            after.data.eax != 0xaabbcc5au || after.data.cs.selector != 0x08u ||
            after.data.eip != 0x101u || frame_ip != 4u ||
            after.data.esp != 0x7ffau ||
            !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,
                VPIC_ISR_IRQ(0u)) || TYPE_GET_BIT(
                state.machine->shared_pic_master.data.irr, VPIC_IRR_IRQ(0u));
    }
    core_machine_destroy(state.machine);
    return !failed;
}

C_INT main(C_VOID)
{
    if (!pda_test_default_access(CORE_MACHINE_CPU_PROFILE_80286) ||
        !pda_test_default_access(CORE_MACHINE_CPU_PROFILE_80386)) {
        STD_PRINTF("PDA stage=default\n");
        return 1;
    }
    if (!pda_test_386_attributes()) {
        STD_PRINTF("PDA stage=attributes\n");
        return 1;
    }
    if (!pda_test_fault_atomicity() || !pda_test_expand_down_success()) {
        STD_PRINTF("PDA stage=boundaries\n");
        return 1;
    }
    if (!pda_test_irq_no_shadow()) {
        STD_PRINTF("PDA stage=irq\n");
        return 1;
    }
    STD_PRINTF("M5:T323:S2:PROTECTED-DATA-ACCESS:OK\n");
    return 0;
}
