#include "type.h"
#include "core/machine/cpu.h"
#include "core/machine/pic.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct xchg_machine {
    core_machine *machine;
} xchg_machine;

static C_VOID xchg_reset(C_VOID *opaque)
{
    xchg_machine *state = (xchg_machine *)opaque;

    if (state != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider xchg_provider = {
    xchg_reset, STD_NULL
};

static C_INT xchg_prepare(core_machine_cpu_profile profile, xchg_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    STD_MEMSET(state, 0, sizeof(*state));
return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &xchg_provider, state, &state->machine);
}

static C_INT xchg_run(xchg_machine *state, const type_unsigned_8 *code, type_unsigned_8 bytes,
    t_cpu *after, core_machine_cpu_diagnostic *diagnostic, type_status *status)
{
    core_machine_run_result result;

    if (core_machine_memory_write(state->machine, 0u, code, bytes) != TYPE_STATUS_OK)
        return 0;
    *status = core_machine_run(state->machine,
        (core_machine_run_budget){ 1u, 0u }, &result);
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) == TYPE_STATUS_OK;
}

typedef struct xchg_vector {
    const type_unsigned_8 *code;
    type_unsigned_8 bytes;
    type_unsigned_8 memory_width;
    type_unsigned_32 memory_address;
    type_unsigned_32 memory_before;
    type_unsigned_32 memory_after;
    type_unsigned_32 eax_after;
    type_unsigned_32 ecx_after;
} xchg_vector;

static C_INT xchg_test_real(C_VOID)
{
    static const type_unsigned_8 r8[] = { 0x86u, 0xc1u };
    static const type_unsigned_8 m8[] = { 0x86u, 0x06u, 0x00u, 0x10u };
    static const type_unsigned_8 r16[] = { 0x87u, 0xc1u };
    static const type_unsigned_8 m16[] = { 0x87u, 0x06u, 0x00u, 0x10u };
    static const type_unsigned_8 r32[] = { 0x66u, 0x87u, 0xc1u };
    static const type_unsigned_8 m32[] = { 0x67u, 0x66u, 0x87u, 0x46u, 0x10u };
    static const xchg_vector vectors[] = {
        { r8, sizeof(r8), 0u, 0u, 0u, 0u, 0xaabb3388u, 0x55667744u },
        { m8, sizeof(m8), 1u, 0x1000u, 0x22u, 0x44u, 0xaabb3322u, 0x55667788u },
        { r16, sizeof(r16), 0u, 0u, 0u, 0u, 0xaabb7788u, 0x55663344u },
        { m16, sizeof(m16), 2u, 0x1000u, 0x7788u, 0x3344u, 0xaabb7788u, 0x55667788u },
        { r32, sizeof(r32), 0u, 0u, 0u, 0u, 0x55667788u, 0xaabb3344u },
        { m32, sizeof(m32), 4u, 0x1010u, 0x11223344u, 0xaabb3344u, 0x11223344u, 0x55667788u }
    };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(vectors) / sizeof(vectors[0]); ++form)
    {
        xchg_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_status status;
        type_unsigned_32 memory_after = 0u;
        C_INT failed = !xchg_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            failed |= !test_core_machine_fixture_prepare_real_mode_execution(state.machine, 0u);
            state.machine->executor_cpu.data.eax = 0xaabb3344u;
            state.machine->executor_cpu.data.ecx = 0x55667788u;
            state.machine->executor_cpu.data.esi = 0x00001000u;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
            if (vectors[form].memory_width != 0u)
                failed |= core_machine_memory_write(state.machine,
                    vectors[form].memory_address, &vectors[form].memory_before,
                    vectors[form].memory_width) != TYPE_STATUS_OK;
            failed |= !xchg_run(&state, vectors[form].code, vectors[form].bytes,
                    &after, &diagnostic, &status) ||
                status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
                after.data.eip != vectors[form].bytes ||
                after.data.eax != vectors[form].eax_after ||
                after.data.ecx != vectors[form].ecx_after ||
                after.data.eflags != (VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF);
            if (vectors[form].memory_width != 0u)
                failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                    vectors[form].memory_address, (type_virtual_address)&memory_after,
                    vectors[form].memory_width) != TYPE_STATUS_OK ||
                    memory_after != vectors[form].memory_after;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT xchg_test_profiles_and_lock(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = { CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286 };
    static const type_unsigned_8 prefixes[][4] = { {0x66u,0x87u,0xc1u,0u},
        {0x67u,0x87u,0xc1u,0u}, {0x66u,0x67u,0x87u,0xc1u} };
    type_unsigned_8 profile, form;
    for (profile = 0u; profile != 3u; ++profile)
    {
        for (form = 0u; form != 3u; ++form)
        {
        xchg_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_status status;
        C_INT failed = !xchg_prepare(profiles[profile], &state);
        if (!failed) {
            failed |= !test_core_machine_fixture_prepare_real_mode_execution(state.machine,0u);
            state.machine->executor_cpu.data.eax=0x11223344u;
            state.machine->executor_cpu.data.eflags=VCPU_EFLAGS_CF;
            failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
                state.machine);
            before=test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !xchg_run(&state,prefixes[form],form==2u?4u:3u,&after,&diagnostic,&status) ||
                status!=TYPE_STATUS_FAULT || !diagnostic.first_fault.valid || !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,VCPUINS_EXCEPT_UD) ||
                after.data.eip!=before.data.eip || after.data.eax!=before.data.eax || after.data.eflags!=before.data.eflags;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
        }
    }
    return 1;
}

