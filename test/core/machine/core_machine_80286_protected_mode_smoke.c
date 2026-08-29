#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/cpu_instructions.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define TEST_GDT_ADDRESS 0x0300u
#define TEST_GDT_POINTER_ADDRESS 0x0100u
#define TEST_IDT_ADDRESS 0x0400u
#define TEST_IDT_POINTER_ADDRESS 0x0110u
#define TEST_CODE_ADDRESS 0x2000u
#define TEST_DATA_ADDRESS 0x3000u
#define TEST_CODE_SELECTOR 0x0008u
#define TEST_DATA_SELECTOR 0x0010u

typedef struct protected_mode_machine {
    core_machine *machine;
} protected_mode_machine;

static C_VOID protected_mode_reset(C_VOID *opaque)
{
    protected_mode_machine *state = (protected_mode_machine *)opaque;

    if (state != STD_NULL) (C_VOID)test_core_machine_fixture_reset_real_mode(
        state->machine);
}

static const core_machine_execution_provider protected_mode_provider = {
    protected_mode_reset,
    STD_NULL
};

static C_INT protected_mode_prepare(protected_mode_machine *state,
    core_machine_cpu_profile profile)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    if (state == STD_NULL) return 0;
    STD_MEMSET(state, 0, sizeof(*state));
    if (core_machine_create(&config, &state->machine) != TYPE_STATUS_OK) return 0;
    if (!test_core_machine_fixture_bind_freeze_reset(state->machine,
            &protected_mode_provider, state)) {
        core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 0;
    }
    return 1;
}

static C_INT protected_mode_install_gdt(core_machine *machine,
    type_unsigned_8 code_access, type_unsigned_8 data_access)
{
    static const type_unsigned_8 gdt_pointer[] = {
        0x17u, 0x00u, 0x00u, 0x03u, 0x00u, 0x00u
    };
    type_unsigned_8 gdt[] = {
        0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
        0xffu, 0xffu, 0x00u, 0x20u, 0x00u, 0x9au, 0x00u, 0x00u,
        0xffu, 0xffu, 0x00u, 0x30u, 0x00u, 0x92u, 0x00u, 0x00u
    };

    if (code_access != 0u) gdt[13u] = code_access;
    if (data_access != 0u) gdt[21u] = data_access;
    return core_machine_memory_write(machine, TEST_GDT_POINTER_ADDRESS,
        gdt_pointer, sizeof(gdt_pointer)) == TYPE_STATUS_OK &&
        core_machine_memory_write(machine, TEST_GDT_ADDRESS, gdt,
            sizeof(gdt)) == TYPE_STATUS_OK;
}

static C_INT protected_mode_install_tss_gdt(core_machine *machine)
{
    static const type_unsigned_8 gdt_pointer[] = {
        0x1fu, 0x00u, 0x00u, 0x03u, 0x00u, 0x00u
    };
    static const type_unsigned_8 gdt[] = {
        0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
        0xffu, 0xffu, 0x00u, 0x20u, 0x00u, 0x9au, 0x00u, 0x00u,
        0xffu, 0xffu, 0x00u, 0x30u, 0x00u, 0x92u, 0x00u, 0x00u,
        0x00u, 0x00u, 0x00u, 0x35u, 0x00u, 0x81u, 0x00u, 0x00u
    };

    return core_machine_memory_write(machine, TEST_GDT_POINTER_ADDRESS,
        gdt_pointer, sizeof(gdt_pointer)) == TYPE_STATUS_OK &&
        core_machine_memory_write(machine, TEST_GDT_ADDRESS, gdt,
            sizeof(gdt)) == TYPE_STATUS_OK;
}

static C_INT protected_mode_install_idt(core_machine *machine)
{
    static const type_unsigned_8 idt_pointer[] = {
        0x67u, 0x00u, 0x00u, 0x04u, 0x00u, 0x00u
    };
    type_unsigned_8 idt[104] = {0};

    idt[24u] = 0x10u;
    idt[26u] = 0x08u;
    idt[29u] = 0x86u;
    idt[80u] = 0x30u;
    idt[82u] = 0x08u;
    idt[85u] = 0x86u;
    idt[96u] = 0x20u;
    idt[98u] = 0x08u;
    idt[101u] = 0x86u;
    return core_machine_memory_write(machine, TEST_IDT_POINTER_ADDRESS,
        idt_pointer, sizeof(idt_pointer)) == TYPE_STATUS_OK &&
        core_machine_memory_write(machine, TEST_IDT_ADDRESS, idt,
            sizeof(idt)) == TYPE_STATUS_OK;
}

