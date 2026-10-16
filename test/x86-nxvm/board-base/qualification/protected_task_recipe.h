#ifndef TEST_BOARD_QUALIFICATION_PROTECTED_TASK_RECIPE_H
#define TEST_BOARD_QUALIFICATION_PROTECTED_TASK_RECIPE_H
#include "lib/types/types_interface.h"
#include "x86-core/machine/machine_interface.h"
#include "x86-core/machine/debug_interface.h"
#include "x86-core/machine/memory_interface.h"

/* Finite guest descriptor/program setup for the manual-ledger recipes.
 * These public-only inputs do not include another package's unit program. */
#define GDT_POINTER 0x0100u
#define GDT_BASE 0x0300u
#define IDT_BASE 0x0400u
#define TASK_A_BASE 0x0600u
#define TASK_B_BASE 0x0700u
#define KERNEL_BASE 0x2000u

typedef struct task_switch_fixture {
    x86_core_machine *machine;
} task_switch_fixture;

typedef enum task_switch_case {
    TASK_SWITCH_CASE_SUCCESS = 0,
    TASK_SWITCH_CASE_INVALID_SELECTOR,
    TASK_SWITCH_CASE_NOT_PRESENT,
    TASK_SWITCH_CASE_BUSY,
    TASK_SWITCH_CASE_SHORT_TSS,
    TASK_SWITCH_CASE_STACK_LIMIT,
    TASK_SWITCH_CASE_INDIRECT_SUCCESS,
    TASK_SWITCH_CASE_OPERAND32_SUCCESS,
    TASK_SWITCH_CASE_INDIRECT_OPERAND32_SUCCESS,
    TASK_SWITCH_CASE_INDIRECT_ADDRESS32_SUCCESS,
    TASK_SWITCH_CASE_INDIRECT_OPERAND_ADDRESS32_SUCCESS,
    TASK_SWITCH_CASE_IRQ_SUCCESS,
    TASK_SWITCH_CASE_LOCK_REJECT,
    TASK_SWITCH_CASE_CALL_SUCCESS,
    TASK_SWITCH_CASE_TASK_GATE_SUCCESS,
    TASK_SWITCH_CASE_IDT_TASK_GATE,
    TASK_SWITCH_CASE_NESTED_RETURN,
    TASK_SWITCH_CASE_LDT_SUCCESS,
    TASK_SWITCH_CASE_LDT_NOT_PRESENT
} task_switch_case;

static lib_i32 write_bytes(x86_core_machine *machine, lib_u32 address,
    const lib_u8 *bytes, lib_size count)
{
    return x86_core_machine_memory_write(machine, address, bytes, count) ==
        LIB_STATUS_OK;
}

static lib_i32 task_switch_prepare(task_switch_fixture *fixture,
    x86_core_cpu_profile profile)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = (2u * 1024u * 1024u),
        .cpu_profile = profile,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };

    if (fixture == LIB_NULL) return 0;
    lib_memory_set(fixture, 0, sizeof(*fixture));
    if (x86_core_machine_neutral_create(&config, &fixture->machine) != LIB_STATUS_OK ||
        fixture->machine == LIB_NULL ||
        x86_core_machine_freeze_execution_providers(fixture->machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(fixture->machine) != LIB_STATUS_OK ||
        x86_core_machine_debug_patch_registers(fixture->machine,
            &(x86_core_cpu_debug_register_patch){
                .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
                    X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_DS) |
                    X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ES) |
                    X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_SS) |
                    X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP)
            }) != LIB_STATUS_OK) {
        x86_core_machine_destroy(fixture->machine);
        fixture->machine = LIB_NULL;
        return 0;
    }
    return 1;
}

