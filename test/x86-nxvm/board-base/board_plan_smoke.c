#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "x86-nxvm/board-base/machine_board_state.h"
#include "composition/plan_core_fixture.h"

static const x86_nxvm_board_timing_seam plan_expected_seams[
    X86_NXVM_BOARD_TIMING_CAPABILITY_COUNT] = {
    X86_NXVM_BOARD_TIMING_SEAM_CPU_PROGRAM,
    X86_NXVM_BOARD_TIMING_SEAM_CPU_PROGRAM,
    X86_NXVM_BOARD_TIMING_SEAM_CPU_PROGRAM,
    X86_NXVM_BOARD_TIMING_SEAM_RETIREMENT,
    X86_NXVM_BOARD_TIMING_SEAM_CPU_PROGRAM,
    X86_NXVM_BOARD_TIMING_SEAM_CLOCK,
    X86_NXVM_BOARD_TIMING_SEAM_LIFECYCLE,
    X86_NXVM_BOARD_TIMING_SEAM_TRANSACTION,
    X86_NXVM_BOARD_TIMING_SEAM_TRANSACTION,
    X86_NXVM_BOARD_TIMING_SEAM_TRANSACTION,
    X86_NXVM_BOARD_TIMING_SEAM_MEMORY,
    X86_NXVM_BOARD_TIMING_SEAM_MEMORY,
    X86_NXVM_BOARD_TIMING_SEAM_CONFIGURATION,
    X86_NXVM_BOARD_TIMING_SEAM_DEVICE,
    X86_NXVM_BOARD_TIMING_SEAM_DEVICE,
    X86_NXVM_BOARD_TIMING_SEAM_DEVICE,
    X86_NXVM_BOARD_TIMING_SEAM_DEVICE,
    X86_NXVM_BOARD_TIMING_SEAM_DEVICE,
    X86_NXVM_BOARD_TIMING_SEAM_DEVICE,
    X86_NXVM_BOARD_TIMING_SEAM_DEVICE,
    X86_NXVM_BOARD_TIMING_SEAM_DEVICE,
    X86_NXVM_BOARD_TIMING_SEAM_DEVICE,
    X86_NXVM_BOARD_TIMING_SEAM_OBSERVATION,
    X86_NXVM_BOARD_TIMING_SEAM_OBSERVATION,
    X86_NXVM_BOARD_TIMING_SEAM_OBSERVATION,
    X86_NXVM_BOARD_TIMING_SEAM_OBSERVATION,
    X86_NXVM_BOARD_TIMING_SEAM_OBSERVATION,
    X86_NXVM_BOARD_TIMING_SEAM_OBSERVATION,
    X86_NXVM_BOARD_TIMING_SEAM_OBSERVATION,
    X86_NXVM_BOARD_TIMING_SEAM_OBSERVATION
};

static lib_i32 plan_capability_is_non_guest_time(
    x86_nxvm_board_timing_capability capability)
{
    return capability == X86_NXVM_BOARD_TIMING_CAPABILITY_DISPLAY_PRESENT ||
        capability == X86_NXVM_BOARD_TIMING_CAPABILITY_INPUT_HOST ||
        capability == X86_NXVM_BOARD_TIMING_CAPABILITY_TRACE_DEBUG ||
        capability == X86_NXVM_BOARD_TIMING_CAPABILITY_PLATFORM_MAILBOX ||
        capability == X86_NXVM_BOARD_TIMING_CAPABILITY_PLATFORM_RESOURCE ||
        capability == X86_NXVM_BOARD_TIMING_CAPABILITY_PLATFORM_WAIT ||
        capability == X86_NXVM_BOARD_TIMING_CAPABILITY_SESSION_COMMAND ||
        capability == X86_NXVM_BOARD_TIMING_CAPABILITY_PRODUCT_DEBUG;
}