static C_INT xchg_test_legacy_default16(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const type_unsigned_8 register_code[] = { 0x87u, 0xc1u };
    static const type_unsigned_8 memory_code[] = { 0x87u, 0x06u, 0x00u, 0x10u };
    type_unsigned_8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
            ++profile) {
        xchg_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_status status;
        type_unsigned_16 memory = 0x7788u;
        C_INT failed = !xchg_prepare(profiles[profile], &state);

        if (!failed) {
            failed |= !test_core_machine_fixture_prepare_real_mode_execution(
                state.machine, 0u);
            state.machine->executor_cpu.data.eax = 0xaabb3344u;
            state.machine->executor_cpu.data.ecx = 0x55667788u;
            state.machine->executor_cpu.data.edx = 0x12345678u;
            state.machine->executor_cpu.data.eflags =
                VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
            failed |= !xchg_run(&state, register_code, sizeof(register_code),
                    &after, &diagnostic, &status) || status != TYPE_STATUS_OK ||
                diagnostic.first_fault.valid || after.data.eip != 2u ||
                after.data.eax != 0xaabb7788u ||
                after.data.ecx != 0x55663344u ||
                after.data.edx != 0x12345678u ||
                after.data.eflags != (VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF);
            state.machine->executor_cpu.data.eip = 0u;
            state.machine->executor_cpu.data.eax = 0xaabb3344u;
            state.machine->executor_cpu.data.ecx = 0x55667788u;
            failed |= core_machine_memory_write(state.machine, 0x1000u, &memory,
                    sizeof(memory)) != TYPE_STATUS_OK ||
                !xchg_run(&state, memory_code, sizeof(memory_code), &after,
                    &diagnostic, &status) || status != TYPE_STATUS_OK ||
                diagnostic.first_fault.valid || after.data.eip != 4u ||
                after.data.eax != 0xaabb7788u ||
                after.data.ecx != 0x55667788u ||
                after.data.edx != 0x12345678u ||
                after.data.eflags != (VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT xchg_prepare_protected(xchg_machine *state)
{
    static const type_unsigned_8 pointer[] = { 0x1fu, 0, 0, 0x03u, 0, 0 };
    static const type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0, 0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0,0,0x92u,0,0, 0xffu,0xffu,0,0x40u,0,0x92u,0,0
    };
    static const type_unsigned_8 bootstrap[] = {
        0x0fu,0x01u,0x16u,0,0x01u,0xb8u,1,0,0x0fu,0x01u,0xf0u,0xb8u,
        0x10u,0,0x8eu,0xd8u,0x8eu,0xc0u,0xb8u,0x18u,0,0x8eu,0xd0u,
        0xbcu,0,0x80u,0xeau,0,0,0x08u,0
    };
    static const type_unsigned_8 hlt[] = { 0xf4u };
    core_machine_run_result result;

    return xchg_prepare(CORE_MACHINE_CPU_PROFILE_80386, state) &&
        core_machine_memory_write(state->machine,0x0100u,pointer,sizeof(pointer))==TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine,0x0300u,gdt,sizeof(gdt))==TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine,0u,bootstrap,sizeof(bootstrap))==TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine,0x2000u,hlt,sizeof(hlt))==TYPE_STATUS_OK &&
        core_machine_run(state->machine,(core_machine_run_budget){96u,0u},&result)==TYPE_STATUS_OK &&
        result.reason==CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
}

