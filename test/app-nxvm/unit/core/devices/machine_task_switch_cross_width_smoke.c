#include "support/pic_fixture.h"
#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"

#include "app-nxvm/devices/cpu.h"
#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/devices/memory_interface.h"
#include "app-nxvm/devices/pic_bus.h"
#include "support/core_machine_cpu_fixture.h"

#define GDT_POINTER 0x0100u
#define GDT_BASE 0x0300u
#define IDT_BASE 0x0400u
#define TASK_A_BASE 0x0600u
#define TASK_B_BASE 0x0700u
#define KERNEL_BASE 0x2000u

typedef struct task_switch_fixture {
    core_machine *machine;
} task_switch_fixture;

typedef enum task_switch_case {
    TASK_SWITCH_CASE_SUCCESS = 0,
    TASK_SWITCH_CASE_INVALID_SELECTOR,
    TASK_SWITCH_CASE_NOT_PRESENT,
    TASK_SWITCH_CASE_BUSY,
    TASK_SWITCH_CASE_SHORT_TSS,
    TASK_SWITCH_CASE_STACK_LIMIT,
    TASK_SWITCH_CASE_INDIRECT_SUCCESS,
    /* S65's 80386 timing includer still uses these construction recipes. */
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

static void task_switch_reset(void *opaque)
{
    task_switch_fixture *fixture = (task_switch_fixture *)opaque;

    if (fixture != LIB_NULL) (void)test_core_machine_fixture_reset_real_mode(
        fixture->machine);
}

static const core_machine_execution_provider task_switch_provider = {
    task_switch_reset, LIB_NULL
};

static lib_i32 write_bytes(core_machine *machine, lib_u32 address,
    const lib_u8 *bytes, lib_size count)
{
    return core_machine_memory_write(machine, address, bytes, count) ==
        LIB_STATUS_OK;
}

static lib_i32 task_switch_prepare(task_switch_fixture *fixture,
    core_machine_cpu_profile profile)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };

    if (fixture == LIB_NULL) return 0;
    lib_memory_set(fixture, 0, sizeof(*fixture));
    if (core_machine_create(&config, &fixture->machine) != LIB_STATUS_OK) return 0;
    if (!test_core_machine_fixture_bind_freeze_reset(fixture->machine,
            &task_switch_provider, fixture)) {
        core_machine_destroy(fixture->machine);
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

typedef struct task_switch_smoke_tss32_selector {
    lib_u16 selector;
    lib_u16 reserved;
} task_switch_smoke_tss32_selector;

typedef struct task_switch_smoke_tss32_state {
    lib_u32 cr3;
    lib_u32 eip;
    lib_u32 eflags;
    lib_u32 eax;
    lib_u32 ecx;
    lib_u32 edx;
    lib_u32 ebx;
    lib_u32 esp;
    lib_u32 ebp;
    lib_u32 esi;
    lib_u32 edi;
    task_switch_smoke_tss32_selector es;
    task_switch_smoke_tss32_selector cs;
    task_switch_smoke_tss32_selector ss;
    task_switch_smoke_tss32_selector ds;
    task_switch_smoke_tss32_selector fs;
    task_switch_smoke_tss32_selector gs;
    task_switch_smoke_tss32_selector ldtr;
} task_switch_smoke_tss32_state;

_Static_assert(sizeof(task_switch_smoke_tss32_state) == 0x48u,
    "TSS32 test image must retain the Intel saved-state span");

typedef enum task_switch_tss32_rejection {
    TASK_SWITCH_TSS32_REJECTION_NONE = 0,
    TASK_SWITCH_TSS32_REJECTION_INVALID_CODE,
    TASK_SWITCH_TSS32_REJECTION_TARGET_BUSY,
    TASK_SWITCH_TSS32_REJECTION_OLD_SHORT,
    TASK_SWITCH_TSS32_REJECTION_TARGET_SHORT,
    TASK_SWITCH_TSS32_REJECTION_STACK_LIMIT,
    TASK_SWITCH_TSS32_REJECTION_NESTED_RETURN,
    TASK_SWITCH_TSS32_LDT_SUCCESS,
    TASK_SWITCH_TSS32_LDT_BAD_DESCRIPTOR,
    TASK_SWITCH_TSS32_LDT_NOT_PRESENT,
    TASK_SWITCH_TSS32_LDT_SHORT,
    TASK_SWITCH_TSS32_LDT_BAD_CODE,
    TASK_SWITCH_TSS32_LDT_BAD_DATA,
    TASK_SWITCH_TSS32_DEBUG_TRAP_SUCCESS
} task_switch_tss32_rejection;

static lib_i32 task_switch_expect_t330_16_to_32(lib_u8 nested,
    lib_u8 task_gate, lib_u8 task_return)
{
    task_switch_fixture fixture;
    core_machine_run_result result;
    task_switch_smoke_tss32_state target = {
        .eip = 0x100u, .eflags = 0x2u, .eax = 0xa1a12222u,
        .ecx = 0xc1c13333u, .edx = 0xd1d14444u, .ebx = 0xb1b15555u,
        .esp = 0x8000u, .ebp = 0xe1e16666u, .esi = 0xf1f17777u,
        .edi = 0x81818888u, .es = {0x10u, 0u}, .cs = {0x08u, 0u},
        .ss = {0x10u, 0u}, .ds = {0x10u, 0u}, .fs = {0x10u, 0u},
        .gs = {0x10u, 0u}, .ldtr = {0u, 0u}
    };
    lib_u8 descriptor[] = { 0x67u, 0u, 0u, 0x07u, 0u, 0x89u, 0u, 0u };
    static const lib_u8 iret[] = { 0xcfu, 0xf4u };
    lib_u16 saved_ldtr = 0xffffu;
    lib_u8 busy[2] = {0u, 0u};
    t_cpu cpu;
    const core_machine_run_budget budget = {128u, 0u};
    lib_i32 failed = !task_switch_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        failed |= !task_switch_install(&fixture, nested ?
            (task_gate ? TASK_SWITCH_CASE_TASK_GATE_SUCCESS :
                TASK_SWITCH_CASE_CALL_SUCCESS) : TASK_SWITCH_CASE_SUCCESS) ||
            !write_bytes(fixture.machine, GDT_BASE + 0x30u, descriptor,
                sizeof(descriptor)) ||
            core_machine_memory_write(fixture.machine, TASK_B_BASE + 0x1cu,
                &target, sizeof(target)) != LIB_STATUS_OK ||
            (task_return && (!write_bytes(fixture.machine, KERNEL_BASE + 0x100u,
                iret, sizeof(iret)) || !write_bytes(fixture.machine,
                KERNEL_BASE + 8u, &iret[1], 1u))) ||
            core_machine_run(fixture.machine, budget, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        cpu = test_core_machine_fixture_capture_cpu_after_run(fixture.machine);
        failed |= cpu.data.tr.selector != (task_return ? 0x28u : 0x30u) ||
            cpu.data.tr.sys.type != (task_return ? VCPU_DESC_SYS_TYPE_TSS_16_BUSY :
                VCPU_DESC_SYS_TYPE_TSS_32_BUSY) || cpu.data.eax !=
                (task_return ? 0xffff1111u : target.eax) ||
            (!task_return && (cpu.data.ecx != target.ecx ||
                cpu.data.edx != target.edx || cpu.data.ebx != target.ebx ||
                cpu.data.esp != target.esp || cpu.data.ebp != target.ebp ||
                cpu.data.esi != target.esi || cpu.data.edi != target.edi)) ||
            cpu.data.cs.selector != 0x08u ||
            cpu.data.ss.selector != 0x10u || cpu.data.ds.selector !=
                (task_return ? 0u : 0x10u) || cpu.data.es.selector !=
                (task_return ? 0u : 0x10u) || cpu.data.fs.selector !=
                (task_return ? 0u : 0x10u) ||
            cpu.data.gs.selector != (task_return ? 0u : 0x10u) ||
            cpu.data.ldtr.flagValid ||
            !CORE_MACHINE_BIT_IS_SET(cpu.data.cr0, VCPU_CR0_TS) ||
            core_machine_memory_read(fixture.machine, TASK_A_BASE + 0x2au,
                &saved_ldtr, sizeof(saved_ldtr)) != LIB_STATUS_OK ||
            saved_ldtr != 0u || core_machine_memory_read(fixture.machine,
                GDT_BASE + 0x2du, &busy[0], 1u) != LIB_STATUS_OK ||
            core_machine_memory_read(fixture.machine, GDT_BASE + 0x35u,
                &busy[1], 1u) != LIB_STATUS_OK ||
            busy[0] != (task_return ? 0x83u : nested ? 0x83u : 0x81u) ||
            busy[1] != (task_return ? 0x89u : 0x8bu) ||
            (!task_return && nested && !CORE_MACHINE_BIT_IS_SET(cpu.data.eflags,
                VCPU_EFLAGS_NT));
    }
    core_machine_destroy(fixture.machine);
    return failed;
}

typedef struct task_switch_smoke_tss16_state {
    lib_u16 ip;
    lib_u16 flags;
    lib_u16 ax;
    lib_u16 cx;
    lib_u16 dx;
    lib_u16 bx;
    lib_u16 sp;
    lib_u16 bp;
    lib_u16 si;
    lib_u16 di;
    lib_u16 es;
    lib_u16 cs;
    lib_u16 ss;
    lib_u16 ds;
    lib_u16 ldtr;
} task_switch_smoke_tss16_state;

_Static_assert(sizeof(task_switch_smoke_tss16_state) == 0x1eu,
    "TSS16 test image must retain the complete Intel saved-state span");

static lib_i32 task_switch_expect_t330_32_to_16(lib_u8 nested,
    lib_u8 task_gate, lib_u8 task_return)
{
    task_switch_fixture fixture;
    core_machine_run_result result;
    task_switch_smoke_tss32_state outgoing;
    task_switch_smoke_tss16_state target = {
        .ip = 0x100u, .flags = 0x2u, .ax = 0x2222u, .cx = 0x3333u,
        .dx = 0x4444u, .bx = 0x5555u, .sp = 0x8000u, .bp = 0x6666u,
        .si = 0x7777u, .di = 0x8888u, .es = 0x10u, .cs = 0x08u,
        .ss = 0x10u, .ds = 0x10u, .ldtr = 0u
    };
    static const lib_u8 gdt_pointer[] = { 0x47u, 0u, 0u, 0x03u, 0u, 0u };
    static const lib_u8 bootstrap[] = {
        0x0fu,0x01u,0x16u,0u,0x01u, 0xb8u,1u,0u,0x0fu,0x01u,0xf0u,
        0xb8u,0x28u,0u,0x0fu,0u,0xd8u, 0xb8u,0x10u,0u,0x8eu,0xd0u,
        0x8eu,0xd8u,0x8eu,0xc0u,0x8eu,0xe0u,0x8eu,0xe8u,0xbcu,0u,
        0x80u,0xeau,0u,0u,0x08u,0u
    };
    static const lib_u8 target_halt[] = {
        0xb8u,0x22u,0x22u,0xa3u,0u,0u,0xf4u
    };
    static const lib_u8 target_iret[] = { 0xcfu, 0xf4u };
    lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0, 0xff,0xff,0,0x20,0,0x9a,0,0,
        0xff,0xff,0,0x30,0,0x92,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0x67,0,0,0x06,0,0x89,0,0,
        0x2b,0,0,0x07,0,0x81,0,0, 0,0,0x30,0,0,0x85,0,0
    };
    lib_u8 source[] = {
        0x66u,0xb8u,0x11u,0x11u,0x11u,0x11u,
        0x66u,0xb9u,0x22u,0x22u,0x22u,0x22u,
        0x66u,0xbau,0x33u,0x33u,0x33u,0x33u,
        0x66u,0xbbu,0x44u,0x44u,0x44u,0x44u,
        0x66u,0xbcu,0u,0x80u,0u,0u,
        0x66u,0xbdu,0x66u,0x66u,0x66u,0x66u,
        0x66u,0xbeu,0x77u,0x77u,0x77u,0x77u,
        0x66u,0xbfu,0x88u,0x88u,0x88u,0x88u,
        0xeau,0u,0u,0x30u,0u, 0xf4u
    };
    lib_u8 busy[2] = {0u, 0u};
    t_cpu cpu;
    const core_machine_run_budget budget = {128u, 0u};
    lib_i32 failed = !task_switch_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);

    if (nested) source[48] = 0x9au;
    if (task_gate) source[51] = 0x38u;
    if (!failed) {
        failed |= !write_bytes(fixture.machine, GDT_POINTER, gdt_pointer,
                sizeof(gdt_pointer)) || !write_bytes(fixture.machine, GDT_BASE,
                gdt, sizeof(gdt)) || !write_bytes(fixture.machine, 0u,
                bootstrap, sizeof(bootstrap)) || !write_bytes(fixture.machine,
                KERNEL_BASE, source, sizeof(source)) || !write_bytes(
                fixture.machine, KERNEL_BASE + 0x100u, task_return ? target_iret :
                target_halt, task_return ? sizeof(target_iret) : sizeof(target_halt)) ||
            core_machine_memory_write(fixture.machine, TASK_A_BASE + 0x1cu,
                &(task_switch_smoke_tss32_state){
                    .eip = 0u, .eflags = 0x2u, .esp = 0x00008000u,
                    .es = {0x10u,0u}, .cs = {0x08u,0u}, .ss = {0x10u,0u},
                    .ds = {0x10u,0u}, .fs = {0x10u,0u}, .gs = {0x10u,0u}
                }, sizeof(task_switch_smoke_tss32_state)) != LIB_STATUS_OK ||
            core_machine_memory_write(fixture.machine, TASK_B_BASE + 0x0eu,
                &target, sizeof(target)) != LIB_STATUS_OK ||
            core_machine_run(fixture.machine, budget, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        cpu = test_core_machine_fixture_capture_cpu_after_run(fixture.machine);
        failed |= cpu.data.tr.selector != (task_return ? 0x28u : 0x30u) ||
            cpu.data.eax != (task_return ? 0x11111111u : 0xffff2222u) ||
            cpu.data.ecx != (task_return ? 0x22222222u : 0xffff3333u) ||
            cpu.data.edx != (task_return ? 0x33333333u : 0xffff4444u) ||
            cpu.data.ebx != (task_return ? 0x44444444u : 0xffff5555u) ||
            cpu.data.esp != (task_return ? 0x00008000u : 0xffff8000u) ||
            cpu.data.ebp != (task_return ? 0x66666666u : 0xffff6666u) ||
            cpu.data.esi != (task_return ? 0x77777777u : 0xffff7777u) ||
            cpu.data.edi != (task_return ? 0x88888888u : 0xffff8888u) ||
            core_machine_memory_read(fixture.machine, TASK_A_BASE + 0x1cu,
                &outgoing, sizeof(outgoing)) != LIB_STATUS_OK ||
            (!task_return && (outgoing.eax != 0x11111111u ||
                outgoing.ldtr.selector != 0u)) ||
            core_machine_memory_read(fixture.machine, GDT_BASE + 0x2du,
                &busy[0], 1u) != LIB_STATUS_OK || core_machine_memory_read(
                fixture.machine, GDT_BASE + 0x35u, &busy[1], 1u) != LIB_STATUS_OK ||
            (!task_return && (busy[0] != (nested ? 0x8bu : 0x89u) ||
                busy[1] != 0x83u));
    }
    core_machine_destroy(fixture.machine);
    return failed;
}

