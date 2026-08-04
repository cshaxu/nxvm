#include "type.h"

#include "core/machine/machine_interface.h"
#include "core/machine/memory.h"

typedef struct firmware_interrupt_portal_probe {
    C_INT calls;
    uint8_t vector;
} firmware_interrupt_portal_probe;

static C_VOID firmware_interrupt_portal_probe_dispatch(C_VOID *opaque,
    core_machine_cpu_execution_context *execution, uint8_t vector)
{
    firmware_interrupt_portal_probe *probe =
        (firmware_interrupt_portal_probe *)opaque;

    (C_VOID)execution;
    if (probe != STD_NULL) {
        ++probe->calls;
        probe->vector = vector;
    }
}

static C_INT run_case(uint8_t vector, uint32_t origin_start,
    uint32_t origin_bytes, C_INT expected_calls, C_INT expect_ivt_fallthrough)
{
    C_UCHAR program[] = { 0xcdu, vector };
    static const C_UCHAR ivt_entry[] = { 0x00u, 0x01u, 0x00u, 0x00u };
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    core_machine_firmware_interrupt_portal portal;
    firmware_interrupt_portal_probe probe = { 0, 0u };
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine *machine = STD_NULL;
    t_ram *memory;
    C_INT failed = 0;

    portal.vector = 0xf0u;
    portal.origin_linear_start = origin_start;
    portal.origin_linear_bytes = origin_bytes;
    portal.provider = firmware_interrupt_portal_probe_dispatch;
    portal.context = &probe;
    if (core_machine_create(&config, &machine) != TYPE_STATUS_OK) goto fail;
    memory = core_machine_configuration_memory_borrow(machine);
    if (memory == STD_NULL || core_machine_memory_register_mapping(memory,
            0xfffffff0u, 0x000ffff0u, sizeof(program)) != TYPE_STATUS_OK ||
        core_machine_install_firmware_interrupt_portal(machine, &portal) !=
            TYPE_STATUS_OK ||
        core_machine_install_firmware_interrupt_portal(machine, &portal) !=
            TYPE_STATUS_UNSUPPORTED ||
        core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
        core_machine_install_firmware_interrupt_portal(machine, &portal) !=
            TYPE_STATUS_INVALID_STATE ||
        core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, 0xfffffff0u, program,
            sizeof(program)) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, (uint32_t)vector * 4u, ivt_entry,
            sizeof(ivt_entry)) != TYPE_STATUS_OK ||
        core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
        probe.calls != expected_calls ||
        (expected_calls && probe.vector != 0xf0u) ||
        (expect_ivt_fallthrough && result.linear_pc != 0x00000100u)) {
        failed = 1;
    }

fail:
    core_machine_destroy(machine);
    return failed;
}

C_INT main(C_VOID)
{
    C_INT failed = 0;

    failed |= run_case(0xf0u, 0xfffffff0u, 0xfffffff2u, 1, 0);
    failed |= run_case(0xf0u, 0u, 2u, 0, 1);
    failed |= run_case(0xf1u, 0xfffffff0u, 0xfffffff2u, 0, 1);
    if (failed) return 1;
    STD_PRINTF("M5:T209:S2:FIRMWARE-INTERRUPT-PORTAL:OK\n");
    return 0;
}
