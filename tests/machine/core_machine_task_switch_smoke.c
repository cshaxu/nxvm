#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "core/machine/memory_interface.h"

#define GDT_POINTER 0x0100u
#define GDT_BASE 0x0300u
#define TASK_A_BASE 0x0600u
#define TASK_B_BASE 0x0700u
#define KERNEL_BASE 0x2000u

typedef struct task_switch_fixture {
    core_machine *machine;
    t_cpu *cpu;
    core_machine_cpu_execution_context *execution;
} task_switch_fixture;

static C_VOID task_switch_reset(C_VOID *opaque)
{
    task_switch_fixture *fixture = (task_switch_fixture *)opaque;

    if (fixture == STD_NULL || fixture->cpu == STD_NULL ||
        fixture->execution == STD_NULL) return;
    (C_VOID)core_machine_cpu_execution_load_segment(fixture->execution,
        &fixture->cpu->data.cs, 0u);
    (C_VOID)core_machine_cpu_execution_load_segment(fixture->execution,
        &fixture->cpu->data.ds, 0u);
    (C_VOID)core_machine_cpu_execution_load_segment(fixture->execution,
        &fixture->cpu->data.es, 0u);
    (C_VOID)core_machine_cpu_execution_load_segment(fixture->execution,
        &fixture->cpu->data.ss, 0u);
    fixture->cpu->data.eip = 0u;
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
    fixture->cpu = core_machine_configuration_cpu_borrow(fixture->machine);
    fixture->execution = core_machine_configuration_cpu_execution_borrow(
        fixture->machine);
    if (fixture->cpu == STD_NULL || fixture->execution == STD_NULL ||
        core_machine_bind_execution_provider(fixture->machine,
            &task_switch_provider, fixture) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(fixture->machine) != TYPE_STATUS_OK ||
        core_machine_reset(fixture->machine) != TYPE_STATUS_OK) {
        core_machine_destroy(fixture->machine);
        fixture->machine = STD_NULL;
        return 0;
    }
    return 1;
}

static C_INT task_switch_install(task_switch_fixture *fixture)
{
    static const uint8_t gdt_pointer[] = { 0x3fu,0,0x00u,0x03u,0,0 };
    static const uint8_t gdt[] = {
        0,0,0,0,0,0,0,0,
        0xff,0xff,0,0x20,0,0x9a,0,0,
        0xff,0xff,0,0x30,0,0x92,0,0,
        0,0,0,0,0,0,0,0,
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
    static const uint8_t kernel_code[] = {
        0xb8,0x11,0x11,0xea,0x00,0x00,0x30,0x00
    };

    return write_bytes(fixture->machine, GDT_POINTER, gdt_pointer,
            sizeof(gdt_pointer)) &&
        write_bytes(fixture->machine, GDT_BASE, gdt, sizeof(gdt)) &&
        write_bytes(fixture->machine, TASK_A_BASE, (const uint8_t[44]){0}, 44u) &&
        write_bytes(fixture->machine, TASK_B_BASE, (const uint8_t[44]){0}, 44u) &&
        write_bytes(fixture->machine, 0u, real_code, sizeof(real_code)) &&
        write_bytes(fixture->machine, KERNEL_BASE, kernel_code,
            sizeof(kernel_code));
}

static C_INT task_switch_expect_baseline(core_machine_cpu_profile profile)
{
    task_switch_fixture fixture;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    const core_machine_run_budget budget = { 128u, 0u };
    C_INT failed = !task_switch_prepare(&fixture, profile);

    if (!failed) {
        failed |= !task_switch_install(&fixture);
        failed |= core_machine_run(fixture.machine, budget, &result) !=
            TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT;
        failed |= core_machine_get_cpu_diagnostic(fixture.machine, &diagnostic) !=
            TYPE_STATUS_OK || !diagnostic.first_fault.valid ||
            !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                VCPUINS_EXCEPT_CE);
        failed |= !fixture.cpu->data.tr.flagValid ||
            fixture.cpu->data.tr.selector != 0x0028u;
        if (failed) {
            STD_FPRINTF(STD_STDERR,
                "T261 baseline=%u result=%u fault=%x tr=%04x\n",
                (unsigned)profile, (unsigned)result.reason,
                (unsigned)diagnostic.first_fault.exception_mask,
                fixture.cpu->data.tr.selector);
        }
    }
    core_machine_destroy(fixture.machine);
    return failed;
}

int main(void)
{
    C_INT failed = 0;

    failed |= task_switch_expect_baseline(CORE_MACHINE_CPU_PROFILE_80286);
    failed |= task_switch_expect_baseline(CORE_MACHINE_CPU_PROFILE_80386);
    if (failed) return 1;
    STD_PRINTF("M5:T261:S1:TASK-SWITCH:BASELINE:OK\n");
    return 0;
}