int main(void)
{
    lib_i32 failed = 0;

    failed |= task_switch_expect_t330_16_to_32(LIB_FALSE, LIB_FALSE,
        LIB_FALSE);
    failed |= task_switch_expect_t330_16_to_32(LIB_TRUE, LIB_FALSE,
        LIB_FALSE);
    failed |= task_switch_expect_t330_16_to_32(LIB_TRUE, LIB_TRUE,
        LIB_FALSE);
    failed |= task_switch_expect_t330_16_to_32(LIB_TRUE, LIB_FALSE,
        LIB_TRUE);
    failed |= task_switch_expect_t330_32_to_16(LIB_FALSE, LIB_FALSE,
        LIB_FALSE);
    failed |= task_switch_expect_t330_32_to_16(LIB_TRUE, LIB_FALSE,
        LIB_FALSE);
    failed |= task_switch_expect_t330_32_to_16(LIB_TRUE, LIB_TRUE,
        LIB_FALSE);
    failed |= task_switch_expect_t330_32_to_16(LIB_TRUE, LIB_FALSE,
        LIB_TRUE);
    if (failed) return 1;
    printf("M5:T261:S2:TASK-SWITCH:OK\n");
    printf("M5:T261:S3:TASK-SWITCH:CORPUS:OK\n");
    printf("M5:T261:S5:SS-CACHE:OK\n");
    printf("M5:T329:S1:TSS16-JMP:OK\n");
    printf("M5:T329:S2:TSS32-JMP:OK\n");
    printf("M5:T329:S3:TSS32-IMAGE:OK\n");
    printf("M5:T329:S4:TSS-CALL-GATE:OK\n");
    printf("M5:T329:S5:TASK-RETURN:OK\n");
    printf("M5:T329:S6:TASK-LDT:OK\n");
    printf("M5:T330:S1:TASK-TRANSITION:OK\n");
    printf("M5:T539:S62:TASK-CROSS-WIDTH:OK\n");
    return 0;
}
