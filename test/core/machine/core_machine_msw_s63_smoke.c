#include "type.h"
#include "core/machine/cpu.h"
#include "core/machine/pic.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct msw_s63_machine { core_machine *machine; } msw_s63_machine;

static C_VOID msw_s63_reset(C_VOID *opaque)
{
    msw_s63_machine *state = (msw_s63_machine *)opaque;
    if (state != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider msw_s63_provider = {
    msw_s63_reset, STD_NULL
};

static C_INT msw_s63_prepare(msw_s63_machine *state, core_machine_cpu_profile profile)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    STD_MEMSET(state, 0, sizeof(*state));
return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &msw_s63_provider, state, &state->machine) &&
        test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u);
}

static C_VOID msw_s63_seed(msw_s63_machine *state)
{
    t_cpu *cpu = &state->machine->executor_cpu;
    cpu->data.eax = 0xdeadbeefu;
    cpu->data.ecx = 0x11223344u;
    cpu->data.edx = 0x55667788u;
    cpu->data.ebx = 0x99aabbccu;
    cpu->data.esp = 0x8000u;
    cpu->data.ebp = 0x120u;
    cpu->data.esi = 0x10u;
    cpu->data.edi = 0x20u;
    cpu->data.eflags = VCPU_EFLAGS_IF | VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_OF;
}

static C_INT msw_s63_sregs_same(const t_cpu *before, const t_cpu *after)
{
    return STD_MEMCMP(&before->data.cs, &after->data.cs, sizeof(before->data.cs)) == 0 &&
        STD_MEMCMP(&before->data.ds, &after->data.ds, sizeof(before->data.ds)) == 0 &&
        STD_MEMCMP(&before->data.es, &after->data.es, sizeof(before->data.es)) == 0 &&
        STD_MEMCMP(&before->data.ss, &after->data.ss, sizeof(before->data.ss)) == 0 &&
        STD_MEMCMP(&before->data.fs, &after->data.fs, sizeof(before->data.fs)) == 0 &&
        STD_MEMCMP(&before->data.gs, &after->data.gs, sizeof(before->data.gs)) == 0;
}

static C_INT msw_s63_gprs_same(const t_cpu *before, const t_cpu *after)
{
    return after->data.eax == before->data.eax && after->data.ecx == before->data.ecx &&
        after->data.edx == before->data.edx && after->data.ebx == before->data.ebx &&
        after->data.esp == before->data.esp && after->data.ebp == before->data.ebp &&
        after->data.esi == before->data.esi && after->data.edi == before->data.edi;
}

static C_INT msw_s63_run(msw_s63_machine *state, const type_unsigned_8 *code,
    type_unsigned_8 bytes, type_unsigned_32 cycles, type_status *status,
    core_machine_run_result *result, core_machine_cpu_diagnostic *diagnostic)
{
    if (core_machine_memory_write(state->machine, 0u, code, bytes) != TYPE_STATUS_OK)
        return 0;
    *status = core_machine_run(state->machine, (core_machine_run_budget){cycles,0u}, result);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) == TYPE_STATUS_OK;
}

static C_INT msw_s63_expect_ud(core_machine_cpu_profile profile, const type_unsigned_8 *code,
    type_unsigned_8 bytes)
{
    msw_s63_machine state; t_cpu before; t_cpu after; core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic; type_status status;
    C_INT failed = !msw_s63_prepare(&state, profile);
    if (!failed) {
        msw_s63_seed(&state); state.machine->executor_cpu.data.cr0 = 0x00a5000du;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !msw_s63_run(&state, code, bytes, 1u, &status, &result, &diagnostic) ||
            status != TYPE_STATUS_FAULT || !diagnostic.first_fault.valid || !TYPE_GET_BIT(
            diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD);
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= STD_MEMCMP(&before.data, &after.data, sizeof(before.data)) != 0;
    }
    core_machine_destroy(state.machine); return !failed;
}

