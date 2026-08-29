#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define GDT_PTR 0x0100u
#define IDT_PTR 0x0110u
#define GDT_BASE 0x0300u
#define IDT_BASE 0x0400u
#define TSS_BASE 0x0600u
#define KERNEL_BASE 0x2000u
#define USER_CODE_BASE 0x4000u
#define USER_DATA_BASE 0x5000u

typedef struct privilege_machine {
    core_machine *machine;
} privilege_machine;

typedef enum privilege_negative_case {
    PRIVILEGE_NEGATIVE_NONE,
    PRIVILEGE_NEGATIVE_GATE_NOT_PRESENT,
    PRIVILEGE_NEGATIVE_GP_GATE_NOT_PRESENT,
    PRIVILEGE_NEGATIVE_CODE_NOT_PRESENT,
    PRIVILEGE_NEGATIVE_STACK_ATOMICITY
} privilege_negative_case;

static C_VOID privilege_reset(C_VOID *opaque)
{
    privilege_machine *state = (privilege_machine *)opaque;

    if (state != STD_NULL) (C_VOID)test_core_machine_fixture_reset_real_mode(
        state->machine);
}

static const core_machine_execution_provider privilege_provider = {
    privilege_reset, STD_NULL
};

static C_INT privilege_prepare(privilege_machine *state,
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
            &privilege_provider, state)) {
        core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 0;
    }
    return 1;
}

static C_INT write_bytes(core_machine *machine, type_unsigned_32 address,
    const type_unsigned_8 *bytes, STD_SIZE_T count)
{
    return core_machine_memory_write(machine, address, bytes, count) ==
        TYPE_STATUS_OK;
}

