#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "core/machine/memory_interface.h"
#include "core/machine/pic.h"
#include "../support/core_machine_cpu_fixture.h"

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
    TASK_SWITCH_CASE_OPERAND32_SUCCESS,
    TASK_SWITCH_CASE_INDIRECT_OPERAND32_SUCCESS,
    TASK_SWITCH_CASE_INDIRECT_ADDRESS32_SUCCESS,
    TASK_SWITCH_CASE_INDIRECT_OPERAND_ADDRESS32_SUCCESS,
    TASK_SWITCH_CASE_IRQ_SUCCESS,
    TASK_SWITCH_CASE_LOCK_REJECT
} task_switch_case;

static C_VOID task_switch_reset(C_VOID *opaque)
{
    task_switch_fixture *fixture = (task_switch_fixture *)opaque;

    if (fixture != STD_NULL) (C_VOID)test_core_machine_fixture_reset_real_mode(
        fixture->machine);
}

static const core_machine_execution_provider task_switch_provider = {
    task_switch_reset, STD_NULL, STD_NULL
};

static C_INT write_bytes(core_machine *machine, type_unsigned_32 address,
    const type_unsigned_8 *bytes, STD_SIZE_T count)
{
    return core_machine_memory_write(machine, address, bytes, count) ==
        TYPE_STATUS_OK;
}

static C_INT task_switch_prepare(task_switch_fixture *fixture,
    core_machine_cpu_profile profile)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    if (fixture == STD_NULL) return 0;
    STD_MEMSET(fixture, 0, sizeof(*fixture));
    if (core_machine_create(&config, &fixture->machine) != TYPE_STATUS_OK) return 0;
    if (core_machine_bind_execution_provider(fixture->machine,
            &task_switch_provider, fixture) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(fixture->machine) != TYPE_STATUS_OK ||
        core_machine_reset(fixture->machine) != TYPE_STATUS_OK) {
        core_machine_destroy(fixture->machine);
        fixture->machine = STD_NULL;
        return 0;
    }
    return 1;
}