static lib_i32 plan_handle_publication(void)
{
    const x86_nxvm_board_config configuration = {
        .memory_bytes = (2u * 1024u * 1024u)
    };
    x86_nxvm_board_plan *plan = LIB_NULL;
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_state *board = LIB_NULL;
    lib_i32 failed = 0;

    if (x86_nxvm_board_plan_create(&configuration, &plan) != LIB_STATUS_OK)
        return 1;
    if (x86_nxvm_board_create_from_plan(plan, &machine, &board) != LIB_STATUS_OK) {
        x86_nxvm_board_plan_destroy(plan);
        return 1;
    }
    failed |= board == LIB_NULL || board->core != machine;
    x86_core_machine_destroy(machine);

    machine = (x86_core_machine *)(lib_uptr)1u;
    board = (x86_nxvm_board_state *)(lib_uptr)1u;
    failed |= x86_nxvm_board_create_from_plan(LIB_NULL, &machine, &board) !=
        LIB_STATUS_INVALID_ARGUMENT || machine != LIB_NULL || board != LIB_NULL;
    board = (x86_nxvm_board_state *)(lib_uptr)1u;
    failed |= x86_nxvm_board_create_from_plan(plan, LIB_NULL, &board) !=
        LIB_STATUS_INVALID_ARGUMENT || board != LIB_NULL;
    machine = (x86_core_machine *)(lib_uptr)1u;
    failed |= x86_nxvm_board_create_from_plan(plan, &machine, LIB_NULL) !=
        LIB_STATUS_INVALID_ARGUMENT || machine != LIB_NULL;
    /* Valid plan, but topology application fails after attachment creation. */
    plan->topology.absent_memory_count = 1u;
    plan->topology.absent_memory[0].physical_start = 0x00100000u;
    machine = (x86_core_machine *)(lib_uptr)1u;
    board = (x86_nxvm_board_state *)(lib_uptr)1u;
    failed |= x86_nxvm_board_create_from_plan(plan, &machine, &board) !=
        LIB_STATUS_INVALID_ARGUMENT || machine != LIB_NULL || board != LIB_NULL;
    x86_nxvm_board_plan_destroy(plan);

    failed |= test_plan_board_device_failures(&configuration);
    return failed;
}

static lib_i32 plan_default_and_copy(void)
{
    x86_nxvm_board_config configuration = { .memory_bytes =
        (2u * 1024u * 1024u) };
    x86_nxvm_board_plan *plan = LIB_NULL;
    x86_nxvm_board_timing_declaration temporary;
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_state *board = LIB_NULL;
    x86_nxvm_board_timing_declaration declaration;
    x86_nxvm_board_timing_disposition disposition;
    lib_size index;
    lib_i32 failed = 0;

    failed |= x86_nxvm_board_plan_create(&configuration, &plan) != LIB_STATUS_OK;
    failed |= !failed && plan->declaration_count != X86_NXVM_BOARD_TIMING_CAPABILITY_COUNT;
    for (index = 0u; index < X86_NXVM_BOARD_TIMING_CAPABILITY_COUNT; ++index) {
        const x86_nxvm_board_timing_declaration *candidate = &plan->declarations[index];
        const x86_nxvm_board_timing_disposition expected =
            plan_capability_is_non_guest_time((x86_nxvm_board_timing_capability)index) ?
            X86_NXVM_BOARD_TIMING_DISPOSITION_NON_GUEST_TIME :
            X86_NXVM_BOARD_TIMING_DISPOSITION_L2_FALLBACK;

        failed |= candidate->capability != (x86_nxvm_board_timing_capability)index ||
            candidate->seam != plan_expected_seams[index] ||
            candidate->disposition != expected;
    }
    temporary = plan->declarations[X86_NXVM_BOARD_TIMING_CAPABILITY_CPU_EXEC];
    plan->declarations[X86_NXVM_BOARD_TIMING_CAPABILITY_CPU_EXEC] =
        plan->declarations[X86_NXVM_BOARD_TIMING_CAPABILITY_CPU_EXCEPT];
    plan->declarations[X86_NXVM_BOARD_TIMING_CAPABILITY_CPU_EXCEPT] = temporary;
    failed |= x86_nxvm_board_create_from_plan(plan, &machine, &board) != LIB_STATUS_OK ||
        machine == LIB_NULL;
    plan->declarations[X86_NXVM_BOARD_TIMING_CAPABILITY_CPU_EXEC].disposition =
        X86_NXVM_BOARD_TIMING_DISPOSITION_L3_REQUIRED;
    failed |= !failed && x86_nxvm_board_get_timing_disposition(board,
        X86_NXVM_BOARD_TIMING_CAPABILITY_CPU_EXEC, &disposition) != LIB_STATUS_OK;
    failed |= !failed && disposition != X86_NXVM_BOARD_TIMING_DISPOSITION_L2_FALLBACK;
    failed |= !failed && x86_nxvm_board_get_timing_declaration(board,
        X86_NXVM_BOARD_TIMING_CAPABILITY_TIME_LIFECYCLE, &declaration) !=
        LIB_STATUS_OK;
    failed |= !failed && declaration.seam != X86_NXVM_BOARD_TIMING_SEAM_LIFECYCLE;
    x86_core_machine_destroy(machine);
    x86_nxvm_board_plan_destroy(plan);
    return failed;
}