static C_INT privilege_install(privilege_machine *state, C_INT fault_delivery,
    privilege_negative_case negative_case)
{
    static const type_unsigned_8 gdt_pointer[] = { 0x37u, 0x00u, 0x00u, 0x03u, 0x00u, 0x00u };
    static const type_unsigned_8 idt_pointer[] = { 0x97u, 0x01u, 0x00u, 0x04u, 0x00u, 0x00u };
    static const type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xff,0xff,0,0x20,0,0x9a,0,0,
        0xff,0xff,0,0x30,0,0x92,0,0,
        0xff,0xff,0,0x40,0,0xfa,0,0,
        0xff,0xff,0,0x50,0,0xf2,0,0,
        0x2b,0,0,0x06,0,0x81,0,0,
        0xff,0xff,0,0x20,0,0x1a,0,0
    };
    type_unsigned_8 idt[0x198u] = {0};
    static const type_unsigned_8 real_code[] = {
        0x0f,0x01,0x16,0x00,0x01,
        0x0f,0x01,0x1e,0x10,0x01,
        0xb8,0x01,0x00,0x0f,0x01,0xf0,
        0xb8,0x28,0x00,0x0f,0x00,0xd8,
        0xb8,0x10,0x00,0x8e,0xd0,0xbc,0x00,0x80,
        0xea,0x00,0x00,0x08,0x00
    };
    static const type_unsigned_8 kernel_entry[] = {
        0xb8,0x10,0x00,0x8e,0xd8,
        0xb8,0x23,0x00,0x50,
        0xb8,0x00,0xa0,0x50,
        0xb8,0x02,0x02,0x50,
        0xb8,0x1b,0x00,0x50,
        0xb8,0x00,0x00,0x50,
        0xb8,0x23,0x00,0x8e,0xd8,
        0xcf
    };
    static const type_unsigned_8 kernel_handler[] = {
        0xb8,0x11,0x11,0xa3,0x00,0x00,0xcf
    };
    static const type_unsigned_8 kernel_stop[] = { 0xf4 };
    static const type_unsigned_8 kernel_fault[] = {
        0xb8,0x33,0x33,0xa3,0x04,0x00,0xf4
    };
    static const type_unsigned_8 user_code[] = {
        0xcd,0x30,0xb8,0x22,0x22,0xa3,0x02,0x00,0xcd,0x31
    };
    static const type_unsigned_8 user_fault_code[] = { 0xcd,0x32 };
    static const type_unsigned_8 user_gate_not_present[] = { 0xcd,0x30 };
    static const type_unsigned_8 user_gp_gate_not_present[] = { 0xcd,0x0d };
    static const type_unsigned_8 user_code_not_present[] = { 0xcd,0x31 };
    const type_unsigned_8 *user_program = user_code;
    STD_SIZE_T user_program_size = sizeof(user_code);
    type_unsigned_16 sp0 = 0x9000u;
    type_unsigned_16 ss0 = 0x0010u;

    idt[0x180u] = 0x00u;
    idt[0x181u] = 0x01u;
    idt[0x182u] = 0x08u;
    idt[0x185u] = 0xe6u;
    idt[0x188u] = 0x10u;
    idt[0x189u] = 0x01u;
    idt[0x18au] = 0x08u;
    idt[0x18du] = 0xe6u;
    if (negative_case == PRIVILEGE_NEGATIVE_GATE_NOT_PRESENT) {
        idt[0x185u] = 0x66u;
        user_program = user_gate_not_present;
        user_program_size = sizeof(user_gate_not_present);
    } else if (negative_case == PRIVILEGE_NEGATIVE_GP_GATE_NOT_PRESENT) {
        user_program = user_gp_gate_not_present;
        user_program_size = sizeof(user_gp_gate_not_present);
    } else if (negative_case == PRIVILEGE_NEGATIVE_CODE_NOT_PRESENT) {
        idt[0x18au] = 0x30u;
        user_program = user_code_not_present;
        user_program_size = sizeof(user_code_not_present);
    } else if (negative_case == PRIVILEGE_NEGATIVE_STACK_ATOMICITY) {
        sp0 = 0x0009u;
    }
    if (fault_delivery) {
        idt[0x58u] = 0x20u;
        idt[0x59u] = 0x01u;
        idt[0x5au] = 0x08u;
        idt[0x5du] = 0x86u;
        idt[0x68u] = 0x20u;
        idt[0x69u] = 0x01u;
        idt[0x6au] = 0x08u;
        idt[0x6du] = 0x86u;
        idt[0x190u] = 0x10u;
        idt[0x191u] = 0x01u;
        idt[0x192u] = 0x08u;
        idt[0x195u] = 0x86u;
    }
    if (negative_case == PRIVILEGE_NEGATIVE_GP_GATE_NOT_PRESENT) {
        idt[0x68u] = 0x20u;
        idt[0x69u] = 0x01u;
        idt[0x6au] = 0x08u;
        idt[0x6du] = 0x06u;
    }
    return write_bytes(state->machine, GDT_PTR, gdt_pointer, sizeof(gdt_pointer)) &&
        write_bytes(state->machine, IDT_PTR, idt_pointer, sizeof(idt_pointer)) &&
        write_bytes(state->machine, GDT_BASE, gdt, sizeof(gdt)) &&
        write_bytes(state->machine, IDT_BASE, idt, sizeof(idt)) &&
        write_bytes(state->machine, TSS_BASE + 2u, (const type_unsigned_8 *)&sp0, sizeof(sp0)) &&
        write_bytes(state->machine, TSS_BASE + 4u, (const type_unsigned_8 *)&ss0, sizeof(ss0)) &&
        write_bytes(state->machine, 0u, real_code, sizeof(real_code)) &&
        write_bytes(state->machine, KERNEL_BASE, kernel_entry, sizeof(kernel_entry)) &&
        write_bytes(state->machine, KERNEL_BASE + 0x100u, kernel_handler,
            sizeof(kernel_handler)) &&
        write_bytes(state->machine, KERNEL_BASE + 0x110u, kernel_stop,
            sizeof(kernel_stop)) &&
        write_bytes(state->machine, KERNEL_BASE + 0x120u, kernel_fault,
            sizeof(kernel_fault)) &&
        write_bytes(state->machine, USER_CODE_BASE,
            fault_delivery && negative_case == PRIVILEGE_NEGATIVE_NONE ?
                user_fault_code : user_program,
            fault_delivery && negative_case == PRIVILEGE_NEGATIVE_NONE ?
                sizeof(user_fault_code) : user_program_size);
}