static C_INT protected_mode_run(core_machine *machine,
    const type_unsigned_8 *real_code, STD_SIZE_T real_code_size,
    const type_unsigned_8 *protected_code, STD_SIZE_T protected_code_size,
    type_unsigned_8 code_access, type_unsigned_8 data_access,
    C_INT expect_fault,
    core_machine_run_result *out_result,
    core_machine_cpu_diagnostic *out_diagnostic)
{
    core_machine_run_budget budget = { 64u, 0u };

    if (machine == STD_NULL || out_result == STD_NULL ||
        out_diagnostic == STD_NULL || !protected_mode_install_gdt(machine,
            code_access, data_access) ||
        core_machine_memory_write(machine, 0u, real_code, real_code_size) !=
            TYPE_STATUS_OK ||
        core_machine_memory_write(machine, TEST_CODE_ADDRESS, protected_code,
            protected_code_size) != TYPE_STATUS_OK ||
        (expect_fault && machine->cpu_profile <= CORE_MACHINE_CPU_PROFILE_80286 &&
            !test_core_machine_fixture_preflight_real_ud_terminal(machine))) return 0;
    if (core_machine_run(machine, budget, out_result) !=
            (expect_fault ? TYPE_STATUS_FAULT : TYPE_STATUS_OK) ||
        out_result->reason != (expect_fault ? CORE_MACHINE_STOP_FAULT :
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) ||
        core_machine_get_cpu_diagnostic(machine, out_diagnostic) !=
            TYPE_STATUS_OK) return 0;
    return 1;
}

