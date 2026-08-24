#include "type.h"

#include "core/machine/machine.h"

static const core_machine_timing_seam plan_expected_seams[
    CORE_MACHINE_TIMING_CAPABILITY_COUNT] = {
    CORE_MACHINE_TIMING_SEAM_CPU_PROGRAM,
    CORE_MACHINE_TIMING_SEAM_CPU_PROGRAM,
    CORE_MACHINE_TIMING_SEAM_CPU_PROGRAM,
    CORE_MACHINE_TIMING_SEAM_RETIREMENT,
    CORE_MACHINE_TIMING_SEAM_CPU_PROGRAM,
    CORE_MACHINE_TIMING_SEAM_CLOCK,
    CORE_MACHINE_TIMING_SEAM_LIFECYCLE,
    CORE_MACHINE_TIMING_SEAM_TRANSACTION,
    CORE_MACHINE_TIMING_SEAM_TRANSACTION,
    CORE_MACHINE_TIMING_SEAM_TRANSACTION,
    CORE_MACHINE_TIMING_SEAM_MEMORY,
    CORE_MACHINE_TIMING_SEAM_MEMORY,
    CORE_MACHINE_TIMING_SEAM_CONFIGURATION,
    CORE_MACHINE_TIMING_SEAM_DEVICE,
    CORE_MACHINE_TIMING_SEAM_DEVICE,
    CORE_MACHINE_TIMING_SEAM_DEVICE,
    CORE_MACHINE_TIMING_SEAM_DEVICE,
    CORE_MACHINE_TIMING_SEAM_DEVICE,
    CORE_MACHINE_TIMING_SEAM_DEVICE,
    CORE_MACHINE_TIMING_SEAM_DEVICE,
    CORE_MACHINE_TIMING_SEAM_DEVICE,
    CORE_MACHINE_TIMING_SEAM_DEVICE,
    CORE_MACHINE_TIMING_SEAM_OBSERVATION,
    CORE_MACHINE_TIMING_SEAM_OBSERVATION,
    CORE_MACHINE_TIMING_SEAM_OBSERVATION,
    CORE_MACHINE_TIMING_SEAM_OBSERVATION,
    CORE_MACHINE_TIMING_SEAM_OBSERVATION,
    CORE_MACHINE_TIMING_SEAM_OBSERVATION,
    CORE_MACHINE_TIMING_SEAM_OBSERVATION,
    CORE_MACHINE_TIMING_SEAM_OBSERVATION
};

static C_INT plan_capability_is_non_guest_time(
    core_machine_timing_capability capability)
{
    return capability == CORE_MACHINE_TIMING_CAPABILITY_DISPLAY_PRESENT ||
        capability == CORE_MACHINE_TIMING_CAPABILITY_INPUT_HOST ||
        capability == CORE_MACHINE_TIMING_CAPABILITY_TRACE_DEBUG ||
        capability == CORE_MACHINE_TIMING_CAPABILITY_PLATFORM_MAILBOX ||
        capability == CORE_MACHINE_TIMING_CAPABILITY_PLATFORM_RESOURCE ||
        capability == CORE_MACHINE_TIMING_CAPABILITY_PLATFORM_WAIT ||
        capability == CORE_MACHINE_TIMING_CAPABILITY_SESSION_COMMAND ||
        capability == CORE_MACHINE_TIMING_CAPABILITY_PRODUCT_DEBUG;
}

