#include "lib/types/types_interface.h"
#include <stdio.h>
#include "x86/core/device_support_interface.h"

#include "x86/core/debug_interface.h"
#include "ibmpc/board-common/machine_board_interface.h"
#include "../core_machine_board_fixture.h"

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
    lib_status reset_status;
} protected_mode_machine;

static lib_status protected_mode_set_entry(core_machine *machine)
{
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP)
    };
    return core_machine_debug_patch_registers(machine, &entry);
}

static lib_i32 protected_mode_prepare(protected_mode_machine *state,
    core_machine_cpu_profile profile)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };

    if (state == LIB_NULL) return 0;
    lib_memory_set(state, 0, sizeof(*state));
    if (core_machine_create(&config, &state->machine, LIB_NULL) != LIB_STATUS_OK) return 0;
    if (!test_core_machine_fixture_bind_freeze_reset(state->machine,
            LIB_NULL, LIB_NULL) ||
        (state->reset_status = protected_mode_set_entry(state->machine)) != LIB_STATUS_OK) {
        core_machine_destroy(state->machine);
        state->machine = LIB_NULL;
        return 0;
    }
    return 1;
}

static lib_i32 protected_mode_install_gdt(core_machine *machine,
    lib_u8 code_access, lib_u8 data_access)
{
    static const lib_u8 gdt_pointer[] = {
        0x17u, 0x00u, 0x00u, 0x03u, 0x00u, 0x00u
    };
    lib_u8 gdt[] = {
        0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
        0xffu, 0xffu, 0x00u, 0x20u, 0x00u, 0x9au, 0x00u, 0x00u,
        0xffu, 0xffu, 0x00u, 0x30u, 0x00u, 0x92u, 0x00u, 0x00u
    };

    if (code_access != 0u) gdt[13u] = code_access;
    if (data_access != 0u) gdt[21u] = data_access;
    return core_machine_memory_write(machine, TEST_GDT_POINTER_ADDRESS,
        gdt_pointer, sizeof(gdt_pointer)) == LIB_STATUS_OK &&
        core_machine_memory_write(machine, TEST_GDT_ADDRESS, gdt,
            sizeof(gdt)) == LIB_STATUS_OK;
}

static lib_i32 protected_mode_install_tss_gdt(core_machine *machine)
{
    static const lib_u8 gdt_pointer[] = {
        0x1fu, 0x00u, 0x00u, 0x03u, 0x00u, 0x00u
    };
    static const lib_u8 gdt[] = {
        0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
        0xffu, 0xffu, 0x00u, 0x20u, 0x00u, 0x9au, 0x00u, 0x00u,
        0xffu, 0xffu, 0x00u, 0x30u, 0x00u, 0x92u, 0x00u, 0x00u,
        0x00u, 0x00u, 0x00u, 0x35u, 0x00u, 0x81u, 0x00u, 0x00u
    };

    return core_machine_memory_write(machine, TEST_GDT_POINTER_ADDRESS,
        gdt_pointer, sizeof(gdt_pointer)) == LIB_STATUS_OK &&
        core_machine_memory_write(machine, TEST_GDT_ADDRESS, gdt,
            sizeof(gdt)) == LIB_STATUS_OK;
}

static lib_i32 protected_mode_install_idt(core_machine *machine)
{
    static const lib_u8 idt_pointer[] = {
        0x67u, 0x00u, 0x00u, 0x04u, 0x00u, 0x00u
    };
    lib_u8 idt[104] = {0};

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
        idt_pointer, sizeof(idt_pointer)) == LIB_STATUS_OK &&
        core_machine_memory_write(machine, TEST_IDT_ADDRESS, idt,
            sizeof(idt)) == LIB_STATUS_OK;
}