static C_VOID msw_s63_enter_protected(msw_s63_machine *state, type_unsigned_8 cpl,
    C_INT vm86)
{
    t_cpu *cpu = &state->machine->executor_cpu;
    TYPE_SET_BIT(cpu->data.cr0, VCPU_CR0_PE);
    cpu->data.cs.selector = (type_unsigned_16)(0x0008u | cpl);
    cpu->data.cs.base = 0u; cpu->data.cs.limit = 0xffffu; cpu->data.cs.dpl = cpl;
    cpu->data.cs.flagValid = TYPE_TRUE; cpu->data.cs.sregtype = SREG_CODE;
    cpu->data.cs.seg.executable = TYPE_TRUE;
    if (vm86) TYPE_SET_BIT(cpu->data.eflags, VCPU_EFLAGS_VM);
}

static C_INT msw_s63_test_success(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386 };
    static const type_unsigned_8 smsw[] = {0x0fu,0x01u,0xe0u};
    static const type_unsigned_8 lmsw[] = {0x0fu,0x01u,0xf0u};
    type_unsigned_8 profile;
    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile) {
        msw_s63_machine state; t_cpu before; t_cpu after; core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic; type_status status = TYPE_STATUS_INVALID_STATE;
        C_INT failed = !msw_s63_prepare(&state, profiles[profile]);
        if (!failed) {
            msw_s63_seed(&state); state.machine->executor_cpu.data.cr0 = 0x00a5000du;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !msw_s63_run(&state, smsw, sizeof(smsw), 1u, &status, &result, &diagnostic);
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= status != TYPE_STATUS_OK || diagnostic.first_fault.valid || after.data.eip != 3u ||
                after.data.eax != 0xdead000du ||
                after.data.ecx != before.data.ecx || after.data.edx != before.data.edx ||
                after.data.ebx != before.data.ebx || after.data.esp != before.data.esp ||
                after.data.ebp != before.data.ebp || after.data.esi != before.data.esi ||
                after.data.edi != before.data.edi || after.data.eflags != before.data.eflags ||
                after.data.cr0 != before.data.cr0 || !msw_s63_sregs_same(&before, &after);
        }
        core_machine_destroy(state.machine); if (failed) return 0;
        failed = !msw_s63_prepare(&state, profiles[profile]);
        if (!failed) {
            msw_s63_seed(&state); state.machine->executor_cpu.data.eax = 0xdead000du;
            state.machine->executor_cpu.data.cr0 = 0x00a50000u;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !msw_s63_run(&state, lmsw, sizeof(lmsw), 1u, &status, &result, &diagnostic);
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= status != TYPE_STATUS_OK || diagnostic.first_fault.valid || after.data.eip != 3u ||
                !msw_s63_gprs_same(&before, &after) || after.data.eflags != before.data.eflags ||
                after.data.cr0 != 0x00a5000du || !msw_s63_sregs_same(&before, &after);
        }
        core_machine_destroy(state.machine); if (failed) return 0;
    }
    return 1;
}