static lib_i32 task_switch_install(task_switch_fixture *fixture,
    task_switch_case test_case)
{
    static const lib_u8 gdt_pointer[] = { 0x47u,0,0x00u,0x03u,0,0 };
    lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xff,0xff,0,0x20,0,0x9a,0,0,
        0xff,0xff,0,0x30,0,0x92,0,0,
        0,0,0,0x30,0,0x92,0,0,
        0,0,0,0,0,0,0,0,
        0x2b,0,0,0x06,0,0x81,0,0,
        0x2b,0,0,0x07,0,0x81,0,0,
        0,0,0x30u,0,0,0x85u,0,0
    };
    static const lib_u8 real_code[] = {
        0x0f,0x01,0x16,0x00,0x01,
        0xb8,0x01,0x00,0x0f,0x01,0xf0,
        0xb8,0x28,0x00,0x0f,0x00,0xd8,
        0xb8,0x10,0x00,0x8e,0xd0,0xbc,0x00,0x80,
        0xea,0x00,0x00,0x08,0x00
    };
    static const lib_u8 real_code_with_ds[] = {
        0x0f,0x01,0x16,0x00,0x01,
        0xb8,0x01,0x00,0x0f,0x01,0xf0,
        0xb8,0x28,0x00,0x0f,0x00,0xd8,
        0xb8,0x10,0x00,0x8e,0xd0,0x8e,0xd8,0x8e,0xc0,0xbc,0x00,0x80,
        0xea,0x00,0x00,0x08,0x00
    };
    const lib_u8 *bootstrap_code = real_code;
    lib_size bootstrap_bytes = sizeof(real_code);
    lib_u8 kernel_code[] = {
        0xb8,0x11,0x11,0xea,0x00,0x00,0x30,0x00,0,0,0,0
    };
    static const lib_u8 indirect_pointer[] = { 0,0,0x30,0 };
    static const lib_u8 indirect_pointer32[] = { 0,0,0,0,0x30,0 };
    lib_u8 ldt[] = {
        0,0,0,0,0,0,0,0,
        0xff,0xff,0,0x20,0,0x9a,0,0,
        0xff,0xff,0,0x30,0,0x92,0,0
    };
    lib_u8 ldt_descriptor[] = {
        0x17u,0,0,0x09u,0,0x82u,0,0
    };
    lib_u8 task_b_state[] = {
        0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0,
        0x00,0x01, 0x02,0x00, 0x22,0x22, 0,0, 0,0, 0,0,
        0x00,0x80, 0,0, 0,0, 0,0,
        0x10,0x00, 0x08,0x00, 0x10,0x00, 0x10,0x00, 0,0
    };
    lib_u8 task_b_code[] = {
        0xb8,0x22,0x22,0xa3,0x00,0x00,0xf4
    };

    switch (test_case) {
    case TASK_SWITCH_CASE_LOCK_REJECT:
        kernel_code[3] = 0xf0u;
        kernel_code[4] = 0xeau;
        kernel_code[5] = 0u;
        kernel_code[6] = 0u;
        kernel_code[7] = 0x30u;
        break;
    case TASK_SWITCH_CASE_IRQ_SUCCESS:
        task_b_state[16] = 0x02u;
        task_b_state[17] = 0x02u;
        break;
    case TASK_SWITCH_CASE_CALL_SUCCESS:
        kernel_code[3] = 0x9au;
        kernel_code[4] = 0u;
        kernel_code[5] = 0u;
        kernel_code[6] = 0x30u;
        kernel_code[7] = 0u;
        break;
    case TASK_SWITCH_CASE_TASK_GATE_SUCCESS:
        kernel_code[3] = 0x9au;
        kernel_code[4] = 0u;
        kernel_code[5] = 0u;
        kernel_code[6] = 0x38u;
        kernel_code[7] = 0u;
        break;
    case TASK_SWITCH_CASE_NESTED_RETURN:
        bootstrap_code = real_code_with_ds;
        bootstrap_bytes = sizeof(real_code_with_ds);
        kernel_code[3] = 0x9au;
        kernel_code[4] = 0u;
        kernel_code[5] = 0u;
        kernel_code[6] = 0x30u;
        kernel_code[7] = 0u;
        break;
    case TASK_SWITCH_CASE_IDT_TASK_GATE:
        kernel_code[3] = 0xccu;
        break;
    case TASK_SWITCH_CASE_LDT_SUCCESS:
    case TASK_SWITCH_CASE_LDT_NOT_PRESENT:
        task_b_state[34] = 0x14u;
        task_b_state[36] = 0x0cu;
        task_b_state[38] = 0x14u;
        task_b_state[40] = 0x14u;
        task_b_state[42] = 0x40u;
        if (test_case == TASK_SWITCH_CASE_LDT_NOT_PRESENT) {
            ldt_descriptor[5] = 0x02u;
        }
        break;
    case TASK_SWITCH_CASE_INDIRECT_OPERAND_ADDRESS32_SUCCESS:
        bootstrap_code = real_code_with_ds;
        bootstrap_bytes = sizeof(real_code_with_ds);
        kernel_code[3] = 0x66u;
        kernel_code[4] = 0x67u;
        kernel_code[5] = 0xffu;
        kernel_code[6] = 0x2du;
        kernel_code[7] = 0x00u;
        kernel_code[8] = 0x22u;
        kernel_code[9] = 0u;
        kernel_code[10] = 0u;
        break;
    case TASK_SWITCH_CASE_INDIRECT_ADDRESS32_SUCCESS:
        bootstrap_code = real_code_with_ds;
        bootstrap_bytes = sizeof(real_code_with_ds);
        kernel_code[3] = 0x67u;
        kernel_code[4] = 0xffu;
        kernel_code[5] = 0x2du;
        kernel_code[6] = 0x00u;
        kernel_code[7] = 0x22u;
        kernel_code[8] = 0u;
        kernel_code[9] = 0u;
        break;
    case TASK_SWITCH_CASE_INDIRECT_OPERAND32_SUCCESS:
        bootstrap_code = real_code_with_ds;
        bootstrap_bytes = sizeof(real_code_with_ds);
        kernel_code[3] = 0x66u;
        kernel_code[4] = 0xffu;
        kernel_code[5] = 0x2eu;
        kernel_code[6] = 0x00u;
        kernel_code[7] = 0x22u;
        break;
    case TASK_SWITCH_CASE_OPERAND32_SUCCESS:
        kernel_code[3] = 0x66u;
        kernel_code[4] = 0xeau;
        kernel_code[5] = 0u;
        kernel_code[6] = 0u;
        kernel_code[7] = 0u;
        kernel_code[8] = 0u;
        kernel_code[9] = 0x30u;
        kernel_code[10] = 0u;
        break;
    case TASK_SWITCH_CASE_INDIRECT_SUCCESS:
        bootstrap_code = real_code_with_ds;
        bootstrap_bytes = sizeof(real_code_with_ds);
        kernel_code[3] = 0xffu;
        kernel_code[4] = 0x2eu;
        kernel_code[5] = 0x00u;
        kernel_code[6] = 0x22u;
        kernel_code[7] = 0x90u;
        break;
    case TASK_SWITCH_CASE_INVALID_SELECTOR:
        kernel_code[6] = 0x40u;
        break;
    case TASK_SWITCH_CASE_NOT_PRESENT:
        gdt[53] = 0x01u;
        break;
    case TASK_SWITCH_CASE_BUSY:
        gdt[53] = 0x83u;
        break;
    case TASK_SWITCH_CASE_SHORT_TSS:
        gdt[48] = 0x2au;
        break;
    case TASK_SWITCH_CASE_STACK_LIMIT:
        task_b_state[26] = 0u;
        task_b_state[27] = 0u;
        task_b_state[34] = 0x18u;
        task_b_state[38] = 0x18u;
        task_b_state[40] = 0x18u;
        task_b_code[0] = 0x58u;
        task_b_code[1] = 0xf4u;
        break;
    default:
        break;
    }

    return write_bytes(fixture->machine, GDT_POINTER, gdt_pointer,
            sizeof(gdt_pointer)) &&
        write_bytes(fixture->machine, GDT_BASE, gdt, sizeof(gdt)) &&
        write_bytes(fixture->machine, TASK_A_BASE, (const lib_u8[44]){0}, 44u) &&
        write_bytes(fixture->machine, TASK_B_BASE, task_b_state,
            sizeof(task_b_state)) &&
        ((test_case != TASK_SWITCH_CASE_LDT_SUCCESS &&
            test_case != TASK_SWITCH_CASE_LDT_NOT_PRESENT) ||
            (write_bytes(fixture->machine, GDT_BASE + 0x40u, ldt_descriptor,
                sizeof(ldt_descriptor)) && write_bytes(fixture->machine,
                0x0900u, ldt, sizeof(ldt)))) &&
        write_bytes(fixture->machine, 0u, bootstrap_code, bootstrap_bytes) &&
        write_bytes(fixture->machine, KERNEL_BASE, kernel_code,
            sizeof(kernel_code)) &&
        (test_case != TASK_SWITCH_CASE_INDIRECT_SUCCESS ||
            write_bytes(fixture->machine, 0x5200u,
                indirect_pointer, sizeof(indirect_pointer))) &&
        (test_case != TASK_SWITCH_CASE_INDIRECT_ADDRESS32_SUCCESS ||
            write_bytes(fixture->machine, 0x5200u,
                indirect_pointer, sizeof(indirect_pointer))) &&
        (test_case != TASK_SWITCH_CASE_INDIRECT_OPERAND32_SUCCESS ||
            write_bytes(fixture->machine, 0x5200u,
                indirect_pointer32, sizeof(indirect_pointer32))) &&
        (test_case != TASK_SWITCH_CASE_INDIRECT_OPERAND_ADDRESS32_SUCCESS ||
            write_bytes(fixture->machine, 0x5200u,
                indirect_pointer32, sizeof(indirect_pointer32))) &&
        write_bytes(fixture->machine, KERNEL_BASE + 0x100u, task_b_code,
            sizeof(task_b_code));
}
#endif