static C_INT protected_mode_test_positive(C_VOID)
{
    static const type_unsigned_8 real_code[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u,
        0xb8u, 0x01u, 0x00u,
        0x0fu, 0x01u, 0xf0u,
        0xb8u, 0x10u, 0x00u,
        0x8eu, 0xd8u,
        0x8eu, 0xd0u,
        0xeau, 0x00u, 0x00u, 0x08u, 0x00u
    };
    static const type_unsigned_8 protected_code[] = {
        0xb8u, 0x34u, 0x12u,
        0xbbu, 0x00u, 0x00u,
        0x89u, 0x07u,
        0x9au, 0x16u, 0x00u, 0x08u, 0x00u,
        0xb8u, 0x78u, 0x56u,
        0xbbu, 0x02u, 0x00u,
        0x89u, 0x07u,
        0xf4u,
        0xb8u, 0xefu, 0xbeu,
        0xcbu
    };
    protected_mode_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_cpu_state cpu;
    type_unsigned_16 first = 0u;
    type_unsigned_16 second = 0u;
    C_INT failed = !protected_mode_prepare(&state,
        CORE_MACHINE_CPU_PROFILE_80286);

    if (!failed) {
        C_INT ran = protected_mode_run(state.machine, real_code, sizeof(real_code),
            protected_code, sizeof(protected_code), 0u, 0u, 0, &result,
            &diagnostic);
        C_INT got_cpu;

        failed |= !ran;
        failed |= diagnostic.first_fault.valid;
        failed |= core_machine_memory_read(state.machine, TEST_DATA_ADDRESS,
            &first, sizeof(first)) != TYPE_STATUS_OK || first != 0x1234u;
        failed |= core_machine_memory_read(state.machine, TEST_DATA_ADDRESS + 2u,
            &second, sizeof(second)) != TYPE_STATUS_OK || second != 0x5678u;
        got_cpu = core_machine_get_cpu_state(state.machine, &cpu) == TYPE_STATUS_OK;
        failed |= !got_cpu || cpu.cs != TEST_CODE_SELECTOR ||
            cpu.cs_base != TEST_CODE_ADDRESS;
        if (failed) {
            STD_FPRINTF(STD_STDERR,
                "T257 positive ran=%d fault=%u/%08x at=%04x:%08x eax=%08x first=%04x second=%04x ds=%04x/%08x ss=%04x/%08x cpu=%d %04x/%08x\n",
                ran, diagnostic.first_fault.valid,
                diagnostic.first_fault.exception_mask,
                diagnostic.first_fault.point.cs, diagnostic.first_fault.point.eip,
                diagnostic.first_fault.eax, first, second,
                test_core_machine_fixture_capture_cpu_after_run(state.machine).data.ds.selector,
                test_core_machine_fixture_capture_cpu_after_run(state.machine).data.ds.base,
                test_core_machine_fixture_capture_cpu_after_run(state.machine).data.ss.selector,
                test_core_machine_fixture_capture_cpu_after_run(state.machine).data.ss.base, got_cpu,
                cpu.cs, cpu.cs_base);
        }
    }
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT protected_mode_test_invalid_selector(C_VOID)
{
    static const type_unsigned_8 real_code[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u,
        0xb8u, 0x01u, 0x00u,
        0x0fu, 0x01u, 0xf0u,
        0xeau, 0x00u, 0x00u, 0x18u, 0x00u
    };
    static const type_unsigned_8 protected_code[] = { 0x90u };
    protected_mode_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    C_INT failed = !protected_mode_prepare(&state,
        CORE_MACHINE_CPU_PROFILE_80286);

    if (!failed) {
        failed |= !protected_mode_run(state.machine, real_code, sizeof(real_code),
            protected_code, sizeof(protected_code), 0u, 0u, 1, &result,
            &diagnostic);
        failed |= !diagnostic.first_fault.valid ||
            !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                VCPUINS_EXCEPT_GP) || diagnostic.first_fault.exception_code !=
                0x18u;
        failed |= test_core_machine_fixture_capture_cpu_after_run(
            state.machine).data.cs.selector != 0u ||
            test_core_machine_fixture_capture_cpu_after_run(
            state.machine).data.cs.base != 0u ||
            test_core_machine_fixture_capture_cpu_after_run(
            state.machine).data.eip != 0x000bu;
    }
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT protected_mode_test_nonpresent_code(C_VOID)
{
    static const type_unsigned_8 real_code[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u,
        0xb8u, 0x01u, 0x00u,
        0x0fu, 0x01u, 0xf0u,
        0xeau, 0x00u, 0x00u, 0x08u, 0x00u
    };
    static const type_unsigned_8 protected_code[] = { 0x90u };
    protected_mode_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    C_INT failed = !protected_mode_prepare(&state,
        CORE_MACHINE_CPU_PROFILE_80286);

    if (!failed) {
        failed |= !protected_mode_run(state.machine, real_code, sizeof(real_code),
            protected_code, sizeof(protected_code), 0x1au, 0u, 1, &result,
            &diagnostic);
        failed |= !diagnostic.first_fault.valid ||
            !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                VCPUINS_EXCEPT_NP) || diagnostic.first_fault.exception_code !=
                TEST_CODE_SELECTOR;
    }
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT protected_mode_test_nonpresent_stack(C_VOID)
{
    static const type_unsigned_8 real_code[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u,
        0xb8u, 0x01u, 0x00u,
        0x0fu, 0x01u, 0xf0u,
        0xb8u, 0x10u, 0x00u,
        0x8eu, 0xd0u
    };
    static const type_unsigned_8 protected_code[] = { 0x90u };
    protected_mode_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    C_INT failed = !protected_mode_prepare(&state,
        CORE_MACHINE_CPU_PROFILE_80286);

    if (!failed) {
        failed |= !protected_mode_run(state.machine, real_code, sizeof(real_code),
            protected_code, sizeof(protected_code), 0u, 0x12u, 1, &result,
            &diagnostic);
        failed |= !diagnostic.first_fault.valid ||
            !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                VCPUINS_EXCEPT_SS) || diagnostic.first_fault.exception_code !=
                TEST_DATA_SELECTOR;
    }
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT protected_mode_test_80286_stack_fault_delivery(C_VOID)
{
    static const type_unsigned_8 real_code[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u,
        0x0fu, 0x01u, 0x1eu, 0x10u, 0x01u,
        0xb8u, 0x01u, 0x00u,
        0x0fu, 0x01u, 0xf0u,
        0xb8u, 0x10u, 0x00u,
        0x8eu, 0xd0u
    };
    static const type_unsigned_8 protected_code[33] = {
        [32] = 0xf4u
    };
    protected_mode_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_cpu_state cpu;
    t_cpu after;
    type_unsigned_16 frame[4] = {0u, 0u, 0u, 0u};
    C_INT failed = !protected_mode_prepare(&state,
        CORE_MACHINE_CPU_PROFILE_80286);

    if (!failed) {
        failed |= !protected_mode_install_idt(state.machine);
        failed |= !protected_mode_run(state.machine, real_code, sizeof(real_code),
            protected_code, sizeof(protected_code), 0u, 0x12u, 0, &result,
            &diagnostic);
        failed |= core_machine_get_cpu_state(state.machine, &cpu) != TYPE_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_memory_read(state.machine, after.data.ss.base +
            TYPE_MASK_UNSIGNED_16(after.data.esp), frame, sizeof(frame)) !=
                TYPE_STATUS_OK;
        failed |= diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid ||
            !TYPE_GET_BIT(diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_SS) ||
            diagnostic.last_delivered_exception.point.eip != 0x0013u ||
            cpu.cs != TEST_CODE_SELECTOR || cpu.eip != 0x00000021u ||
            frame[0] != TEST_DATA_SELECTOR || frame[1] != 0x0013u ||
            frame[2] != 0x0000u || frame[3] != 0x0002u;
    }
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT protected_mode_test_80286_task_fault_delivery(C_VOID)
{
    static const type_unsigned_8 real_code[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u,
        0x0fu, 0x01u, 0x1eu, 0x10u, 0x01u,
        0xb8u, 0x01u, 0x00u,
        0x0fu, 0x01u, 0xf0u,
        0xeau, 0x00u, 0x00u, 0x08u, 0x00u
    };
    static const type_unsigned_8 protected_code[49] = {
        0xb8u, 0x18u, 0x00u,
        0x0fu, 0x00u, 0xd8u,
        0xcfu,
        [48] = 0xf4u
    };
    protected_mode_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_cpu_state cpu;
    t_cpu after;
    type_unsigned_16 frame[4] = {0u, 0u, 0u, 0u};
    C_INT failed = !protected_mode_prepare(&state,
        CORE_MACHINE_CPU_PROFILE_80286);

    if (!failed) {
        failed |= !protected_mode_install_idt(state.machine);
        failed |= !protected_mode_install_tss_gdt(state.machine);
        failed |= core_machine_memory_write(state.machine, 0u, real_code,
            sizeof(real_code)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, TEST_CODE_ADDRESS,
                protected_code, sizeof(protected_code)) != TYPE_STATUS_OK;
        state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_NT;
        failed |= core_machine_run(state.machine, (core_machine_run_budget){64u, 0u},
            &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                TYPE_STATUS_OK;
        failed |= core_machine_get_cpu_state(state.machine, &cpu) != TYPE_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_memory_read(state.machine, after.data.ss.base +
            TYPE_MASK_UNSIGNED_16(after.data.esp), frame, sizeof(frame)) !=
                TYPE_STATUS_OK;
        failed |= diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid ||
            !TYPE_GET_BIT(diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_TS) ||
            diagnostic.last_delivered_exception.point.eip != 0x0006u ||
            cpu.cs != TEST_CODE_SELECTOR || cpu.eip != 0x00000031u ||
            frame[0] != 0x0000u || frame[1] != 0x0006u ||
            frame[2] != TEST_CODE_SELECTOR ||
            frame[3] != (0x0002u | VCPU_EFLAGS_NT);
    }
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT protected_mode_test_protected_lidt_admitted(C_VOID)
{
    static const type_unsigned_8 real_code[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u,
        0xb8u, 0x01u, 0x00u,
        0x0fu, 0x01u, 0xf0u,
        0xeau, 0x00u, 0x00u, 0x08u, 0x00u
    };
    static const type_unsigned_8 protected_code[] = {
        0xb8u, 0x10u, 0x00u,
        0x8eu, 0xd8u,
        0x0fu, 0x01u, 0x1eu, 0x10u, 0x01u,
        0xf4u
    };
    protected_mode_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    C_INT failed = !protected_mode_prepare(&state,
        CORE_MACHINE_CPU_PROFILE_80286);

    if (!failed) {
        failed |= !protected_mode_run(state.machine, real_code, sizeof(real_code),
            protected_code, sizeof(protected_code), 0u, 0u, 0, &result,
            &diagnostic);
        failed |= diagnostic.first_fault.valid;
    }
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT protected_mode_test_configured_idt_interrupts(C_VOID)
{
    static const type_unsigned_8 real_code[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u,
        0x0fu, 0x01u, 0x1eu, 0x10u, 0x01u,
        0xb8u, 0x01u, 0x00u,
        0x0fu, 0x01u, 0xf0u,
        0xb8u, 0x10u, 0x00u,
        0x8eu, 0xd8u,
        0x8eu, 0xd0u,
        0xeau, 0x00u, 0x00u, 0x08u, 0x00u
    };
    static const type_unsigned_8 protected_code[] = {
        0xccu, 0x90u, 0x90u, 0x90u, 0x90u, 0x90u, 0x90u, 0x90u,
        0x90u, 0x90u, 0x90u, 0x90u, 0x90u, 0x90u, 0x90u, 0x90u,
        0xf4u
    };
    protected_mode_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    C_INT failed = !protected_mode_prepare(&state,
        CORE_MACHINE_CPU_PROFILE_80286);

    if (!failed) {
        failed |= !protected_mode_install_idt(state.machine);
        failed |= !protected_mode_run(state.machine, real_code, sizeof(real_code),
            protected_code, sizeof(protected_code), 0u, 0u, 0, &result,
            &diagnostic);
        failed |= diagnostic.first_fault.valid;
    }
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT protected_mode_test_80186_gate(C_VOID)
{
    static const type_unsigned_8 real_code[] = { 0x0fu, 0x01u, 0x16u, 0x00u, 0x01u };
    static const type_unsigned_8 protected_code[] = { 0x90u };
    protected_mode_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    C_INT failed = !protected_mode_prepare(&state,
        CORE_MACHINE_CPU_PROFILE_80186);

    if (!failed) {
        failed |= !protected_mode_run(state.machine, real_code, sizeof(real_code),
            protected_code, sizeof(protected_code), 0u, 0u, 1, &result,
            &diagnostic);
        failed |= !diagnostic.first_fault.valid ||
            !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                VCPUINS_EXCEPT_UD);
    }
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT protected_mode_test_80286_rejects_386(C_VOID)
{
    static const type_unsigned_8 real_code[] = { 0x0fu, 0x20u, 0xc0u };
    static const type_unsigned_8 protected_code[] = { 0x90u };
    protected_mode_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    C_INT failed = !protected_mode_prepare(&state,
        CORE_MACHINE_CPU_PROFILE_80286);

    if (!failed) {
        failed |= !protected_mode_run(state.machine, real_code, sizeof(real_code),
            protected_code, sizeof(protected_code), 0u, 0u, 1, &result,
            &diagnostic);
        failed |= !diagnostic.first_fault.valid ||
            !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                VCPUINS_EXCEPT_UD);
    }
    core_machine_destroy(state.machine);
    return failed;
}

C_INT main(C_VOID)
{
    C_INT positive;
    C_INT invalid_selector;
    C_INT nonpresent_code;
    C_INT nonpresent_stack;
    C_INT stack_fault_delivery;
    C_INT task_fault_delivery;
    C_INT protected_lidt;
    C_INT configured_idt;
    C_INT profile_gate;
    C_INT rejects_386;

    positive = protected_mode_test_positive();
    invalid_selector = protected_mode_test_invalid_selector();
    nonpresent_code = protected_mode_test_nonpresent_code();
    nonpresent_stack = protected_mode_test_nonpresent_stack();
    stack_fault_delivery = protected_mode_test_80286_stack_fault_delivery();
    task_fault_delivery = protected_mode_test_80286_task_fault_delivery();
    protected_lidt = protected_mode_test_protected_lidt_admitted();
    configured_idt = protected_mode_test_configured_idt_interrupts();
    profile_gate = protected_mode_test_80186_gate();
    rejects_386 = protected_mode_test_80286_rejects_386();
    if (positive || invalid_selector || nonpresent_code || nonpresent_stack ||
        stack_fault_delivery || task_fault_delivery ||
        protected_lidt || configured_idt || profile_gate || rejects_386) {
        STD_FPRINTF(STD_STDERR,
            "M5:T257:S6:80286-PROTECTED-MODE:FAIL positive=%d selector=%d npcode=%d npstack=%d stackdelivery=%d taskdelivery=%d lidt=%d idt=%d profile=%d i386=%d\n",
            positive, invalid_selector, nonpresent_code, nonpresent_stack,
            stack_fault_delivery, task_fault_delivery, protected_lidt,
            configured_idt, profile_gate, rejects_386);
        return 1;
    }
    STD_PRINTF("M5:T257:S6:80286-PROTECTED-MODE:OK\n");
    STD_PRINTF("M5:T358:S2:EXCEPTION-IRQ:OK\n");
    return 0;
}
