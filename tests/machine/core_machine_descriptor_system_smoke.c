#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define DT_STORE_ADDRESS 0x0200u
#define DT_LOAD_ADDRESS 0x0240u

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

static C_INT dt_prepare(descriptor_system_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
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
        !dt_test_memory_faults_preserve_tables()) return 1;
    STD_PRINTF("M5:T304:DESCRIPTOR-SYSTEM:OK\n");
    return 0;
}
