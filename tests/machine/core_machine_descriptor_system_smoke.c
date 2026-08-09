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
    STD_NULL,
    STD_NULL
};

static C_INT dt_prepare_profile(descriptor_system_machine *state,
    core_machine_cpu_profile profile)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    if (state == STD_NULL) return 0;
    STD_MEMSET(state, 0, sizeof(*state));
    if (core_machine_create(&config, &state->machine) != TYPE_STATUS_OK ||
        core_machine_bind_execution_provider(state->machine, &dt_provider, state) !=
            TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(state->machine) != TYPE_STATUS_OK ||
        core_machine_reset(state->machine) != TYPE_STATUS_OK) {
        core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 0;
    }
    return 1;
}

static C_INT dt_prepare(descriptor_system_machine *state)
{
    return dt_prepare_profile(state, CORE_MACHINE_CPU_PROFILE_80386);
}

static C_INT dt_write(descriptor_system_machine *state, uint32_t address,
    const uint8_t *data, STD_SIZE_T bytes)
{
    return state != STD_NULL && state->machine != STD_NULL &&
        core_machine_memory_write(state->machine, address, data, bytes) ==
            TYPE_STATUS_OK;
}

static C_INT dt_read(descriptor_system_machine *state, uint32_t address,
    uint8_t *data, STD_SIZE_T bytes)
{
    return state != STD_NULL && state->machine != STD_NULL &&
        core_machine_memory_read(state->machine, address, data, bytes) ==
            TYPE_STATUS_OK;
}

static C_INT dt_read_private(descriptor_system_machine *state, uint32_t address,
    uint8_t *data, STD_SIZE_T bytes)
{
    return state != STD_NULL && state->machine != STD_NULL &&
        core_machine_memory_read_physical(&state->machine->executor_memory,
            address, (type_virtual_address)data, bytes) == TYPE_STATUS_OK;
}

static C_INT dt_run(descriptor_system_machine *state, const uint8_t *code,
    STD_SIZE_T bytes, C_INT expect_fault, uint32_t expect_exception)
{
    const core_machine_run_budget budget = {32u, 0u};
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;

    if (!dt_write(state, 0u, code, bytes) ||
        core_machine_run(state->machine, budget, &result) !=
            (expect_fault ? TYPE_STATUS_FAULT : TYPE_STATUS_OK) ||
        result.reason != (expect_fault ? CORE_MACHINE_STOP_FAULT :
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) ||
        core_machine_get_cpu_diagnostic(state->machine, &diagnostic) !=
            TYPE_STATUS_OK) return 0;
    return expect_fault ? diagnostic.first_fault.valid &&
        TYPE_GET_BIT(diagnostic.first_fault.exception_mask, expect_exception) :
        !diagnostic.first_fault.valid;
}

static C_INT dt_run_fault_code(descriptor_system_machine *state,
    const uint8_t *code, STD_SIZE_T bytes, uint32_t exception,
    uint32_t exception_code)
{
    core_machine_cpu_diagnostic diagnostic;

    return dt_run(state, code, bytes, 1, exception) &&
        core_machine_get_cpu_diagnostic(state->machine, &diagnostic) ==
            TYPE_STATUS_OK &&
        diagnostic.first_fault.exception_code == exception_code;
}

