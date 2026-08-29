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
    plan->topology.absent_memory_count = 1u;
    plan->topology.absent_memory[0].physical_start = 0x00100000u;
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

static C_INT plan_controller_timing_rules_are_copied_and_validated(C_VOID)
{
    core_machine_config configuration = { .memory_bytes =
        CORE_MACHINE_MINIMUM_MEMORY_BYTES };
    const core_machine_controller_timing_rules source_rules = {
        CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK,
        CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK,
        CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_DMA_SERVICE_PHASES,
        CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK,
        CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK
    };
    core_machine_plan *plan = STD_NULL;
    core_machine *machine = STD_NULL;
    core_machine_timing_disposition disposition;
    C_INT failed = 0;

    configuration.clock_plan.dma = (core_machine_clock_ratio) {3u, 8u, 0u};
    configuration.clock_plan.pit = (core_machine_clock_ratio) {1193182u, 8000000u, 0u};
    failed |= core_machine_plan_create(&configuration, &plan) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_plan_set_controller_timing_rules(plan,
        &source_rules) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_create_from_plan(plan, &machine) !=
        TYPE_STATUS_OK;
    failed |= !failed && core_machine_get_timing_disposition(machine,
        CORE_MACHINE_TIMING_CAPABILITY_CTRL_PIC, &disposition) != TYPE_STATUS_OK;
    failed |= !failed && disposition != CORE_MACHINE_TIMING_DISPOSITION_L2_FALLBACK;
    failed |= !failed && core_machine_get_timing_disposition(machine,
        CORE_MACHINE_TIMING_CAPABILITY_CTRL_DMA, &disposition) != TYPE_STATUS_OK;
    failed |= !failed && disposition != CORE_MACHINE_TIMING_DISPOSITION_L3_REQUIRED;
    failed |= !failed && core_machine_get_timing_disposition(machine,
        CORE_MACHINE_TIMING_CAPABILITY_CTRL_PIT, &disposition) != TYPE_STATUS_OK;
    failed |= !failed && disposition != CORE_MACHINE_TIMING_DISPOSITION_L3_REQUIRED;
    failed |= !failed && machine->timing_plan.controller_timing.pit_clock !=
        CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK;
    core_machine_destroy(machine);
    core_machine_plan_destroy(plan);
    return failed;
}

static C_INT plan_rejects_invalid_controller_timing_rules(C_VOID)
{
    core_machine_config configuration = { .memory_bytes =
        CORE_MACHINE_MINIMUM_MEMORY_BYTES };
    core_machine_controller_timing_rules rules = {
        CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK,
        CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK,
        CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_DMA_SERVICE_PHASES,
        CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK,
        CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK
    };
    core_machine_plan *plan = STD_NULL;
    core_machine *machine = (core_machine *)(type_virtual_address)1u;
    C_INT failed = 0;

    failed |= core_machine_plan_create(&configuration, &plan) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_plan_set_controller_timing_rules(plan,
        &rules) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_create_from_plan(plan, &machine) !=
        TYPE_STATUS_INVALID_ARGUMENT || machine != STD_NULL;
    core_machine_plan_destroy(plan);
    configuration.clock_plan.dma = (core_machine_clock_ratio) {3u, 8u, 0u};
    configuration.clock_plan.pit = (core_machine_clock_ratio) {1u, 4u, 0u};
    failed |= core_machine_plan_create(&configuration, &plan) != TYPE_STATUS_OK;
    rules.pic_visibility = CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK;
    failed |= !failed && core_machine_plan_set_controller_timing_rules(plan,
        &rules) != TYPE_STATUS_OK;
    machine = (core_machine *)(type_virtual_address)1u;
    failed |= !failed && core_machine_create_from_plan(plan, &machine) !=
        TYPE_STATUS_INVALID_ARGUMENT || machine != STD_NULL;
    core_machine_plan_destroy(plan);
    rules.pic_visibility = CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK;
    rules.dma_clock = CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK;
    failed |= core_machine_plan_create(&configuration, &plan) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_plan_set_controller_timing_rules(plan,
        &rules) != TYPE_STATUS_OK;
    machine = (core_machine *)(type_virtual_address)1u;
    failed |= !failed && core_machine_create_from_plan(plan, &machine) !=
        TYPE_STATUS_INVALID_ARGUMENT || machine != STD_NULL;
    core_machine_plan_destroy(plan);
    return failed;
}