static lib_i32 protected_mode_run(core_machine *machine,
    const lib_u8 *real_code, lib_size real_code_size,
    const lib_u8 *protected_code, lib_size protected_code_size,
    lib_u8 code_access, lib_u8 data_access,
    lib_i32 expect_fault,
    core_machine_run_result *out_result,
    core_machine_cpu_diagnostic *out_diagnostic)
{
    core_machine_run_budget budget = { 64u, 0u };
    /* T337_REAL_UD_TERMINAL_GUEST_LIDT: prepare the original unavailable
       exception table using 286 instructions, outside the measured program. */
    const lib_u8 load_idt[] = { 0x0fu, 0x01u, 0x1eu, 0x00u, 0x06u };
    const lib_u8 idtr[] = { 0x17u, 0u, 0u, 0u, 0u, 0u };

    if (machine == LIB_NULL || out_result == LIB_NULL ||
        out_diagnostic == LIB_NULL) return 0;
    if (expect_fault) {
        if (core_machine_memory_write(machine, 0x0500u, load_idt,
                sizeof(load_idt)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x0600u, idtr,
                sizeof(idtr)) != LIB_STATUS_OK ||
            core_machine_debug_write_register(machine, CORE_MACHINE_DEBUG_EIP,
                0x0500u) != LIB_STATUS_OK ||
            core_machine_run(machine, (core_machine_run_budget){1u, 0u},
                out_result) != LIB_STATUS_OK ||
            out_result->reason != CORE_MACHINE_STOP_BUDGET || out_result->executed != 1u ||
            core_machine_debug_write_register(machine, CORE_MACHINE_DEBUG_EIP,
                0u) != LIB_STATUS_OK) return 0;
    }

    if (!protected_mode_install_gdt(machine,
            code_access, data_access) ||
        core_machine_memory_write(machine, 0u, real_code, real_code_size) !=
            LIB_STATUS_OK ||
        core_machine_memory_write(machine, TEST_CODE_ADDRESS, protected_code,
            protected_code_size) != LIB_STATUS_OK) return 0;
    if (test_core_machine_fixture_run_after_delivery(machine, budget, out_result) !=
            (expect_fault ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK) ||
        out_result->reason != (expect_fault ? CORE_MACHINE_STOP_FAULT :
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) ||
        core_machine_get_cpu_diagnostic(machine, out_diagnostic) !=
            LIB_STATUS_OK) return 0;
    return 1;
}