static C_INT xchg_test_write_fault_atomicity(C_VOID)
{
    static const type_unsigned_8 code[] = { 0x87u,0x06u,0x00u,0x10u };
    xchg_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_unsigned_16 memory_before = 0x7788u;
    type_unsigned_16 memory_after = 0u;
    C_INT failed = !xchg_prepare_protected(&state);

    if (!failed)
    {
        state.machine->executor_cpu.data.eax = 0xaabb3344u;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
        state.machine->executor_cpu.data.ds.seg.data.writable = TYPE_FALSE;
        failed |= core_machine_memory_write(state.machine,0x1000u,&memory_before,
            sizeof(memory_before)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine,0x2000u,code,sizeof(code)) != TYPE_STATUS_OK;
        test_core_machine_fixture_resume_after_halt_at(state.machine,0u);
        before=test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_run(state.machine,(core_machine_run_budget){1u,0u},&result) != TYPE_STATUS_FAULT ||
            core_machine_get_cpu_diagnostic(state.machine,&diagnostic) != TYPE_STATUS_OK;
        after=test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_memory_read_physical(&state.machine->executor_memory,0x1000u,
            (type_virtual_address)&memory_after,sizeof(memory_after)) != TYPE_STATUS_OK;
        failed |= after.data.eax != before.data.eax || after.data.eflags != before.data.eflags ||
            after.data.eip != before.data.eip || memory_after != memory_before;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT xchg_test_read_fault_atomicity(C_VOID)
{
    static const type_unsigned_8 codes[][5] = { {0x86u,0x06u,0x02u,0x10u,0u},
        {0x66u,0x87u,0x06u,0x02u,0x10u} };
    type_unsigned_8 form;
    for (form = 0u; form != 2u; ++form)
    {
        xchg_machine state;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        type_status status;
        t_cpu before;
        t_cpu after;
        type_unsigned_32 memory_before = 0x11223344u;
        type_unsigned_32 memory_after = 0u;
        C_INT failed = !xchg_prepare_protected(&state);
        if (!failed)
        {
            state.machine->executor_cpu.data.ds.limit = 0x1001u;
            state.machine->executor_cpu.data.eax = 0xaabb3344u;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
            failed |= core_machine_memory_write(state.machine,0x1002u,&memory_before,4u)!=TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine,0x2000u,codes[form],form?5u:4u)!=TYPE_STATUS_OK;
            test_core_machine_fixture_resume_after_halt_at(state.machine,0u);
            before=test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= (status=core_machine_run(state.machine,(core_machine_run_budget){1u,0u},&result))!=TYPE_STATUS_FAULT ||
                core_machine_get_cpu_diagnostic(state.machine,&diagnostic)!=TYPE_STATUS_OK;
            after=test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_memory_read_physical(&state.machine->executor_memory,0x1002u,
                (type_virtual_address)&memory_after,4u)!=TYPE_STATUS_OK || after.data.eax!=before.data.eax ||
                after.data.eip!=before.data.eip || after.data.eflags!=before.data.eflags || memory_after!=memory_before;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT xchg_test_lock(C_VOID)
{
    static const type_unsigned_8 plain_code[] = {0x87u,0x06u,0x00u,0x10u};
    static const type_unsigned_8 memory_code[] = {0xf0u,0x87u,0x06u,0x00u,0x10u};
    static const type_unsigned_8 register_code[] = {0xf0u,0x87u,0xc1u};
    xchg_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    type_status status;
    type_unsigned_16 memory = 0x7788u;
    C_INT failed = !xchg_prepare(CORE_MACHINE_CPU_PROFILE_80386,&state);
    if (!failed)
    {
        failed |= !test_core_machine_fixture_prepare_real_mode_execution(state.machine,0u);
        state.machine->executor_cpu.data.eax = 0xaabb3344u;
        failed |= core_machine_memory_write(state.machine,0x1000u,&memory,2u)!=TYPE_STATUS_OK ||
            !xchg_run(&state,plain_code,sizeof(plain_code),&after,&diagnostic,&status);
        state.machine->executor_cpu.data.eip = 0u;
        state.machine->executor_cpu.data.eax = 0xaabb3344u;
        memory = 0x7788u;
        failed |= core_machine_memory_write(state.machine,0x1000u,&memory,2u)!=TYPE_STATUS_OK ||
            !xchg_run(&state,memory_code,sizeof(memory_code),&after,&diagnostic,&status) || status!=TYPE_STATUS_OK ||
            after.data.eax!=0xaabb7788u;
        state.machine->executor_cpu.data.eip = 0u;
        failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
            state.machine);
        before=test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !xchg_run(&state,register_code,sizeof(register_code),&after,&diagnostic,&status) ||
            status!=TYPE_STATUS_FAULT || !diagnostic.first_fault.valid || !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,VCPUINS_EXCEPT_UD) ||
            after.data.eip!=before.data.eip || after.data.eax!=before.data.eax || after.data.eflags!=before.data.eflags;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT xchg_test_irq_no_shadow(C_VOID)
{
    static const type_unsigned_8 code[] = { 0x87u, 0x06u, 0x00u, 0x10u, 0x90u };
    static const type_unsigned_8 hlt = 0xf4u;
    xchg_machine state;
    core_machine_pic_irq_source source;
    core_machine_run_result result;
    t_cpu after;
    type_unsigned_16 offset = 0x0100u;
    type_unsigned_16 segment = 0u;
    type_unsigned_16 frame = 0u;
    type_unsigned_16 memory = 0x7788u;
    C_INT failed = !xchg_prepare(CORE_MACHINE_CPU_PROFILE_80386,&state);
    if (!failed)
    {
        failed |= !test_core_machine_fixture_prepare_real_mode_execution(state.machine,0u) ||
            core_machine_memory_write(state.machine,0x1000u,&memory,2u)!=TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine,0u,code,sizeof(code))!=TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine,0x80u,&offset,2u)!=TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine,0x82u,&segment,2u)!=TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine,0x100u,&hlt,1u)!=TYPE_STATUS_OK;
        state.machine->executor_cpu.data.eax = 0xaabb3344u;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_IF;
        STD_MEMSET(&source,0,sizeof(source));
        state.machine->shared_pic_master.data.icw2 = 0x20u;
        core_machine_pic_irq_source_bind(&source,&state.machine->shared_pic_master,&state.machine->shared_pic_slave,0u);
        core_machine_pic_irq_source_assert(&source);
        core_machine_pic_irq_source_deassert(&source);
        failed |= core_machine_run(state.machine,(core_machine_run_budget){2u,0u},&result)!=TYPE_STATUS_OK ||
            result.reason!=CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        after=test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_memory_read_physical(&state.machine->executor_memory,after.data.ss.base+(type_unsigned_16)after.data.esp,
            (type_virtual_address)&frame,2u)!=TYPE_STATUS_OK || after.data.eip!=0x101u || frame!=4u ||
            !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,VPIC_ISR_IRQ(0u));
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static type_unsigned_32 *xchg_acc_target(t_cpu *cpu, type_unsigned_8 opcode)
{
    switch (opcode)
    {
    case 0x91u: return &cpu->data.ecx;
    case 0x92u: return &cpu->data.edx;
    case 0x93u: return &cpu->data.ebx;
    case 0x94u: return &cpu->data.esp;
    case 0x95u: return &cpu->data.ebp;
    case 0x96u: return &cpu->data.esi;
    case 0x97u: return &cpu->data.edi;
    default: return STD_NULL;
    }
}

static C_INT xchg_acc_state_equal(const t_cpu *before, const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi &&
        before->data.eflags == after->data.eflags &&
        before->data.eip == after->data.eip;
}

static C_INT xchg_acc_gpr_flags_equal(const t_cpu *before, const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi &&
        before->data.eflags == after->data.eflags;
}

static C_INT xchg_acc_nonparticipants_equal(const t_cpu *before,
    const t_cpu *after, type_unsigned_8 opcode)
{
    if (opcode == 0x90u)
        return xchg_acc_gpr_flags_equal(before, after);

    return before->data.eflags == after->data.eflags &&
        (opcode == 0x91u || before->data.ecx == after->data.ecx) &&
        (opcode == 0x92u || before->data.edx == after->data.edx) &&
        (opcode == 0x93u || before->data.ebx == after->data.ebx) &&
        (opcode == 0x94u || before->data.esp == after->data.esp) &&
        (opcode == 0x95u || before->data.ebp == after->data.ebp) &&
        (opcode == 0x96u || before->data.esi == after->data.esi) &&
        (opcode == 0x97u || before->data.edi == after->data.edi);
}

static C_INT xchg_test_accumulator(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    type_unsigned_8 profile;
    type_unsigned_8 opcode;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
            ++profile)
    {
        for (opcode = 0x90u; opcode <= 0x97u; ++opcode)
        {
            xchg_machine state;
            t_cpu before;
            t_cpu after;
            core_machine_cpu_diagnostic diagnostic;
            type_status status;
            type_unsigned_8 code[] = { opcode };
            type_unsigned_32 *target;
            C_INT failed;

            STD_MEMSET(&state, 0, sizeof(state));
            STD_MEMSET(&before, 0, sizeof(before));
            STD_MEMSET(&after, 0, sizeof(after));
            STD_MEMSET(&diagnostic, 0, sizeof(diagnostic));
            status = TYPE_STATUS_INVALID_ARGUMENT;
            failed = !xchg_prepare(profiles[profile], &state);

            if (!failed)
            {
                failed |= !test_core_machine_fixture_prepare_real_mode_execution(
                    state.machine, 0u);
                state.machine->executor_cpu.data.eax = 0xaabb3344u;
                state.machine->executor_cpu.data.ecx = 0x11112222u;
                state.machine->executor_cpu.data.edx = 0x33334444u;
                state.machine->executor_cpu.data.ebx = 0x55556666u;
                state.machine->executor_cpu.data.esp = 0x77778888u;
                state.machine->executor_cpu.data.ebp = 0x9999aaaau;
                state.machine->executor_cpu.data.esi = 0xbbbbccccu;
                state.machine->executor_cpu.data.edi = 0xddddeeeeu;
                state.machine->executor_cpu.data.eflags =
                    VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
                target = xchg_acc_target(&state.machine->executor_cpu, opcode);
                if (target != STD_NULL)
                    *target = 0x55667788u;
                before = test_core_machine_fixture_capture_cpu_after_run(
                    state.machine);
                failed |= !xchg_run(&state, code, sizeof(code), &after,
                    &diagnostic, &status) ||
                    status != TYPE_STATUS_OK ||
                    diagnostic.first_fault.valid ||
                    after.data.eip != 1u ||
                    after.data.eflags != before.data.eflags;
                failed |= !xchg_acc_nonparticipants_equal(&before, &after,
                    opcode);
                if (opcode != 0x90u)
                {
                    target = xchg_acc_target(&after, opcode);
                    failed |= after.data.eax != 0xaabb7788u ||
                        target == STD_NULL || *target != 0x55663344u;
                }
            }
            if (failed)
            {
                STD_PRINTF(
                    "XCHG acc default profile=%u opcode=%02x status=%d "
                    "fault=%08x before=%08x/%08x/%08x/%08x "
                    "after=%08x/%08x/%08x/%08x\n",
                    profile,
                    opcode,
                    status,
                    diagnostic.first_fault.exception_mask,
                    before.data.eip,
                    before.data.eax,
                    before.data.ecx,
                    before.data.eflags,
                    after.data.eip,
                    after.data.eax,
                    after.data.ecx,
                    after.data.eflags);
            }
            core_machine_destroy(state.machine);
            if (failed)
                return 0;
        }
    }
    return 1;
}