static lib_i32 plan_rejects_incomplete_or_unavailable(void)
{
    x86_nxvm_board_plan *plan = LIB_NULL;
    x86_core_machine *machine = (x86_core_machine *)(lib_uptr)1u;
    x86_nxvm_board_state *board = LIB_NULL;
    lib_size index;
    lib_i32 failed = 0;

    failed |= x86_nxvm_board_plan_create(LIB_NULL, &plan) != LIB_STATUS_OK;
    --plan->declaration_count;
    failed |= x86_nxvm_board_create_from_plan(plan, &machine, &board) !=
        LIB_STATUS_INVALID_ARGUMENT || machine != LIB_NULL;
    x86_nxvm_board_plan_destroy(plan);
    failed |= x86_nxvm_board_plan_create(LIB_NULL, &plan) != LIB_STATUS_OK;
    plan->declarations[1].capability = plan->declarations[0].capability;
    failed |= x86_nxvm_board_create_from_plan(plan, &machine, &board) !=
        LIB_STATUS_INVALID_ARGUMENT || machine != LIB_NULL;
    x86_nxvm_board_plan_destroy(plan);
    failed |= x86_nxvm_board_plan_create(LIB_NULL, &plan) != LIB_STATUS_OK;
    plan->declarations[X86_NXVM_BOARD_TIMING_CAPABILITY_CPU_EXEC].disposition =
        X86_NXVM_BOARD_TIMING_DISPOSITION_L3_REQUIRED;
    failed |= x86_nxvm_board_create_from_plan(plan, &machine, &board) !=
        LIB_STATUS_INVALID_ARGUMENT || machine != LIB_NULL;
    x86_nxvm_board_plan_destroy(plan);
    failed |= x86_nxvm_board_plan_create(LIB_NULL, &plan) != LIB_STATUS_OK;
    plan->declarations[X86_NXVM_BOARD_TIMING_CAPABILITY_DISPLAY_PRESENT].disposition =
        X86_NXVM_BOARD_TIMING_DISPOSITION_L2_FALLBACK;
    failed |= x86_nxvm_board_create_from_plan(plan, &machine, &board) !=
        LIB_STATUS_INVALID_ARGUMENT || machine != LIB_NULL;
    x86_nxvm_board_plan_destroy(plan);
    failed |= x86_nxvm_board_plan_create(LIB_NULL, &plan) != LIB_STATUS_OK;
    plan->declarations[X86_NXVM_BOARD_TIMING_CAPABILITY_CTRL_PIC].seam =
        X86_NXVM_BOARD_TIMING_SEAM_TRANSACTION;
    failed |= x86_nxvm_board_create_from_plan(plan, &machine, &board) !=
        LIB_STATUS_INVALID_ARGUMENT || machine != LIB_NULL;
    for (index = 0u; index < X86_NXVM_BOARD_TIMING_CAPABILITY_COUNT; ++index) {
        const x86_nxvm_board_timing_capability capability =
            (x86_nxvm_board_timing_capability)index;

        x86_nxvm_board_plan_destroy(plan);
        failed |= x86_nxvm_board_plan_create(LIB_NULL, &plan) != LIB_STATUS_OK;
        machine = (x86_core_machine *)(lib_uptr)1u;
        plan->declarations[index].disposition =
            plan_capability_is_non_guest_time(capability) ?
            X86_NXVM_BOARD_TIMING_DISPOSITION_L2_FALLBACK :
            X86_NXVM_BOARD_TIMING_DISPOSITION_L3_REQUIRED;
        failed |= x86_nxvm_board_create_from_plan(plan, &machine, &board) !=
            LIB_STATUS_INVALID_ARGUMENT || machine != LIB_NULL;
    }
    x86_nxvm_board_plan_destroy(plan);
    return failed;
}