static C_INT task_switch_install(task_switch_fixture *fixture,
    task_switch_case test_case)
{
    static const type_unsigned_8 gdt_pointer[] = { 0x3fu,0,0x00u,0x03u,0,0 };
    type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xff,0xff,0,0x20,0,0x9a,0,0,
        0xff,0xff,0,0x30,0,0x92,0,0,
        0,0,0,0x30,0,0x92,0,0,
        0,0,0,0,0,0,0,0,
        0x2b,0,0,0x06,0,0x81,0,0,
        0x2b,0,0,0x07,0,0x81,0,0,
        0,0,0,0,0,0,0,0
    };
    static const type_unsigned_8 real_code[] = {
        0x0f,0x01,0x16,0x00,0x01,
        0xb8,0x01,0x00,0x0f,0x01,0xf0,
        0xb8,0x28,0x00,0x0f,0x00,0xd8,
        0xb8,0x10,0x00,0x8e,0xd0,0xbc,0x00,0x80,
        0xea,0x00,0x00,0x08,0x00
    };
    static const type_unsigned_8 real_code_with_ds[] = {
        0x0f,0x01,0x16,0x00,0x01,
        0xb8,0x01,0x00,0x0f,0x01,0xf0,
        0xb8,0x28,0x00,0x0f,0x00,0xd8,
        0xb8,0x10,0x00,0x8e,0xd0,0x8e,0xd8,0xbc,0x00,0x80,
        0xea,0x00,0x00,0x08,0x00
    };
    const type_unsigned_8 *bootstrap_code = real_code;
    STD_SIZE_T bootstrap_bytes = sizeof(real_code);
    type_unsigned_8 kernel_code[] = {
        0xb8,0x11,0x11,0xea,0x00,0x00,0x30,0x00,0,0,0,0
    };
    static const type_unsigned_8 indirect_pointer[] = { 0,0,0x30,0 };
    static const type_unsigned_8 indirect_pointer32[] = { 0,0,0,0,0x30,0 };
    type_unsigned_8 task_b_state[] = {
        0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0,
        0x00,0x01, 0x02,0x00, 0x22,0x22, 0,0, 0,0, 0,0,
        0x00,0x80, 0,0, 0,0, 0,0,
        0x10,0x00, 0x08,0x00, 0x10,0x00, 0x10,0x00, 0,0
    };
    type_unsigned_8 task_b_code[] = {
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
        kernel_code[6] = 0x38u;
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
        write_bytes(fixture->machine, TASK_A_BASE, (const type_unsigned_8[44]){0}, 44u) &&
        write_bytes(fixture->machine, TASK_B_BASE, task_b_state,
            sizeof(task_b_state)) &&
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

static C_INT task_switch_expect_switch(core_machine_cpu_profile profile,
    task_switch_case test_case)
{
    task_switch_fixture fixture;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    type_unsigned_16 marker = 0u;
    type_unsigned_16 saved_ip = 0u;
    type_unsigned_16 saved_ax = 0u;
    type_unsigned_8 access[2] = {0u, 0u};
    t_cpu cpu;
    const core_machine_run_budget budget = { 128u, 0u };
    C_INT failed = !task_switch_prepare(&fixture, profile);

    if (!failed) {
        failed |= !task_switch_install(&fixture, test_case);
        failed |= core_machine_run(fixture.machine, budget, &result) !=
            TYPE_STATUS_OK || result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        cpu = test_core_machine_fixture_capture_cpu_after_run(fixture.machine);
        failed |= core_machine_get_cpu_diagnostic(fixture.machine, &diagnostic) !=
            TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            diagnostic.last_delivered_exception.valid;
        failed |= core_machine_memory_read(fixture.machine, 0x3000u, &marker,
            sizeof(marker)) != TYPE_STATUS_OK || marker != 0x2222u;
        failed |= core_machine_memory_read(fixture.machine, TASK_A_BASE + 0x0eu,
            &saved_ip, sizeof(saved_ip)) != TYPE_STATUS_OK || saved_ip !=
            (test_case == TASK_SWITCH_CASE_INDIRECT_SUCCESS ? 0x0007u :
                test_case == TASK_SWITCH_CASE_INDIRECT_OPERAND32_SUCCESS ? 0x0008u :
                test_case == TASK_SWITCH_CASE_INDIRECT_ADDRESS32_SUCCESS ? 0x000au :
                test_case == TASK_SWITCH_CASE_INDIRECT_OPERAND_ADDRESS32_SUCCESS ? 0x000bu :
                test_case == TASK_SWITCH_CASE_OPERAND32_SUCCESS ? 0x000bu :
                0x0008u);
        failed |= core_machine_memory_read(fixture.machine, TASK_A_BASE + 0x12u,
            &saved_ax, sizeof(saved_ax)) != TYPE_STATUS_OK || saved_ax != 0x1111u;
        failed |= core_machine_memory_read(fixture.machine, GDT_BASE + 0x2du,
            &access[0], 1u) != TYPE_STATUS_OK || access[0] != 0x81u;
        failed |= core_machine_memory_read(fixture.machine, GDT_BASE + 0x35u,
            &access[1], 1u) != TYPE_STATUS_OK || access[1] != 0x83u;
        failed |= !cpu.data.tr.flagValid || cpu.data.tr.selector != 0x0030u ||
            cpu.data.ax != 0x2222u || cpu.data.ss.sregtype != SREG_STACK ||
            !TYPE_GET_BIT(cpu.data.cr0, VCPU_CR0_TS);
        if (profile == CORE_MACHINE_CPU_PROFILE_80386) {
            failed |= cpu.data.eax != 0xffff2222u;
        }
        if (failed) {
            STD_FPRINTF(STD_STDERR,
                "T261 switch=%u case=%u result=%u marker=%04x ip=%04x ax=%04x access=%02x/%02x tr=%04x eax=%08x\n",
                (unsigned)profile, (unsigned)test_case, (unsigned)result.reason,
                marker, saved_ip, saved_ax, access[0], access[1],
                cpu.data.tr.selector, cpu.data.eax);
        }
    }
    core_machine_destroy(fixture.machine);
    return failed;
}

static C_INT task_switch_expect_stack_fault(core_machine_cpu_profile profile)
{
    task_switch_fixture fixture;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu cpu;
    const core_machine_run_budget budget = { 128u, 0u };
    C_INT failed = !task_switch_prepare(&fixture, profile);

    if (!failed) {
        failed |= !task_switch_install(&fixture, TASK_SWITCH_CASE_STACK_LIMIT);
        failed |= core_machine_run(fixture.machine, budget, &result) !=
            TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT;
        cpu = test_core_machine_fixture_capture_cpu_after_run(fixture.machine);
        failed |= core_machine_get_cpu_diagnostic(fixture.machine, &diagnostic) !=
            TYPE_STATUS_OK || !diagnostic.first_fault.valid ||
            !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                profile == CORE_MACHINE_CPU_PROFILE_80386 ?
                    VCPUINS_EXCEPT_DF : VCPUINS_EXCEPT_SS) ||
            diagnostic.first_fault.exception_code != 0u ||
            diagnostic.last_delivered_exception.valid ||
            cpu.data.ip != 0x0100u || cpu.data.sp != 0u ||
            cpu.data.ss.sregtype != SREG_STACK || !cpu.data.tr.flagValid ||
            cpu.data.tr.selector != 0x0030u;
        if (failed) {
            STD_FPRINTF(STD_STDERR,
                "T261 stack=%u result=%u mask=%x code=%04x ss=%u tr=%04x\n",
                (unsigned)profile, (unsigned)result.reason,
                (unsigned)diagnostic.first_fault.exception_mask,
                diagnostic.first_fault.exception_code,
                (unsigned)cpu.data.ss.sregtype, cpu.data.tr.selector);
        }
    }
    core_machine_destroy(fixture.machine);
    return failed;
}