static C_INT plan_default_and_copy(C_VOID)
{
    core_machine_config configuration = { .memory_bytes =
        CORE_MACHINE_MINIMUM_MEMORY_BYTES };
    core_machine_plan *plan = STD_NULL;
    core_machine_timing_declaration temporary;
    core_machine *machine = STD_NULL;
    core_machine_timing_declaration declaration;
    core_machine_timing_disposition disposition;
    STD_SIZE_T index;
    C_INT failed = 0;

    failed |= core_machine_plan_create(&configuration, &plan) != TYPE_STATUS_OK;
    failed |= !failed && plan->declaration_count != CORE_MACHINE_TIMING_CAPABILITY_COUNT;
    for (index = 0u; index < CORE_MACHINE_TIMING_CAPABILITY_COUNT; ++index) {
        const core_machine_timing_declaration *candidate = &plan->declarations[index];
        const core_machine_timing_disposition expected =
            plan_capability_is_non_guest_time((core_machine_timing_capability)index) ?
            CORE_MACHINE_TIMING_DISPOSITION_NON_GUEST_TIME :
            CORE_MACHINE_TIMING_DISPOSITION_L2_FALLBACK;

        failed |= candidate->capability != (core_machine_timing_capability)index ||
            candidate->seam != plan_expected_seams[index] ||
            candidate->disposition != expected;
    }
    temporary = plan->declarations[CORE_MACHINE_TIMING_CAPABILITY_CPU_EXEC];
    plan->declarations[CORE_MACHINE_TIMING_CAPABILITY_CPU_EXEC] =
        plan->declarations[CORE_MACHINE_TIMING_CAPABILITY_CPU_EXCEPT];
    plan->declarations[CORE_MACHINE_TIMING_CAPABILITY_CPU_EXCEPT] = temporary;
    failed |= core_machine_create_from_plan(plan, &machine) != TYPE_STATUS_OK ||
        machine == STD_NULL;
    plan->declarations[CORE_MACHINE_TIMING_CAPABILITY_CPU_EXEC].disposition =
        CORE_MACHINE_TIMING_DISPOSITION_L3_REQUIRED;
    failed |= !failed && core_machine_get_timing_disposition(machine,
        CORE_MACHINE_TIMING_CAPABILITY_CPU_EXEC, &disposition) != TYPE_STATUS_OK;
    failed |= !failed && disposition != CORE_MACHINE_TIMING_DISPOSITION_L2_FALLBACK;
    failed |= !failed && core_machine_get_timing_declaration(machine,
        CORE_MACHINE_TIMING_CAPABILITY_TIME_LIFECYCLE, &declaration) !=
        TYPE_STATUS_OK;
    failed |= !failed && declaration.seam != CORE_MACHINE_TIMING_SEAM_LIFECYCLE;
    core_machine_destroy(machine);
    core_machine_plan_destroy(plan);
    return failed;
}

static C_INT plan_rejects_incomplete_or_unavailable(C_VOID)
{
    core_machine_plan *plan = STD_NULL;
    core_machine *machine = (core_machine *)(type_virtual_address)1u;
    STD_SIZE_T index;
    C_INT failed = 0;

    failed |= core_machine_plan_create(STD_NULL, &plan) != TYPE_STATUS_OK;
    --plan->declaration_count;
    failed |= core_machine_create_from_plan(plan, &machine) !=
        TYPE_STATUS_INVALID_ARGUMENT || machine != STD_NULL;
    core_machine_plan_destroy(plan);
    failed |= core_machine_plan_create(STD_NULL, &plan) != TYPE_STATUS_OK;
    plan->declarations[1].capability = plan->declarations[0].capability;
    failed |= core_machine_create_from_plan(plan, &machine) !=
        TYPE_STATUS_INVALID_ARGUMENT || machine != STD_NULL;
    core_machine_plan_destroy(plan);
    failed |= core_machine_plan_create(STD_NULL, &plan) != TYPE_STATUS_OK;
    plan->declarations[CORE_MACHINE_TIMING_CAPABILITY_CPU_EXEC].disposition =
        CORE_MACHINE_TIMING_DISPOSITION_L3_REQUIRED;
    failed |= core_machine_create_from_plan(plan, &machine) !=
        TYPE_STATUS_INVALID_ARGUMENT || machine != STD_NULL;
    core_machine_plan_destroy(plan);
    failed |= core_machine_plan_create(STD_NULL, &plan) != TYPE_STATUS_OK;
    plan->declarations[CORE_MACHINE_TIMING_CAPABILITY_DISPLAY_PRESENT].disposition =
        CORE_MACHINE_TIMING_DISPOSITION_L2_FALLBACK;
    failed |= core_machine_create_from_plan(plan, &machine) !=
        TYPE_STATUS_INVALID_ARGUMENT || machine != STD_NULL;
    core_machine_plan_destroy(plan);
    failed |= core_machine_plan_create(STD_NULL, &plan) != TYPE_STATUS_OK;
    plan->declarations[CORE_MACHINE_TIMING_CAPABILITY_CTRL_PIC].seam =
        CORE_MACHINE_TIMING_SEAM_TRANSACTION;
    failed |= core_machine_create_from_plan(plan, &machine) !=
        TYPE_STATUS_INVALID_ARGUMENT || machine != STD_NULL;
    for (index = 0u; index < CORE_MACHINE_TIMING_CAPABILITY_COUNT; ++index) {
        const core_machine_timing_capability capability =
            (core_machine_timing_capability)index;

        core_machine_plan_destroy(plan);
        failed |= core_machine_plan_create(STD_NULL, &plan) != TYPE_STATUS_OK;
        machine = (core_machine *)(type_virtual_address)1u;
        plan->declarations[index].disposition =
            plan_capability_is_non_guest_time(capability) ?
            CORE_MACHINE_TIMING_DISPOSITION_L2_FALLBACK :
            CORE_MACHINE_TIMING_DISPOSITION_L3_REQUIRED;
        failed |= core_machine_create_from_plan(plan, &machine) !=
            TYPE_STATUS_INVALID_ARGUMENT || machine != STD_NULL;
    }
    core_machine_plan_destroy(plan);
    return failed;
}