static lib_i32 protected_mode_test_positive(void)
{
    static const lib_u8 real_code[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u,
        0xb8u, 0x01u, 0x00u,
        0x0fu, 0x01u, 0xf0u,
        0xb8u, 0x10u, 0x00u,
        0x8eu, 0xd8u,
        0x8eu, 0xd0u,
        0xeau, 0x00u, 0x00u, 0x08u, 0x00u
    };
    static const lib_u8 protected_code[] = {
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
    core_machine_cpu_diagnostic diagnostic = {0};
    core_machine_cpu_state cpu = {0};
    core_machine_debug_cpu_snapshot after = {0};
    lib_u16 first = 0u;
    lib_u16 second = 0u;
    lib_i32 failed = !protected_mode_prepare(&state,
        CORE_MACHINE_CPU_PROFILE_80286);

    if (!failed) {
        lib_i32 ran = protected_mode_run(state.machine, real_code, sizeof(real_code),
            protected_code, sizeof(protected_code), 0u, 0u, 0, &result,
            &diagnostic);
        lib_i32 got_cpu = 0;

        failed |= !ran || diagnostic.first_fault.valid ||
            core_machine_memory_read(state.machine, TEST_DATA_ADDRESS,
                &first, sizeof(first)) != LIB_STATUS_OK || first != 0x1234u ||
            core_machine_memory_read(state.machine, TEST_DATA_ADDRESS + 2u,
                &second, sizeof(second)) != LIB_STATUS_OK || second != 0x5678u ||
            !(got_cpu = core_machine_get_cpu_state(state.machine, &cpu) == LIB_STATUS_OK) ||
            core_machine_debug_capture_cpu_snapshot(state.machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
            cpu.cs != TEST_CODE_SELECTOR ||
            cpu.cs_base != TEST_CODE_ADDRESS;
        if (failed) {
            fprintf(stderr,
                "T257 positive ran=%d fault=%u/%08x at=%04x:%08x eax=%08x first=%04x second=%04x ds=%04x/%08x ss=%04x/%08x cpu=%d %04x/%08x\n",
                ran, diagnostic.first_fault.valid,
                diagnostic.first_fault.exception_mask,
                diagnostic.first_fault.point.cs, diagnostic.first_fault.point.eip,
                diagnostic.first_fault.eax, first, second,
                after.ds.selector, after.ds.base,
                after.ss.selector, after.ss.base, got_cpu,
                cpu.cs, cpu.cs_base);
        }
    }
    core_machine_destroy(state.machine);
    return failed;
}

static lib_i32 protected_mode_test_invalid_selector(void)
{
    static const lib_u8 real_code[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u,
        0xb8u, 0x01u, 0x00u,
        0x0fu, 0x01u, 0xf0u,
        0xeau, 0x00u, 0x00u, 0x18u, 0x00u
    };
    static const lib_u8 protected_code[] = { 0x90u };
    protected_mode_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_debug_cpu_snapshot after = {0};
    lib_i32 failed = !protected_mode_prepare(&state,
        CORE_MACHINE_CPU_PROFILE_80286);

    if (!failed) {
        failed |= !protected_mode_run(state.machine, real_code, sizeof(real_code),
            protected_code, sizeof(protected_code), 0u, 0u, 1, &result,
            &diagnostic) || !diagnostic.first_fault.valid ||
            !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask,
                VCPUINS_EXCEPT_GP) || diagnostic.first_fault.exception_code !=
                0x18u || core_machine_debug_capture_cpu_snapshot(state.machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
            after.cs.selector != 0u || after.cs.base != 0u || after.eip != 0x000bu;
    }
    core_machine_destroy(state.machine);
    return failed;
}

static lib_i32 protected_mode_test_nonpresent_code(void)
{
    static const lib_u8 real_code[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u,
        0xb8u, 0x01u, 0x00u,
        0x0fu, 0x01u, 0xf0u,
        0xeau, 0x00u, 0x00u, 0x08u, 0x00u
    };
    static const lib_u8 protected_code[] = { 0x90u };
    protected_mode_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    lib_i32 failed = !protected_mode_prepare(&state,
        CORE_MACHINE_CPU_PROFILE_80286);

    if (!failed) {
        failed |= !protected_mode_run(state.machine, real_code, sizeof(real_code),
            protected_code, sizeof(protected_code), 0x1au, 0u, 1, &result,
            &diagnostic) || !diagnostic.first_fault.valid ||
            !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask,
                VCPUINS_EXCEPT_NP) || diagnostic.first_fault.exception_code !=
                TEST_CODE_SELECTOR;
    }
    core_machine_destroy(state.machine);
    return failed;
}

static lib_i32 protected_mode_test_nonpresent_stack(void)
{
    static const lib_u8 real_code[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u,
        0xb8u, 0x01u, 0x00u,
        0x0fu, 0x01u, 0xf0u,
        0xb8u, 0x10u, 0x00u,
        0x8eu, 0xd0u
    };
    static const lib_u8 protected_code[] = { 0x90u };
    protected_mode_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    lib_i32 failed = !protected_mode_prepare(&state,
        CORE_MACHINE_CPU_PROFILE_80286);

    if (!failed) {
        failed |= !protected_mode_run(state.machine, real_code, sizeof(real_code),
            protected_code, sizeof(protected_code), 0u, 0x12u, 1, &result,
            &diagnostic) || !diagnostic.first_fault.valid ||
            !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask,
                VCPUINS_EXCEPT_SS) || diagnostic.first_fault.exception_code !=
                TEST_DATA_SELECTOR;
    }
    core_machine_destroy(state.machine);
    return failed;
}