static C_INT plan_selects_single_controller_xt_board(C_VOID)
{
    core_machine_config configuration = {
        .memory_bytes = 256u * 1024u,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8088,
        .pic_topology = CORE_MACHINE_PIC_TOPOLOGY_SINGLE,
        .dma_controller_count = 1u
    };
    core_machine_plan_topology topology = {0};
    core_machine_plan *plan = STD_NULL;
    core_machine *machine = STD_NULL;
    C_INT failed = 0;

    topology.dma_present = TYPE_TRUE;
    topology.dma = (core_machine_dma_wiring) {
        CORE_MACHINE_DMA_FDC_CHANNEL_UNBOUND, 1u, 0u};
    failed |= core_machine_plan_create(&configuration, &plan) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_plan_set_topology(plan, &topology) !=
        TYPE_STATUS_OK;
    failed |= !failed && core_machine_create_from_plan(plan, &machine) !=
        TYPE_STATUS_OK;
    failed |= !failed && (!core_machine_port_has_read(&machine->executor_port, 0x0020u) ||
        !core_machine_port_has_write(&machine->executor_port, 0x0000u) ||
        !core_machine_port_has_read(&machine->executor_port, 0x0081u) ||
        !core_machine_port_has_write(&machine->executor_port, 0x0083u) ||
        core_machine_port_has_read(&machine->executor_port, 0x0087u) ||
        core_machine_port_has_write(&machine->executor_port, 0x0089u) ||
        core_machine_port_has_read(&machine->executor_port, 0x008au) ||
        core_machine_port_has_write(&machine->executor_port, 0x008bu) ||
        core_machine_port_has_read(&machine->executor_port, 0x008fu) ||
        core_machine_port_has_read(&machine->executor_port, 0x00a0u) ||
        core_machine_port_has_write(&machine->executor_port, 0x00d0u) ||
        core_machine_port_has_read(&machine->executor_port, 0x0070u) ||
        core_machine_port_has_write(&machine->executor_port, 0x0071u));
    failed |= !failed && core_machine_get_fdc_dma_request_binding(machine,
        &(core_machine_dma_request_binding) {0}) != TYPE_STATUS_INVALID_STATE;
    failed |= !failed && core_machine_freeze_execution_providers(machine) !=
        TYPE_STATUS_OK;
    failed |= !failed && core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= !failed && (!core_machine_port_has_read(&machine->executor_port, 0x0020u) ||
        core_machine_port_has_read(&machine->executor_port, 0x00a0u) ||
        core_machine_port_has_read(&machine->executor_port, 0x0070u));
    core_machine_destroy(machine);
    core_machine_plan_destroy(plan);
    return failed;
}

static C_INT plan_l2_pit_deadline_remains_schedulable(C_VOID)
{
    core_machine_config configuration = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .clock_plan.pit = {1u, 1u, 0u}
    };
    core_machine_plan *plan = STD_NULL;
    core_machine *machine = STD_NULL;
    core_machine_time_observation observation;
    core_machine_timing_disposition disposition;
    type_bool advanced = TYPE_FALSE;
    C_INT failed = 0;

    failed |= core_machine_plan_create(&configuration, &plan) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_create_from_plan(plan, &machine) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_get_timing_disposition(machine,
        CORE_MACHINE_TIMING_CAPABILITY_CTRL_PIT, &disposition) != TYPE_STATUS_OK;
    failed |= !failed && disposition != CORE_MACHINE_TIMING_DISPOSITION_L2_FALLBACK;
    core_machine_port_write(&machine->executor_port, 0x0043u, 0x34u);
    core_machine_port_write(&machine->executor_port, 0x0040u, 4u);
    core_machine_port_write(&machine->executor_port, 0x0040u, 0u);
    failed |= !failed && core_machine_capture_time_observation(machine, &observation) !=
        TYPE_STATUS_OK;
    failed |= !failed && (!observation.next_deadline_valid ||
        observation.next_deadline_tick != 1u);
    failed |= !failed && core_machine_advance_to_next_deadline(machine, &advanced) !=
        TYPE_STATUS_OK;
    failed |= !failed && (!advanced || machine->elapsed_ticks != 1u);
    core_machine_destroy(machine);
    core_machine_plan_destroy(plan);
    return failed;
}

C_INT main(C_VOID)
{
    if (plan_default_and_copy() || plan_rejects_incomplete_or_unavailable() ||
        plan_rejects_topology_before_publication() ||
        plan_rejects_invalid_transaction_contract_before_publication() ||
        plan_controller_timing_rules_are_copied_and_validated() ||
        plan_rejects_invalid_controller_timing_rules() ||
        plan_selects_single_controller_xt_board() ||
        plan_l2_pit_deadline_remains_schedulable()) {
        return 1;
    }
    puts("M5:T434:S1:PLAN-DECLARATIONS:OK");
    puts("M5:T434:S1:PLAN-VALIDATION:OK");
    puts("M5:T434:S1:PLAN-COPY:OK");
    puts("M5:T434:S2:ROLLBACK-EQUIVALENCE:OK");
    puts("M5:T434:S3:ALL-DECLARATIONS:OK");
    puts("M5:T449:S2:TRANSACTION-CONTRACT:OK");
    puts("M5:T462:S2:CONTROLLER-RULE-PLAN:OK");
    puts("M5:T462:S2:CONTROLLER-RULE-REJECTION:OK");
    puts("M5:T462:S4:PIC-L2-BOUNDARY:OK");
    puts("M5:T462:S4:CONTROLLER-LEDGER-CLOSURE:OK");
    puts("M5:T484:S5:XT-B2-PLAN:OK");
    puts("M5:T484:S5:XT-NO-AT-TOPOLOGY:OK");
    puts("M5:T499:S3:L2-PIT-DEADLINE:OK");
    return 0;
}