static C_INT plan_rejects_topology_before_publication(C_VOID)
{
    core_machine_config configuration = { .memory_bytes =
        CORE_MACHINE_MINIMUM_MEMORY_BYTES };
    core_machine_plan *plan = STD_NULL;
    core_machine *machine = (core_machine *)(type_virtual_address)1u;
    C_INT failed = 0;

    failed |= core_machine_plan_create(&configuration, &plan) != TYPE_STATUS_OK;
    plan->topology.fdc_present = TYPE_TRUE;
    failed |= core_machine_create_from_plan(plan, &machine) !=
        TYPE_STATUS_INVALID_ARGUMENT || machine != STD_NULL;
    core_machine_plan_destroy(plan);
    failed |= core_machine_plan_create(&configuration, &plan) != TYPE_STATUS_OK;
    plan->topology.absent_memory_present = TYPE_TRUE;
    plan->topology.absent_memory.physical_start = 0x00100000u;
    failed |= core_machine_create_from_plan(plan, &machine) !=
        TYPE_STATUS_INVALID_ARGUMENT || machine != STD_NULL;
    core_machine_plan_destroy(plan);
    return failed;
}

static C_INT plan_rejects_invalid_transaction_contract_before_publication(C_VOID)
{
    core_machine_config configuration = { .memory_bytes =
        CORE_MACHINE_MINIMUM_MEMORY_BYTES };
    core_machine_plan *plan = STD_NULL;
    core_machine *machine = (core_machine *)(type_virtual_address)1u;
    C_INT failed = 0;

    configuration.transaction_contract.external_cycle_timing.page_bytes = 3u;
    failed |= core_machine_plan_create(&configuration, &plan) != TYPE_STATUS_OK;
    failed |= core_machine_create_from_plan(plan, &machine) !=
        TYPE_STATUS_INVALID_ARGUMENT || machine != STD_NULL;
    core_machine_plan_destroy(plan);
    return failed;
}

C_INT main(C_VOID)
{
    if (plan_default_and_copy() || plan_rejects_incomplete_or_unavailable() ||
        plan_rejects_topology_before_publication() ||
        plan_rejects_invalid_transaction_contract_before_publication()) {
        return 1;
    }
    puts("M5:T434:S1:PLAN-DECLARATIONS:OK");
    puts("M5:T434:S1:PLAN-VALIDATION:OK");
    puts("M5:T434:S1:PLAN-COPY:OK");
    puts("M5:T434:S2:ROLLBACK-EQUIVALENCE:OK");
    puts("M5:T434:S3:ALL-DECLARATIONS:OK");
    puts("M5:T449:S2:TRANSACTION-CONTRACT:OK");
    return 0;
}