static C_INT task_switch_expect_fault(core_machine_cpu_profile profile,
    task_switch_case test_case, type_unsigned_32 expected_mask, type_unsigned_16 expected_code)
{
    task_switch_fixture fixture;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu cpu;
    const core_machine_run_budget budget = { 128u, 0u };
    C_INT failed = !task_switch_prepare(&fixture, profile);

    if (!failed) {
        failed |= !task_switch_install(&fixture, test_case);
        failed |= core_machine_run(fixture.machine, budget, &result) !=
            TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT;
        cpu = test_core_machine_fixture_capture_cpu_after_run(fixture.machine);
        failed |= core_machine_get_cpu_diagnostic(fixture.machine, &diagnostic) !=
            TYPE_STATUS_OK || !diagnostic.first_fault.valid ||
            !TYPE_GET_BIT(diagnostic.first_fault.exception_mask, expected_mask) ||
            diagnostic.first_fault.exception_code != expected_code ||
            diagnostic.last_delivered_exception.valid ||
            !cpu.data.tr.flagValid || cpu.data.tr.selector != 0x0028u;
        if (failed) {
            STD_FPRINTF(STD_STDERR,
                "T261 fault=%u/%u result=%u mask=%x code=%04x tr=%04x\n",
                (unsigned)profile, (unsigned)test_case, (unsigned)result.reason,
                (unsigned)diagnostic.first_fault.exception_mask,
                diagnostic.first_fault.exception_code, cpu.data.tr.selector);
        }
    }
    core_machine_destroy(fixture.machine);
    return failed;
}

static C_INT task_switch_expect_pending_irq(core_machine_cpu_profile profile)
{
    task_switch_fixture fixture;
    core_machine_pic_irq_source irq;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    type_unsigned_16 marker = 0xffffu;
    t_cpu cpu;
    static const type_unsigned_8 gate[] = { 0x80u,0x01u,0x08u,0u,0u,0x86u,0u,0u };
    const core_machine_run_budget budget = { 128u, 0u };
    C_INT failed = !task_switch_prepare(&fixture, profile);

    STD_MEMSET(&irq, 0, sizeof(irq));
    if (!failed) {
        failed |= !task_switch_install(&fixture, TASK_SWITCH_CASE_IRQ_SUCCESS) ||
            core_machine_memory_write(fixture.machine, IDT_BASE + 0x100u, gate,
                sizeof(gate)) != TYPE_STATUS_OK ||
            core_machine_memory_write(fixture.machine, KERNEL_BASE + 0x180u,
                (const type_unsigned_8[]){ 0xf4u }, 1u) != TYPE_STATUS_OK;
        fixture.machine->executor_cpu.data.idtr.flagValid = TYPE_TRUE;
        fixture.machine->executor_cpu.data.idtr.sregtype = SREG_IDTR;
        fixture.machine->executor_cpu.data.idtr.base = IDT_BASE;
        fixture.machine->executor_cpu.data.idtr.limit = 0x0107u;
        fixture.machine->shared_pic_master.data.icw2 = 0x20u;
        core_machine_pic_irq_source_bind(&irq, &fixture.machine->shared_pic_master,
            &fixture.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&irq);
        core_machine_pic_irq_source_deassert(&irq);
        failed |= core_machine_run(fixture.machine, budget, &result) !=
            TYPE_STATUS_OK || result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        cpu = test_core_machine_fixture_capture_cpu_after_run(fixture.machine);
        failed |= core_machine_get_cpu_diagnostic(fixture.machine, &diagnostic) !=
            TYPE_STATUS_OK || diagnostic.first_fault.valid;
        failed |=
            cpu.data.eip != 0x0181u || cpu.data.ax != 0x2222u ||
            cpu.data.sp != 0x7ffau || TYPE_GET_BIT(cpu.data.eflags,
                VCPU_EFLAGS_IF) ||
            cpu.data.tr.selector != 0x0030u ||
            !TYPE_GET_BIT(cpu.data.cr0, VCPU_CR0_TS) ||
            !TYPE_GET_BIT(fixture.machine->shared_pic_master.data.isr,
                VPIC_ISR_IRQ(0u)) ||
            TYPE_GET_BIT(fixture.machine->shared_pic_master.data.irr,
                VPIC_IRR_IRQ(0u));
        failed |= core_machine_memory_read(fixture.machine, 0x3000u, &marker,
            sizeof(marker)) != TYPE_STATUS_OK || marker != 0u;
        if (failed) {
            STD_FPRINTF(STD_STDERR,
                "T329 irq=%u result=%u eip=%04x ax=%04x sp=%04x ss=%05x tr=%04x isr=%02x irr=%02x marker=%04x\n",
                (unsigned)profile, (unsigned)result.reason, cpu.data.eip,
                cpu.data.ax, cpu.data.sp, cpu.data.ss.base, cpu.data.tr.selector,
                fixture.machine->shared_pic_master.data.isr,
                fixture.machine->shared_pic_master.data.irr, marker);
        }
    }
    core_machine_destroy(fixture.machine);
    return failed;
}

