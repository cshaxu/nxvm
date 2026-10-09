#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "core/board-base/machine_board_interface.h"
#include "core/board-base/machine_board_state.h"
#include "composition/plan_core_fixture.h"

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

static lib_i32 plan_capability_is_non_guest_time(
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

static lib_i32 plan_handle_publication(void)
{
    const core_machine_config configuration = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES
    };
    core_machine_plan *plan = LIB_NULL;
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    lib_i32 failed = 0;

    if (core_machine_plan_create(&configuration, &plan) != LIB_STATUS_OK)
        return 1;
    if (core_machine_create_from_plan(plan, &machine, &board) != LIB_STATUS_OK) {
        core_machine_plan_destroy(plan);
        return 1;
    }
    failed |= test_plan_core_attachment(machine, board);
    failed |= board != LIB_NULL && board->core != machine;
    core_machine_destroy(machine);

    machine = (core_machine *)(lib_uptr)1u;
    board = (core_machine_board_state *)(lib_uptr)1u;
    failed |= core_machine_create_from_plan(LIB_NULL, &machine, &board) !=
        LIB_STATUS_INVALID_ARGUMENT || machine != LIB_NULL || board != LIB_NULL;
    board = (core_machine_board_state *)(lib_uptr)1u;
    failed |= core_machine_create_from_plan(plan, LIB_NULL, &board) !=
        LIB_STATUS_INVALID_ARGUMENT || board != LIB_NULL;
    machine = (core_machine *)(lib_uptr)1u;
    failed |= core_machine_create_from_plan(plan, &machine, LIB_NULL) !=
        LIB_STATUS_INVALID_ARGUMENT || machine != LIB_NULL;
    /* Valid plan, but topology application fails after attachment creation. */
    plan->topology.absent_memory_count = 1u;
    plan->topology.absent_memory[0].physical_start = 0x00100000u;
    machine = (core_machine *)(lib_uptr)1u;
    board = (core_machine_board_state *)(lib_uptr)1u;
    failed |= core_machine_create_from_plan(plan, &machine, &board) !=
        LIB_STATUS_INVALID_ARGUMENT || machine != LIB_NULL || board != LIB_NULL;
    core_machine_plan_destroy(plan);

    failed |= test_plan_core_allocation_failures(&configuration);
    return failed;
}

static lib_i32 plan_default_and_copy(void)
{
    core_machine_config configuration = { .memory_bytes =
        CORE_MACHINE_MINIMUM_MEMORY_BYTES };
    core_machine_plan *plan = LIB_NULL;
    core_machine_timing_declaration temporary;
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    core_machine_timing_declaration declaration;
    core_machine_timing_disposition disposition;
    lib_size index;
    lib_i32 failed = 0;

    failed |= core_machine_plan_create(&configuration, &plan) != LIB_STATUS_OK;
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
    failed |= core_machine_create_from_plan(plan, &machine, &board) != LIB_STATUS_OK ||
        machine == LIB_NULL;
    plan->declarations[CORE_MACHINE_TIMING_CAPABILITY_CPU_EXEC].disposition =
        CORE_MACHINE_TIMING_DISPOSITION_L3_REQUIRED;
    failed |= !failed && core_machine_get_timing_disposition(machine,
        CORE_MACHINE_TIMING_CAPABILITY_CPU_EXEC, &disposition) != LIB_STATUS_OK;
    failed |= !failed && disposition != CORE_MACHINE_TIMING_DISPOSITION_L2_FALLBACK;
    failed |= !failed && core_machine_get_timing_declaration(machine,
        CORE_MACHINE_TIMING_CAPABILITY_TIME_LIFECYCLE, &declaration) !=
        LIB_STATUS_OK;
    failed |= !failed && declaration.seam != CORE_MACHINE_TIMING_SEAM_LIFECYCLE;
    core_machine_destroy(machine);
    core_machine_plan_destroy(plan);
    return failed;
}