static lib_i32 plan_rejects_topology_before_publication(void)
{
    x86_nxvm_board_config configuration = { .memory_bytes =
        (2u * 1024u * 1024u) };
    x86_nxvm_board_plan *plan = LIB_NULL;
    x86_core_machine *machine = (x86_core_machine *)(lib_uptr)1u;
    x86_nxvm_board_state *board = LIB_NULL;
    lib_i32 failed = 0;

    failed |= x86_nxvm_board_plan_create(&configuration, &plan) != LIB_STATUS_OK;
    plan->topology.fdc_present = LIB_TRUE;
    failed |= x86_nxvm_board_create_from_plan(plan, &machine, &board) !=
        LIB_STATUS_INVALID_ARGUMENT || machine != LIB_NULL;
    x86_nxvm_board_plan_destroy(plan);
    failed |= x86_nxvm_board_plan_create(&configuration, &plan) != LIB_STATUS_OK;
    plan->topology.absent_memory_count = 1u;
    plan->topology.absent_memory[0].physical_start = 0x00100000u;
    failed |= x86_nxvm_board_create_from_plan(plan, &machine, &board) !=
        LIB_STATUS_INVALID_ARGUMENT || machine != LIB_NULL;
    x86_nxvm_board_plan_destroy(plan);
    return failed;
}

static lib_i32 plan_rejects_invalid_transaction_contract_before_publication(void)
{
    x86_nxvm_board_config configuration = { .memory_bytes =
        (2u * 1024u * 1024u) };
    x86_nxvm_board_plan *plan = LIB_NULL;
    x86_core_machine *machine = (x86_core_machine *)(lib_uptr)1u;
    x86_nxvm_board_state *board = LIB_NULL;
    lib_i32 failed = 0;

    configuration.transaction_contract.external_cycle_timing.page_bytes = 3u;
    failed |= x86_nxvm_board_plan_create(&configuration, &plan) != LIB_STATUS_OK;
    failed |= !failed && x86_nxvm_board_plan_validate(plan) != LIB_STATUS_OK;
    failed |= x86_nxvm_board_create_from_plan(plan, &machine, &board) !=
        LIB_STATUS_INVALID_ARGUMENT || machine != LIB_NULL;
    x86_nxvm_board_plan_destroy(plan);
    return failed;
}