typedef struct task_switch_smoke_tss32_selector {
    type_unsigned_16 selector;
    type_unsigned_16 reserved;
} task_switch_smoke_tss32_selector;

typedef struct task_switch_smoke_tss32_state {
    type_unsigned_32 cr3;
    type_unsigned_32 eip;
    type_unsigned_32 eflags;
    type_unsigned_32 eax;
    type_unsigned_32 ecx;
    type_unsigned_32 edx;
    type_unsigned_32 ebx;
    type_unsigned_32 esp;
    type_unsigned_32 ebp;
    type_unsigned_32 esi;
    type_unsigned_32 edi;
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
    TASK_SWITCH_TSS32_REJECTION_STACK_LIMIT
} task_switch_tss32_rejection;

static C_INT task_switch_expect_tss32_direct(type_bool operand32,
    type_bool indirect, type_bool address32, task_switch_tss32_rejection rejection,
    type_bool lock, type_bool pending_irq)
{
    task_switch_fixture fixture;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_pic_irq_source irq;
    type_status status;
    type_unsigned_32 saved_eip = 0u;
    task_switch_smoke_tss32_state outgoing;
    type_unsigned_8 busy[2] = {0u, 0u};
    t_cpu cpu;
    static const type_unsigned_8 gdt_pointer[] = { 0x3fu,0,0,0x03u,0,0 };
    type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0, 0xff,0xff,0,0x20,0,0x9a,0,0,
        0xff,0xff,0,0x30,0,0x92,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0xff,0,0,0x06,0,0x89,0,0,
        0xff,0,0,0x07,0,0x89,0,0, 0,0,0,0,0,0,0,0
    };
    static const type_unsigned_8 bootstrap[] = {
        0x0f,0x01,0x16,0,0x01, 0xb8,1,0,0x0f,0x01,0xf0,
        0xb8,0x28,0,0x0f,0,0xd8, 0xb8,0x10,0,0x8e,0xd0,0x8e,0xd8,
        0x8e,0xc0,0x8e,0xe0,0x8e,0xe8, 0xea,0,0,0x08,0
    };
#define TASK_SWITCH_SOURCE_GPRS \
    0x66,0xb8,0x11,0x11,0x11,0x11, \
    0x66,0xb9,0x22,0x22,0x22,0x22, \
    0x66,0xba,0x33,0x33,0x33,0x33, \
    0x66,0xbb,0x44,0x44,0x44,0x44, \
    0x66,0xbc,0x00,0x00,0x55,0x55, \
    0x66,0xbd,0x66,0x66,0x66,0x66, \
    0x66,0xbe,0x77,0x77,0x77,0x77, \
    0x66,0xbf,0x88,0x88,0x88,0x88
    static const type_unsigned_8 source16[] = {
        TASK_SWITCH_SOURCE_GPRS, 0xea,0,0,0x30,0
    };
    static const type_unsigned_8 source32[] = {
        TASK_SWITCH_SOURCE_GPRS, 0x66,0xea,0,0,0,0,0x30,0
    };
    static const type_unsigned_8 source_lock[] = {
        TASK_SWITCH_SOURCE_GPRS, 0xf0u,0xeau,0,0,0x30u,0
    };
    static const type_unsigned_8 source_lock_indirect[] = {
        TASK_SWITCH_SOURCE_GPRS, 0xf0u,0xffu,0x2eu,0x00u,0x52u
    };
    static const type_unsigned_8 indirect16[] = {
        TASK_SWITCH_SOURCE_GPRS, 0xff,0x2eu,0x00u,0x52u
    };
    static const type_unsigned_8 indirect32[] = {
        TASK_SWITCH_SOURCE_GPRS, 0x66,0xff,0x2eu,0x00u,0x52u
    };
    static const type_unsigned_8 indirect_address32[] = {
        TASK_SWITCH_SOURCE_GPRS, 0x67,0xff,0x2du,0x00u,0x52u,0x00u,0x00u
    };
    static const type_unsigned_8 indirect_combined[] = {
        TASK_SWITCH_SOURCE_GPRS, 0x66,0x67,0xff,0x2du,0x00u,0x52u,0x00u,0x00u
    };
