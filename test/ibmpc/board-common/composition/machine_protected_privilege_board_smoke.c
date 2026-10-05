#include "lib/types/types_interface.h"
#include <stdio.h>
#include "x86/core/device_support_interface.h"

#include "x86/chips/cpu/cpu_interface.h"
#include "x86/core/debug_interface.h"
#include "ibmpc/board-common/machine_board_interface.h"

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

static lib_i32 privilege_prepare(privilege_machine *state,
    core_machine_cpu_profile profile)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP),
        .values = {0}
    };

    if (state == LIB_NULL) return 0;
    lib_memory_set(state, 0, sizeof(*state));
    if (core_machine_create(&config, &state->machine, LIB_NULL) != LIB_STATUS_OK ||
        state->machine == LIB_NULL ||
        core_machine_freeze_execution_providers(state->machine) != LIB_STATUS_OK ||
        core_machine_reset(state->machine) != LIB_STATUS_OK ||
        core_machine_debug_patch_registers(state->machine, &entry) != LIB_STATUS_OK) {
        core_machine_destroy(state->machine);
        state->machine = LIB_NULL;
        return 0;
    }
    return 1;
}

static lib_i32 write_bytes(core_machine *machine, lib_u32 address,
    const lib_u8 *bytes, lib_size count)
{
    return core_machine_memory_write(machine, address, bytes, count) ==
        LIB_STATUS_OK;
}

/* A delivered exception is a completed CPU event, not necessarily the guest
 * handler's terminal instruction.  Continue exactly once when the first run
 * stopped at that delivery, preserving the former fixture's explicit test
 * contract without restoring its private macro alias. */
static lib_status privilege_run(core_machine *machine,
    core_machine_run_budget budget, core_machine_run_result *out_result)
{
    core_machine_cpu_diagnostic diagnostic = {0};
    lib_status status = core_machine_run(machine, budget, out_result);

    if (status == LIB_STATUS_OK && out_result != LIB_NULL &&
        out_result->reason == CORE_MACHINE_STOP_BUDGET) {
        status = core_machine_get_cpu_diagnostic(machine, &diagnostic);
        if (status == LIB_STATUS_OK && diagnostic.last_delivered_exception.valid)
            status = core_machine_run(machine, budget, out_result);
    }
    return status;
}

static lib_i32 privilege_install(privilege_machine *state, lib_i32 fault_delivery,
    privilege_negative_case negative_case)
{
    static const lib_u8 gdt_pointer[] = { 0x37u, 0x00u, 0x00u, 0x03u, 0x00u, 0x00u };
    static const lib_u8 idt_pointer[] = { 0x97u, 0x01u, 0x00u, 0x04u, 0x00u, 0x00u };
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xff,0xff,0,0x20,0,0x9a,0,0,
        0xff,0xff,0,0x30,0,0x92,0,0,
        0xff,0xff,0,0x40,0,0xfa,0,0,
        0xff,0xff,0,0x50,0,0xf2,0,0,
        0x2b,0,0,0x06,0,0x81,0,0,
        0xff,0xff,0,0x20,0,0x1a,0,0
    };
    lib_u8 idt[0x198u] = {0};
    static const lib_u8 real_code[] = {
        0x0f,0x01,0x16,0x00,0x01,
        0x0f,0x01,0x1e,0x10,0x01,
        0xb8,0x01,0x00,0x0f,0x01,0xf0,
        0xb8,0x28,0x00,0x0f,0x00,0xd8,
        0xb8,0x10,0x00,0x8e,0xd0,0xbc,0x00,0x80,
        0xea,0x00,0x00,0x08,0x00
    };
    static const lib_u8 kernel_entry[] = {
        0xb8,0x10,0x00,0x8e,0xd8,
        0xb8,0x23,0x00,0x50,
        0xb8,0x00,0xa0,0x50,
        0xb8,0x02,0x02,0x50,
        0xb8,0x1b,0x00,0x50,
        0xb8,0x00,0x00,0x50,
        0xb8,0x23,0x00,0x8e,0xd8,
        0xcf
    };
    static const lib_u8 kernel_handler[] = {
        0xb8,0x11,0x11,0xa3,0x00,0x00,0xcf
    };
    static const lib_u8 kernel_stop[] = { 0xf4 };
    static const lib_u8 kernel_fault[] = {
        0xb8,0x33,0x33,0xa3,0x04,0x00,0xf4
    };
    static const lib_u8 user_code[] = {
        0xcd,0x30,0xb8,0x22,0x22,0xa3,0x02,0x00,0xcd,0x31
    };
    static const lib_u8 user_fault_code[] = { 0xcd,0x32 };
    static const lib_u8 user_gate_not_present[] = { 0xcd,0x30 };
    static const lib_u8 user_gp_gate_not_present[] = { 0xcd,0x0d };
    static const lib_u8 user_code_not_present[] = { 0xcd,0x31 };
    const lib_u8 *user_program = user_code;
    lib_size user_program_size = sizeof(user_code);
    lib_u16 sp0 = 0x9000u;
    lib_u16 ss0 = 0x0010u;

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
        write_bytes(state->machine, TSS_BASE + 2u, (const lib_u8 *)&sp0, sizeof(sp0)) &&
        write_bytes(state->machine, TSS_BASE + 4u, (const lib_u8 *)&ss0, sizeof(ss0)) &&
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