static lib_i32 plan_rejects_incomplete_or_unavailable(void)
{
    core_machine_plan *plan = LIB_NULL;
    core_machine *machine = (core_machine *)(lib_uptr)1u;
    core_machine_board_state *board = LIB_NULL;
    lib_size index;
    lib_i32 failed = 0;

    failed |= core_machine_plan_create(LIB_NULL, &plan) != LIB_STATUS_OK;
    --plan->declaration_count;
    failed |= core_machine_create_from_plan(plan, &machine, &board) !=
        LIB_STATUS_INVALID_ARGUMENT || machine != LIB_NULL;
    core_machine_plan_destroy(plan);
    failed |= core_machine_plan_create(LIB_NULL, &plan) != LIB_STATUS_OK;
    plan->declarations[1].capability = plan->declarations[0].capability;
    failed |= core_machine_create_from_plan(plan, &machine, &board) !=
        LIB_STATUS_INVALID_ARGUMENT || machine != LIB_NULL;
    core_machine_plan_destroy(plan);
    failed |= core_machine_plan_create(LIB_NULL, &plan) != LIB_STATUS_OK;
    plan->declarations[CORE_MACHINE_TIMING_CAPABILITY_CPU_EXEC].disposition =
        CORE_MACHINE_TIMING_DISPOSITION_L3_REQUIRED;
    failed |= core_machine_create_from_plan(plan, &machine, &board) !=
        LIB_STATUS_INVALID_ARGUMENT || machine != LIB_NULL;
    core_machine_plan_destroy(plan);
    failed |= core_machine_plan_create(LIB_NULL, &plan) != LIB_STATUS_OK;
    plan->declarations[CORE_MACHINE_TIMING_CAPABILITY_DISPLAY_PRESENT].disposition =
        CORE_MACHINE_TIMING_DISPOSITION_L2_FALLBACK;
    failed |= core_machine_create_from_plan(plan, &machine, &board) !=
        LIB_STATUS_INVALID_ARGUMENT || machine != LIB_NULL;
    core_machine_plan_destroy(plan);
    failed |= core_machine_plan_create(LIB_NULL, &plan) != LIB_STATUS_OK;
    plan->declarations[CORE_MACHINE_TIMING_CAPABILITY_CTRL_PIC].seam =
        CORE_MACHINE_TIMING_SEAM_TRANSACTION;
    failed |= core_machine_create_from_plan(plan, &machine, &board) !=
        LIB_STATUS_INVALID_ARGUMENT || machine != LIB_NULL;
    for (index = 0u; index < CORE_MACHINE_TIMING_CAPABILITY_COUNT; ++index) {
        const core_machine_timing_capability capability =
            (core_machine_timing_capability)index;

        core_machine_plan_destroy(plan);
        failed |= core_machine_plan_create(LIB_NULL, &plan) != LIB_STATUS_OK;
        machine = (core_machine *)(lib_uptr)1u;
        plan->declarations[index].disposition =
            plan_capability_is_non_guest_time(capability) ?
            CORE_MACHINE_TIMING_DISPOSITION_L2_FALLBACK :
            CORE_MACHINE_TIMING_DISPOSITION_L3_REQUIRED;
        failed |= core_machine_create_from_plan(plan, &machine, &board) !=
            LIB_STATUS_INVALID_ARGUMENT || machine != LIB_NULL;
    }
    core_machine_plan_destroy(plan);
    return failed;
}

static lib_i32 plan_rejects_topology_before_publication(void)
{
    core_machine_config configuration = { .memory_bytes =
        CORE_MACHINE_MINIMUM_MEMORY_BYTES };
    core_machine_plan *plan = LIB_NULL;
    core_machine *machine = (core_machine *)(lib_uptr)1u;
    core_machine_board_state *board = LIB_NULL;
    lib_i32 failed = 0;

    failed |= core_machine_plan_create(&configuration, &plan) != LIB_STATUS_OK;
    plan->topology.fdc_present = LIB_TRUE;
    failed |= core_machine_create_from_plan(plan, &machine, &board) !=
        LIB_STATUS_INVALID_ARGUMENT || machine != LIB_NULL;
    core_machine_plan_destroy(plan);
    failed |= core_machine_plan_create(&configuration, &plan) != LIB_STATUS_OK;
    plan->topology.absent_memory_count = 1u;
    plan->topology.absent_memory[0].physical_start = 0x00100000u;
    failed |= core_machine_create_from_plan(plan, &machine, &board) !=
        LIB_STATUS_INVALID_ARGUMENT || machine != LIB_NULL;
    core_machine_plan_destroy(plan);
    return failed;
}

static lib_i32 plan_rejects_invalid_transaction_contract_before_publication(void)
{
    core_machine_config configuration = { .memory_bytes =
        CORE_MACHINE_MINIMUM_MEMORY_BYTES };
    core_machine_plan *plan = LIB_NULL;
    core_machine *machine = (core_machine *)(lib_uptr)1u;
    core_machine_board_state *board = LIB_NULL;
    lib_i32 failed = 0;

    configuration.transaction_contract.external_cycle_timing.page_bytes = 3u;
    failed |= core_machine_plan_create(&configuration, &plan) != LIB_STATUS_OK;
    failed |= !failed && core_machine_plan_validate(plan) != LIB_STATUS_OK;
    failed |= core_machine_create_from_plan(plan, &machine, &board) !=
        LIB_STATUS_INVALID_ARGUMENT || machine != LIB_NULL;
    core_machine_plan_destroy(plan);
    return failed;
}