#undef TASK_SWITCH_SOURCE_GPRS
    static const type_unsigned_8 pointer16[] = { 0,0,0x30u,0 };
    static const type_unsigned_8 pointer32[] = { 0,0,0,0,0x30u,0 };
    static const type_unsigned_8 halt[] = { 0xf4u };
    static const type_unsigned_8 irq_gate[] = {
        0x80u,0x01u,0x08u,0,0,0x86u,0,0
    };
    task_switch_smoke_tss32_state target = {
        .cr3 = 0x00001000u, .eip = 0x100u, .eflags = 0x2u, .eax = 0xa1a12222u,
        .ecx = 0xc1c13333u, .edx = 0xd1d14444u, .ebx = 0xb1b15555u,
        .esp = 0x8000u, .ebp = 0xe1e16666u, .esi = 0xf1f17777u,
        .edi = 0x81818888u, .es = {0x10u,0u}, .cs = {0x08u,0u},
        .ss = {0x10u,0u}, .ds = {0x10u,0u}, .fs = {0x10u,0u},
        .gs = {0x10u,0u}, .ldtr = {0u,0u}
    };
    const core_machine_run_budget budget = { 128u, 0u };
    const type_unsigned_8 *source = lock ? source_lock :
        (operand32 ? source32 : source16);
    type_unsigned_32 source_bytes = lock ? sizeof(source_lock) :
        (operand32 ? sizeof(source32) : sizeof(source16));
    const type_unsigned_8 *pointer = operand32 ? pointer32 : pointer16;
    const type_unsigned_32 pointer_bytes = operand32 ? sizeof(pointer32) : sizeof(pointer16);
    C_INT failed = !task_switch_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);

    STD_MEMSET(&irq, 0, sizeof(irq));
    if (indirect) {
        if (lock) {
            source = source_lock_indirect;
            source_bytes = sizeof(source_lock_indirect);
        } else if (operand32 && address32) {
            source = indirect_combined;
            source_bytes = sizeof(indirect_combined);
        } else if (operand32) {
            source = indirect32;
            source_bytes = sizeof(indirect32);
        } else if (address32) {
            source = indirect_address32;
            source_bytes = sizeof(indirect_address32);
        } else {
            source = indirect16;
            source_bytes = sizeof(indirect16);
        }
    }
    if (rejection == TASK_SWITCH_TSS32_REJECTION_INVALID_CODE)
        target.cs.selector = 0x10u;
    if (rejection == TASK_SWITCH_TSS32_REJECTION_TARGET_BUSY)
        gdt[0x35u] = 0x8bu;
    if (rejection == TASK_SWITCH_TSS32_REJECTION_OLD_SHORT)
        gdt[0x28u] = 0x60u;
    if (rejection == TASK_SWITCH_TSS32_REJECTION_TARGET_SHORT)
        gdt[0x30u] = 0x60u;
    if (rejection == TASK_SWITCH_TSS32_REJECTION_STACK_LIMIT) {
        gdt[0x20u] = 0u;
        gdt[0x21u] = 0u;
        gdt[0x22u] = 0u;
        gdt[0x23u] = 0x30u;
        gdt[0x24u] = 0u;
        gdt[0x25u] = 0x92u;
        gdt[0x26u] = 0u;
        target.ss.selector = 0x20u;
    }
    if (pending_irq) target.eflags |= VCPU_EFLAGS_IF;

    if (!failed) {
        failed |= !write_bytes(fixture.machine, GDT_POINTER, gdt_pointer,
                sizeof(gdt_pointer)) || !write_bytes(fixture.machine, GDT_BASE,
                gdt, sizeof(gdt)) || !write_bytes(fixture.machine, 0u,
                bootstrap, sizeof(bootstrap)) || !write_bytes(fixture.machine,
                KERNEL_BASE, source, source_bytes) || !write_bytes(
                fixture.machine, KERNEL_BASE + 0x100u, halt, sizeof(halt)) ||
            (indirect && !write_bytes(fixture.machine, 0x8200u, pointer,
                pointer_bytes)) ||
            (pending_irq && (!write_bytes(fixture.machine, IDT_BASE + 0x100u,
                irq_gate, sizeof(irq_gate)) || !write_bytes(fixture.machine,
                KERNEL_BASE + 0x180u, halt, sizeof(halt)))) ||
            core_machine_memory_write(fixture.machine, TASK_B_BASE + 0x1cu,
                &target, sizeof(target)) != TYPE_STATUS_OK;
        if (pending_irq) {
            fixture.machine->executor_cpu.data.idtr.flagValid = TYPE_TRUE;
            fixture.machine->executor_cpu.data.idtr.sregtype = SREG_IDTR;
            fixture.machine->executor_cpu.data.idtr.base = IDT_BASE;
            fixture.machine->executor_cpu.data.idtr.limit = 0x0107u;
            fixture.machine->shared_pic_master.data.icw2 = 0x20u;
            core_machine_pic_irq_source_bind(&irq, &fixture.machine->shared_pic_master,
                &fixture.machine->shared_pic_slave, 0u);
            core_machine_pic_irq_source_assert(&irq);
            core_machine_pic_irq_source_deassert(&irq);
        }
        status = core_machine_run(fixture.machine, budget, &result);
        failed |= status != (rejection != TASK_SWITCH_TSS32_REJECTION_NONE || lock ?
                TYPE_STATUS_FAULT : TYPE_STATUS_OK) ||
            result.reason != (rejection != TASK_SWITCH_TSS32_REJECTION_NONE || lock ? CORE_MACHINE_STOP_FAULT :
                CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT);
        cpu = test_core_machine_fixture_capture_cpu_after_run(fixture.machine);
        failed |= core_machine_get_cpu_diagnostic(fixture.machine, &diagnostic) !=
            TYPE_STATUS_OK || diagnostic.first_fault.valid !=
                (rejection != TASK_SWITCH_TSS32_REJECTION_NONE || lock) ||
            (rejection != TASK_SWITCH_TSS32_REJECTION_NONE && (!TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                VCPUINS_EXCEPT_DF) || diagnostic.first_fault.exception_code !=
                0u)) || (lock && (!TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                VCPUINS_EXCEPT_UD) || diagnostic.first_fault.exception_code !=
                0u)) || (rejection == TASK_SWITCH_TSS32_REJECTION_NONE && !lock &&
                diagnostic.last_delivered_exception.valid);
        if (rejection != TASK_SWITCH_TSS32_REJECTION_NONE) {
            failed |= cpu.data.eip != source_bytes - (operand32 ? 8u : 5u) ||
                cpu.data.eax != 0x11111111u ||
                cpu.data.tr.selector != 0x28u;
            core_machine_destroy(fixture.machine);
            return failed;
        }
        if (lock) {
            failed |= cpu.data.eip != source_bytes - (indirect ? 5u : 6u) ||
                cpu.data.eax != 0x11111111u || cpu.data.ecx != 0x22222222u ||
                cpu.data.edx != 0x33333333u || cpu.data.ebx != 0x44444444u ||
                cpu.data.esp != 0x55550000u || cpu.data.ebp != 0x66666666u ||
                cpu.data.esi != 0x77777777u || cpu.data.edi != 0x88888888u ||
                cpu.data.tr.selector != 0x28u || cpu.data.es.selector != 0x10u ||
                cpu.data.cs.selector != 0x08u || cpu.data.ss.selector != 0x10u ||
                cpu.data.ds.selector != 0x10u || cpu.data.fs.selector != 0x10u ||
                cpu.data.gs.selector != 0x10u;
            core_machine_destroy(fixture.machine);
            return failed;
        }
        if (pending_irq) {
            failed |= cpu.data.eip != 0x181u || cpu.data.esp != 0x7ffau ||
                TYPE_GET_BIT(cpu.data.eflags, VCPU_EFLAGS_IF) ||
                !TYPE_GET_BIT(fixture.machine->shared_pic_master.data.isr,
                    VPIC_ISR_IRQ(0u)) || TYPE_GET_BIT(
                    fixture.machine->shared_pic_master.data.irr, VPIC_IRR_IRQ(0u));
        }
        STD_MEMSET(&outgoing, 0, sizeof(outgoing));
        failed |= core_machine_memory_read(fixture.machine, TASK_A_BASE + 0x1cu,
                &outgoing, sizeof(outgoing)) != TYPE_STATUS_OK ||
            (saved_eip = outgoing.eip) != source_bytes ||
            outgoing.cr3 != 0u || outgoing.eflags != 0x2u ||
            outgoing.eax != 0x11111111u || outgoing.ecx != 0x22222222u ||
            outgoing.edx != 0x33333333u || outgoing.ebx != 0x44444444u ||
            outgoing.esp != 0x55550000u || outgoing.ebp != 0x66666666u ||
            outgoing.esi != 0x77777777u || outgoing.edi != 0x88888888u ||
            outgoing.es.selector != 0x10u || outgoing.cs.selector != 0x08u ||
            outgoing.ss.selector != 0x10u || outgoing.ds.selector != 0x10u ||
            outgoing.fs.selector != 0x10u || outgoing.gs.selector != 0x10u ||
            outgoing.ldtr.selector != 0u || core_machine_memory_read(fixture.machine,
                GDT_BASE + 0x2du, &busy[0], 1u) != TYPE_STATUS_OK ||
            core_machine_memory_read(fixture.machine, GDT_BASE + 0x35u,
                &busy[1], 1u) != TYPE_STATUS_OK || busy[0] != 0x89u ||
            busy[1] != 0x8bu;
        failed |= cpu.data.eip != (pending_irq ? 0x181u : 0x101u) ||
            (!pending_irq && cpu.data.eflags != target.eflags) ||
            cpu.data.eax != target.eax ||
            cpu.data.ecx != target.ecx || cpu.data.edx != target.edx ||
            cpu.data.ebx != target.ebx || cpu.data.esp != (pending_irq ?
                0x7ffau : target.esp) ||
            cpu.data.ebp != target.ebp || cpu.data.esi != target.esi ||
            cpu.data.edi != target.edi || cpu.data.tr.selector != 0x30u ||
            cpu.data.cr3 != target.cr3 ||
            cpu.data.es.selector != target.es.selector ||
            cpu.data.cs.selector != target.cs.selector ||
            cpu.data.ss.selector != target.ss.selector ||
            cpu.data.ds.selector != target.ds.selector ||
            cpu.data.fs.selector != target.fs.selector ||
            cpu.data.gs.selector != target.gs.selector ||
            cpu.data.ldtr.flagValid || !TYPE_GET_BIT(cpu.data.cr0, VCPU_CR0_TS);
        if (failed) {
            STD_FPRINTF(STD_STDERR,
                "T329 tss32 result=%u eip=%08x eax=%08x tr=%04x saved=%08x busy=%02x/%02x fault=%u\n",
                (unsigned)result.reason, cpu.data.eip, cpu.data.eax,
                cpu.data.tr.selector, saved_eip, busy[0], busy[1],
                (unsigned)diagnostic.first_fault.exception_mask);
        }
    }
    core_machine_destroy(fixture.machine);
    return failed;
}

