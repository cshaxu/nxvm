#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "core/machine/memory_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define GDT_POINTER 0x0100u
#define GDT_BASE 0x0300u
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
    TASK_SWITCH_CASE_STACK_LIMIT
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

static C_INT write_bytes(core_machine *machine, uint32_t address,
    const uint8_t *bytes, STD_SIZE_T count)
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
    static const uint8_t gdt_pointer[] = { 0x3fu,0,0x00u,0x03u,0,0 };
    uint8_t gdt[] = {
        0,0,0,0,0,0,0,0,
        0xff,0xff,0,0x20,0,0x9a,0,0,
        0xff,0xff,0,0x30,0,0x92,0,0,
        0,0,0,0x30,0,0x92,0,0,
        0,0,0,0,0,0,0,0,
        0x2b,0,0,0x06,0,0x81,0,0,
        0x2b,0,0,0x07,0,0x81,0,0,
        0,0,0,0,0,0,0,0
    };
    static const uint8_t real_code[] = {
        0x0f,0x01,0x16,0x00,0x01,
        0xb8,0x01,0x00,0x0f,0x01,0xf0,
        0xb8,0x28,0x00,0x0f,0x00,0xd8,
        0xb8,0x10,0x00,0x8e,0xd0,0xbc,0x00,0x80,
        0xea,0x00,0x00,0x08,0x00
    };
    uint8_t kernel_code[] = {
        0xb8,0x11,0x11,0xea,0x00,0x00,0x30,0x00
    };
    uint8_t task_b_state[] = {
        0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0,
        0x00,0x01, 0x02,0x00, 0x22,0x22, 0,0, 0,0, 0,0,
        0x00,0x80, 0,0, 0,0, 0,0,
        0x10,0x00, 0x08,0x00, 0x10,0x00, 0x10,0x00, 0,0
    };
    uint8_t task_b_code[] = {
        0xb8,0x22,0x22,0xa3,0x00,0x00,0xf4
    };

    switch (test_case) {
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
        write_bytes(fixture->machine, TASK_A_BASE, (const uint8_t[44]){0}, 44u) &&
        write_bytes(fixture->machine, TASK_B_BASE, task_b_state,
            sizeof(task_b_state)) &&
        write_bytes(fixture->machine, 0u, real_code, sizeof(real_code)) &&
        write_bytes(fixture->machine, KERNEL_BASE, kernel_code,
            sizeof(kernel_code)) &&
        write_bytes(fixture->machine, KERNEL_BASE + 0x100u, task_b_code,
            sizeof(task_b_code));
}

static C_INT task_switch_expect_switch(core_machine_cpu_profile profile)
{
    task_switch_fixture fixture;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    uint16_t marker = 0u;
    uint16_t saved_ip = 0u;
    uint16_t saved_ax = 0u;
    uint8_t access[2] = {0u, 0u};
    t_cpu cpu;
    const core_machine_run_budget budget = { 128u, 0u };
    C_INT failed = !task_switch_prepare(&fixture, profile);

    if (!failed) {
        failed |= !task_switch_install(&fixture, TASK_SWITCH_CASE_SUCCESS);
        failed |= core_machine_run(fixture.machine, budget, &result) !=
            TYPE_STATUS_OK || result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        cpu = test_core_machine_fixture_capture_cpu_after_run(fixture.machine);
        failed |= core_machine_get_cpu_diagnostic(fixture.machine, &diagnostic) !=
            TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            diagnostic.last_delivered_exception.valid;
        failed |= core_machine_memory_read(fixture.machine, 0x3000u, &marker,
            sizeof(marker)) != TYPE_STATUS_OK || marker != 0x2222u;
        failed |= core_machine_memory_read(fixture.machine, TASK_A_BASE + 0x0eu,
            &saved_ip, sizeof(saved_ip)) != TYPE_STATUS_OK || saved_ip != 0x0008u;
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
                "T261 switch=%u result=%u marker=%04x ip=%04x ax=%04x access=%02x/%02x tr=%04x eax=%08x\n",
                (unsigned)profile, (unsigned)result.reason,
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
            !TYPE_GET_BIT(diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_SS) ||
            diagnostic.first_fault.exception_code != 0u ||
            diagnostic.last_delivered_exception.valid ||
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
    task_switch_case test_case, uint32_t expected_mask, uint16_t expected_code)
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

int main(void)
{
    C_INT failed = 0;

    failed |= task_switch_expect_switch(CORE_MACHINE_CPU_PROFILE_80286);
    failed |= task_switch_expect_switch(CORE_MACHINE_CPU_PROFILE_80386);
    failed |= task_switch_expect_fault(CORE_MACHINE_CPU_PROFILE_80286,
        TASK_SWITCH_CASE_INVALID_SELECTOR, VCPUINS_EXCEPT_GP, 0x0038u);
    failed |= task_switch_expect_fault(CORE_MACHINE_CPU_PROFILE_80286,
        TASK_SWITCH_CASE_NOT_PRESENT, VCPUINS_EXCEPT_NP, 0x0030u);
    failed |= task_switch_expect_fault(CORE_MACHINE_CPU_PROFILE_80386,
        TASK_SWITCH_CASE_BUSY, VCPUINS_EXCEPT_GP, 0x0030u);
    failed |= task_switch_expect_fault(CORE_MACHINE_CPU_PROFILE_80386,
        TASK_SWITCH_CASE_SHORT_TSS, VCPUINS_EXCEPT_TS, 0x0030u);
    failed |= task_switch_expect_stack_fault(CORE_MACHINE_CPU_PROFILE_80286);
    failed |= task_switch_expect_stack_fault(CORE_MACHINE_CPU_PROFILE_80386);
    if (failed) return 1;
    STD_PRINTF("M5:T261:S2:TASK-SWITCH:OK\n");
    STD_PRINTF("M5:T261:S3:TASK-SWITCH:CORPUS:OK\n");
    STD_PRINTF("M5:T261:S5:SS-CACHE:OK\n");
    return 0;
}