static C_INT xchg_test_accumulator_reject(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    type_unsigned_8 profile;
    type_unsigned_8 opcode;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
            ++profile)
    {
        for (opcode = 0x90u; opcode <= 0x97u; ++opcode)
        {
            xchg_machine state;
            t_cpu before;
            t_cpu after;
            core_machine_cpu_diagnostic diagnostic;
            type_status status;
            type_unsigned_8 code[] = { 0x66u, opcode };
            C_INT failed;

            STD_MEMSET(&state, 0, sizeof(state));
            STD_MEMSET(&before, 0, sizeof(before));
            STD_MEMSET(&after, 0, sizeof(after));
            STD_MEMSET(&diagnostic, 0, sizeof(diagnostic));
            status = TYPE_STATUS_INVALID_ARGUMENT;
            failed = !xchg_prepare(profiles[profile], &state);
            if (!failed)
            {
                failed |= !test_core_machine_fixture_prepare_real_mode_execution(
                    state.machine, 0u);
                state.machine->executor_cpu.data.eax = 0xaabb3344u;
                state.machine->executor_cpu.data.ecx = 0x55667788u;
                state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF;
                failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
                    state.machine);
                before = test_core_machine_fixture_capture_cpu_after_run(
                    state.machine);
                failed |= !xchg_run(&state, code, sizeof(code), &after,
                    &diagnostic, &status) ||
                    status != TYPE_STATUS_FAULT ||
                    !diagnostic.first_fault.valid ||
                    !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                        VCPUINS_EXCEPT_UD) ||
                    !xchg_acc_state_equal(&before, &after);
            }
            core_machine_destroy(state.machine);
            if (failed)
            {
                STD_PRINTF(
                    "XCHG acc 66 profile=%u opcode=%02x status=%d "
                    "fault=%08x before=%08x/%08x/%08x after=%08x/%08x/%08x\n",
                    profile,
                    opcode,
                    status,
                    diagnostic.first_fault.exception_mask,
                    before.data.eip,
                    before.data.eax,
                    before.data.eflags,
                    after.data.eip,
                    after.data.eax,
                    after.data.eflags);
                return 0;
            }
        }
    }
    return 1;
}