int main(void)
{
    C_INT failed = 0;

    failed |= task_switch_expect_switch(CORE_MACHINE_CPU_PROFILE_80286,
        TASK_SWITCH_CASE_SUCCESS);
    failed |= task_switch_expect_switch(CORE_MACHINE_CPU_PROFILE_80386,
        TASK_SWITCH_CASE_SUCCESS);
    failed |= task_switch_expect_switch(CORE_MACHINE_CPU_PROFILE_80286,
        TASK_SWITCH_CASE_INDIRECT_SUCCESS);
    failed |= task_switch_expect_switch(CORE_MACHINE_CPU_PROFILE_80386,
        TASK_SWITCH_CASE_INDIRECT_SUCCESS);
    failed |= task_switch_expect_switch(CORE_MACHINE_CPU_PROFILE_80386,
        TASK_SWITCH_CASE_OPERAND32_SUCCESS);
    failed |= task_switch_expect_switch(CORE_MACHINE_CPU_PROFILE_80386,
        TASK_SWITCH_CASE_INDIRECT_OPERAND32_SUCCESS);
    failed |= task_switch_expect_switch(CORE_MACHINE_CPU_PROFILE_80386,
        TASK_SWITCH_CASE_INDIRECT_ADDRESS32_SUCCESS);
    failed |= task_switch_expect_switch(CORE_MACHINE_CPU_PROFILE_80386,
        TASK_SWITCH_CASE_INDIRECT_OPERAND_ADDRESS32_SUCCESS);
    failed |= task_switch_expect_pending_irq(CORE_MACHINE_CPU_PROFILE_80286);
    failed |= task_switch_expect_pending_irq(CORE_MACHINE_CPU_PROFILE_80386);
    failed |= task_switch_expect_fault(CORE_MACHINE_CPU_PROFILE_80286,
        TASK_SWITCH_CASE_INVALID_SELECTOR, VCPUINS_EXCEPT_GP, 0x0038u);
    failed |= task_switch_expect_fault(CORE_MACHINE_CPU_PROFILE_80286,
        TASK_SWITCH_CASE_NOT_PRESENT, VCPUINS_EXCEPT_NP, 0x0030u);
    failed |= task_switch_expect_fault(CORE_MACHINE_CPU_PROFILE_80386,
        TASK_SWITCH_CASE_BUSY, VCPUINS_EXCEPT_DF, 0u);
    failed |= task_switch_expect_fault(CORE_MACHINE_CPU_PROFILE_80386,
        TASK_SWITCH_CASE_SHORT_TSS, VCPUINS_EXCEPT_DF, 0u);
    failed |= task_switch_expect_fault(CORE_MACHINE_CPU_PROFILE_80386,
        TASK_SWITCH_CASE_LOCK_REJECT, VCPUINS_EXCEPT_UD, 0u);
    failed |= task_switch_expect_stack_fault(CORE_MACHINE_CPU_PROFILE_80286);
    failed |= task_switch_expect_stack_fault(CORE_MACHINE_CPU_PROFILE_80386);
    failed |= task_switch_expect_tss32_direct(TYPE_FALSE, TYPE_FALSE, TYPE_FALSE,
        TASK_SWITCH_TSS32_REJECTION_NONE, TYPE_FALSE, TYPE_FALSE);
    failed |= task_switch_expect_tss32_direct(TYPE_TRUE, TYPE_FALSE, TYPE_FALSE,
        TASK_SWITCH_TSS32_REJECTION_NONE, TYPE_FALSE, TYPE_FALSE);
    failed |= task_switch_expect_tss32_direct(TYPE_FALSE, TYPE_TRUE, TYPE_FALSE,
        TASK_SWITCH_TSS32_REJECTION_NONE, TYPE_FALSE, TYPE_FALSE);
    failed |= task_switch_expect_tss32_direct(TYPE_TRUE, TYPE_TRUE, TYPE_FALSE,
        TASK_SWITCH_TSS32_REJECTION_NONE, TYPE_FALSE, TYPE_FALSE);
    failed |= task_switch_expect_tss32_direct(TYPE_FALSE, TYPE_TRUE, TYPE_TRUE,
        TASK_SWITCH_TSS32_REJECTION_NONE, TYPE_FALSE, TYPE_FALSE);
    failed |= task_switch_expect_tss32_direct(TYPE_TRUE, TYPE_TRUE, TYPE_TRUE,
        TASK_SWITCH_TSS32_REJECTION_NONE, TYPE_FALSE, TYPE_FALSE);
    failed |= task_switch_expect_tss32_direct(TYPE_FALSE, TYPE_FALSE, TYPE_FALSE,
        TASK_SWITCH_TSS32_REJECTION_INVALID_CODE, TYPE_FALSE, TYPE_FALSE);
    failed |= task_switch_expect_tss32_direct(TYPE_FALSE, TYPE_FALSE, TYPE_FALSE,
        TASK_SWITCH_TSS32_REJECTION_NONE, TYPE_TRUE, TYPE_FALSE);
    failed |= task_switch_expect_tss32_direct(TYPE_FALSE, TYPE_TRUE, TYPE_FALSE,
        TASK_SWITCH_TSS32_REJECTION_NONE, TYPE_TRUE, TYPE_FALSE);
    failed |= task_switch_expect_tss32_direct(TYPE_FALSE, TYPE_FALSE, TYPE_FALSE,
        TASK_SWITCH_TSS32_REJECTION_NONE, TYPE_FALSE, TYPE_TRUE);
    failed |= task_switch_expect_tss32_direct(TYPE_FALSE, TYPE_FALSE, TYPE_FALSE,
        TASK_SWITCH_TSS32_REJECTION_TARGET_BUSY, TYPE_FALSE, TYPE_FALSE);
    failed |= task_switch_expect_tss32_direct(TYPE_FALSE, TYPE_FALSE, TYPE_FALSE,
        TASK_SWITCH_TSS32_REJECTION_OLD_SHORT, TYPE_FALSE, TYPE_FALSE);
    failed |= task_switch_expect_tss32_direct(TYPE_FALSE, TYPE_FALSE, TYPE_FALSE,
        TASK_SWITCH_TSS32_REJECTION_TARGET_SHORT, TYPE_FALSE, TYPE_FALSE);
    failed |= task_switch_expect_tss32_direct(TYPE_FALSE, TYPE_FALSE, TYPE_FALSE,
        TASK_SWITCH_TSS32_REJECTION_STACK_LIMIT, TYPE_FALSE, TYPE_FALSE);
    if (failed) return 1;
    STD_PRINTF("M5:T261:S2:TASK-SWITCH:OK\n");
    STD_PRINTF("M5:T261:S3:TASK-SWITCH:CORPUS:OK\n");
    STD_PRINTF("M5:T261:S5:SS-CACHE:OK\n");
    STD_PRINTF("M5:T329:S1:TSS16-JMP:OK\n");
    STD_PRINTF("M5:T329:S2:TSS32-JMP:OK\n");
    return 0;
}
