#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "core/machine/memory.h"
#include "../support/core_machine_cpu_fixture.h"

#define DT_STORE_ADDRESS 0x0200u
#define DT_LOAD_ADDRESS 0x0240u
#define DT_GDT_ADDRESS 0x0300u
#define DT_LDT_SELECTOR 0x0018u
#define DT_TSS16_SELECTOR 0x0020u
#define DT_LDT_NOT_PRESENT_SELECTOR 0x0028u
#define DT_TSS16_BUSY_SELECTOR 0x0030u
#define DT_TSS16_NOT_PRESENT_SELECTOR 0x0038u
#define DT_TSS32_SELECTOR 0x0040u

typedef struct descriptor_system_machine {
    core_machine *machine;
} descriptor_system_machine;

static C_VOID dt_reset(C_VOID *opaque)
{
    descriptor_system_machine *state = (descriptor_system_machine *)opaque;

    if (state != STD_NULL) (C_VOID)test_core_machine_fixture_reset_real_mode(
        state->machine);
}

static const core_machine_execution_provider dt_provider = {
    dt_reset,
    STD_NULL
};

static C_INT dt_prepare_profile(descriptor_system_machine *state,
    core_machine_cpu_profile profile, type_bool cpu_80386_cr_mov_ignores_mod)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
        .cpu_80386_cr_mov_ignores_mod = cpu_80386_cr_mov_ignores_mod
    };

    if (state == STD_NULL) return 0;
    STD_MEMSET(state, 0, sizeof(*state));
    if (core_machine_create(&config, &state->machine) != TYPE_STATUS_OK ||
        !test_core_machine_fixture_bind_freeze_reset(state->machine,
            &dt_provider, state)) {
        core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 0;
    }
    return 1;
}

static C_INT dt_prepare(descriptor_system_machine *state)
{
    return dt_prepare_profile(state, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE);
}

static C_INT dt_prepare_early_80386(descriptor_system_machine *state)
{
    return dt_prepare_profile(state, CORE_MACHINE_CPU_PROFILE_80386, TYPE_TRUE);
}

static C_INT dt_write(descriptor_system_machine *state, type_unsigned_32 address,
    const type_unsigned_8 *data, STD_SIZE_T bytes)
{
    return state != STD_NULL && state->machine != STD_NULL &&
        core_machine_memory_write(state->machine, address, data, bytes) ==
            TYPE_STATUS_OK;
}

static C_INT dt_read(descriptor_system_machine *state, type_unsigned_32 address,
    type_unsigned_8 *data, STD_SIZE_T bytes)
{
    return state != STD_NULL && state->machine != STD_NULL &&
        core_machine_memory_read(state->machine, address, data, bytes) ==
            TYPE_STATUS_OK;
}

static C_INT dt_read_private(descriptor_system_machine *state, type_unsigned_32 address,
    type_unsigned_8 *data, STD_SIZE_T bytes)
{
    return state != STD_NULL && state->machine != STD_NULL &&
        core_machine_memory_read_physical(&state->machine->executor_memory,
            address, (type_virtual_address)data, bytes) == TYPE_STATUS_OK;
}

static C_INT dt_run(descriptor_system_machine *state, const type_unsigned_8 *code,
    STD_SIZE_T bytes, C_INT expect_fault, type_unsigned_32 expect_exception)
{
    const core_machine_run_budget budget = {32u, 0u};
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;

    if (expect_fault && expect_exception == VCPUINS_EXCEPT_UD &&
        !TYPE_GET_BIT(state->machine->executor_cpu.data.cr0, VCPU_CR0_PE) &&
        !test_core_machine_fixture_preflight_real_ud_terminal(state->machine))
        return 0;
    if (!dt_write(state, 0u, code, bytes) ||
        core_machine_run(state->machine, budget, &result) !=
            (expect_fault ? TYPE_STATUS_FAULT : TYPE_STATUS_OK) ||
        result.reason != (expect_fault ? CORE_MACHINE_STOP_FAULT :
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) ||
        core_machine_get_cpu_diagnostic(state->machine, &diagnostic) !=
            TYPE_STATUS_OK) return 0;
    if (expect_fault && state->machine->cpu_profile >=
            CORE_MACHINE_CPU_PROFILE_80386 &&
        TYPE_GET_BIT(state->machine->executor_cpu.data.cr0, VCPU_CR0_PE) &&
        (expect_exception == VCPUINS_EXCEPT_TS ||
            expect_exception == VCPUINS_EXCEPT_NP ||
            expect_exception == VCPUINS_EXCEPT_SS ||
            expect_exception == VCPUINS_EXCEPT_GP)) {
        expect_exception = VCPUINS_EXCEPT_DF;
    }
    return expect_fault ? diagnostic.first_fault.valid &&
        TYPE_GET_BIT(diagnostic.first_fault.exception_mask, expect_exception) :
        !diagnostic.first_fault.valid;
}

static C_INT dt_run_fault_code(descriptor_system_machine *state,
    const type_unsigned_8 *code, STD_SIZE_T bytes, type_unsigned_32 exception,
    type_unsigned_32 exception_code)
{
    core_machine_cpu_diagnostic diagnostic;

    if (state != STD_NULL && state->machine != STD_NULL &&
        state->machine->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386 &&
        TYPE_GET_BIT(state->machine->executor_cpu.data.cr0, VCPU_CR0_PE) &&
        (exception == VCPUINS_EXCEPT_TS || exception == VCPUINS_EXCEPT_NP ||
            exception == VCPUINS_EXCEPT_SS || exception == VCPUINS_EXCEPT_GP)) {
        exception_code = 0u;
    }
    return dt_run(state, code, bytes, 1, exception) &&
        core_machine_get_cpu_diagnostic(state->machine, &diagnostic) ==
            TYPE_STATUS_OK &&
        diagnostic.first_fault.exception_code == exception_code;
}