static C_INT dt_run_one(descriptor_system_machine *state, const uint8_t *code,
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
    uint32_t gdtr_base, uint16_t gdtr_limit, uint32_t idtr_base,
    uint16_t idtr_limit)
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
    uint8_t cpl)
{
    t_cpu *cpu = &state->machine->executor_cpu;

    TYPE_SET_BIT(cpu->data.cr0, VCPU_CR0_PE);
    cpu->data.cs.selector = (uint16_t)(0x0008u | cpl);
    cpu->data.cs.dpl = cpl;
    cpu->data.cs.base = 0u;
    cpu->data.cs.limit = 0xffffu;
    cpu->data.cs.flagValid = TYPE_TRUE;
    cpu->data.cs.sregtype = SREG_CODE;
    cpu->data.cs.seg.executable = TYPE_TRUE;
    cpu->data.ds.base = 0u;
    cpu->data.ds.limit = 0xffffu;
    cpu->data.ds.selector = (uint16_t)(0x0010u | cpl);
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
    t_cpu_data_sreg_type type, uint16_t selector)
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
    static const uint8_t smsw_register[] = {0x66u,0x0fu,0x01u,0xe0u,0xf4u};
    static const uint8_t smsw_memory[] = {0x66u,0x0fu,0x01u,0x26u,0x00u,0x02u,0xf4u};
    static const uint8_t lmsw_register[] = {0x0fu,0x01u,0xf0u,0xf4u};
    static const uint8_t clts[] = {0x0fu,0x06u,0xf4u};
    static const uint8_t mov_read[][5] = {
        {0x66u,0x0fu,0x20u,0xc0u,0xf4u},
        {0x66u,0x0fu,0x20u,0xd0u,0xf4u},
        {0x66u,0x0fu,0x20u,0xd8u,0xf4u}
    };
    static const uint8_t mov_write[][5] = {
        {0x66u,0x0fu,0x22u,0xc0u,0xf4u},
        {0x66u,0x0fu,0x22u,0xd0u,0xf4u},
        {0x66u,0x0fu,0x22u,0xd8u,0xf4u}
    };
    static const uint8_t reserved_read[] = {0x0fu,0x20u,0xe0u,0xf4u};
    static const uint8_t memory_read[] = {0x0fu,0x20u,0x00u,0xf4u};
    static const uint8_t protected_lmsw[] = {0x0fu,0x01u,0xf0u,0xf4u};
    static const uint8_t protected_mov[] = {0x0fu,0x20u,0xc0u,0xf4u};
    static const uint8_t invalid_cr0_write[] = {0x0fu,0x22u,0xc0u,0xf4u};
    uint32_t values[] = {0x00000001u, 0x12345678u, 0x00123000u};
    STD_SIZE_T index;
    {
        descriptor_system_machine state;
        uint16_t observed = 0u;
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
        uint16_t observed = 0u;
        C_INT failed = !dt_prepare(&state);

        if (!failed) {
            state.machine->executor_cpu.data.cr0 = 0x0000000cu;
            failed = !dt_run(&state, smsw_memory, sizeof(smsw_memory), 0, 0u) ||
                !dt_read(&state, DT_STORE_ADDRESS, (uint8_t *)&observed,
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
        const uint32_t flags = 0x00000246u;
        C_INT failed = !dt_prepare_profile(&state, profile);

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
        const uint32_t flags = 0x00000246u;
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
        C_INT failed = !dt_prepare_profile(&state, CORE_MACHINE_CPU_PROFILE_80186);

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
        const uint8_t *fault_code[] = {reserved_read, memory_read, invalid_cr0_write};
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
        const uint8_t *code = index == 0u ? protected_lmsw : protected_mov;
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
    return 1;
}

static C_INT dt_install_selector_tables(descriptor_system_machine *state)
{
    static const uint8_t gdt[] = {
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

    dt_set_tables(state, DT_GDT_ADDRESS, (uint16_t)(sizeof(gdt) - 1u),
        0u, 0u);
    return dt_write(state, DT_GDT_ADDRESS, gdt, sizeof(gdt));
}

static C_INT dt_test_selector_stores(C_VOID)
{
    static const uint8_t register_code[][5] = {
        {0x66u,0x0fu,0x00u,0xc0u,0xf4u},
        {0x66u,0x0fu,0x00u,0xc8u,0xf4u}
    };
    static const uint8_t memory_code[][6] = {
        {0x0fu,0x00u,0x06u,0x00u,0x02u,0xf4u},
        {0x0fu,0x00u,0x0eu,0x00u,0x02u,0xf4u}
    };
    const uint16_t selectors[] = { DT_LDT_SELECTOR, DT_TSS16_SELECTOR };
    STD_SIZE_T index;

    for (index = 0u; index < 2u; ++index) {
        descriptor_system_machine state;
        uint16_t observed = 0u;
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
        uint16_t observed = 0u;
        C_INT failed = !dt_prepare(&state);

        if (!failed) {
            dt_enter_protected(&state, 0u);
            dt_seed_system_sreg(&state.machine->executor_cpu.data.ldtr,
                SREG_LDTR, DT_LDT_SELECTOR);
            dt_seed_system_sreg(&state.machine->executor_cpu.data.tr,
                SREG_TR, DT_TSS16_SELECTOR);
            failed = !dt_run(&state, memory_code[index], sizeof(memory_code[index]),
                0, 0u) || !dt_read(&state, DT_STORE_ADDRESS,
                    (uint8_t *)&observed, sizeof(observed)) ||
                observed != selectors[index];
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT dt_test_selector_loads(C_VOID)
{
    static const uint8_t lldt[] = {0x0fu,0x00u,0xd0u,0xf4u};
    static const uint8_t ltr[] = {0x0fu,0x00u,0xd8u,0xf4u};
    static const uint8_t memory_load_code[][7] = {
        {0x66u,0x0fu,0x00u,0x16u,0x40u,0x02u,0xf4u},
        {0x66u,0x0fu,0x00u,0x1eu,0x40u,0x02u,0xf4u}
    };
    static const uint8_t real_code[][4] = {
        {0x0fu,0x00u,0xc0u,0u}, {0x0fu,0x00u,0xc8u,0u},
        {0x0fu,0x00u,0xd0u,0u}, {0x0fu,0x00u,0xd8u,0u}
    };
    static const uint16_t lldt_selectors[] = {
        0x0004u, DT_TSS16_SELECTOR, DT_LDT_NOT_PRESENT_SELECTOR
    };
    static const uint32_t lldt_exceptions[] = {
        VCPUINS_EXCEPT_GP, VCPUINS_EXCEPT_GP, VCPUINS_EXCEPT_NP
    };
    static const uint16_t ltr_selectors[] = {
        0x0000u, 0x0004u, DT_LDT_SELECTOR, DT_TSS16_BUSY_SELECTOR,
        DT_TSS16_NOT_PRESENT_SELECTOR, DT_TSS32_SELECTOR
    };
    static const uint32_t ltr_exceptions[] = {
        VCPUINS_EXCEPT_GP, VCPUINS_EXCEPT_GP, VCPUINS_EXCEPT_GP,
        VCPUINS_EXCEPT_GP, VCPUINS_EXCEPT_NP, VCPUINS_EXCEPT_GP
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
        const uint16_t selector = index == 0u ? DT_LDT_SELECTOR :
            DT_TSS16_SELECTOR;
        C_INT failed = !dt_prepare(&state);

        if (!failed) {
            dt_enter_protected(&state, 0u);
            failed = !dt_install_selector_tables(&state) || !dt_write(&state,
                DT_LOAD_ADDRESS, (const uint8_t *)&selector, sizeof(selector)) ||
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
        uint8_t access = 0u;
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
    for (index = 0u; index < 6u; ++index) {
        descriptor_system_machine state;
        t_cpu before;
        t_cpu after;
        uint8_t before_access = 0u;
        uint8_t after_access = 0u;
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
    static const uint8_t code[][7] = {
        {0x0fu, 0x01u, 0x06u, 0x00u, 0x02u, 0xf4u, 0u},
        {0x0fu, 0x01u, 0x0eu, 0x00u, 0x02u, 0xf4u, 0u},
        {0x66u, 0x0fu, 0x01u, 0x06u, 0x00u, 0x02u, 0xf4u},
        {0x66u, 0x0fu, 0x01u, 0x0eu, 0x00u, 0x02u, 0xf4u}
    };
    static const uint8_t expected[][6] = {
        {0x34u, 0x12u, 0x12u, 0xdeu, 0xbcu, 0u},
        {0x78u, 0x56u, 0x78u, 0x56u, 0x34u, 0u},
        {0x34u, 0x12u, 0x12u, 0xdeu, 0xbcu, 0x7au},
        {0x78u, 0x56u, 0x78u, 0x56u, 0x34u, 0x12u}
    };
    STD_SIZE_T index;

    for (index = 0u; index < 4u; ++index) {
        descriptor_system_machine state;
        uint8_t observed[6] = {0};
        uint8_t clear[6] = {0};
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
    static const uint8_t code[][7] = {
        {0x66u, 0x0fu, 0x01u, 0x06u, 0x00u, 0x02u, 0xf4u},
        {0x66u, 0x0fu, 0x01u, 0x0eu, 0x00u, 0x02u, 0xf4u}
    };
    static const uint8_t expected[][6] = {
        {0x34u, 0x12u, 0x12u, 0xdeu, 0xbcu, 0x7au},
        {0x78u, 0x56u, 0x78u, 0x56u, 0x34u, 0x12u}
    };
    STD_SIZE_T index;

    for (index = 0u; index < 2u; ++index) {
        descriptor_system_machine state;
        uint8_t observed[6] = {0};
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
    static const uint8_t code[][7] = {
        {0x0fu, 0x01u, 0x16u, 0x40u, 0x02u, 0xf4u, 0u},
        {0x0fu, 0x01u, 0x1eu, 0x40u, 0x02u, 0xf4u, 0u},
        {0x66u, 0x0fu, 0x01u, 0x16u, 0x40u, 0x02u, 0xf4u},
        {0x66u, 0x0fu, 0x01u, 0x1eu, 0x40u, 0x02u, 0xf4u}
    };
    static const uint8_t source[] = {0xbcu, 0x9au, 0x78u, 0x56u, 0x34u, 0x12u};
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
    static const uint8_t register_code[][4] = {
        {0x0fu, 0x01u, 0xc0u, 0u}, {0x0fu, 0x01u, 0xc8u, 0u},
        {0x0fu, 0x01u, 0xd0u, 0u}, {0x0fu, 0x01u, 0xd8u, 0u}
    };
    static const uint8_t load_code[][6] = {
        {0x0fu, 0x01u, 0x16u, 0x40u, 0x02u, 0xf4u},
        {0x0fu, 0x01u, 0x1eu, 0x40u, 0x02u, 0xf4u}
    };
    static const uint8_t source[] = {0xbcu, 0x9au, 0x78u, 0x56u, 0x34u, 0x12u};
    STD_SIZE_T index;

    for (index = 0u; index < 4u; ++index) {
        descriptor_system_machine state;
        t_cpu before;
        t_cpu after;
        C_INT failed = !dt_prepare(&state);

        if (!failed) {
            dt_set_tables(&state, 0x11112222u, 0x3333u, 0x44445555u, 0x6666u);
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed = !dt_run(&state, register_code[index], 3u, 1,
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
            dt_set_tables(&state, 0x11112222u, 0x3333u, 0x44445555u, 0x6666u);
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
    static const uint8_t store_code[][6] = {
        {0x0fu, 0x01u, 0x06u, 0x00u, 0x02u, 0xf4u},
        {0x0fu, 0x01u, 0x0eu, 0x00u, 0x02u, 0xf4u}
    };
    static const uint8_t load_code[][6] = {
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
            dt_set_tables(&state, 0x11112222u, 0x3333u, 0x44445555u, 0x6666u);
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
            dt_set_tables(&state, 0x11112222u, 0x3333u, 0x44445555u, 0x6666u);
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

C_INT main(C_VOID)
{
    if (!dt_test_store_layout() || !dt_test_protected_stores() ||
        !dt_test_load_layout() || !dt_test_register_and_privilege_faults() ||
        !dt_test_memory_faults_preserve_tables() || !dt_test_selector_stores() ||
        !dt_test_selector_loads() || !dt_test_msw_and_control_registers()) return 1;
    STD_PRINTF("M5:T304:DESCRIPTOR-SYSTEM:OK\n");
    return 0;
}