static lib_i32 plan_controller_timing_rules_are_copied_and_validated(void)
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
    core_machine_plan *plan = LIB_NULL;
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    core_machine_timing_disposition disposition;
    lib_i32 failed = 0;

    configuration.clock_plan.dma = (core_machine_clock_ratio) {3u, 8u, 0u};
    configuration.clock_plan.pit = (core_machine_clock_ratio) {1193182u, 8000000u, 0u};
    failed |= core_machine_plan_create(&configuration, &plan) != LIB_STATUS_OK;
    failed |= !failed && core_machine_plan_set_controller_timing_rules(plan,
        &source_rules) != LIB_STATUS_OK;
    failed |= !failed && core_machine_create_from_plan(plan, &machine, &board) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_get_timing_disposition(machine,
        CORE_MACHINE_TIMING_CAPABILITY_CTRL_PIC, &disposition) != LIB_STATUS_OK;
    failed |= !failed && disposition != CORE_MACHINE_TIMING_DISPOSITION_L2_FALLBACK;
    failed |= !failed && core_machine_get_timing_disposition(machine,
        CORE_MACHINE_TIMING_CAPABILITY_CTRL_DMA, &disposition) != LIB_STATUS_OK;
    failed |= !failed && disposition != CORE_MACHINE_TIMING_DISPOSITION_L3_REQUIRED;
    failed |= !failed && core_machine_get_timing_disposition(machine,
        CORE_MACHINE_TIMING_CAPABILITY_CTRL_PIT, &disposition) != LIB_STATUS_OK;
    failed |= !failed && disposition != CORE_MACHINE_TIMING_DISPOSITION_L3_REQUIRED;
    failed |= !failed && board->controller_timing.pit_clock !=
        CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK;
    core_machine_destroy(machine);
    core_machine_plan_destroy(plan);
    return failed;
}

static lib_i32 plan_rejects_invalid_controller_timing_rules(void)
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
    core_machine_plan *plan = LIB_NULL;
    core_machine *machine = (core_machine *)(lib_uptr)1u;
    core_machine_board_state *board = LIB_NULL;
    lib_i32 failed = 0;

    failed |= core_machine_plan_create(&configuration, &plan) != LIB_STATUS_OK;
    failed |= !failed && core_machine_plan_set_controller_timing_rules(plan,
        &rules) != LIB_STATUS_OK;
    failed |= !failed && (core_machine_create_from_plan(plan, &machine, &board) !=
        LIB_STATUS_INVALID_ARGUMENT || machine != LIB_NULL);
    core_machine_plan_destroy(plan);
    configuration.clock_plan.dma = (core_machine_clock_ratio) {3u, 8u, 0u};
    configuration.clock_plan.pit = (core_machine_clock_ratio) {1u, 4u, 0u};
    failed |= core_machine_plan_create(&configuration, &plan) != LIB_STATUS_OK;
    rules.pic_visibility = CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK;
    failed |= !failed && core_machine_plan_set_controller_timing_rules(plan,
        &rules) != LIB_STATUS_OK;
    machine = (core_machine *)(lib_uptr)1u;
    failed |= !failed && (core_machine_create_from_plan(plan, &machine, &board) !=
        LIB_STATUS_INVALID_ARGUMENT || machine != LIB_NULL);
    core_machine_plan_destroy(plan);
    rules.pic_visibility = CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK;
    rules.dma_clock = CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK;
    failed |= core_machine_plan_create(&configuration, &plan) != LIB_STATUS_OK;
    failed |= !failed && core_machine_plan_set_controller_timing_rules(plan,
        &rules) != LIB_STATUS_OK;
    machine = (core_machine *)(lib_uptr)1u;
    failed |= !failed && (core_machine_create_from_plan(plan, &machine, &board) !=
        LIB_STATUS_INVALID_ARGUMENT || machine != LIB_NULL);
    core_machine_plan_destroy(plan);
    return failed;
}