static C_INT msw_s63_test_memory_and_protected(C_VOID)
{
    static const type_unsigned_8 smsw_memory[] = {0x0fu,0x01u,0x26u,0x00u,0x04u};
    static const type_unsigned_8 lmsw_memory[] = {0x0fu,0x01u,0x36u,0x00u,0x04u};
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    type_unsigned_8 profile;
    msw_s63_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    type_status status;
    type_unsigned_32 image;
    C_INT failed = !msw_s63_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        msw_s63_seed(&state);
        state.machine->executor_cpu.data.cr0 = 0x00a5000cu;
        image = 0x11223344u;
        failed |= core_machine_memory_write(state.machine, 0x0400u,
            &image, sizeof(image)) != TYPE_STATUS_OK;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !msw_s63_run(&state, smsw_memory, sizeof(smsw_memory), 1u,
            &status, &result, &diagnostic) || status != TYPE_STATUS_OK ||
            diagnostic.first_fault.valid;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
            0x0400u, TYPE_REFERENCE_OF(image), sizeof(image)) != TYPE_STATUS_OK ||
            image != 0x1122000cu || after.data.eip != 5u ||
            !msw_s63_gprs_same(&before, &after) ||
            after.data.eflags != before.data.eflags ||
            after.data.cr0 != before.data.cr0 ||
            !msw_s63_sregs_same(&before, &after);
    }
    core_machine_destroy(state.machine);
    if (failed)
        return 0;

    failed = !msw_s63_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    if (!failed) {
        msw_s63_seed(&state);
        state.machine->executor_cpu.data.cr0 = 0x00a50000u;
        image = 0x1122000cu;
        failed |= core_machine_memory_write(state.machine, 0x0400u,
            &image, sizeof(image)) != TYPE_STATUS_OK;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !msw_s63_run(&state, lmsw_memory, sizeof(lmsw_memory), 1u,
            &status, &result, &diagnostic) || status != TYPE_STATUS_OK ||
            diagnostic.first_fault.valid;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= after.data.eip != 5u || !msw_s63_gprs_same(&before, &after) ||
            after.data.eflags != before.data.eflags ||
            after.data.cr0 != 0x00a5000cu ||
            !msw_s63_sregs_same(&before, &after);
    }
    core_machine_destroy(state.machine);
    if (failed)
        return 0;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        static const type_unsigned_8 lmsw_register[] = {0x0fu,0x01u,0xf0u};

        failed = !msw_s63_prepare(&state, profiles[profile]);
        if (!failed) {
            msw_s63_seed(&state);
            msw_s63_enter_protected(&state, 0u, 0);
            state.machine->executor_cpu.data.eax = 0xdead0000u;
            state.machine->executor_cpu.data.cr0 |= VCPU_CR0_TS;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !msw_s63_run(&state, lmsw_register,
                sizeof(lmsw_register), 1u, &status, &result, &diagnostic) ||
                status != TYPE_STATUS_OK || diagnostic.first_fault.valid;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= after.data.eip != 3u || !msw_s63_gprs_same(&before,
                &after) || after.data.eflags != before.data.eflags ||
                after.data.cr0 != (before.data.cr0 & ~VCPU_CR0_TS) ||
                !msw_s63_sregs_same(&before, &after);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}
static C_INT msw_s63_test_attributes(C_VOID)
{
    static const type_unsigned_8 forms[][6] = {
        {0x66u,0x0fu,0x01u,0xe0u,0u,0u}, {0x67u,0x0fu,0x01u,0xe0u,0u,0u},
        {0x66u,0x67u,0x0fu,0x01u,0xe0u,0u}, {0x66u,0x0fu,0x01u,0xf0u,0u,0u},
        {0x67u,0x0fu,0x01u,0xf0u,0u,0u}, {0x66u,0x67u,0x0fu,0x01u,0xf0u,0u} };
    static const type_unsigned_8 lengths[] = {4u,4u,5u,4u,4u,5u};
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286 };
    type_unsigned_8 i, p;
    for (p=0u; p != sizeof(legacy)/sizeof(legacy[0]); ++p)
        for (i=0u; i != sizeof(forms)/sizeof(forms[0]); ++i)
            if (!msw_s63_expect_ud(legacy[p], forms[i], lengths[i])) return 0;
    for (i=0u; i != sizeof(forms)/sizeof(forms[0]); ++i) {
        msw_s63_machine state; t_cpu before; t_cpu after; core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic; type_status status; C_INT failed = !msw_s63_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        if (!failed) {
            msw_s63_seed(&state); state.machine->executor_cpu.data.eax = 0xdead000du;
            state.machine->executor_cpu.data.cr0 = 0x00a50000u; before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !msw_s63_run(&state, forms[i], lengths[i], 1u, &status, &result, &diagnostic) || status != TYPE_STATUS_OK || diagnostic.first_fault.valid;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            if (i < 3u) failed |= after.data.eax != 0xdead0000u || after.data.cr0 != before.data.cr0;
            else failed |= !msw_s63_gprs_same(&before,&after) || after.data.cr0 != 0x00a5000du;
            failed |= after.data.eip != lengths[i] || after.data.eflags != before.data.eflags || !msw_s63_sregs_same(&before,&after);
        }
        core_machine_destroy(state.machine); if (failed) return 0;
    }
    return msw_s63_expect_ud(CORE_MACHINE_CPU_PROFILE_80186, forms[0], 4u);
}