static lib_i32 protected_mode_test_80286_stack_fault_delivery(void)
{
    static const lib_u8 real_code[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u,
        0x0fu, 0x01u, 0x1eu, 0x10u, 0x01u,
        0xb8u, 0x01u, 0x00u,
        0x0fu, 0x01u, 0xf0u,
        0xb8u, 0x10u, 0x00u,
        0x8eu, 0xd0u
    };
    static const lib_u8 protected_code[33] = {
        [32] = 0xf4u
    };
    protected_mode_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_cpu_state cpu;
    core_machine_debug_cpu_snapshot after = {0};
    lib_u16 frame[4] = {0u, 0u, 0u, 0u};
    lib_i32 failed = !protected_mode_prepare(&state,
        CORE_MACHINE_CPU_PROFILE_80286);

    if (!failed) {
        failed |= !protected_mode_install_idt(state.machine) ||
            !protected_mode_run(state.machine, real_code, sizeof(real_code),
            protected_code, sizeof(protected_code), 0u, 0x12u, 0, &result,
            &diagnostic) || core_machine_get_cpu_state(state.machine, &cpu) != LIB_STATUS_OK ||
            core_machine_debug_capture_cpu_snapshot(state.machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
            core_machine_memory_read(state.machine, after.ss.base +
            CORE_MACHINE_MASK_U16(after.esp), frame, sizeof(frame)) !=
                LIB_STATUS_OK || diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid ||
            !CORE_MACHINE_BIT_IS_SET(diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_SS) ||
            diagnostic.last_delivered_exception.point.eip != 0x0013u ||
            cpu.cs != TEST_CODE_SELECTOR || cpu.eip != 0x00000021u ||
            frame[0] != TEST_DATA_SELECTOR || frame[1] != 0x0013u ||
            frame[2] != 0x0000u || frame[3] != 0x0002u;
    }
    core_machine_destroy(state.machine);
    return failed;
}

static lib_i32 protected_mode_test_80286_task_fault_delivery(void)
{
    static const lib_u8 real_code[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u,
        0x0fu, 0x01u, 0x1eu, 0x10u, 0x01u,
        0xb8u, 0x01u, 0x00u,
        0x0fu, 0x01u, 0xf0u,
        0xeau, 0x00u, 0x00u, 0x08u, 0x00u
    };
    static const lib_u8 protected_code[49] = {
        0xb8u, 0x18u, 0x00u,
        0x0fu, 0x00u, 0xd8u,
        0xcfu,
        [48] = 0xf4u
    };
    protected_mode_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_cpu_state cpu;
    core_machine_debug_cpu_snapshot after = {0};
    lib_u16 frame[4] = {0u, 0u, 0u, 0u};
    lib_i32 failed = !protected_mode_prepare(&state,
        CORE_MACHINE_CPU_PROFILE_80286);

    if (!failed) {
        failed |= !protected_mode_install_idt(state.machine) ||
            !protected_mode_install_tss_gdt(state.machine) ||
            core_machine_memory_write(state.machine, 0u, real_code,
            sizeof(real_code)) != LIB_STATUS_OK ||
            core_machine_memory_write(state.machine, TEST_CODE_ADDRESS,
                protected_code, sizeof(protected_code)) != LIB_STATUS_OK ||
            core_machine_debug_write_register(state.machine,
            CORE_MACHINE_DEBUG_EFLAGS, 0x0002u | CORE_MACHINE_DEBUG_EFLAGS_NT) !=
                LIB_STATUS_OK || test_core_machine_fixture_run_after_delivery(state.machine,
            (core_machine_run_budget){64u, 0u},
            &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                LIB_STATUS_OK || core_machine_get_cpu_state(state.machine, &cpu) != LIB_STATUS_OK ||
            core_machine_debug_capture_cpu_snapshot(state.machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
            core_machine_memory_read(state.machine, after.ss.base +
            CORE_MACHINE_MASK_U16(after.esp), frame, sizeof(frame)) !=
                LIB_STATUS_OK || diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid ||
            !CORE_MACHINE_BIT_IS_SET(diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_TS) ||
            diagnostic.last_delivered_exception.point.eip != 0x0006u ||
            cpu.cs != TEST_CODE_SELECTOR || cpu.eip != 0x00000031u ||
            frame[0] != 0x0000u || frame[1] != 0x0006u ||
            frame[2] != TEST_CODE_SELECTOR ||
            frame[3] != (0x0002u | CORE_MACHINE_DEBUG_EFLAGS_NT);
    }
    core_machine_destroy(state.machine);
    return failed;
}

static lib_i32 protected_mode_test_protected_lidt_admitted(void)
{
    static const lib_u8 real_code[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u,
        0xb8u, 0x01u, 0x00u,
        0x0fu, 0x01u, 0xf0u,
        0xeau, 0x00u, 0x00u, 0x08u, 0x00u
    };
    static const lib_u8 protected_code[] = {
        0xb8u, 0x10u, 0x00u,
        0x8eu, 0xd8u,
        0x0fu, 0x01u, 0x1eu, 0x10u, 0x01u,
        0xf4u
    };
    protected_mode_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    lib_i32 failed = !protected_mode_prepare(&state,
        CORE_MACHINE_CPU_PROFILE_80286);

    if (!failed) {
        failed |= !protected_mode_run(state.machine, real_code, sizeof(real_code),
            protected_code, sizeof(protected_code), 0u, 0u, 0, &result,
            &diagnostic) || diagnostic.first_fault.valid;
    }
    core_machine_destroy(state.machine);
    return failed;
}

static lib_i32 protected_mode_test_configured_idt_interrupts(void)
{
    static const lib_u8 real_code[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u,
        0x0fu, 0x01u, 0x1eu, 0x10u, 0x01u,
        0xb8u, 0x01u, 0x00u,
        0x0fu, 0x01u, 0xf0u,
        0xb8u, 0x10u, 0x00u,
        0x8eu, 0xd8u,
        0x8eu, 0xd0u,
        0xeau, 0x00u, 0x00u, 0x08u, 0x00u
    };
    static const lib_u8 protected_code[] = {
        0xccu, 0x90u, 0x90u, 0x90u, 0x90u, 0x90u, 0x90u, 0x90u,
        0x90u, 0x90u, 0x90u, 0x90u, 0x90u, 0x90u, 0x90u, 0x90u,
        0xf4u
    };
    protected_mode_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    lib_i32 failed = !protected_mode_prepare(&state,
        CORE_MACHINE_CPU_PROFILE_80286);

    if (!failed) {
        failed |= !protected_mode_install_idt(state.machine) ||
            !protected_mode_run(state.machine, real_code, sizeof(real_code),
            protected_code, sizeof(protected_code), 0u, 0u, 0, &result,
            &diagnostic) || diagnostic.first_fault.valid;
    }
    core_machine_destroy(state.machine);
    return failed;
}

static lib_i32 protected_mode_test_80286_rejects_386(void)
{
    static const lib_u8 real_code[] = { 0x0fu, 0x20u, 0xc0u };
    static const lib_u8 protected_code[] = { 0x90u };
    protected_mode_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    lib_i32 failed = !protected_mode_prepare(&state,
        CORE_MACHINE_CPU_PROFILE_80286);

    if (!failed) {
        failed |= !protected_mode_run(state.machine, real_code, sizeof(real_code),
            protected_code, sizeof(protected_code), 0u, 0u, 1, &result,
            &diagnostic) || !diagnostic.first_fault.valid ||
            !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask,
                VCPUINS_EXCEPT_UD);
    }
    core_machine_destroy(state.machine);
    return failed;
}

lib_i32 main(void)
{
    lib_i32 positive;
    lib_i32 invalid_selector;
    lib_i32 nonpresent_code;
    lib_i32 nonpresent_stack;
    lib_i32 stack_fault_delivery;
    lib_i32 task_fault_delivery;
    lib_i32 protected_lidt;
    lib_i32 configured_idt;
    lib_i32 rejects_386;

    positive = protected_mode_test_positive();
    invalid_selector = protected_mode_test_invalid_selector();
    nonpresent_code = protected_mode_test_nonpresent_code();
    nonpresent_stack = protected_mode_test_nonpresent_stack();
    stack_fault_delivery = protected_mode_test_80286_stack_fault_delivery();
    task_fault_delivery = protected_mode_test_80286_task_fault_delivery();
    protected_lidt = protected_mode_test_protected_lidt_admitted();
    configured_idt = protected_mode_test_configured_idt_interrupts();
    rejects_386 = protected_mode_test_80286_rejects_386();
    if (positive || invalid_selector || nonpresent_code || nonpresent_stack ||
        stack_fault_delivery || task_fault_delivery ||
        protected_lidt || configured_idt || rejects_386) {
        fprintf(stderr,
            "M5:T257:S6:80286-PROTECTED-MODE:FAIL positive=%d selector=%d npcode=%d npstack=%d stackdelivery=%d taskdelivery=%d lidt=%d idt=%d i386=%d\n",
            positive, invalid_selector, nonpresent_code, nonpresent_stack,
            stack_fault_delivery, task_fault_delivery, protected_lidt,
            configured_idt, rejects_386);
        return 1;
    }
    printf("M5:T257:S6:80286-PROTECTED-MODE:OK\n");
    printf("M5:T358:S2:EXCEPTION-IRQ:OK\n");
    return 0;
}