static lib_i32 plan_selects_single_controller_xt_board(void)
{
    core_machine_config configuration = {
        .memory_bytes = 256u * 1024u,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8088,
        .pic_topology = CORE_MACHINE_PIC_TOPOLOGY_SINGLE,
        .dma_controller_count = 1u
    };
    core_machine_plan_topology topology = {0};
    core_machine_plan *plan = LIB_NULL;
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    lib_i32 failed = 0;

    topology.dma_present = LIB_TRUE;
    topology.dma = (core_machine_dma_wiring) {
        CORE_MACHINE_DMA_FDC_CHANNEL_UNBOUND, 1u, 0u};
    failed |= core_machine_plan_create(&configuration, &plan) != LIB_STATUS_OK;
    failed |= !failed && core_machine_plan_set_topology(plan, &topology) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_create_from_plan(plan, &machine, &board) !=
        LIB_STATUS_OK;
    failed |= !failed && test_plan_core_xt_routes(machine, LIB_FALSE);
    failed |= !failed && core_machine_get_fdc_dma_request_binding(board,
        &(core_machine_dma_request_binding) {0}) != LIB_STATUS_INVALID_STATE;
    failed |= !failed && core_machine_freeze_execution_providers(machine) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= !failed && test_plan_core_xt_routes(machine, LIB_TRUE);
    core_machine_destroy(machine);
    core_machine_plan_destroy(plan);
    return failed;
}

static lib_i32 plan_l2_pit_deadline_remains_schedulable(void)
{
    core_machine_config configuration = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .clock_plan.pit = {1u, 1u, 0u}
    };
    core_machine_plan *plan = LIB_NULL;
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    lib_i32 failed = 0;

    failed |= core_machine_plan_create(&configuration, &plan) != LIB_STATUS_OK;
    failed |= !failed && core_machine_create_from_plan(plan, &machine, &board) != LIB_STATUS_OK;
    failed |= !failed && test_plan_core_pit_deadline(machine);
    core_machine_destroy(machine);
    core_machine_plan_destroy(plan);
    return failed;
}

static lib_i32 plan_source_dma_deadline_is_schedulable(void)
{
    static const core_machine_dma_channel_provider provider = {0};
    const core_machine_controller_timing_rules rules = {
        CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK,
        CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK,
        CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_DMA_SERVICE_PHASES,
        CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK,
        CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK
    };
    core_machine_config configuration = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .clock_plan.dma = {3u, 8u, 0u}
    };
    core_machine_plan *plan = LIB_NULL;
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    core_machine_dma_request_binding binding = {0};
    lib_i32 failed = 0;

    failed |= core_machine_plan_create(&configuration, &plan) != LIB_STATUS_OK;
    failed |= !failed && core_machine_plan_set_controller_timing_rules(plan,
        &rules) != LIB_STATUS_OK;
    failed |= !failed && core_machine_create_from_plan(plan, &machine, &board) != LIB_STATUS_OK;
    failed |= !failed && core_machine_dma_bind_channel(board->shared_dma, 2u, &provider,
        LIB_NULL, &binding) != LIB_STATUS_OK;
    failed |= !failed && test_plan_core_dma_deadline(machine,
        board->shared_dma, &binding);
    core_machine_destroy(machine);
    core_machine_plan_destroy(plan);
    return failed;
}

lib_i32 main(void)
{
    if (plan_handle_publication() || plan_default_and_copy() ||
        plan_rejects_incomplete_or_unavailable() ||
        plan_rejects_topology_before_publication() ||
        plan_rejects_invalid_transaction_contract_before_publication() ||
        plan_controller_timing_rules_are_copied_and_validated() ||
        plan_rejects_invalid_controller_timing_rules() ||
        plan_selects_single_controller_xt_board() ||
        plan_l2_pit_deadline_remains_schedulable() ||
        plan_source_dma_deadline_is_schedulable()) {
        return 1;
    }
    lib_c_printf("%s\n", "PLAN-DECLARATIONS:OK");
    lib_c_printf("%s\n", "PLAN-VALIDATION:OK");
    lib_c_printf("%s\n", "PLAN-COPY:OK");
    lib_c_printf("%s\n", "ROLLBACK-EQUIVALENCE:OK");
    lib_c_printf("%s\n", "ALL-DECLARATIONS:OK");
    lib_c_printf("%s\n", "TRANSACTION-CONTRACT:OK");
    lib_c_printf("%s\n", "CONTROLLER-RULE-PLAN:OK");
    lib_c_printf("%s\n", "CONTROLLER-RULE-REJECTION:OK");
    lib_c_printf("%s\n", "PIC-L2-BOUNDARY:OK");
    lib_c_printf("%s\n", "CONTROLLER-LEDGER-CLOSURE:OK");
    lib_c_printf("%s\n", "XT-B2-PLAN:OK");
    lib_c_printf("%s\n", "XT-NO-AT-TOPOLOGY:OK");
    lib_c_printf("%s\n", "L2-PIT-DEADLINE:OK");
    lib_c_printf("%s\n", "BOARD-HANDLE-PUBLICATION:OK");
    return 0;
}
