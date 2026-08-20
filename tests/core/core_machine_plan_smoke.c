#include "type.h"

#include "core/machine/machine.h"

static C_INT plan_default_and_copy(C_VOID)
{
    core_machine_config configuration = { .memory_bytes =
        CORE_MACHINE_MINIMUM_MEMORY_BYTES };
    core_machine_plan plan;
    core_machine_timing_declaration temporary;
    core_machine *machine = STD_NULL;
    core_machine_timing_declaration declaration;
    core_machine_timing_disposition disposition;
    C_INT failed = 0;

    core_machine_plan_initialize(&plan, &configuration);
    failed |= plan.declaration_count != CORE_MACHINE_TIMING_CAPABILITY_COUNT;
    failed |= plan.declarations[CORE_MACHINE_TIMING_CAPABILITY_CPU_EXEC].disposition !=
        CORE_MACHINE_TIMING_DISPOSITION_L2_FALLBACK;
    failed |= plan.declarations[CORE_MACHINE_TIMING_CAPABILITY_DISPLAY_PRESENT].disposition !=
        CORE_MACHINE_TIMING_DISPOSITION_NON_GUEST_TIME;
    failed |= plan.declarations[CORE_MACHINE_TIMING_CAPABILITY_CTRL_PIT].seam !=
        CORE_MACHINE_TIMING_SEAM_DEVICE;
    temporary = plan.declarations[CORE_MACHINE_TIMING_CAPABILITY_CPU_EXEC];
    plan.declarations[CORE_MACHINE_TIMING_CAPABILITY_CPU_EXEC] =
        plan.declarations[CORE_MACHINE_TIMING_CAPABILITY_CPU_EXCEPT];
    plan.declarations[CORE_MACHINE_TIMING_CAPABILITY_CPU_EXCEPT] = temporary;
    failed |= core_machine_create_from_plan(&plan, &machine) != TYPE_STATUS_OK ||
        machine == STD_NULL;
    plan.declarations[CORE_MACHINE_TIMING_CAPABILITY_CPU_EXEC].disposition =
        CORE_MACHINE_TIMING_DISPOSITION_L3_REQUIRED;
    failed |= !failed && core_machine_get_timing_disposition(machine,
        CORE_MACHINE_TIMING_CAPABILITY_CPU_EXEC, &disposition) != TYPE_STATUS_OK;
    failed |= !failed && disposition != CORE_MACHINE_TIMING_DISPOSITION_L2_FALLBACK;
    failed |= !failed && core_machine_get_timing_declaration(machine,
        CORE_MACHINE_TIMING_CAPABILITY_TIME_LIFECYCLE, &declaration) !=
        TYPE_STATUS_OK;
    failed |= !failed && declaration.seam != CORE_MACHINE_TIMING_SEAM_LIFECYCLE;
    core_machine_destroy(machine);
    return failed;
}

static C_INT plan_rejects_incomplete_or_unavailable(C_VOID)
{
    core_machine_plan plan;
    core_machine *machine = (core_machine *)(type_virtual_address)1u;
    C_INT failed = 0;

    core_machine_plan_initialize(&plan, STD_NULL);
    --plan.declaration_count;
    failed |= core_machine_create_from_plan(&plan, &machine) !=
        TYPE_STATUS_INVALID_ARGUMENT || machine != STD_NULL;
    core_machine_plan_initialize(&plan, STD_NULL);
    plan.declarations[1].capability = plan.declarations[0].capability;
    failed |= core_machine_create_from_plan(&plan, &machine) !=
        TYPE_STATUS_INVALID_ARGUMENT || machine != STD_NULL;
    core_machine_plan_initialize(&plan, STD_NULL);
    plan.declarations[CORE_MACHINE_TIMING_CAPABILITY_CPU_EXEC].disposition =
        CORE_MACHINE_TIMING_DISPOSITION_L3_REQUIRED;
    failed |= core_machine_create_from_plan(&plan, &machine) !=
        TYPE_STATUS_INVALID_ARGUMENT || machine != STD_NULL;
    core_machine_plan_initialize(&plan, STD_NULL);
    plan.declarations[CORE_MACHINE_TIMING_CAPABILITY_DISPLAY_PRESENT].disposition =
        CORE_MACHINE_TIMING_DISPOSITION_L2_FALLBACK;
    failed |= core_machine_create_from_plan(&plan, &machine) !=
        TYPE_STATUS_INVALID_ARGUMENT || machine != STD_NULL;
    core_machine_plan_initialize(&plan, STD_NULL);
    plan.declarations[CORE_MACHINE_TIMING_CAPABILITY_CTRL_PIC].seam =
        CORE_MACHINE_TIMING_SEAM_TRANSACTION;
    failed |= core_machine_create_from_plan(&plan, &machine) !=
        TYPE_STATUS_INVALID_ARGUMENT || machine != STD_NULL;
    return failed;
}

C_INT main(C_VOID)
{
    if (plan_default_and_copy() || plan_rejects_incomplete_or_unavailable()) {
        return 1;
    }
    puts("M5:T434:S1:PLAN-DECLARATIONS:OK");
    puts("M5:T434:S1:PLAN-VALIDATION:OK");
    puts("M5:T434:S1:PLAN-COPY:OK");
    return 0;
}