static C_INT xchg_test_accumulator_lock(C_VOID)
{
    type_unsigned_8 opcode;

    for (opcode = 0x90u; opcode <= 0x97u; ++opcode)
    {
        xchg_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_status status;
        type_unsigned_8 code[] = { 0xf0u, opcode };
        C_INT failed;

        STD_MEMSET(&state, 0, sizeof(state));
        STD_MEMSET(&before, 0, sizeof(before));
        STD_MEMSET(&after, 0, sizeof(after));
        STD_MEMSET(&diagnostic, 0, sizeof(diagnostic));
        status = TYPE_STATUS_INVALID_ARGUMENT;
        failed = !xchg_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);
        if (!failed)
        {
            failed |= !test_core_machine_fixture_prepare_real_mode_execution(
                state.machine, 0u);
            state.machine->executor_cpu.data.eax = 0xaabb3344u;
            state.machine->executor_cpu.data.ecx = 0x55667788u;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF;
            failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
                state.machine);
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !xchg_run(&state, code, sizeof(code), &after,
                &diagnostic, &status) ||
                status != TYPE_STATUS_FAULT ||
                !diagnostic.first_fault.valid ||
                !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                    VCPUINS_EXCEPT_UD) ||
                !xchg_acc_state_equal(&before, &after);
        }
        core_machine_destroy(state.machine);
        if (failed)
        {
            STD_PRINTF(
                "XCHG acc lock opcode=%02x status=%d fault=%08x "
                "before=%08x/%08x/%08x after=%08x/%08x/%08x\n",
                opcode,
                status,
                diagnostic.first_fault.exception_mask,
                before.data.eip,
                before.data.eax,
                before.data.eflags,
                after.data.eip,
                after.data.eax,
                after.data.eflags);
            return 0;
        }
    }
    return 1;
}