static C_INT dt_run_one(descriptor_system_machine *state, const type_unsigned_8 *code,
    STD_SIZE_T bytes)
{
    const core_machine_run_budget budget = {1u, 0u};
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;

    return dt_write(state, 0u, code, bytes) &&
        core_machine_run(state->machine, budget, &result) == TYPE_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_BUDGET && result.executed == 1u &&
        core_machine_get_cpu_diagnostic(state->machine, &diagnostic) ==
            TYPE_STATUS_OK && !diagnostic.first_fault.valid;
}

static C_VOID dt_set_tables(descriptor_system_machine *state,
    type_unsigned_32 gdtr_base, type_unsigned_16 gdtr_limit, type_unsigned_32 idtr_base,
    type_unsigned_16 idtr_limit)
{
    state->machine->executor_cpu.data.gdtr.base = gdtr_base;
    state->machine->executor_cpu.data.gdtr.limit = gdtr_limit;
    state->machine->executor_cpu.data.idtr.base = idtr_base;
    state->machine->executor_cpu.data.idtr.limit = idtr_limit;
}

static C_INT dt_tables_equal(const t_cpu *first, const t_cpu *second)
{
    return first->data.gdtr.base == second->data.gdtr.base &&
        first->data.gdtr.limit == second->data.gdtr.limit &&
        first->data.idtr.base == second->data.idtr.base &&
        first->data.idtr.limit == second->data.idtr.limit;
}

static C_VOID dt_enter_protected(descriptor_system_machine *state,
    type_unsigned_8 cpl)
{
    t_cpu *cpu = &state->machine->executor_cpu;

    TYPE_SET_BIT(cpu->data.cr0, VCPU_CR0_PE);
    cpu->data.cs.selector = (type_unsigned_16)(0x0008u | cpl);
    cpu->data.cs.dpl = cpl;
    cpu->data.cs.base = 0u;
    cpu->data.cs.limit = 0xffffu;
    cpu->data.cs.flagValid = TYPE_TRUE;
    cpu->data.cs.sregtype = SREG_CODE;
    cpu->data.cs.seg.executable = TYPE_TRUE;
    cpu->data.ds.base = 0u;
    cpu->data.ds.limit = 0xffffu;
    cpu->data.ds.selector = (type_unsigned_16)(0x0010u | cpl);
    cpu->data.ds.flagValid = TYPE_TRUE;
    cpu->data.ds.sregtype = SREG_DATA;
    cpu->data.ds.seg.executable = TYPE_FALSE;
    cpu->data.ds.seg.data.writable = TYPE_TRUE;
    cpu->data.ds.dpl = cpl;
}

static C_VOID dt_enter_user_protected(descriptor_system_machine *state)
{
    dt_enter_protected(state, 3u);
    state->machine->executor_cpu.data.cs.selector = 0x004bu;
    state->machine->executor_cpu.data.ds.selector = 0x0053u;
}

static C_INT dt_sreg_equal(const t_cpu_data_sreg *first,
    const t_cpu_data_sreg *second)
{
    return first->flagValid == second->flagValid &&
        first->selector == second->selector &&
        first->sregtype == second->sregtype && first->base == second->base &&
        first->limit == second->limit && first->dpl == second->dpl &&
        first->sys.type == second->sys.type;
}

static C_VOID dt_seed_system_sreg(t_cpu_data_sreg *sreg,
    t_cpu_data_sreg_type type, type_unsigned_16 selector)
{
    STD_MEMSET(sreg, 0, sizeof(*sreg));
    sreg->flagValid = TYPE_TRUE;
    sreg->selector = selector;
    sreg->sregtype = type;
    sreg->base = 0x00000500u;
    sreg->limit = 0x000000ffu;
    sreg->dpl = 0u;
    sreg->sys.type = type == SREG_TR ? VCPU_DESC_SYS_TYPE_TSS_16_BUSY :
        VCPU_DESC_SYS_TYPE_LDT;
}

static C_INT dt_control_equal(const t_cpu *first, const t_cpu *second)
{
    return first->data.cr0 == second->data.cr0 &&
        first->data.cr2 == second->data.cr2 &&
        first->data.cr3 == second->data.cr3;
}