static C_INT privilege_test_fault_delivery(core_machine_cpu_profile profile)
{
    privilege_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    type_unsigned_16 marker = 0u;
    const core_machine_run_budget budget = { 1024u, 0u };
    C_INT failed = !privilege_prepare(&state, profile);

    if (!failed) {
        failed |= !privilege_install(&state, 1, PRIVILEGE_NEGATIVE_NONE);
        failed |= core_machine_run(state.machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        failed |= core_machine_memory_read(state.machine, USER_DATA_BASE + 4u,
            &marker, sizeof(marker)) != TYPE_STATUS_OK || marker != 0x3333u;
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid ||
            diagnostic.delivered_exception_count != 1u ||
            !TYPE_GET_BIT(diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_GP) ||
            diagnostic.last_delivered_exception.exception_code != 0x0192u;
    }
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT privilege_test_not_present(core_machine_cpu_profile profile,
    privilege_negative_case negative_case, type_unsigned_16 expected_code)
{
    privilege_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    type_unsigned_16 marker = 0u;
    const core_machine_run_budget budget = { 1024u, 0u };
    C_INT failed = !privilege_prepare(&state, profile);

    if (!failed) {
        failed |= !privilege_install(&state, 1, negative_case);
        failed |= core_machine_run(state.machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        failed |= core_machine_memory_read(state.machine, USER_DATA_BASE + 4u,
            &marker, sizeof(marker)) != TYPE_STATUS_OK || marker != 0x3333u;
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid ||
            diagnostic.delivered_exception_count != 1u ||
            !TYPE_GET_BIT(diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_NP) ||
            diagnostic.last_delivered_exception.exception_code != expected_code;
        if (failed) {
            STD_FPRINTF(STD_STDERR,
                "T263 S5 np result=%u first=%d delivered=%x/%x count=%u marker=%04x\n",
                (unsigned)result.reason, diagnostic.first_fault.valid,
                diagnostic.last_delivered_exception.exception_mask,
                diagnostic.last_delivered_exception.exception_code,
                diagnostic.delivered_exception_count, marker);
        }
    }
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT privilege_test_stack_atomicity(C_VOID)
{
    privilege_machine state;
    core_machine_run_result result;
    type_status run_status;
    t_cpu cpu;
    const core_machine_run_budget budget = { 1024u, 0u };
    C_INT failed = !privilege_prepare(&state, CORE_MACHINE_CPU_PROFILE_80286);

    if (!failed) {
        failed |= !privilege_install(&state, 0,
            PRIVILEGE_NEGATIVE_STACK_ATOMICITY);
        run_status = core_machine_run(state.machine, budget, &result);
        cpu = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= run_status != TYPE_STATUS_FAULT ||
            result.reason != CORE_MACHINE_STOP_FAULT;
        failed |= cpu.data.cs.selector != 0x001bu || cpu.data.cs.dpl != 3u ||
            cpu.data.ss.selector != 0x0023u || cpu.data.sp != 0xa000u;
        if (failed) {
            STD_FPRINTF(STD_STDERR,
                "T263 S5 atomic status=%u result=%u cs=%04x/%u ss=%04x sp=%04x\n",
                (unsigned)run_status, (unsigned)result.reason, cpu.data.cs.selector,
                cpu.data.cs.dpl, cpu.data.ss.selector, cpu.data.sp);
        }
    }
    core_machine_destroy(state.machine);
    return failed;
}

int main(void)
{
    privilege_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu cpu;
    type_unsigned_16 markers[2] = {0u, 0u};
    const core_machine_run_budget budget = { 1024u, 0u };
    C_INT failed = !privilege_prepare(&state, CORE_MACHINE_CPU_PROFILE_80286);

    if (!failed) {
        failed |= !privilege_install(&state, 0, PRIVILEGE_NEGATIVE_NONE);
        failed |= core_machine_run(state.machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        failed |= core_machine_memory_read(state.machine, USER_DATA_BASE, markers,
            sizeof(markers)) != TYPE_STATUS_OK || markers[0] != 0x1111u ||
            markers[1] != 0x2222u;
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            diagnostic.last_delivered_exception.valid ||
            diagnostic.delivered_exception_count != 0u;
        cpu = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        if (failed) {
            STD_FPRINTF(STD_STDERR,
                "T259 result=%u markers=%04x/%04x fault=%d delivered=%d/%u cs=%04x sp=%04x\n",
                (unsigned)result.reason, markers[0], markers[1],
                diagnostic.first_fault.valid,
                diagnostic.last_delivered_exception.valid,
                diagnostic.delivered_exception_count, cpu.data.cs.selector,
                cpu.data.sp);
        }
    }
    core_machine_destroy(state.machine);
    failed |= privilege_test_fault_delivery(CORE_MACHINE_CPU_PROFILE_80286);
    failed |= privilege_test_fault_delivery(CORE_MACHINE_CPU_PROFILE_80386);
    failed |= privilege_test_not_present(CORE_MACHINE_CPU_PROFILE_80286,
        PRIVILEGE_NEGATIVE_GATE_NOT_PRESENT, 0x0182u);
    failed |= privilege_test_not_present(CORE_MACHINE_CPU_PROFILE_80286,
        PRIVILEGE_NEGATIVE_GP_GATE_NOT_PRESENT, 0x006au);
    failed |= privilege_test_not_present(CORE_MACHINE_CPU_PROFILE_80286,
        PRIVILEGE_NEGATIVE_CODE_NOT_PRESENT, 0x0030u);
    failed |= privilege_test_stack_atomicity();
    if (failed) return 1;
    STD_PRINTF("M5:T259:S2:PROTECTED-PRIVILEGE:OK\n");
    STD_PRINTF("M5:T259:S3:PROTECTED-PRIVILEGE:CORPUS:OK\n");
    STD_PRINTF("M5:T263:S5:PROTECTED-IDT-ATOMICITY:OK\n");
    STD_PRINTF("M5:T263:S6:SYNC-IDT-ERROR-CODE:OK\n");
    STD_PRINTF("M5:T263:S6:SYNC-IDT-ERROR-CODE:OK\n");
    return 0;
}