static C_INT xchg_test_accumulator_irq(C_VOID)
{
    static const type_unsigned_8 code[] = { 0x91u, 0x90u };
    static const type_unsigned_8 hlt = 0xf4u;
    xchg_machine state;
    core_machine_pic_irq_source source;
    core_machine_run_result result;
    t_cpu after;
    type_unsigned_16 offset = 0x0100u;
    type_unsigned_16 segment = 0u;
    type_unsigned_16 frame = 0u;
    C_INT failed;

    STD_MEMSET(&state, 0, sizeof(state));
    STD_MEMSET(&source, 0, sizeof(source));
    STD_MEMSET(&result, 0, sizeof(result));
    STD_MEMSET(&after, 0, sizeof(after));
    failed = !xchg_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed)
    {
        failed |= !test_core_machine_fixture_prepare_real_mode_execution(
            state.machine, 0u) ||
            core_machine_memory_write(state.machine, 0u, code,
                sizeof(code)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x80u, &offset,
                2u) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x82u, &segment,
                2u) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x100u, &hlt,
                1u) != TYPE_STATUS_OK;
        state.machine->executor_cpu.data.eax = 0xaabb3344u;
        state.machine->executor_cpu.data.ecx = 0x55667788u;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_IF;
        state.machine->shared_pic_master.data.icw2 = 0x20u;
        core_machine_pic_irq_source_bind(&source,
            &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&source);
        core_machine_pic_irq_source_deassert(&source);
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){ 2u, 0u }, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
            after.data.ss.base + (type_unsigned_16)after.data.esp,
            (type_virtual_address)&frame, 2u) != TYPE_STATUS_OK ||
            after.data.eip != 0x101u ||
            frame != 1u ||
            after.data.eax != 0xaabb7788u ||
            after.data.ecx != 0x55663344u ||
            !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,
                VPIC_ISR_IRQ(0u));
    }
    if (failed)
    {
        STD_PRINTF(
            "XCHG acc irq reason=%d eip=%08x frame=%04x eax=%08x "
            "ecx=%08x irr=%02x isr=%02x\n",
            result.reason,
            after.data.eip,
            frame,
            after.data.eax,
            after.data.ecx,
            state.machine->shared_pic_master.data.irr,
            state.machine->shared_pic_master.data.isr);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT xchg_test_accumulator_386_boundaries(C_VOID)
{
    type_unsigned_8 opcode;

    for (opcode = 0x90u; opcode <= 0x97u; ++opcode)
    {
        xchg_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_status status;
        type_unsigned_8 code[] = { 0x66u, opcode };
        type_unsigned_32 *target;
        C_INT failed;

        STD_MEMSET(&state, 0, sizeof(state));
        STD_MEMSET(&before, 0, sizeof(before));
        STD_MEMSET(&after, 0, sizeof(after));
        STD_MEMSET(&diagnostic, 0, sizeof(diagnostic));
        status = TYPE_STATUS_INVALID_ARGUMENT;
        failed = !xchg_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed)
        {
            failed |= !test_core_machine_fixture_prepare_real_mode_execution(
                state.machine, 0u);
            state.machine->executor_cpu.data.eax = 0xaabb3344u;
            state.machine->executor_cpu.data.ecx = 0x11112222u;
            state.machine->executor_cpu.data.edx = 0x33334444u;
            state.machine->executor_cpu.data.ebx = 0x55556666u;
            state.machine->executor_cpu.data.esp = 0x77778888u;
            state.machine->executor_cpu.data.ebp = 0x9999aaaau;
            state.machine->executor_cpu.data.esi = 0xbbbbccccu;
            state.machine->executor_cpu.data.edi = 0xddddeeeeu;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF;
            target = xchg_acc_target(&state.machine->executor_cpu, opcode);
            if (target != STD_NULL)
                *target = 0x55667788u;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !xchg_run(&state, code, sizeof(code), &after, &diagnostic,
                    &status) || status != TYPE_STATUS_OK ||
                diagnostic.first_fault.valid || after.data.eip != 2u ||
                after.data.eflags != before.data.eflags;
            failed |= !xchg_acc_nonparticipants_equal(&before, &after,
                opcode);
            if (opcode != 0x90u)
            {
                target = xchg_acc_target(&after, opcode);
                failed |= after.data.eax != 0x55667788u || target == STD_NULL ||
                    *target != 0xaabb3344u;
            }
        }
        if (failed)
        {
            STD_PRINTF(
                "XCHG acc 386 opcode=%02x status=%d fault=%08x "
                "before=%08x/%08x/%08x after=%08x/%08x/%08x\n",
                opcode,
                status,
                diagnostic.first_fault.exception_mask,
                before.data.eip,
                before.data.eax,
                before.data.eflags,
                after.data.eip,
                after.data.eax,
                after.data.eflags);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

C_INT main(C_VOID)
{
    if (!xchg_test_real())
    {
        STD_PRINTF("XCHG stage=real\n");
        return 1;
    }
    if (!xchg_test_profiles_and_lock())
    {
        STD_PRINTF("XCHG stage=profile\n");
        return 1;
    }
    if (!xchg_test_legacy_default16())
    {
        STD_PRINTF("XCHG stage=legacy\n");
        return 1;
    }
    if (!xchg_test_write_fault_atomicity())
    {
        STD_PRINTF("XCHG stage=write-fault\n");
        return 1;
    }
    if (!xchg_test_read_fault_atomicity())
    {
        STD_PRINTF("XCHG stage=read-fault\n");
        return 1;
    }
    if (!xchg_test_lock())
    {
        STD_PRINTF("XCHG stage=lock\n");
        return 1;
    }
    if (!xchg_test_irq_no_shadow())
    {
        STD_PRINTF("XCHG stage=irq\n");
        return 1;
    }
    if (!xchg_test_accumulator())
    {
        STD_PRINTF("XCHG acc stage=default\n");
        return 1;
    }
    if (!xchg_test_accumulator_386_boundaries())
    {
        STD_PRINTF("XCHG acc stage=386\n");
        return 1;
    }
    if (!xchg_test_accumulator_reject())
    {
        STD_PRINTF("XCHG acc stage=reject\n");
        return 1;
    }
    if (!xchg_test_accumulator_lock())
    {
        STD_PRINTF("XCHG acc stage=lock\n");
        return 1;
    }
    if (!xchg_test_accumulator_irq())
    {
        STD_PRINTF("XCHG acc stage=irq\n");
        return 1;
    }
    STD_PRINTF("M5:T316:S27:XCHG:OK\n");
    STD_PRINTF("M5:T316:S28:XCHG-ACC:OK\n");
    STD_PRINTF("M5:T401:S12:ACCUMULATOR-XCHG-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S46:XCHG-MODRM-PROFILES:OK\n");
    return 0;
}