static C_INT msw_s63_test_privilege_and_faults(C_VOID)
{
    static const type_unsigned_8 smsw[] = {0x0fu,0x01u,0xe0u}; static const type_unsigned_8 lmsw[] = {0x0fu,0x01u,0xf0u};
    static const type_unsigned_8 smsw_memory[] = {0x0fu,0x01u,0x26u,0x10u,0u};
    static const type_unsigned_8 lmsw_memory[] = {0x0fu,0x01u,0x36u,0x10u,0u};
    type_unsigned_8 mode;
    for (mode=0u; mode != 3u; ++mode) {
        msw_s63_machine state; t_cpu before; t_cpu after; core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic; type_status status = TYPE_STATUS_INVALID_STATE; const type_unsigned_8 *code = mode == 0u ? smsw : lmsw;
        C_INT failed = !msw_s63_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        if (!failed) {
            msw_s63_seed(&state); msw_s63_enter_protected(&state, mode == 0u ? 3u : 3u, mode == 2u);
            state.machine->executor_cpu.data.cr0 |= VCPU_CR0_TS; before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !msw_s63_run(&state, code, 3u, 1u, &status, &result, &diagnostic);
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            if (mode == 0u) failed |= status != TYPE_STATUS_OK || diagnostic.first_fault.valid || after.data.eax != 0xdead0009u || after.data.cr0 != before.data.cr0;
            else failed |= status != TYPE_STATUS_FAULT || !diagnostic.first_fault.valid || !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,VCPUINS_EXCEPT_DF) || after.data.cr0 != before.data.cr0 || after.data.eip != 0u;
            failed |= after.data.eflags != before.data.eflags || !msw_s63_sregs_same(&before,&after);
        }
        core_machine_destroy(state.machine); if (failed) return 0;
    }
    for (mode=0u; mode != 2u; ++mode) {
        msw_s63_machine state; t_cpu before; t_cpu after; core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic; type_status status = TYPE_STATUS_INVALID_STATE; const type_unsigned_8 *code = mode == 0u ? smsw_memory : lmsw_memory;
        C_INT failed = !msw_s63_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        if (!failed) {
            msw_s63_seed(&state); msw_s63_enter_protected(&state, 0u, 0); state.machine->executor_cpu.data.ds.limit = 0x0fu;
            state.machine->executor_cpu.data.cr0 |= VCPU_CR0_TS; before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !msw_s63_run(&state, code, 5u, 1u, &status, &result, &diagnostic);
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= status != TYPE_STATUS_FAULT || !diagnostic.first_fault.valid || !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,VCPUINS_EXCEPT_DF) || after.data.eip != 0u || after.data.cr0 != before.data.cr0 || after.data.eflags != before.data.eflags || !msw_s63_sregs_same(&before,&after);
        }
        core_machine_destroy(state.machine); if (failed) return 0;
    }
    return 1;
}

