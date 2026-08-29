#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/cpu_instructions.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"


typedef struct bound_s54_machine {
    core_machine *machine;
} bound_s54_machine;

#define BOUND_S54_GDT_POINTER 0x0100u
#define BOUND_S54_GDT_BASE 0x0300u
#define BOUND_S54_IDT_BASE 0x0400u
#define BOUND_S54_CODE_BASE 0x2000u

static C_INT bound_s54_prepare(bound_s54_machine *state,
    core_machine_cpu_profile profile);

static C_INT bound_s54_boot_protected(bound_s54_machine *state,
    const type_unsigned_8 *program, STD_SIZE_T program_bytes)
{
    static const type_unsigned_8 gdt_pointer[] = {
        0x1fu, 0x00u, 0x00u, 0x03u, 0x00u, 0x00u
    };
    static const type_unsigned_8 idt_pointer[] = {
        0xffu, 0x00u, 0x00u, 0x04u, 0x00u, 0x00u
    };
    static const type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0,0,
        0xffu,0xffu,0,0x40u,0,0x92u,0,0
    };
    static const type_unsigned_8 real_code[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,
        0x0fu,0x01u,0x1eu,0x10u,0x01u,
        0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,
        0xb8u,0x18u,0x00u,0x8eu,0xd0u,
        0xeau,0x00u,0x00u,0x08u,0x00u
    };
    type_unsigned_8 idt[0x100u] = { 0u };

    idt[5u * 8u + 1u] = 0x01u;
    idt[5u * 8u + 2u] = 0x08u;
    idt[5u * 8u + 5u] = 0x86u;
    idt[13u * 8u + 1u] = 0x01u;
    idt[13u * 8u + 2u] = 0x08u;
    idt[13u * 8u + 5u] = 0x86u;
    return bound_s54_prepare(state, CORE_MACHINE_CPU_PROFILE_80386) &&
        core_machine_memory_write(state->machine, BOUND_S54_GDT_POINTER,
            gdt_pointer, sizeof(gdt_pointer)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, BOUND_S54_GDT_BASE, gdt,
            sizeof(gdt)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x0110u, idt_pointer,
            sizeof(idt_pointer)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, BOUND_S54_IDT_BASE, idt,
            sizeof(idt)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0u, real_code,
            sizeof(real_code)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, BOUND_S54_CODE_BASE,
            program, program_bytes) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, BOUND_S54_CODE_BASE + 0x100u,
            (const type_unsigned_8[]){ 0xf4u }, 1u) == TYPE_STATUS_OK;
}

static C_VOID bound_s54_reset(C_VOID *opaque)
{
    bound_s54_machine *state = (bound_s54_machine *)opaque;

    if (state != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider bound_s54_execution_provider = {
    bound_s54_reset,
    STD_NULL
};

static C_INT bound_s54_prepare(bound_s54_machine *state,
    core_machine_cpu_profile profile)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    if (state == STD_NULL)
        return 0;
    STD_MEMSET(state, 0, sizeof(*state));
    if (core_machine_create(&config, &state->machine) != TYPE_STATUS_OK)
        return 0;
    if (!test_core_machine_fixture_bind_freeze_reset(state->machine,
            &bound_s54_execution_provider, state)) {
        core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 0;
    }
    return 1;
}

static C_VOID bound_s54_seed(t_cpu *cpu)
{
    cpu->data.eax = 0xa1a10000u;
    cpu->data.ecx = 0xb2b20000u;
    cpu->data.edx = 0xc3c30000u;
    cpu->data.ebx = 0xd4d40000u;
    cpu->data.esp = 0x00008000u;
    cpu->data.ebp = 0xe5e50000u;
    cpu->data.esi = 0xf6f60000u;
    cpu->data.edi = 0x97970000u;
    cpu->data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF |
        VCPU_EFLAGS_IF | VCPU_EFLAGS_DF | VCPU_EFLAGS_OF;
}

static C_INT bound_s54_gprs_same(const t_cpu *before, const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi;
}

static C_INT bound_s54_sregs_same(const t_cpu *before, const t_cpu *after)
{
    return STD_MEMCMP(&before->data.es, &after->data.es,
            sizeof(before->data.es)) == 0 &&
        STD_MEMCMP(&before->data.cs, &after->data.cs,
            sizeof(before->data.cs)) == 0 &&
        STD_MEMCMP(&before->data.ss, &after->data.ss,
            sizeof(before->data.ss)) == 0 &&
        STD_MEMCMP(&before->data.ds, &after->data.ds,
            sizeof(before->data.ds)) == 0 &&
        STD_MEMCMP(&before->data.fs, &after->data.fs,
            sizeof(before->data.fs)) == 0 &&
        STD_MEMCMP(&before->data.gs, &after->data.gs,
            sizeof(before->data.gs)) == 0;
}

static C_INT bound_s54_run(bound_s54_machine *state, type_unsigned_32 budget,
    core_machine_run_result *result, t_cpu *after,
    core_machine_cpu_diagnostic *diagnostic)
{
    type_status status = core_machine_run(state->machine,
        (core_machine_run_budget){ budget, 0u }, result);

    if (status != TYPE_STATUS_OK && status != TYPE_STATUS_FAULT)
        return 0;
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) ==
        TYPE_STATUS_OK;
}