static lib_i32 privilege_test_delivery(core_machine_cpu_profile profile,
    privilege_negative_case negative_case, lib_u32 exception_mask, lib_u16 expected_code)
{
    privilege_machine state = {0};
    core_machine_run_result result = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    lib_u16 marker = 0u;
    const core_machine_run_budget budget = { 1024u, 0u };
    lib_i32 failed = 1;

    if (!privilege_prepare(&state, profile) || !privilege_install(&state, 1, negative_case) ||
        privilege_run(state.machine, budget, &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        core_machine_memory_read(state.machine, USER_DATA_BASE + 4u,
            &marker, sizeof(marker)) != LIB_STATUS_OK || marker != 0x3333u ||
        core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != LIB_STATUS_OK ||
        diagnostic.first_fault.valid || !diagnostic.last_delivered_exception.valid ||
        diagnostic.delivered_exception_count != 1u ||
        !CORE_MACHINE_BIT_IS_SET(diagnostic.last_delivered_exception.exception_mask,
            exception_mask) ||
        diagnostic.last_delivered_exception.exception_code != expected_code) goto done;
    failed = 0;
done:
    if (failed) {
        fprintf(stderr,
            "T263 S5 delivery result=%u first=%d delivered=%x/%x count=%u marker=%04x\n",
            (unsigned)result.reason, diagnostic.first_fault.valid,
            diagnostic.last_delivered_exception.exception_mask,
            diagnostic.last_delivered_exception.exception_code,
            diagnostic.delivered_exception_count, marker);
    }
    core_machine_destroy(state.machine);
    return failed;
}

static lib_i32 privilege_test_stack_atomicity(void)
{
    privilege_machine state = {0};
    core_machine_run_result result = {0};
    lib_status run_status = LIB_STATUS_INVALID_STATE;
    core_machine_debug_cpu_snapshot cpu = {0};
    const core_machine_run_budget budget = { 1024u, 0u };
    lib_i32 failed = 1;

    if (!privilege_prepare(&state, CORE_MACHINE_CPU_PROFILE_80286) ||
        !privilege_install(&state, 0, PRIVILEGE_NEGATIVE_STACK_ATOMICITY)) goto done;
    run_status = privilege_run(state.machine, budget, &result);
    if (run_status != LIB_STATUS_INTERNAL_ERROR || result.reason != CORE_MACHINE_STOP_FAULT ||
        core_machine_debug_capture_cpu_snapshot(state.machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &cpu) != LIB_STATUS_OK ||
        cpu.cs.selector != 0x001bu || cpu.cs.dpl != 3u ||
        cpu.ss.selector != 0x0023u || cpu.esp != 0x0000a000u) goto done;
    failed = 0;
done:
    if (failed) {
        fprintf(stderr,
            "T263 S5 atomic status=%u result=%u cs=%04x/%u ss=%04x sp=%04x\n",
            (unsigned)run_status, (unsigned)result.reason, cpu.cs.selector,
            cpu.cs.dpl, cpu.ss.selector, cpu.esp);
    }
    core_machine_destroy(state.machine);
    return failed;
}

int main(void)
{
    privilege_machine state = {0};
    core_machine_run_result result = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    core_machine_debug_cpu_snapshot cpu = {0};
    lib_u16 markers[2] = {0u, 0u};
    const core_machine_run_budget budget = { 1024u, 0u };
    lib_i32 failed = 1;

    if (!privilege_prepare(&state, CORE_MACHINE_CPU_PROFILE_80286) ||
        !privilege_install(&state, 0, PRIVILEGE_NEGATIVE_NONE) ||
        privilege_run(state.machine, budget, &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        core_machine_memory_read(state.machine, USER_DATA_BASE, markers,
            sizeof(markers)) != LIB_STATUS_OK || markers[0] != 0x1111u || markers[1] != 0x2222u ||
        core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != LIB_STATUS_OK ||
        diagnostic.first_fault.valid || diagnostic.last_delivered_exception.valid ||
        diagnostic.delivered_exception_count != 0u ||
        core_machine_debug_capture_cpu_snapshot(state.machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &cpu) != LIB_STATUS_OK) goto done;
    failed = 0;
done:
    if (failed) {
        fprintf(stderr,
            "T259 result=%u markers=%04x/%04x fault=%d delivered=%d/%u cs=%04x sp=%04x\n",
            (unsigned)result.reason, markers[0], markers[1],
            diagnostic.first_fault.valid, diagnostic.last_delivered_exception.valid,
            diagnostic.delivered_exception_count, cpu.cs.selector, cpu.esp);
    }
    core_machine_destroy(state.machine);
    if (failed ||
        privilege_test_delivery(CORE_MACHINE_CPU_PROFILE_80286,
            PRIVILEGE_NEGATIVE_NONE, VCPUINS_EXCEPT_GP, 0x0192u) ||
        privilege_test_delivery(CORE_MACHINE_CPU_PROFILE_80386,
            PRIVILEGE_NEGATIVE_NONE, VCPUINS_EXCEPT_GP, 0x0192u) ||
        privilege_test_delivery(CORE_MACHINE_CPU_PROFILE_80286,
            PRIVILEGE_NEGATIVE_GATE_NOT_PRESENT, VCPUINS_EXCEPT_NP, 0x0182u) ||
        privilege_test_delivery(CORE_MACHINE_CPU_PROFILE_80286,
            PRIVILEGE_NEGATIVE_GP_GATE_NOT_PRESENT, VCPUINS_EXCEPT_NP, 0x006au) ||
        privilege_test_delivery(CORE_MACHINE_CPU_PROFILE_80286,
            PRIVILEGE_NEGATIVE_CODE_NOT_PRESENT, VCPUINS_EXCEPT_NP, 0x0030u) ||
        privilege_test_stack_atomicity()) return 1;
    printf("M5:T259:S2:PROTECTED-PRIVILEGE:OK\n");
    printf("M5:T259:S3:PROTECTED-PRIVILEGE:CORPUS:OK\n");
    printf("M5:T263:S5:PROTECTED-IDT-ATOMICITY:OK\n");
    printf("M5:T263:S6:SYNC-IDT-ERROR-CODE:OK\n");
    return 0;
}