static lib_i32 plan_controller_timing_rules_are_copied_and_validated(void)
{
    x86_nxvm_board_config configuration = { .memory_bytes =
        (2u * 1024u * 1024u) };
    const x86_nxvm_board_controller_timing_rules source_rules = {
        X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_L2_FALLBACK,
        X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK,
        X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_SOURCE_DMA_SERVICE_PHASES,
        X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK,
        X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_L2_FALLBACK
    };
    x86_nxvm_board_plan *plan = LIB_NULL;
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_state *board = LIB_NULL;
    x86_nxvm_board_timing_disposition disposition;
    lib_i32 failed = 0;

    configuration.clock_plan.dma = (x86_core_machine_clock_ratio) {3u, 8u, 0u};
    configuration.clock_plan.pit = (x86_core_machine_clock_ratio) {1193182u, 8000000u, 0u};
    failed |= x86_nxvm_board_plan_create(&configuration, &plan) != LIB_STATUS_OK;
    failed |= !failed && x86_nxvm_board_plan_set_controller_timing_rules(plan,
        &source_rules) != LIB_STATUS_OK;
    failed |= !failed && x86_nxvm_board_create_from_plan(plan, &machine, &board) !=
        LIB_STATUS_OK;
    failed |= !failed && x86_nxvm_board_get_timing_disposition(board,
        X86_NXVM_BOARD_TIMING_CAPABILITY_CTRL_PIC, &disposition) != LIB_STATUS_OK;
    failed |= !failed && disposition != X86_NXVM_BOARD_TIMING_DISPOSITION_L2_FALLBACK;
    failed |= !failed && x86_nxvm_board_get_timing_disposition(board,
        X86_NXVM_BOARD_TIMING_CAPABILITY_CTRL_DMA, &disposition) != LIB_STATUS_OK;
    failed |= !failed && disposition != X86_NXVM_BOARD_TIMING_DISPOSITION_L3_REQUIRED;
    failed |= !failed && x86_nxvm_board_get_timing_disposition(board,
        X86_NXVM_BOARD_TIMING_CAPABILITY_CTRL_PIT, &disposition) != LIB_STATUS_OK;
    failed |= !failed && disposition != X86_NXVM_BOARD_TIMING_DISPOSITION_L3_REQUIRED;
    failed |= !failed && board->controller_timing.pit_clock !=
        X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK;
    x86_core_machine_destroy(machine);
    x86_nxvm_board_plan_destroy(plan);
    return failed;
}

static lib_i32 plan_rejects_invalid_controller_timing_rules(void)
{
    x86_nxvm_board_config configuration = { .memory_bytes =
        (2u * 1024u * 1024u) };
    x86_nxvm_board_controller_timing_rules rules = {
        X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_L2_FALLBACK,
        X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK,
        X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_SOURCE_DMA_SERVICE_PHASES,
        X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK,
        X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_L2_FALLBACK
    };
    x86_nxvm_board_plan *plan = LIB_NULL;
    x86_core_machine *machine = (x86_core_machine *)(lib_uptr)1u;
    x86_nxvm_board_state *board = LIB_NULL;
    lib_i32 failed = 0;

    failed |= x86_nxvm_board_plan_create(&configuration, &plan) != LIB_STATUS_OK;
    failed |= !failed && x86_nxvm_board_plan_set_controller_timing_rules(plan,
        &rules) != LIB_STATUS_OK;
    failed |= !failed && (x86_nxvm_board_create_from_plan(plan, &machine, &board) !=
        LIB_STATUS_INVALID_ARGUMENT || machine != LIB_NULL);
    x86_nxvm_board_plan_destroy(plan);
    configuration.clock_plan.dma = (x86_core_machine_clock_ratio) {3u, 8u, 0u};
    configuration.clock_plan.pit = (x86_core_machine_clock_ratio) {1u, 4u, 0u};
    failed |= x86_nxvm_board_plan_create(&configuration, &plan) != LIB_STATUS_OK;
    rules.pic_visibility = X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK;
    failed |= !failed && x86_nxvm_board_plan_set_controller_timing_rules(plan,
        &rules) != LIB_STATUS_OK;
    machine = (x86_core_machine *)(lib_uptr)1u;
    failed |= !failed && (x86_nxvm_board_create_from_plan(plan, &machine, &board) !=
        LIB_STATUS_INVALID_ARGUMENT || machine != LIB_NULL);
    x86_nxvm_board_plan_destroy(plan);
    rules.pic_visibility = X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_L2_FALLBACK;
    rules.dma_clock = X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_L2_FALLBACK;
    failed |= x86_nxvm_board_plan_create(&configuration, &plan) != LIB_STATUS_OK;
    failed |= !failed && x86_nxvm_board_plan_set_controller_timing_rules(plan,
        &rules) != LIB_STATUS_OK;
    machine = (x86_core_machine *)(lib_uptr)1u;
    failed |= !failed && (x86_nxvm_board_create_from_plan(plan, &machine, &board) !=
        LIB_STATUS_INVALID_ARGUMENT || machine != LIB_NULL);
    x86_nxvm_board_plan_destroy(plan);
    return failed;
}