static C_INT bound_s54_test_profiles_and_width(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 code[] = { 0x62u, 0x06u, 0x00u, 0x04u, 0xf4u };
    const type_signed_16 bounds[] = { -2, 2 };
    type_unsigned_8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        bound_s54_machine state;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu before;
        t_cpu after;
        C_INT failed = !bound_s54_prepare(&state, profiles[profile]);

        if (!failed) {
            bound_s54_seed(&state.machine->executor_cpu);
            state.machine->executor_cpu.data.eax = 0xa1a10001u;
            before = state.machine->executor_cpu;
            failed = core_machine_memory_write(state.machine, 0u, code,
                sizeof(code)) != TYPE_STATUS_OK;
            failed |= core_machine_memory_write(state.machine, 0x0400u, bounds,
                sizeof(bounds)) != TYPE_STATUS_OK;
            failed |= !bound_s54_run(&state, 2u, &result, &after, &diagnostic);
            failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            failed |= diagnostic.first_fault.valid;
            failed |= after.data.eip != sizeof(code);
            failed |= after.data.eflags != before.data.eflags;
            failed |= !bound_s54_gprs_same(&before, &after);
            failed |= !bound_s54_sregs_same(&before, &after);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT bound_s54_test_386_dword_pair(C_VOID)
{
    static const type_unsigned_8 code[] = {
        0x66u, 0x62u, 0x06u, 0x00u, 0x04u, 0xf4u
    };
    const type_signed_32 bounds[] = { -4, 4 };
    bound_s54_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    C_INT failed = !bound_s54_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        bound_s54_seed(&state.machine->executor_cpu);
        state.machine->executor_cpu.data.eax = 1u;
        before = state.machine->executor_cpu;
        failed = core_machine_memory_write(state.machine, 0u, code,
            sizeof(code)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 0x0400u, bounds,
            sizeof(bounds)) != TYPE_STATUS_OK;
        failed |= !bound_s54_run(&state, 2u, &result, &after, &diagnostic);
        failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        failed |= diagnostic.first_fault.valid;
        failed |= after.data.eip != sizeof(code);
        failed |= after.data.eflags != before.data.eflags;
        failed |= !bound_s54_gprs_same(&before, &after);
        failed |= !bound_s54_sregs_same(&before, &after);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT bound_s54_test_386_attributes(C_VOID)
{
    static const type_unsigned_8 forms[][9] = {
        { 0x67u, 0x62u, 0x05u, 0x00u, 0x04u, 0x00u, 0x00u, 0xf4u, 0u },
        { 0x66u, 0x67u, 0x62u, 0x05u, 0x00u, 0x04u, 0x00u, 0x00u, 0xf4u }
    };
    const type_unsigned_8 bytes[] = { 8u, 9u };
    const type_signed_32 bounds32[] = { -4, 4 };
    const type_signed_16 bounds16[] = { -2, 2 };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(forms) / sizeof(forms[0]); ++form) {
        bound_s54_machine state;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu before;
        t_cpu after;
        C_INT failed = !bound_s54_prepare(&state,
            CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            bound_s54_seed(&state.machine->executor_cpu);
            state.machine->executor_cpu.data.eax = 1u;
            before = state.machine->executor_cpu;
            failed = core_machine_memory_write(state.machine, 0u, forms[form],
                bytes[form]) != TYPE_STATUS_OK;
            if (form == 0u)
                failed |= core_machine_memory_write(state.machine, 0x0400u,
                    bounds16, sizeof(bounds16)) != TYPE_STATUS_OK;
            else
                failed |= core_machine_memory_write(state.machine, 0x0400u,
                    bounds32, sizeof(bounds32)) != TYPE_STATUS_OK;
            failed |= !bound_s54_run(&state, 2u, &result, &after, &diagnostic);
            failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            failed |= diagnostic.first_fault.valid;
            failed |= after.data.eip != bytes[form];
            failed |= after.data.eflags != before.data.eflags;
            failed |= !bound_s54_gprs_same(&before, &after);
            failed |= !bound_s54_sregs_same(&before, &after);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT bound_s54_test_rejections(C_VOID)
{
    static const type_unsigned_8 codes[][6] = {
        { 0x62u, 0xc0u },
        { 0xf0u, 0x62u, 0x06u, 0x00u, 0x04u },
        { 0x66u, 0x62u, 0x06u, 0x00u, 0x04u },
        { 0x67u, 0x62u, 0x06u, 0x00u, 0x04u },
        { 0x66u, 0x67u, 0x62u, 0x06u, 0x00u, 0x04u },
        { 0x62u, 0x06u, 0x00u, 0x04u }
    };
    static const type_unsigned_8 bytes[] = { 2u, 5u, 5u, 5u, 6u, 4u };
    static const core_machine_cpu_profile pre386[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const type_signed_16 pair[] = { -2, 2 };
    type_unsigned_8 index;

    for (index = 0u; index != sizeof(codes) / sizeof(codes[0]); ++index) {
        type_unsigned_8 profile_first = 0u;
        type_unsigned_8 profile_count = 1u;

        if (index >= 2u && index <= 4u)
            profile_count = sizeof(pre386) / sizeof(pre386[0]);
        bound_s54_machine state;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu before;
        t_cpu after;
        for (; profile_first != profile_count; ++profile_first) {
            core_machine_cpu_profile profile =
                (index >= 2u && index <= 4u) ? pre386[profile_first] :
                (index == 5u ? CORE_MACHINE_CPU_PROFILE_8086 :
                    CORE_MACHINE_CPU_PROFILE_80386);
            C_INT failed = !bound_s54_prepare(&state, profile);

            if (!failed) {
                bound_s54_seed(&state.machine->executor_cpu);
                failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
                    state.machine);
                before = state.machine->executor_cpu;
                failed = core_machine_memory_write(state.machine, 0u,
                    codes[index], bytes[index]) != TYPE_STATUS_OK;
                failed |= core_machine_memory_write(state.machine, 0x0400u,
                    pair, sizeof(pair)) != TYPE_STATUS_OK;
                failed |= !bound_s54_run(&state, 1u, &result, &after,
                    &diagnostic);
                failed |= !diagnostic.first_fault.valid;
                failed |= !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                    VCPUINS_EXCEPT_UD);
                failed |= after.data.eip != before.data.eip;
                failed |= after.data.eflags != before.data.eflags;
                failed |= !bound_s54_gprs_same(&before, &after);
                failed |= !bound_s54_sregs_same(&before, &after);
            }
            core_machine_destroy(state.machine);
            if (failed)
                return 0;
        }
    }
    return 1;
}

static C_INT bound_s54_test_real_br_delivery_profile(
    core_machine_cpu_profile profile)
{
    static const type_unsigned_8 code[] = { 0x62u, 0x06u, 0x00u, 0x04u };
    static const type_unsigned_8 handler[] = { 0xf4u };
    const type_signed_16 bounds[] = { -2, 2 };
    const type_unsigned_16 vector[] = { 0x0100u, 0x0000u };
    type_unsigned_16 frame[3] = { 0u, 0u, 0u };
    bound_s54_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    C_INT failed = !bound_s54_prepare(&state, profile);

    if (!failed) {
        bound_s54_seed(&state.machine->executor_cpu);
        state.machine->executor_cpu.data.idtr.limit = 0x017fu;
        state.machine->executor_cpu.data.eax = 0xa1a10003u;
        before = state.machine->executor_cpu;
        failed = core_machine_memory_write(state.machine, 0u, code,
            sizeof(code)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 5u * 4u, vector,
            sizeof(vector)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 0x0100u, handler,
            sizeof(handler)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 0x0400u, bounds,
            sizeof(bounds)) != TYPE_STATUS_OK;
        failed |= !bound_s54_run(&state, 1u, &result, &after, &diagnostic);
        failed |= result.reason != CORE_MACHINE_STOP_BUDGET;
        failed |= diagnostic.first_fault.valid;
        failed |= !diagnostic.last_delivered_exception.valid;
        failed |= after.data.eip != 0x0100u;
        failed |= after.data.eax != before.data.eax;
        failed |= !bound_s54_sregs_same(&before, &after);
        failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
            after.data.ss.base + (type_unsigned_16)after.data.esp,
            (type_virtual_address)frame, sizeof(frame)) != TYPE_STATUS_OK;
        failed |= frame[0u] != 0u;
        failed |= frame[1u] != before.data.cs.selector;
        failed |= frame[2u] != (type_unsigned_16)((before.data.eflags &
            ~VCPU_EFLAGS_RESERVED) | 0x02u);
        failed |= !bound_s54_run(&state, 1u, &result, &after, &diagnostic);
        failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        failed |= after.data.eip != 0x0101u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT bound_s54_test_real_br_delivery(C_VOID)
{
    return bound_s54_test_real_br_delivery_profile(
        CORE_MACHINE_CPU_PROFILE_80186) &&
        bound_s54_test_real_br_delivery_profile(
            CORE_MACHINE_CPU_PROFILE_80386);
}

static C_INT bound_s54_test_signed_lower_bound(C_VOID)
{
    static const type_unsigned_8 code[] = { 0x62u, 0x06u, 0x00u, 0x04u };
    static const type_unsigned_8 handler[] = { 0xf4u };
    const type_signed_16 bounds[] = { -2, 2 };
    const type_unsigned_16 vector[] = { 0x0100u, 0x0000u };
    bound_s54_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu after;
    C_INT failed = !bound_s54_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        bound_s54_seed(&state.machine->executor_cpu);
        state.machine->executor_cpu.data.eax = 0xa1a1fffdu;
        state.machine->executor_cpu.data.idtr.limit = 0x017fu;
        failed = core_machine_memory_write(state.machine, 0u, code,
            sizeof(code)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 0x0400u, bounds,
            sizeof(bounds)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 5u * 4u, vector,
            sizeof(vector)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 0x0100u, handler,
            sizeof(handler)) != TYPE_STATUS_OK;
        failed |= !bound_s54_run(&state, 1u, &result, &after, &diagnostic);
        failed |= result.reason != CORE_MACHINE_STOP_BUDGET;
        failed |= !diagnostic.last_delivered_exception.valid;
        failed |= after.data.eip != 0x0100u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT bound_s54_test_protected_br_delivery(C_VOID)
{
    static const type_unsigned_8 hlt[] = { 0xf4u };
    static const type_unsigned_8 code[] = { 0x62u, 0x06u, 0x00u, 0x04u };
    const type_signed_16 bounds[] = { -2, 2 };
    bound_s54_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    C_INT initial_run;
    C_INT failed = !bound_s54_boot_protected(&state, hlt, sizeof(hlt));

    if (!failed) {
        initial_run = bound_s54_run(&state, 64u, &result, &after, &diagnostic);
        failed = !initial_run;
        failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
    }
    if (!failed) {
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        bound_s54_seed(&state.machine->executor_cpu);
        state.machine->executor_cpu.data.eax = 0xa1a10003u;
        before = state.machine->executor_cpu;
        failed = core_machine_memory_write(state.machine, 0x00003400u, bounds,
            sizeof(bounds)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, BOUND_S54_CODE_BASE,
            code, sizeof(code)) != TYPE_STATUS_OK;
        failed |= !bound_s54_run(&state, 64u, &result, &after, &diagnostic);
        failed |= result.reason != CORE_MACHINE_STOP_BUDGET;
        failed |= diagnostic.first_fault.valid;
        failed |= !diagnostic.last_delivered_exception.valid;
        failed |= !TYPE_GET_BIT(diagnostic.last_delivered_exception.exception_mask,
            VCPUINS_EXCEPT_BR);
        failed |= result.executed != 0u || result.ticks != 0u;
        failed |= after.data.eip != 0x00000100u;
        failed |= after.data.eax != before.data.eax;
        failed |= !bound_s54_run(&state, 1u, &result, &after, &diagnostic);
        failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        failed |= after.data.eip != 0x00000101u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT bound_s54_test_protected_ds_upper_limit(C_VOID)
{
    static const type_unsigned_8 hlt[] = { 0xf4u };
    static const type_unsigned_8 code[] = { 0x62u, 0x06u, 0x00u, 0x04u };
    const type_signed_16 bounds[] = { -2, 2 };
    bound_s54_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    C_INT initial_run;
    C_INT failed = !bound_s54_boot_protected(&state, hlt, sizeof(hlt));

    if (!failed) {
        initial_run = bound_s54_run(&state, 64u, &result, &after, &diagnostic);
        failed = !initial_run;
        failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
    }
    if (!failed) {
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        bound_s54_seed(&state.machine->executor_cpu);
        state.machine->executor_cpu.data.eax = 0xa1a10001u;
        state.machine->executor_cpu.data.ds.limit = 0x0401u;
        before = state.machine->executor_cpu;
        failed = core_machine_memory_write(state.machine, 0x00003400u, bounds,
            sizeof(bounds)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, BOUND_S54_CODE_BASE,
            code, sizeof(code)) != TYPE_STATUS_OK;
        failed |= !bound_s54_run(&state, 64u, &result, &after, &diagnostic);
        failed |= result.reason != CORE_MACHINE_STOP_BUDGET ||
            result.executed != 0u || result.ticks != 0u;
        failed |= diagnostic.first_fault.valid;
        failed |= !diagnostic.last_delivered_exception.valid;
        failed |= !TYPE_GET_BIT(diagnostic.last_delivered_exception.exception_mask,
            VCPUINS_EXCEPT_GP);
        failed |= after.data.eip != 0x00000100u;
        failed |= after.data.eax != before.data.eax;
        failed |= !bound_s54_run(&state, 1u, &result, &after, &diagnostic);
        failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            after.data.eip != 0x00000101u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT bound_s54_test_protected_ss_upper_limit(C_VOID)
{
    static const type_unsigned_8 hlt[] = { 0xf4u };
    static const type_unsigned_8 code[] = { 0x36u, 0x62u, 0x06u, 0x00u, 0x04u };
    const type_signed_16 bounds[] = { -2, 2 };
    bound_s54_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    C_INT setup_failed;
    C_INT initial_run;
    C_INT pair_write;
    C_INT code_write;
    C_INT bound_run;
    C_INT failed = !bound_s54_boot_protected(&state, hlt, sizeof(hlt));

    if (!failed) {
        initial_run = bound_s54_run(&state, 64u, &result, &after, &diagnostic);
        if (!initial_run || result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT)
            failed = 1;
    }
    if (!failed) {
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        bound_s54_seed(&state.machine->executor_cpu);
        state.machine->executor_cpu.data.eax = 0xa1a10001u;
        state.machine->executor_cpu.data.ss.limit = 0x0401u;
        before = state.machine->executor_cpu;
        pair_write = core_machine_memory_write(state.machine, 0x00004400u,
            bounds, sizeof(bounds)) == TYPE_STATUS_OK;
        code_write = core_machine_memory_write(state.machine,
            BOUND_S54_CODE_BASE, code, sizeof(code)) == TYPE_STATUS_OK;
        setup_failed = !pair_write || !code_write;
        bound_run = !setup_failed && bound_s54_run(&state, 64u, &result,
            &after, &diagnostic);
        if (setup_failed || !bound_run || !diagnostic.first_fault.valid ||
            !(diagnostic.first_fault.exception_mask & VCPUINS_EXCEPT_DF) ||
            result.reason != CORE_MACHINE_STOP_FAULT || after.data.eip != 0u ||
            after.data.eax != before.data.eax)
            failed = 1;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT bound_s54_test_irq_no_shadow(C_VOID)
{
    static const type_unsigned_8 code[] = { 0x62u, 0x06u, 0x00u, 0x04u, 0x90u };
    static const type_unsigned_8 hlt = 0xf4u;
    const type_signed_16 bounds[] = { -2, 2 };
    type_unsigned_16 vector_offset = 0x0100u;
    type_unsigned_16 vector_segment = 0u;
    type_unsigned_16 frame_ip = 0u;
    bound_s54_machine state;
    core_machine_pic_irq_source source;
    core_machine_run_result result;
    t_cpu before;
    t_cpu after;
    C_INT failed = !bound_s54_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        bound_s54_seed(&state.machine->executor_cpu);
        state.machine->executor_cpu.data.eax = 0xa1a10001u;
        state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_IF;
        state.machine->executor_cpu.data.idtr.limit = 0x017fu;
        before = state.machine->executor_cpu;
        failed = core_machine_memory_write(state.machine, 0u, code,
            sizeof(code)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 0x0400u, bounds,
            sizeof(bounds)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 0x20u * 4u,
            &vector_offset, sizeof(vector_offset)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 0x20u * 4u + 2u,
            &vector_segment, sizeof(vector_segment)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 0x0100u, &hlt,
            sizeof(hlt)) != TYPE_STATUS_OK;
    }
    if (!failed) {
        STD_MEMSET(&source, 0, sizeof(source));
        state.machine->shared_pic_master.data.icw2 = 0x20u;
        core_machine_pic_irq_source_bind(&source, &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&source);
        core_machine_pic_irq_source_deassert(&source);
        failed = !bound_s54_run(&state, 2u, &result, &after, &(core_machine_cpu_diagnostic){0});
        failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        failed |= after.data.eip != 0x0101u;
        failed |= after.data.eax != before.data.eax;
        failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
            after.data.ss.base + (type_unsigned_16)after.data.esp,
            (type_virtual_address)&frame_ip, sizeof(frame_ip)) != TYPE_STATUS_OK;
        failed |= frame_ip != 4u;
        failed |= !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,
            VPIC_ISR_IRQ(0u));
        failed |= TYPE_GET_BIT(state.machine->shared_pic_master.data.irr,
            VPIC_IRR_IRQ(0u));
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT bound_s54_test_segments(C_VOID)
{
    static const type_unsigned_8 codes[][5] = {
        { 0x62u,0x06u,0x10u,0,0 }, { 0x62u,0x46u,0,0,0 },
        { 0x2eu,0x62u,0x06u,0x10u,0 }, { 0x26u,0x62u,0x06u,0x10u,0 },
        { 0x64u,0x62u,0x06u,0x10u,0 }, { 0x65u,0x62u,0x06u,0x10u,0 }
    };
    static const type_unsigned_8 bytes[] = { 4u,3u,5u,5u,5u,5u };
    const type_signed_16 bounds[] = { -2, 2 };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(bytes); ++form) {
        bound_s54_machine state;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu before;
        t_cpu after;
        type_unsigned_16 selector = (type_unsigned_16)(0x1000u + form * 0x1000u);
        type_unsigned_32 address = ((type_unsigned_32)selector << 4u) + 0x10u;
        C_INT failed = !bound_s54_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            t_cpu_data_sreg *segment = &state.machine->executor_cpu.data.ds;

            if (form == 1u)
                segment = &state.machine->executor_cpu.data.ss;
            else if (form == 2u)
                segment = &state.machine->executor_cpu.data.cs;
            else if (form == 3u)
                segment = &state.machine->executor_cpu.data.es;
            else if (form == 4u)
                segment = &state.machine->executor_cpu.data.fs;
            else if (form == 5u)
                segment = &state.machine->executor_cpu.data.gs;
            failed = core_machine_cpu_execution_load_segment(
                &state.machine->executor_cpu_execution, segment, selector);
            if (form == 2u)
                failed |= core_machine_memory_write(state.machine,
                    (type_unsigned_32)selector << 4u, codes[form], bytes[form]) != TYPE_STATUS_OK;
            else
                failed |= core_machine_memory_write(state.machine, 0u, codes[form],
                    bytes[form]) != TYPE_STATUS_OK;
            if (form == 1u)
                state.machine->executor_cpu.data.ebp = 0x10u;
            bound_s54_seed(&state.machine->executor_cpu);
            state.machine->executor_cpu.data.eax = 0xa1a10001u;
            if (form == 1u)
                state.machine->executor_cpu.data.ebp = 0x10u;
            before = state.machine->executor_cpu;
            failed |= core_machine_memory_write(state.machine, address, bounds,
                sizeof(bounds)) != TYPE_STATUS_OK;
            failed |= !bound_s54_run(&state, 1u, &result, &after, &diagnostic);
            failed |= result.reason != CORE_MACHINE_STOP_BUDGET;
            failed |= diagnostic.first_fault.valid;
            failed |= after.data.eip != bytes[form];
            failed |= after.data.eax != before.data.eax;
            failed |= after.data.eflags != before.data.eflags;
            failed |= !bound_s54_gprs_same(&before, &after);
            failed |= !bound_s54_sregs_same(&before, &after);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT bound_s54_test_signed_boundaries(C_VOID)
{
    static const type_unsigned_8 code[] = { 0x62u, 0x06u, 0x00u, 0x04u };
    static const type_signed_16 pair[] = { -2, 2 };
    static const type_unsigned_16 values[] = { 0xfffeu, 0x0002u };
    type_unsigned_8 value;

    for (value = 0u; value != sizeof(values) / sizeof(values[0]); ++value) {
        bound_s54_machine state;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu before;
        t_cpu after;
        C_INT failed = !bound_s54_prepare(&state,
            CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            bound_s54_seed(&state.machine->executor_cpu);
            state.machine->executor_cpu.data.eax =
                0xa1a10000u | values[value];
            before = state.machine->executor_cpu;
            failed = core_machine_memory_write(state.machine, 0u, code,
                sizeof(code)) != TYPE_STATUS_OK;
            failed |= core_machine_memory_write(state.machine, 0x0400u, pair,
                sizeof(pair)) != TYPE_STATUS_OK;
            failed |= !bound_s54_run(&state, 1u, &result, &after,
                &diagnostic);
            failed |= result.reason != CORE_MACHINE_STOP_BUDGET;
            failed |= diagnostic.first_fault.valid;
            failed |= after.data.eip != sizeof(code);
            failed |= after.data.eax != before.data.eax;
            failed |= after.data.eflags != before.data.eflags;
            failed |= !bound_s54_gprs_same(&before, &after);
            failed |= !bound_s54_sregs_same(&before, &after);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT bound_s54_test_67_sib_stack_segment(C_VOID)
{
    static const type_unsigned_8 code[] = {
        0x67u, 0x62u, 0x44u, 0x24u, 0x10u
    };
    static const type_signed_16 pair[] = { -2, 2 };
    bound_s54_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    const type_unsigned_32 address = 0x20010u;
    C_INT failed = !bound_s54_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        failed = core_machine_cpu_execution_load_segment(
            &state.machine->executor_cpu_execution,
            &state.machine->executor_cpu.data.ss, 0x1800u) != 0;
    }
    if (!failed) {
        bound_s54_seed(&state.machine->executor_cpu);
        state.machine->executor_cpu.data.eax = 0xa1a10001u;
        before = state.machine->executor_cpu;
        failed = core_machine_memory_write(state.machine, 0u, code,
            sizeof(code)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, address, pair,
            sizeof(pair)) != TYPE_STATUS_OK;
        failed |= !bound_s54_run(&state, 1u, &result, &after, &diagnostic);
        failed |= result.reason != CORE_MACHINE_STOP_BUDGET;
        failed |= diagnostic.first_fault.valid;
        failed |= after.data.eip != sizeof(code);
        failed |= after.data.eax != before.data.eax;
        failed |= after.data.eflags != before.data.eflags;
        failed |= !bound_s54_gprs_same(&before, &after);
        failed |= !bound_s54_sregs_same(&before, &after);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT bound_s54_test_vm86(C_VOID)
{
    static const type_unsigned_8 code[] = { 0x62u, 0x06u, 0x00u, 0x04u };
    static const type_signed_16 pair[] = { -2, 2 };
    bound_s54_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    C_INT failed = !bound_s54_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        failed = !test_core_machine_fixture_prepare_real_mode_execution(
            state.machine, 0u);
    }
    if (!failed) {
        bound_s54_seed(&state.machine->executor_cpu);
        state.machine->executor_cpu.data.cr0 |= VCPU_CR0_PE;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_VM |
            VCPU_EFLAGS_IOPL | VCPU_EFLAGS_IF | VCPU_EFLAGS_CF;
        state.machine->executor_cpu.data.cs.selector = 0u;
        state.machine->executor_cpu.data.cs.base = 0u;
        state.machine->executor_cpu.data.cs.limit = 0xffffu;
        state.machine->executor_cpu.data.cs.dpl = 3u;
        state.machine->executor_cpu.data.cs.flagValid = TYPE_TRUE;
        state.machine->executor_cpu.data.cs.seg.exec.defsize = TYPE_FALSE;
        state.machine->executor_cpu.data.ds.selector = 0u;
        state.machine->executor_cpu.data.ds.base = 0u;
        state.machine->executor_cpu.data.ds.limit = 0xffffu;
        state.machine->executor_cpu.data.ds.dpl = 3u;
        state.machine->executor_cpu.data.ds.flagValid = TYPE_TRUE;
        state.machine->executor_cpu.data.ss.selector = 0u;
        state.machine->executor_cpu.data.ss.base = 0u;
        state.machine->executor_cpu.data.ss.limit = 0xffffu;
        state.machine->executor_cpu.data.ss.dpl = 3u;
        state.machine->executor_cpu.data.ss.flagValid = TYPE_TRUE;
        state.machine->executor_cpu.data.ss.seg.data.big = TYPE_FALSE;
        state.machine->executor_cpu.data.eax = 0xa1a10001u;
        before = state.machine->executor_cpu;
        failed = core_machine_memory_write(state.machine, 0u, code,
            sizeof(code)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 0x0400u, pair,
            sizeof(pair)) != TYPE_STATUS_OK;
        failed |= !bound_s54_run(&state, 1u, &result, &after, &diagnostic);
        failed |= result.reason != CORE_MACHINE_STOP_BUDGET;
        failed |= diagnostic.first_fault.valid;
        failed |= after.data.eip != sizeof(code);
        failed |= after.data.eax != before.data.eax;
        failed |= after.data.eflags != before.data.eflags;
        failed |= !bound_s54_gprs_same(&before, &after);
        failed |= !bound_s54_sregs_same(&before, &after);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

C_INT main(C_VOID)
{
    if (!bound_s54_test_profiles_and_width()) {
        STD_FPRINTF(STD_STDERR, "M5:T316:S54:BOUND profiles failed\n");
        return 1;
    }
    if (!bound_s54_test_386_dword_pair()) {
        STD_FPRINTF(STD_STDERR, "M5:T316:S54:BOUND dword failed\n");
        return 1;
    }
    if (!bound_s54_test_386_attributes()) {
        STD_FPRINTF(STD_STDERR, "M5:T316:S54:BOUND attributes failed\n");
        return 1;
    }
    if (!bound_s54_test_rejections()) {
        STD_FPRINTF(STD_STDERR, "M5:T316:S54:BOUND reject failed\n");
        return 1;
    }
    if (!bound_s54_test_real_br_delivery()) {
        STD_FPRINTF(STD_STDERR, "M5:T316:S54:BOUND real #BR failed\n");
        return 1;
    }
    if (!bound_s54_test_signed_lower_bound()) {
        STD_FPRINTF(STD_STDERR, "M5:T316:S54:BOUND lower failed\n");
        return 1;
    }
    if (!bound_s54_test_protected_br_delivery()) {
        STD_FPRINTF(STD_STDERR, "M5:T316:S54:BOUND protected #BR failed\n");
        return 1;
    }
    if (!bound_s54_test_protected_ds_upper_limit()) {
        STD_FPRINTF(STD_STDERR, "M5:T316:S54:BOUND DS limit failed\n");
        return 1;
    }
    if (!bound_s54_test_protected_ss_upper_limit()) {
        STD_FPRINTF(STD_STDERR, "M5:T316:S54:BOUND SS limit failed\n");
        return 1;
    }
    if (!bound_s54_test_irq_no_shadow()) {
        STD_FPRINTF(STD_STDERR, "M5:T316:S54:BOUND irq failed\n");
        return 1;
    }
    if (!bound_s54_test_segments()) {
        STD_FPRINTF(STD_STDERR, "M5:T316:S54:BOUND segments failed\n");
        return 1;
    }
    if (!bound_s54_test_signed_boundaries()) {
        STD_FPRINTF(STD_STDERR, "M5:T316:S54:BOUND signed boundaries failed\n");
        return 1;
    }
    if (!bound_s54_test_67_sib_stack_segment()) {
        STD_FPRINTF(STD_STDERR, "M5:T316:S54:BOUND 67 SIB failed\n");
        return 1;
    }
    if (!bound_s54_test_vm86()) {
        STD_FPRINTF(STD_STDERR, "M5:T316:S54:BOUND VM86 failed\n");
        return 1;
    }
    STD_PRINTF("M5:T316:S54:BOUND:OK\n");
    STD_PRINTF("M5:T401:S29:BOUND-PROFILES:OK\n");
    return 0;
}