static C_INT dt_test_msw_and_control_registers(C_VOID)
{
    static const type_unsigned_8 smsw_register[] = {0x66u,0x0fu,0x01u,0xe0u,0xf4u};
    static const type_unsigned_8 smsw_memory[] = {0x66u,0x0fu,0x01u,0x26u,0x00u,0x02u,0xf4u};
    static const type_unsigned_8 lmsw_register[] = {0x0fu,0x01u,0xf0u,0xf4u};
    static const type_unsigned_8 clts[] = {0x0fu,0x06u,0xf4u};
    static const type_unsigned_8 mov_read[][5] = {
        {0x66u,0x0fu,0x20u,0xc0u,0xf4u},
        {0x66u,0x0fu,0x20u,0xd0u,0xf4u},
        {0x66u,0x0fu,0x20u,0xd8u,0xf4u}
    };
    static const type_unsigned_8 mov_write[][5] = {
        {0x66u,0x0fu,0x22u,0xc0u,0xf4u},
        {0x66u,0x0fu,0x22u,0xd0u,0xf4u},
        {0x66u,0x0fu,0x22u,0xd8u,0xf4u}
    };
    static const type_unsigned_8 reserved_read[] = {0x0fu,0x20u,0xe0u,0xf4u};
    static const type_unsigned_8 memory_read[] = {0x0fu,0x20u,0x00u,0xf4u};
    static const type_unsigned_8 protected_lmsw[] = {0x0fu,0x01u,0xf0u,0xf4u};
    static const type_unsigned_8 protected_mov[] = {0x0fu,0x20u,0xc0u,0xf4u};
    static const type_unsigned_8 invalid_cr0_write[] = {0x0fu,0x22u,0xc0u,0xf4u};
    type_unsigned_32 values[] = {0x00000001u, 0x12345678u, 0x00123000u};
    type_unsigned_16 observed = 0u;
    STD_SIZE_T index;
    {
        descriptor_system_machine state;
        C_INT failed = !dt_prepare(&state);

        if (!failed) {
            state.machine->executor_cpu.data.cr0 = 0x0000000cu;
            state.machine->executor_cpu.data.eax = 0xdeadbeefu;
            failed = !dt_run(&state, smsw_register, sizeof(smsw_register), 0, 0u) ||
                state.machine->executor_cpu.data.eax != 0xdead000cu ||
                state.machine->executor_cpu.data.cr0 != 0x0000000cu;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        descriptor_system_machine state;
        C_INT failed = !dt_prepare(&state);

        if (!failed) {
            dt_enter_user_protected(&state);
            TYPE_SET_BIT(state.machine->executor_cpu.data.cr0, VCPU_CR0_TS);
            state.machine->executor_cpu.data.eax = 0xdeadbeefu;
            failed = !dt_run_one(&state, smsw_register, sizeof(smsw_register)) ||
                state.machine->executor_cpu.data.eax != 0xdead0009u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        descriptor_system_machine state;
        C_INT failed = !dt_prepare(&state);

        if (!failed) {
            state.machine->executor_cpu.data.cr0 = 0x0000000cu;
            failed = !dt_run(&state, smsw_memory, sizeof(smsw_memory), 0, 0u) ||
                !dt_read(&state, DT_STORE_ADDRESS, (type_unsigned_8 *)&observed,
                    sizeof(observed)) || observed != 0x000cu;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        descriptor_system_machine state;
        C_INT failed = !dt_prepare(&state);

        if (!failed) {
            dt_enter_protected(&state, 0u);
            TYPE_SET_BIT(state.machine->executor_cpu.data.cr0, VCPU_CR0_TS);
            state.machine->executor_cpu.data.eax = 0xabcd0000u;
            failed = !dt_run_one(&state, lmsw_register, sizeof(lmsw_register)) ||
                state.machine->executor_cpu.data.cr0 !=
                    VCPU_CR0_PE;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    for (index = 0u; index < 2u; ++index) {
        descriptor_system_machine state;
        const core_machine_cpu_profile profile = index == 0u ?
            CORE_MACHINE_CPU_PROFILE_80286 : CORE_MACHINE_CPU_PROFILE_80386;
        const type_unsigned_32 flags = 0x00000246u;
        C_INT failed = !dt_prepare_profile(&state, profile, TYPE_FALSE);

        if (!failed) {
            state.machine->executor_cpu.data.cr0 = VCPU_CR0_TS;
            state.machine->executor_cpu.data.eflags = flags;
            failed = !dt_run(&state, clts, sizeof(clts), 0, 0u) ||
                state.machine->executor_cpu.data.cr0 != 0u ||
                state.machine->executor_cpu.data.eflags != flags;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        descriptor_system_machine state;
        const type_unsigned_32 flags = 0x00000246u;
        C_INT failed = !dt_prepare(&state);

        if (!failed) {
            dt_enter_protected(&state, 0u);
            TYPE_SET_BIT(state.machine->executor_cpu.data.cr0, VCPU_CR0_TS);
            state.machine->executor_cpu.data.eflags = flags;
            failed = !dt_run(&state, clts, sizeof(clts), 0, 0u) ||
                state.machine->executor_cpu.data.cr0 != VCPU_CR0_PE ||
                state.machine->executor_cpu.data.eflags != flags;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        descriptor_system_machine state;
        t_cpu before;
        t_cpu after;
        C_INT failed = !dt_prepare_profile(&state, CORE_MACHINE_CPU_PROFILE_80186, TYPE_FALSE);

        if (!failed) {
            state.machine->executor_cpu.data.cr0 = VCPU_CR0_TS;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed = !dt_run_fault_code(&state, clts, sizeof(clts),
                VCPUINS_EXCEPT_UD, 0u);
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !dt_control_equal(&before, &after);
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    for (index = 0u; index < 3u; ++index) {
        descriptor_system_machine state;
        C_INT failed = !dt_prepare(&state);

        if (!failed) {
            state.machine->executor_cpu.data.cr0 = values[0];
            state.machine->executor_cpu.data.cr2 = values[1];
            state.machine->executor_cpu.data.cr3 = values[2];
            state.machine->executor_cpu.data.eax = 0xdeadbeefu;
            failed = !dt_run(&state, mov_read[index], sizeof(mov_read[index]),
                0, 0u) || state.machine->executor_cpu.data.eax != values[index];
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    for (index = 0u; index < 3u; ++index) {
        descriptor_system_machine state;
        C_INT failed = !dt_prepare(&state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = values[index];
            failed = !dt_run(&state, mov_write[index], sizeof(mov_write[index]),
                0, 0u);
            if (index == 0u) failed |= state.machine->executor_cpu.data.cr0 != values[0];
            else if (index == 1u) failed |= state.machine->executor_cpu.data.cr2 != values[1];
            else failed |= state.machine->executor_cpu.data.cr3 != values[2];
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        const type_unsigned_8 *fault_code[] = {reserved_read, memory_read, invalid_cr0_write};
        STD_SIZE_T bytes[] = {sizeof(reserved_read), sizeof(memory_read),
            sizeof(invalid_cr0_write)};

        for (index = 0u; index < 3u; ++index) {
            descriptor_system_machine state;
            t_cpu before;
            t_cpu after;
            C_INT failed = !dt_prepare(&state);

            if (!failed) {
                state.machine->executor_cpu.data.cr0 = 0x00000001u;
                state.machine->executor_cpu.data.cr2 = values[1];
                state.machine->executor_cpu.data.cr3 = values[2];
                state.machine->executor_cpu.data.eax = 0x80000000u;
                before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
                failed = !dt_run_fault_code(&state, fault_code[index], bytes[index],
                    VCPUINS_EXCEPT_UD, 0u);
                after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
                failed |= !dt_control_equal(&before, &after) ||
                    after.data.eax != before.data.eax;
            }
            core_machine_destroy(state.machine);
            if (failed) return 0;
        }
    }
    for (index = 0u; index < 2u; ++index) {
        descriptor_system_machine state;
        t_cpu before;
        t_cpu after;
        const type_unsigned_8 *code = index == 0u ? protected_lmsw : protected_mov;
        C_INT failed = !dt_prepare(&state);

        if (!failed) {
            dt_enter_protected(&state, 3u);
            state.machine->executor_cpu.data.eax = 0x12345678u;
            state.machine->executor_cpu.data.cr2 = values[1];
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed = !dt_run_fault_code(&state, code, 4u, VCPUINS_EXCEPT_GP, 0u);
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !dt_control_equal(&before, &after) ||
                after.data.eax != before.data.eax;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        descriptor_system_machine state;
        t_cpu before;
        t_cpu after;
        C_INT failed = !dt_prepare(&state);

        if (!failed) {
            dt_enter_protected(&state, 3u);
            TYPE_SET_BIT(state.machine->executor_cpu.data.cr0, VCPU_CR0_TS);
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed = !dt_run_fault_code(&state, clts, sizeof(clts),
                VCPUINS_EXCEPT_GP, 0u);
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !dt_control_equal(&before, &after) ||
                after.data.eflags != before.data.eflags;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        static const type_unsigned_8 ignored_mod_read[] = {
            0x66u, 0x0fu, 0x20u, 0x80u, 0xf4u
        };
        static const type_unsigned_8 ignored_mod_write[] = {
            0x66u, 0x0fu, 0x22u, 0x80u, 0xf4u
        };
        descriptor_system_machine state;
        C_INT failed = !dt_prepare_early_80386(&state);

        if (!failed) {
            state.machine->executor_cpu.data.cr0 = 0x0000000cu;
            state.machine->executor_cpu.data.eax = 0xdeadbeefu;
            failed = !dt_run(&state, ignored_mod_read, sizeof(ignored_mod_read),
                0, 0u) || state.machine->executor_cpu.data.eax != 0x0000000cu;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
        failed = !dt_prepare_early_80386(&state);
        if (!failed) {
            state.machine->executor_cpu.data.eax = 0u;
            failed = !dt_run(&state, ignored_mod_write, sizeof(ignored_mod_write),
                0, 0u) || state.machine->executor_cpu.data.cr0 != 0u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT dt_install_selector_tables(descriptor_system_machine *state)
{
    static const type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0,0,0x9au,0,0,
        0xffu,0xffu,0,0,0,0x92u,0,0,
        0xffu,0,0,0x05u,0,0x82u,0,0,
        0x2bu,0,0,0x06u,0,0x81u,0,0,
        0xffu,0,0,0x07u,0,0x02u,0,0,
        0x2bu,0,0,0x08u,0,0x83u,0,0,
        0x2bu,0,0,0x09u,0,0x01u,0,0,
        0x67u,0,0,0x0au,0,0x89u,0,0,
        0xffu,0xffu,0,0,0,0xfau,0,0,
        0xffu,0xffu,0,0,0,0xf2u,0,0
    };

    dt_set_tables(state, DT_GDT_ADDRESS, (type_unsigned_16)(sizeof(gdt) - 1u),
        0u, 0u);
    return dt_write(state, DT_GDT_ADDRESS, gdt, sizeof(gdt));
}

static C_INT dt_test_selector_stores(C_VOID)
{
    static const type_unsigned_8 register_code[][5] = {
        {0x66u,0x0fu,0x00u,0xc0u,0xf4u},
        {0x66u,0x0fu,0x00u,0xc8u,0xf4u}
    };
    static const type_unsigned_8 memory_code[][6] = {
        {0x0fu,0x00u,0x06u,0x00u,0x02u,0xf4u},
        {0x0fu,0x00u,0x0eu,0x00u,0x02u,0xf4u}
    };
    const type_unsigned_16 selectors[] = { DT_LDT_SELECTOR, DT_TSS16_SELECTOR };
    STD_SIZE_T index;

    for (index = 0u; index < 2u; ++index) {
        descriptor_system_machine state;
        C_INT failed = !dt_prepare(&state);

        if (!failed) {
            failed = !dt_install_selector_tables(&state);
            dt_enter_user_protected(&state);
            dt_seed_system_sreg(&state.machine->executor_cpu.data.ldtr,
                SREG_LDTR, DT_LDT_SELECTOR);
            dt_seed_system_sreg(&state.machine->executor_cpu.data.tr,
                SREG_TR, DT_TSS16_SELECTOR);
            state.machine->executor_cpu.data.eax = 0xdeadbeefu;
            failed |= !dt_run_one(&state, register_code[index],
                sizeof(register_code[index])) ||
                state.machine->executor_cpu.data.eax !=
                    (0xdead0000u | selectors[index]);
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    for (index = 0u; index < 2u; ++index) {
        descriptor_system_machine state;
        type_unsigned_16 observed = 0u;
        C_INT failed = !dt_prepare(&state);

        if (!failed) {
            dt_enter_protected(&state, 0u);
            dt_seed_system_sreg(&state.machine->executor_cpu.data.ldtr,
                SREG_LDTR, DT_LDT_SELECTOR);
            dt_seed_system_sreg(&state.machine->executor_cpu.data.tr,
                SREG_TR, DT_TSS16_SELECTOR);
            failed = !dt_run(&state, memory_code[index], sizeof(memory_code[index]),
                0, 0u) || !dt_read(&state, DT_STORE_ADDRESS,
                    (type_unsigned_8 *)&observed, sizeof(observed)) ||
                observed != selectors[index];
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT dt_test_selector_loads(C_VOID)
{
    static const type_unsigned_8 lldt[] = {0x0fu,0x00u,0xd0u,0xf4u};
    static const type_unsigned_8 ltr[] = {0x0fu,0x00u,0xd8u,0xf4u};
    static const type_unsigned_8 memory_load_code[][7] = {
        {0x66u,0x0fu,0x00u,0x16u,0x40u,0x02u,0xf4u},
        {0x66u,0x0fu,0x00u,0x1eu,0x40u,0x02u,0xf4u}
    };
    static const type_unsigned_8 real_code[][4] = {
        {0x0fu,0x00u,0xc0u,0u}, {0x0fu,0x00u,0xc8u,0u},
        {0x0fu,0x00u,0xd0u,0u}, {0x0fu,0x00u,0xd8u,0u}
    };
    static const type_unsigned_16 lldt_selectors[] = {
        0x0004u, DT_TSS16_SELECTOR, DT_LDT_NOT_PRESENT_SELECTOR
    };
    static const type_unsigned_32 lldt_exceptions[] = {
        VCPUINS_EXCEPT_GP, VCPUINS_EXCEPT_GP, VCPUINS_EXCEPT_NP
    };
    static const type_unsigned_16 ltr_selectors[] = {
        0x0000u, 0x0004u, DT_LDT_SELECTOR, DT_TSS16_BUSY_SELECTOR,
        DT_TSS16_NOT_PRESENT_SELECTOR
    };
    static const type_unsigned_32 ltr_exceptions[] = {
        VCPUINS_EXCEPT_GP, VCPUINS_EXCEPT_GP, VCPUINS_EXCEPT_GP,
        VCPUINS_EXCEPT_GP, VCPUINS_EXCEPT_NP
    };
    STD_SIZE_T index;

    for (index = 0u; index < 4u; ++index) {
        descriptor_system_machine state;
        t_cpu before;
        t_cpu after;
        C_INT failed = !dt_prepare(&state);

        if (!failed) {
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed = !dt_run_fault_code(&state, real_code[index], 3u,
                VCPUINS_EXCEPT_UD, 0u);
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !dt_sreg_equal(&before.data.ldtr, &after.data.ldtr) ||
                !dt_sreg_equal(&before.data.tr, &after.data.tr);
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        descriptor_system_machine state;
        C_INT failed = !dt_prepare(&state);

        if (!failed) {
            dt_enter_protected(&state, 0u);
            failed = !dt_install_selector_tables(&state);
            state.machine->executor_cpu.data.eax = 0xffff0000u | DT_LDT_SELECTOR;
            failed |= !dt_run(&state, lldt, sizeof(lldt), 0, 0u) ||
                !state.machine->executor_cpu.data.ldtr.flagValid ||
                state.machine->executor_cpu.data.ldtr.selector != DT_LDT_SELECTOR ||
                state.machine->executor_cpu.data.ldtr.base != 0x00000500u ||
                state.machine->executor_cpu.data.ldtr.limit != 0x000000ffu ||
                state.machine->executor_cpu.data.ldtr.sys.type !=
                    VCPU_DESC_SYS_TYPE_LDT;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        descriptor_system_machine state;
        C_INT failed = !dt_prepare(&state);

        if (!failed) {
            dt_enter_protected(&state, 0u);
            failed = !dt_install_selector_tables(&state);
            state.machine->executor_cpu.data.eax = 0xffff0000u;
            failed |= !dt_run(&state, lldt, sizeof(lldt), 0, 0u) ||
                state.machine->executor_cpu.data.ldtr.flagValid ||
                state.machine->executor_cpu.data.ldtr.selector != 0u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    for (index = 0u; index < 2u; ++index) {
        descriptor_system_machine state;
        const type_unsigned_16 selector = index == 0u ? DT_LDT_SELECTOR :
            DT_TSS16_SELECTOR;
        C_INT failed = !dt_prepare(&state);

        if (!failed) {
            dt_enter_protected(&state, 0u);
            failed = !dt_install_selector_tables(&state) || !dt_write(&state,
                DT_LOAD_ADDRESS, (const type_unsigned_8 *)&selector, sizeof(selector)) ||
                !dt_run(&state, memory_load_code[index],
                    sizeof(memory_load_code[index]), 0, 0u);
            if (index == 0u) {
                failed |= !state.machine->executor_cpu.data.ldtr.flagValid ||
                    state.machine->executor_cpu.data.ldtr.selector != selector;
            } else {
                failed |= !state.machine->executor_cpu.data.tr.flagValid ||
                    state.machine->executor_cpu.data.tr.selector != selector;
            }
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    for (index = 0u; index < 3u; ++index) {
        descriptor_system_machine state;
        t_cpu before;
        t_cpu after;
        C_INT failed = !dt_prepare(&state);

        if (!failed) {
            dt_enter_protected(&state, 0u);
            failed = !dt_install_selector_tables(&state);
            dt_seed_system_sreg(&state.machine->executor_cpu.data.ldtr,
                SREG_LDTR, DT_LDT_SELECTOR);
            state.machine->executor_cpu.data.eax = 0xbeef0000u |
                lldt_selectors[index];
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !dt_run_fault_code(&state, lldt, sizeof(lldt),
                lldt_exceptions[index], lldt_selectors[index]);
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !dt_sreg_equal(&before.data.ldtr, &after.data.ldtr);
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        descriptor_system_machine state;
        type_unsigned_8 access = 0u;
        C_INT failed = !dt_prepare(&state);

        if (!failed) {
            dt_enter_protected(&state, 0u);
            failed = !dt_install_selector_tables(&state);
            state.machine->executor_cpu.data.eax = 0xffff0000u | DT_TSS16_SELECTOR;
            failed |= !dt_run(&state, ltr, sizeof(ltr), 0, 0u) ||
                !state.machine->executor_cpu.data.tr.flagValid ||
                state.machine->executor_cpu.data.tr.selector != DT_TSS16_SELECTOR ||
                state.machine->executor_cpu.data.tr.base != 0x00000600u ||
                state.machine->executor_cpu.data.tr.limit != 0x0000002bu ||
                state.machine->executor_cpu.data.tr.sys.type !=
                    VCPU_DESC_SYS_TYPE_TSS_16_BUSY ||
                !dt_read(&state, DT_GDT_ADDRESS + DT_TSS16_SELECTOR + 5u,
                    &access, sizeof(access)) || access != 0x83u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        descriptor_system_machine state;
        type_unsigned_8 access = 0u;
        C_INT failed = !dt_prepare(&state);

        if (!failed) {
            dt_enter_protected(&state, 0u);
            failed = !dt_install_selector_tables(&state);
            state.machine->executor_cpu.data.eax = 0xffff0000u |
                DT_TSS32_SELECTOR;
            failed |= !dt_run(&state, ltr, sizeof(ltr), 0, 0u) ||
                !state.machine->executor_cpu.data.tr.flagValid ||
                state.machine->executor_cpu.data.tr.selector != DT_TSS32_SELECTOR ||
                state.machine->executor_cpu.data.tr.base != 0x00000a00u ||
                state.machine->executor_cpu.data.tr.limit != 0x00000067u ||
                state.machine->executor_cpu.data.tr.sys.type !=
                    VCPU_DESC_SYS_TYPE_TSS_32_BUSY ||
                !dt_read(&state, DT_GDT_ADDRESS + DT_TSS32_SELECTOR + 5u,
                    &access, sizeof(access)) || access != 0x8bu;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    for (index = 0u; index < 5u; ++index) {
        descriptor_system_machine state;
        t_cpu before;
        t_cpu after;
        type_unsigned_8 before_access = 0u;
        type_unsigned_8 after_access = 0u;
        C_INT failed = !dt_prepare(&state);

        if (!failed) {
            dt_enter_protected(&state, 0u);
            failed = !dt_install_selector_tables(&state);
            dt_seed_system_sreg(&state.machine->executor_cpu.data.tr, SREG_TR,
                DT_TSS16_SELECTOR);
            state.machine->executor_cpu.data.eax = 0xbeef0000u |
                ltr_selectors[index];
            if (ltr_selectors[index] != 0u &&
                (ltr_selectors[index] & 0x0004u) == 0u) failed |= !dt_read_private(&state,
                DT_GDT_ADDRESS + ltr_selectors[index] + 5u, &before_access,
                sizeof(before_access));
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !dt_run_fault_code(&state, ltr, sizeof(ltr),
                ltr_exceptions[index], ltr_selectors[index]);
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            if (ltr_selectors[index] != 0u &&
                (ltr_selectors[index] & 0x0004u) == 0u) failed |= !dt_read_private(&state,
                DT_GDT_ADDRESS + ltr_selectors[index] + 5u, &after_access,
                sizeof(after_access)) || before_access != after_access;
            failed |= !dt_sreg_equal(&before.data.tr, &after.data.tr);
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT dt_test_store_layout(C_VOID)
{
    static const type_unsigned_8 code[][7] = {
        {0x0fu, 0x01u, 0x06u, 0x00u, 0x02u, 0xf4u, 0u},
        {0x0fu, 0x01u, 0x0eu, 0x00u, 0x02u, 0xf4u, 0u},
        {0x66u, 0x0fu, 0x01u, 0x06u, 0x00u, 0x02u, 0xf4u},
        {0x66u, 0x0fu, 0x01u, 0x0eu, 0x00u, 0x02u, 0xf4u}
    };
    static const type_unsigned_8 expected[][6] = {
        {0x34u, 0x12u, 0x12u, 0xdeu, 0xbcu, 0u},
        {0x78u, 0x56u, 0x78u, 0x56u, 0x34u, 0u},
        {0x34u, 0x12u, 0x12u, 0xdeu, 0xbcu, 0x7au},
        {0x78u, 0x56u, 0x78u, 0x56u, 0x34u, 0x12u}
    };
    STD_SIZE_T index;

    for (index = 0u; index < 4u; ++index) {
        descriptor_system_machine state;
        type_unsigned_8 observed[6] = {0};
        type_unsigned_8 clear[6] = {0};
        C_INT failed = !dt_prepare(&state);

        if (!failed) {
            dt_set_tables(&state, 0x7abcde12u, 0x1234u, 0x12345678u, 0x5678u);
            failed = !dt_write(&state, DT_STORE_ADDRESS, clear, sizeof(clear)) ||
                !dt_run(&state, code[index], sizeof(code[index]) - (index < 2u),
                    0, 0u) || !dt_read(&state, DT_STORE_ADDRESS, observed,
                        sizeof(observed)) || STD_MEMCMP(observed, expected[index],
                            sizeof(observed)) != 0;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT dt_test_protected_stores(C_VOID)
{
    static const type_unsigned_8 code[][7] = {
        {0x66u, 0x0fu, 0x01u, 0x06u, 0x00u, 0x02u, 0xf4u},
        {0x66u, 0x0fu, 0x01u, 0x0eu, 0x00u, 0x02u, 0xf4u}
    };
    static const type_unsigned_8 expected[][6] = {
        {0x34u, 0x12u, 0x12u, 0xdeu, 0xbcu, 0x7au},
        {0x78u, 0x56u, 0x78u, 0x56u, 0x34u, 0x12u}
    };
    STD_SIZE_T index;

    for (index = 0u; index < 2u; ++index) {
        descriptor_system_machine state;
        type_unsigned_8 observed[6] = {0};
        C_INT failed = !dt_prepare(&state);

        if (!failed) {
            dt_set_tables(&state, 0x7abcde12u, 0x1234u, 0x12345678u, 0x5678u);
            dt_enter_protected(&state, 0u);
            failed = !dt_run(&state, code[index], sizeof(code[index]), 0, 0u) ||
                !dt_read(&state, DT_STORE_ADDRESS, observed, sizeof(observed)) ||
                STD_MEMCMP(observed, expected[index], sizeof(observed)) != 0;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT dt_test_load_layout(C_VOID)
{
    static const type_unsigned_8 code[][7] = {
        {0x0fu, 0x01u, 0x16u, 0x40u, 0x02u, 0xf4u, 0u},
        {0x0fu, 0x01u, 0x1eu, 0x40u, 0x02u, 0xf4u, 0u},
        {0x66u, 0x0fu, 0x01u, 0x16u, 0x40u, 0x02u, 0xf4u},
        {0x66u, 0x0fu, 0x01u, 0x1eu, 0x40u, 0x02u, 0xf4u}
    };
    static const type_unsigned_8 source[] = {0xbcu, 0x9au, 0x78u, 0x56u, 0x34u, 0x12u};
    STD_SIZE_T index;

    for (index = 0u; index < 4u; ++index) {
        descriptor_system_machine state;
        t_cpu after;
        C_INT failed = !dt_prepare(&state);

        if (!failed) {
            failed = !dt_write(&state, DT_LOAD_ADDRESS, source, sizeof(source)) ||
                !dt_run(&state, code[index], sizeof(code[index]) - (index < 2u),
                    0, 0u);
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            if (index == 0u || index == 2u) {
                failed |= after.data.gdtr.limit != 0x9abcu ||
                    after.data.gdtr.base != (index == 0u ? 0x00345678u : 0x12345678u);
            } else {
                failed |= after.data.idtr.limit != 0x9abcu ||
                    after.data.idtr.base != (index == 1u ? 0x00345678u : 0x12345678u);
            }
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    for (index = 0u; index < 2u; ++index) {
        descriptor_system_machine state;
        t_cpu after;
        C_INT failed = !dt_prepare(&state);

        if (!failed) {
            dt_enter_protected(&state, 0u);
            failed = !dt_write(&state, DT_LOAD_ADDRESS, source, sizeof(source)) ||
                !dt_run(&state, code[index], sizeof(code[index]) - 1u, 0, 0u);
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            if (index == 0u) {
                failed |= after.data.gdtr.limit != 0x9abcu ||
                    after.data.gdtr.base != 0x00345678u;
            } else {
                failed |= after.data.idtr.limit != 0x9abcu ||
                    after.data.idtr.base != 0x00345678u;
            }
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT dt_test_register_and_privilege_faults(C_VOID)
{
    static const type_unsigned_8 register_code[][4] = {
        {0x0fu, 0x01u, 0xc0u, 0u}, {0x0fu, 0x01u, 0xc8u, 0u},
        {0x0fu, 0x01u, 0xd0u, 0u}, {0x0fu, 0x01u, 0xd8u, 0u}
    };
    static const type_unsigned_8 load_code[][6] = {
        {0x0fu, 0x01u, 0x16u, 0x40u, 0x02u, 0xf4u},
        {0x0fu, 0x01u, 0x1eu, 0x40u, 0x02u, 0xf4u}
    };
    static const type_unsigned_8 source[] = {0xbcu, 0x9au, 0x78u, 0x56u, 0x34u, 0x12u};
    STD_SIZE_T index;

    for (index = 0u; index < 4u; ++index) {
        descriptor_system_machine state;
        t_cpu before;
        t_cpu after;
        C_INT failed = !dt_prepare(&state);

        if (!failed) {
            dt_set_tables(&state, 0x11112222u, 0x3333u, 0x44445555u, 0x6666u);
            failed = !test_core_machine_fixture_preflight_real_ud_terminal(
                state.machine);
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !dt_run(&state, register_code[index], 3u, 1,
                VCPUINS_EXCEPT_UD);
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !dt_tables_equal(&before, &after);
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    for (index = 0u; index < 2u; ++index) {
        descriptor_system_machine state;
        t_cpu before;
        t_cpu after;
        C_INT failed = !dt_prepare(&state);

        if (!failed) {
            dt_set_tables(&state, 0x11112222u, 0x3333u, 0u, 0u);
            dt_enter_protected(&state, 3u);
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed = !dt_write(&state, DT_LOAD_ADDRESS, source, sizeof(source)) ||
                !dt_run(&state, load_code[index], sizeof(load_code[index]), 1,
                    VCPUINS_EXCEPT_GP);
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !dt_tables_equal(&before, &after);
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT dt_test_memory_faults_preserve_tables(C_VOID)
{
    static const type_unsigned_8 store_code[][6] = {
        {0x0fu, 0x01u, 0x06u, 0x00u, 0x02u, 0xf4u},
        {0x0fu, 0x01u, 0x0eu, 0x00u, 0x02u, 0xf4u}
    };
    static const type_unsigned_8 load_code[][6] = {
        {0x0fu, 0x01u, 0x16u, 0x00u, 0x02u, 0xf4u},
        {0x0fu, 0x01u, 0x1eu, 0x00u, 0x02u, 0xf4u}
    };
    STD_SIZE_T index;

    for (index = 0u; index < 2u; ++index) {
        descriptor_system_machine state;
        t_cpu before;
        t_cpu after;
        C_INT failed = !dt_prepare(&state);

        if (!failed) {
            dt_set_tables(&state, 0x11112222u, 0x3333u, 0u, 0u);
            dt_enter_protected(&state, 0u);
            state.machine->executor_cpu.data.ds.limit = 0x01ffu;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed = !dt_run(&state, store_code[index], sizeof(store_code[index]), 1,
                VCPUINS_EXCEPT_GP);
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !dt_tables_equal(&before, &after);
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }

    for (index = 0u; index < 2u; ++index) {
        descriptor_system_machine state;
        t_cpu before;
        t_cpu after;
        C_INT failed = !dt_prepare(&state);

        if (!failed) {
            dt_set_tables(&state, 0x11112222u, 0x3333u, 0u, 0u);
            dt_enter_protected(&state, 0u);
            state.machine->executor_cpu.data.ds.limit = 0x01ffu;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed = !dt_run(&state, load_code[index], sizeof(load_code[index]), 1,
                VCPUINS_EXCEPT_GP);
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !dt_tables_equal(&before, &after);
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}


static C_INT dt_test_c7_segment_override_real_mode(C_VOID)
{
    static const type_unsigned_8 code[] = {
        0x26u, 0xc7u, 0x47u, 0x02u, 0xffu, 0xffu, 0xf4u
    };
    descriptor_system_machine state;
    type_unsigned_16 observed = 0u;
    C_INT failed = !dt_prepare(&state);

    if (!failed) {
        state.machine->executor_cpu.data.ebx = 0u;
        failed = !dt_run(&state, code, sizeof(code), 0, 0u) ||
            !dt_read(&state, 2u, (type_unsigned_8 *)&observed, sizeof(observed)) ||
            observed != 0xffffu;
    }
    core_machine_destroy(state.machine);
    return !failed;
}
static C_INT dt_real_data_cache(const t_cpu_data_sreg *sreg,
    type_unsigned_16 selector, t_cpu_data_sreg_type type)
{
    return sreg->flagValid && sreg->selector == selector &&
        sreg->base == (type_unsigned_32)selector << 4u &&
        sreg->limit == 0xffffu && sreg->dpl == 0u &&
        sreg->sregtype == type && sreg->seg.accessed &&
        !sreg->seg.executable && sreg->seg.data.writable &&
        !sreg->seg.data.big && !sreg->seg.data.expdown;
}
static C_INT dt_test_leave_protected_mode(C_VOID)
{
    static const type_unsigned_8 code[] = {
        0x0fu, 0x22u, 0xc0u, 0xeau, 0x0au, 0x00u, 0x00u, 0x00u,
        0x00u, 0x00u, 0xbbu, 0x48u, 0x00u, 0x8eu, 0xc3u, 0x8eu,
        0xd3u, 0x8eu, 0xdbu, 0xf4u
    };
    descriptor_system_machine state;
    C_INT failed = !dt_prepare(&state);

    if (!failed) {
        dt_enter_protected(&state, 0u);
        state.machine->executor_cpu.data.cs.seg.exec.defsize = TYPE_TRUE;
        state.machine->executor_cpu.data.ds.seg.data.big = TYPE_TRUE;
        state.machine->executor_cpu.data.es.seg.data.big = TYPE_TRUE;
        state.machine->executor_cpu.data.ss.seg.data.big = TYPE_TRUE;
        state.machine->executor_cpu.data.eax = 0u;
        failed = !dt_run(&state, code, sizeof(code), 0, 0u) ||
            state.machine->executor_cpu.data.cr0 != 0u ||
            state.machine->executor_cpu.data.cs.selector != 0u ||
            state.machine->executor_cpu.data.cs.base != 0u ||
            state.machine->executor_cpu.data.cs.limit != 0xffffu ||
            !state.machine->executor_cpu.data.cs.flagValid ||
            !state.machine->executor_cpu.data.cs.seg.accessed ||
            !state.machine->executor_cpu.data.cs.seg.executable ||
            state.machine->executor_cpu.data.cs.seg.exec.defsize ||
            state.machine->executor_cpu.data.cs.seg.exec.conform ||
            !state.machine->executor_cpu.data.cs.seg.exec.readable ||
            state.machine->executor_cpu.data.ebx != 0x00000048u ||
            !dt_real_data_cache(&state.machine->executor_cpu.data.es, 0x0048u,
                SREG_DATA) ||
            !dt_real_data_cache(&state.machine->executor_cpu.data.ss, 0x0048u,
                SREG_STACK) ||
            !dt_real_data_cache(&state.machine->executor_cpu.data.ds, 0x0048u,
                SREG_DATA);
    }
    core_machine_destroy(state.machine);
    return !failed;
}
C_INT main(C_VOID)
{
    if (!dt_test_store_layout() || !dt_test_protected_stores() ||
        !dt_test_load_layout() || !dt_test_register_and_privilege_faults() ||
        !dt_test_memory_faults_preserve_tables() || !dt_test_selector_stores() ||
        !dt_test_selector_loads() || !dt_test_msw_and_control_registers() ||
        !dt_test_c7_segment_override_real_mode() || !dt_test_leave_protected_mode()) return 1;
    STD_PRINTF("M5:T304:DESCRIPTOR-SYSTEM:OK\n");
    return 0;
}