static lib_i32 plan_selects_single_controller_xt_board(void)
{
    x86_nxvm_board_config configuration = {
        .memory_bytes = 256u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_8088,
        .pic_topology = X86_NXVM_BOARD_PIC_TOPOLOGY_SINGLE,
        .dma_controller_count = 1u
    };
    x86_nxvm_board_plan_topology topology = {0};
    x86_nxvm_board_plan *plan = LIB_NULL;
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_state *board = LIB_NULL;
    lib_i32 failed = 0;

    topology.dma_present = LIB_TRUE;
    topology.dma = (x86_nxvm_board_dma_wiring) {
        X86_NXVM_BOARD_DMA_FDC_CHANNEL_UNBOUND, 1u, 0u};
    failed |= x86_nxvm_board_plan_create(&configuration, &plan) != LIB_STATUS_OK;
    failed |= !failed && x86_nxvm_board_plan_set_topology(plan, &topology) !=
        LIB_STATUS_OK;
    failed |= !failed && x86_nxvm_board_create_from_plan(plan, &machine, &board) !=
        LIB_STATUS_OK;
    failed |= !failed && test_plan_core_xt_routes(machine, LIB_FALSE);
    failed |= !failed && x86_nxvm_board_get_fdc_dma_request_binding(board,
        &(x86_nxvm_board_dma_request_binding) {0}) != LIB_STATUS_INVALID_STATE;
    failed |= !failed && x86_core_machine_freeze_execution_providers(machine) !=
        LIB_STATUS_OK;
    failed |= !failed && x86_core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= !failed && test_plan_core_xt_routes(machine, LIB_TRUE);
    x86_core_machine_destroy(machine);
    x86_nxvm_board_plan_destroy(plan);
    return failed;
}

static lib_i32 plan_l2_pit_deadline_remains_schedulable(void)
{
    x86_nxvm_board_config configuration = {
        .memory_bytes = (2u * 1024u * 1024u),
        .clock_plan.pit = {1u, 1u, 0u}
    };
    x86_nxvm_board_plan *plan = LIB_NULL;
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_state *board = LIB_NULL;
    lib_i32 failed = 0;

    failed |= x86_nxvm_board_plan_create(&configuration, &plan) != LIB_STATUS_OK;
    failed |= !failed && x86_nxvm_board_create_from_plan(plan, &machine, &board) != LIB_STATUS_OK;
    failed |= !failed && test_plan_core_pit_deadline(machine, board);
    x86_core_machine_destroy(machine);
    x86_nxvm_board_plan_destroy(plan);
    return failed;
}

static lib_i32 plan_source_dma_deadline_is_schedulable(void)
{
    static const x86_nxvm_board_dma_channel_provider provider = {0};
    const x86_nxvm_board_controller_timing_rules rules = {
        X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_L2_FALLBACK,
        X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK,
        X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_SOURCE_DMA_SERVICE_PHASES,
        X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_L2_FALLBACK,
        X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_L2_FALLBACK
    };
    x86_nxvm_board_config configuration = {
        .memory_bytes = (2u * 1024u * 1024u),
        .clock_plan.dma = {3u, 8u, 0u}
    };
    x86_nxvm_board_plan *plan = LIB_NULL;
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_state *board = LIB_NULL;
    x86_nxvm_board_dma_request_binding binding = {0};
    lib_i32 failed = 0;

    failed |= x86_nxvm_board_plan_create(&configuration, &plan) != LIB_STATUS_OK;
    failed |= !failed && x86_nxvm_board_plan_set_controller_timing_rules(plan,
        &rules) != LIB_STATUS_OK;
    failed |= !failed && x86_nxvm_board_create_from_plan(plan, &machine, &board) != LIB_STATUS_OK;
    failed |= !failed && x86_nxvm_board_dma_bind_channel(board->shared_dma, 2u, &provider,
        LIB_NULL, &binding) != LIB_STATUS_OK;
    failed |= !failed && test_plan_core_dma_deadline(machine,
        board->shared_dma, &binding);
    x86_core_machine_destroy(machine);
    x86_nxvm_board_plan_destroy(plan);
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