static C_INT msw_s63_test_lock_irq(C_VOID)
{
    static const type_unsigned_8 lock_forms[][6] = {
        {0xf0u,0x0fu,0x01u,0xe0u,0u,0u},
        {0xf0u,0x66u,0x0fu,0x01u,0xe0u,0u},
        {0xf0u,0x67u,0x0fu,0x01u,0xe0u,0u},
        {0xf0u,0x66u,0x67u,0x0fu,0x01u,0xe0u},
        {0xf0u,0x0fu,0x01u,0xf0u,0u,0u},
        {0xf0u,0x66u,0x0fu,0x01u,0xf0u,0u},
        {0xf0u,0x67u,0x0fu,0x01u,0xf0u,0u},
        {0xf0u,0x66u,0x67u,0x0fu,0x01u,0xf0u}
    };
    static const type_unsigned_8 lock_lengths[] = {4u,5u,5u,6u,4u,5u,5u,6u};
    static const type_unsigned_8 hlt=0xf4u; static const type_unsigned_8 smsw_code[]={0x0fu,0x01u,0xe0u,0x90u}; static const type_unsigned_8 lmsw_code[]={0x0fu,0x01u,0xf0u,0x90u};
    const type_unsigned_8 *codes[]={smsw_code,lmsw_code}; type_unsigned_8 form;
    for(form=0u;form!=sizeof(lock_forms)/sizeof(lock_forms[0]);++form)
        if(!msw_s63_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, lock_forms[form],
            lock_lengths[form])) return 0;
    for(form=0u;form!=2u;++form) {
        msw_s63_machine state; core_machine_pic_irq_source irq; core_machine_run_result result; t_cpu before; t_cpu after; type_unsigned_16 offset=0x100u,segment=0u,frame=0u;
        C_INT failed=!msw_s63_prepare(&state,CORE_MACHINE_CPU_PROFILE_80386);
        if(!failed) failed |= core_machine_memory_write(state.machine,0u,codes[form],4u)!=TYPE_STATUS_OK || core_machine_memory_write(state.machine,0x80u,&offset,2u)!=TYPE_STATUS_OK || core_machine_memory_write(state.machine,0x82u,&segment,2u)!=TYPE_STATUS_OK || core_machine_memory_write(state.machine,0x100u,&hlt,1u)!=TYPE_STATUS_OK;
        if(!failed) { msw_s63_seed(&state); state.machine->executor_cpu.data.eax=0xdead000cu; state.machine->executor_cpu.data.cr0=0x00a50000u; before=test_core_machine_fixture_capture_cpu_after_run(state.machine); STD_MEMSET(&irq,0,sizeof(irq)); state.machine->shared_pic_master.data.icw2=0x20u; core_machine_pic_irq_source_bind(&irq,&state.machine->shared_pic_master,&state.machine->shared_pic_slave,0u); core_machine_pic_irq_source_assert(&irq); core_machine_pic_irq_source_deassert(&irq); failed |= core_machine_run(state.machine,(core_machine_run_budget){2u,0u},&result)!=TYPE_STATUS_OK || result.reason!=CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT; after=test_core_machine_fixture_capture_cpu_after_run(state.machine); failed |= core_machine_memory_read_physical(&state.machine->executor_memory,after.data.ss.base+(type_unsigned_16)after.data.esp,TYPE_REFERENCE_OF(frame),2u)!=TYPE_STATUS_OK || after.data.eip!=0x101u || frame!=3u || after.data.eflags!=(before.data.eflags&~VCPU_EFLAGS_IF) || !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,VPIC_ISR_IRQ(0u)) || TYPE_GET_BIT(state.machine->shared_pic_master.data.irr,VPIC_IRR_IRQ(0u)); if(form==0u) failed |= after.data.eax!=0xdead0000u || after.data.cr0!=before.data.cr0; else failed |= after.data.cr0!=0x00a5000cu || after.data.eax!=before.data.eax; }
        core_machine_destroy(state.machine); if(failed)return 0;
    } return 1;
}

C_INT main(C_VOID)
{
    if(!msw_s63_test_success()){STD_PRINTF("MSW stage=success\n");return 1;}
    if(!msw_s63_test_memory_and_protected()){STD_PRINTF("MSW stage=memory-protected\n");return 1;}
    if(!msw_s63_test_attributes()){STD_PRINTF("MSW stage=attributes\n");return 1;}
    if(!msw_s63_test_privilege_and_faults()){STD_PRINTF("MSW stage=privilege-faults\n");return 1;}
    if(!msw_s63_test_lock_irq()){STD_PRINTF("MSW stage=lock-irq\n");return 1;}
    STD_PRINTF("M5:T316:S63:MSW:OK\n"); return 0;
}
