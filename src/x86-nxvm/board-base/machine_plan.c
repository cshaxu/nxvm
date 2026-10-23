#include "lib/types/types_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"

#include "x86-nxvm/board-base/machine_board_state.h"

_Static_assert(X86_NXVM_BOARD_TIMING_CAPABILITY_PRODUCT_DEBUG + 1u ==
    X86_NXVM_BOARD_TIMING_CAPABILITY_COUNT,
    "timing capability count must match the frozen T433 universe");

static lib_i32 x86_nxvm_board_timing_capability_is_valid(
    x86_nxvm_board_timing_capability capability)
{
    return capability >= X86_NXVM_BOARD_TIMING_CAPABILITY_CPU_EXEC &&
        capability <= X86_NXVM_BOARD_TIMING_CAPABILITY_PRODUCT_DEBUG;
}

lib_status x86_nxvm_board_get_timing_disposition(const x86_nxvm_board_state *board,
    x86_nxvm_board_timing_capability capability,
    x86_nxvm_board_timing_disposition *out_disposition)
{
    if (board == LIB_NULL || out_disposition == LIB_NULL ||
        !board->timing_declarations_copied ||
        !x86_nxvm_board_timing_capability_is_valid(capability)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_disposition = board->timing_declarations[capability].disposition;
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_board_validate_timing_declarations(
    const x86_nxvm_board_timing_declaration *declarations, lib_size count)
{
    lib_bool seen[X86_NXVM_BOARD_TIMING_CAPABILITY_COUNT] = {LIB_FALSE};
    lib_size index;

    if (declarations == LIB_NULL || count != X86_NXVM_BOARD_TIMING_CAPABILITY_COUNT)
        return LIB_STATUS_INVALID_ARGUMENT;
    for (index = 0u; index < count; ++index) {
        const x86_nxvm_board_timing_declaration *declaration = &declarations[index];

        if (!x86_nxvm_board_timing_capability_is_valid(declaration->capability) ||
            declaration->disposition < X86_NXVM_BOARD_TIMING_DISPOSITION_L2_FALLBACK ||
            declaration->disposition > X86_NXVM_BOARD_TIMING_DISPOSITION_L3_REQUIRED ||
            declaration->seam < X86_NXVM_BOARD_TIMING_SEAM_CPU_PROGRAM ||
            declaration->seam > X86_NXVM_BOARD_TIMING_SEAM_OBSERVATION ||
            seen[declaration->capability]) return LIB_STATUS_INVALID_ARGUMENT;
        seen[declaration->capability] = LIB_TRUE;
    }
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_board_install_timing_declarations(x86_nxvm_board_state *board,
    const x86_nxvm_board_timing_declaration *declarations, lib_size count)
{
    lib_size index;

    if (board == LIB_NULL || !x86_core_machine_configuration_is_open(board->core) ||
        board->timing_declarations_copied) return LIB_STATUS_INVALID_STATE;
    if (x86_nxvm_board_validate_timing_declarations(declarations, count) !=
            LIB_STATUS_OK) return LIB_STATUS_INVALID_ARGUMENT;
    for (index = 0u; index < count; ++index)
        board->timing_declarations[declarations[index].capability] = declarations[index];
    board->timing_declarations_copied = LIB_TRUE;
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_board_get_timing_declaration(const x86_nxvm_board_state *board,
    x86_nxvm_board_timing_capability capability,
    x86_nxvm_board_timing_declaration *out_declaration)
{
    if (board == LIB_NULL || out_declaration == LIB_NULL ||
        !board->timing_declarations_copied ||
        !x86_nxvm_board_timing_capability_is_valid(capability)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_declaration = board->timing_declarations[capability];
    return LIB_STATUS_OK;
}

static lib_i32 x86_nxvm_board_controller_timing_rule_is_valid(
    x86_nxvm_board_controller_timing_rule rule)
{
    return rule == X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_L2_FALLBACK ||
        rule == X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK ||
        rule == X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_SOURCE_DMA_SERVICE_PHASES;
}

static lib_i32 x86_nxvm_board_clock_ratio_is_explicit(
    const x86_core_machine_clock_ratio *ratio)
{
    return ratio != LIB_NULL && ratio->numerator != 0u &&
        ratio->denominator != 0u;
}

static x86_nxvm_board_timing_disposition x86_nxvm_board_controller_timing_disposition(
    const x86_nxvm_board_plan *plan,
    x86_nxvm_board_timing_capability capability)
{
    if (capability == X86_NXVM_BOARD_TIMING_CAPABILITY_CTRL_PIT &&
        plan->controller_timing.pit_clock ==
            X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK) {
        return X86_NXVM_BOARD_TIMING_DISPOSITION_L3_REQUIRED;
    }
    if (capability == X86_NXVM_BOARD_TIMING_CAPABILITY_CTRL_DMA &&
        plan->controller_timing.dma_clock ==
            X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK &&
        plan->controller_timing.dma_service ==
            X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_SOURCE_DMA_SERVICE_PHASES) {
        return X86_NXVM_BOARD_TIMING_DISPOSITION_L3_REQUIRED;
    }
    if (capability == X86_NXVM_BOARD_TIMING_CAPABILITY_CTRL_RTC_CMOS &&
        plan->controller_timing.rtc_clock ==
            X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK) {
        return X86_NXVM_BOARD_TIMING_DISPOSITION_L3_REQUIRED;
    }
    return X86_NXVM_BOARD_TIMING_DISPOSITION_L2_FALLBACK;
}

static lib_i32 x86_nxvm_board_controller_timing_rules_are_valid(
    const x86_nxvm_board_plan *plan)
{
    const x86_nxvm_board_controller_timing_rules *rules =
        &plan->controller_timing;

    if (!x86_nxvm_board_controller_timing_rule_is_valid(rules->pic_visibility) ||
        !x86_nxvm_board_controller_timing_rule_is_valid(rules->dma_clock) ||
        !x86_nxvm_board_controller_timing_rule_is_valid(rules->dma_service) ||
        !x86_nxvm_board_controller_timing_rule_is_valid(rules->pit_clock) ||
        !x86_nxvm_board_controller_timing_rule_is_valid(rules->rtc_clock) ||
        rules->pic_visibility != X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_L2_FALLBACK ||
        (rules->dma_clock != X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_L2_FALLBACK &&
         rules->dma_clock !=
            X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK) ||
        (rules->dma_service != X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_L2_FALLBACK &&
         rules->dma_service !=
            X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_SOURCE_DMA_SERVICE_PHASES) ||
        (rules->pit_clock != X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_L2_FALLBACK &&
         rules->pit_clock !=
            X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK) ||
        (rules->rtc_clock != X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_L2_FALLBACK &&
         rules->rtc_clock !=
            X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK)) {
        return 0;
    }
    if (rules->pit_clock ==
            X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK &&
        !x86_nxvm_board_clock_ratio_is_explicit(&plan->configuration.clock_plan.pit)) {
        return 0;
    }
    if (rules->dma_clock ==
            X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK &&
        !x86_nxvm_board_clock_ratio_is_explicit(&plan->configuration.clock_plan.dma)) {
        return 0;
    }
    if (rules->rtc_clock ==
            X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK &&
        (!x86_nxvm_board_clock_ratio_is_explicit(&plan->configuration.clock_plan.rtc) ||
         !plan->topology.rtc_cmos_present ||
         plan->topology.rtc_cmos.timing.provenance !=
            X86_NXVM_BOARD_RTC_TIMING_L3_SOURCE)) return 0;
    return rules->dma_service !=
            X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_SOURCE_DMA_SERVICE_PHASES ||
        rules->dma_clock ==
            X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK;
}

static lib_i32 x86_nxvm_board_timing_capability_is_non_guest_time(
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

static x86_nxvm_board_timing_seam x86_nxvm_board_timing_capability_seam(
    x86_nxvm_board_timing_capability capability)
{
    if (capability == X86_NXVM_BOARD_TIMING_CAPABILITY_CPU_RETIRE) {
        return X86_NXVM_BOARD_TIMING_SEAM_RETIREMENT;
    }
    if (capability <= X86_NXVM_BOARD_TIMING_CAPABILITY_CPU_FPU) {
        return X86_NXVM_BOARD_TIMING_SEAM_CPU_PROGRAM;
    }
    if (capability == X86_NXVM_BOARD_TIMING_CAPABILITY_TIME_CLOCK) {
        return X86_NXVM_BOARD_TIMING_SEAM_CLOCK;
    }
    if (capability == X86_NXVM_BOARD_TIMING_CAPABILITY_TIME_LIFECYCLE) {
        return X86_NXVM_BOARD_TIMING_SEAM_LIFECYCLE;
    }
    if (capability <= X86_NXVM_BOARD_TIMING_CAPABILITY_TXN_ARBITRATION) {
        return X86_NXVM_BOARD_TIMING_SEAM_TRANSACTION;
    }
    if (capability <= X86_NXVM_BOARD_TIMING_CAPABILITY_MEM_ROM_FIRMWARE) {
        return X86_NXVM_BOARD_TIMING_SEAM_MEMORY;
    }
    if (capability == X86_NXVM_BOARD_TIMING_CAPABILITY_MACHINE_CONFIG) {
        return X86_NXVM_BOARD_TIMING_SEAM_CONFIGURATION;
    }
    if (capability <= X86_NXVM_BOARD_TIMING_CAPABILITY_DISPLAY_VADP) {
        return X86_NXVM_BOARD_TIMING_SEAM_DEVICE;
    }
    return X86_NXVM_BOARD_TIMING_SEAM_OBSERVATION;
}

lib_status x86_nxvm_board_plan_validate(const x86_nxvm_board_plan *plan)
{
    lib_size index;

    if (plan == LIB_NULL || x86_nxvm_board_validate_timing_declarations(
            plan->declarations, plan->declaration_count) != LIB_STATUS_OK ||
        (plan->configuration.keyboard_topology != X86_NXVM_BOARD_KEYBOARD_TOPOLOGY_8042 &&
        plan->configuration.keyboard_topology != X86_NXVM_BOARD_KEYBOARD_TOPOLOGY_XT_PPI &&
        plan->configuration.keyboard_topology != X86_NXVM_BOARD_KEYBOARD_TOPOLOGY_NONE &&
        plan->configuration.keyboard_topology != X86_NXVM_BOARD_KEYBOARD_TOPOLOGY_8042_NEUTRAL) ||
        (plan->configuration.keyboard_topology == X86_NXVM_BOARD_KEYBOARD_TOPOLOGY_XT_PPI &&
        !x86_nxvm_board_xt_ppi_keyboard_config_is_valid(
            &plan->configuration.xt_ppi_keyboard)) ||
        !x86_nxvm_board_controller_timing_rules_are_valid(plan)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (plan->topology.absent_memory_count > X86_NXVM_BOARD_ABSENT_MEMORY_WINDOW_COUNT ||
        plan->topology.memory_alias_count > X86_NXVM_BOARD_MEMORY_ALIAS_COUNT ||
        (plan->topology.planar_parity_present != LIB_FALSE &&
         plan->topology.planar_parity_present != LIB_TRUE) ||
        (plan->topology.display_present != LIB_FALSE &&
         plan->topology.display_present != LIB_TRUE) ||
        (plan->topology.dma_present != LIB_FALSE &&
         plan->topology.dma_present != LIB_TRUE) ||
        (plan->topology.rtc_cmos_present != LIB_FALSE &&
         plan->topology.rtc_cmos_present != LIB_TRUE) ||
        (plan->topology.fdc_present != LIB_FALSE &&
         plan->topology.fdc_present != LIB_TRUE) ||
        (plan->topology.hdc_present != LIB_FALSE &&
         plan->topology.hdc_present != LIB_TRUE) ||
        (plan->topology.fdc_present && (!plan->topology.dma_present ||
         plan->topology.dma.fdc_channel == X86_NXVM_BOARD_DMA_FDC_CHANNEL_UNBOUND)) ||
        (plan->topology.hdc_present && !plan->topology.fdc_present &&
         plan->topology.hdc.protocol ==
             X86_NXVM_BOARD_HDC_PROTOCOL_COMPAQ_WD_40MB) ||
        (plan->topology.dma_present && plan->topology.dma.controller_count !=
            (plan->configuration.dma_controller_count == 0u ?
                X86_NXVM_BOARD_DMA_CONTROLLER_COUNT :
                plan->configuration.dma_controller_count)) ||
        ((plan->topology.fdc_present || plan->topology.hdc_present) &&
         plan->media_registry == LIB_NULL)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    for (index = 0u; index < plan->declaration_count; ++index) {
        const x86_nxvm_board_timing_declaration *declaration =
            &plan->declarations[index];

        if (declaration->seam != x86_nxvm_board_timing_capability_seam(
                declaration->capability)) {
            return LIB_STATUS_INVALID_ARGUMENT;
        }
        if (x86_nxvm_board_timing_capability_is_non_guest_time(
                declaration->capability)) {
            if (declaration->disposition !=
                X86_NXVM_BOARD_TIMING_DISPOSITION_NON_GUEST_TIME) {
                return LIB_STATUS_INVALID_ARGUMENT;
            }
        } else if (declaration->disposition !=
            x86_nxvm_board_controller_timing_disposition(plan,
                declaration->capability)) {
            return LIB_STATUS_INVALID_ARGUMENT;
        }
    }
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_board_plan_apply_topology(x86_core_machine *machine,
    x86_nxvm_board_state *board,
    const x86_nxvm_board_plan *plan)
{
    x86_nxvm_board_fdc_topology fdc;
    x86_nxvm_board_hdc_topology hdc;
    x86_nxvm_board_display_config display;
    const x86_nxvm_board_plan_topology *topology;
    lib_status status;
    lib_size index;

    if (machine == LIB_NULL || board == LIB_NULL || plan == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    topology = &plan->topology;
    status = x86_core_machine_install_memory_aliases(machine, topology->memory_alias,
        topology->memory_alias_count, LIB_TRUE);
    if (status != LIB_STATUS_OK) return status;
    for (index = 0u; index < topology->absent_memory_count; ++index) {
        status = x86_nxvm_board_configure_absent_memory(board,
            &topology->absent_memory[index]);
        if (status != LIB_STATUS_OK) return status;
    }
    if (topology->planar_parity_present && (status = x86_nxvm_board_configure_planar_parity(
            board, &topology->planar_parity)) != LIB_STATUS_OK) return status;
    if (plan->profile_factory != LIB_NULL && (status = x86_nxvm_board_construct_profile(
            board, plan->profile_factory, plan->profile_factory_context)) != LIB_STATUS_OK)
        return status;
    if (topology->display_present) {
        display = topology->display;
        if ((status = x86_nxvm_board_configure_display(board, &display)) !=
                LIB_STATUS_OK) return status;
        x86_nxvm_board_display_provider_slot_freeze(plan->display_provider);
    }
    if (topology->dma_present && (status = x86_nxvm_board_configure_dma(board,
            &topology->dma, &board->fdc_dma_request)) != LIB_STATUS_OK) return status;
    if (topology->rtc_cmos_present && (status = x86_nxvm_board_configure_rtc_cmos(
            board, &topology->rtc_cmos)) != LIB_STATUS_OK) return status;
    if (topology->fdc_present) {
        fdc.media_registry = plan->media_registry;
        fdc.drives = topology->fdc_drives;
        fdc.config = topology->fdc;
        fdc.observation_provider = plan->fdc_observation_provider;
        fdc.dma_request = board->fdc_dma_request;
        if ((status = x86_nxvm_board_configure_fdc(board, &fdc)) != LIB_STATUS_OK) {
            return status;
        }
    }
    if (topology->hdc_present) {
        hdc.media_registry = plan->media_registry;
        hdc.media_id = topology->hdc_media_id;
        hdc.slave_media_id = topology->hdc_slave_media_id;
        hdc.config = topology->hdc;
        if ((status = x86_nxvm_board_configure_hdc(board, &hdc)) != LIB_STATUS_OK) {
            return status;
        }
    }
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_board_create_from_plan(const x86_nxvm_board_plan *plan,
    x86_core_machine **out_machine, x86_nxvm_board_state **out_board)
{
    x86_core_machine *machine;
    x86_nxvm_board_state *board;
    lib_status status;

    if (out_machine != LIB_NULL) *out_machine = LIB_NULL;
    if (out_board != LIB_NULL) *out_board = LIB_NULL;
    if (out_machine == LIB_NULL || out_board == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (x86_nxvm_board_plan_validate(plan) != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    status = x86_nxvm_board_create(&plan->configuration, &machine, &board);
    if (status != LIB_STATUS_OK) return status;
    status = x86_nxvm_board_plan_apply_topology(machine, board, plan);
    if (status == LIB_STATUS_OK)
        status = x86_nxvm_board_install_timing_declarations(board,
            plan->declarations, plan->declaration_count);
    if (status != LIB_STATUS_OK) {
        x86_core_machine_destroy(machine);
        return status;
    }
    board->controller_timing = plan->controller_timing;
    *out_machine = machine;
    *out_board = board;
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_board_plan_create(const x86_nxvm_board_config *configuration,
    x86_nxvm_board_plan **out_plan)
{
    lib_size index;
    x86_nxvm_board_plan *plan;

    if (out_plan == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_plan = LIB_NULL;
    plan = (x86_nxvm_board_plan *)lib_allocate_zero(1u, sizeof(*plan));
    if (plan == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    if (configuration != LIB_NULL) plan->configuration = *configuration;
    plan->declaration_count = X86_NXVM_BOARD_TIMING_CAPABILITY_COUNT;
    for (index = 0u; index < plan->declaration_count; ++index) {
        x86_nxvm_board_timing_capability capability =
            (x86_nxvm_board_timing_capability)index;

        plan->declarations[index].capability = capability;
        plan->declarations[index].seam =
            x86_nxvm_board_timing_capability_seam(capability);
        plan->declarations[index].disposition =
            x86_nxvm_board_timing_capability_is_non_guest_time(capability) ?
            X86_NXVM_BOARD_TIMING_DISPOSITION_NON_GUEST_TIME :
            X86_NXVM_BOARD_TIMING_DISPOSITION_L2_FALLBACK;
    }
    *out_plan = plan;
    return LIB_STATUS_OK;
}

void x86_nxvm_board_plan_destroy(x86_nxvm_board_plan *plan)
{
    lib_release(plan);
}

lib_status x86_nxvm_board_plan_set_topology(x86_nxvm_board_plan *plan,
    const x86_nxvm_board_plan_topology *topology)
{
    if (plan == LIB_NULL || topology == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    plan->topology = *topology;
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_board_plan_set_controller_timing_rules(x86_nxvm_board_plan *plan,
    const x86_nxvm_board_controller_timing_rules *rules)
{
    if (plan == LIB_NULL || rules == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    plan->controller_timing = *rules;
    plan->declarations[X86_NXVM_BOARD_TIMING_CAPABILITY_CTRL_PIC].disposition =
        x86_nxvm_board_controller_timing_disposition(plan,
            X86_NXVM_BOARD_TIMING_CAPABILITY_CTRL_PIC);
    plan->declarations[X86_NXVM_BOARD_TIMING_CAPABILITY_CTRL_DMA].disposition =
        x86_nxvm_board_controller_timing_disposition(plan,
            X86_NXVM_BOARD_TIMING_CAPABILITY_CTRL_DMA);
    plan->declarations[X86_NXVM_BOARD_TIMING_CAPABILITY_CTRL_PIT].disposition =
        x86_nxvm_board_controller_timing_disposition(plan,
            X86_NXVM_BOARD_TIMING_CAPABILITY_CTRL_PIT);
    plan->declarations[X86_NXVM_BOARD_TIMING_CAPABILITY_CTRL_RTC_CMOS].disposition =
        x86_nxvm_board_controller_timing_disposition(plan,
            X86_NXVM_BOARD_TIMING_CAPABILITY_CTRL_RTC_CMOS);
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_board_plan_bind_media_registry(x86_nxvm_board_plan *plan,
    const x86_nxvm_board_media_registry *registry)
{
    if (plan == LIB_NULL || registry == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    plan->media_registry = registry;
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_board_plan_bind_display_provider(x86_nxvm_board_plan *plan,
    x86_nxvm_board_display_provider_slot *provider)
{
    if (plan == LIB_NULL || provider == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    plan->display_provider = provider;
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_board_plan_bind_fdc_terminal_observation(x86_nxvm_board_plan *plan,
    x86_nxvm_board_fdc_terminal_observation_provider provider)
{
    if (plan == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    plan->fdc_observation_provider = provider;
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_board_plan_configure_fdc(x86_nxvm_board_plan *plan,
    const x86_nxvm_board_fdc_drive_bindings *drives,
    const x86_nxvm_board_fdc_config *config)
{
    if (plan == LIB_NULL || drives == LIB_NULL || config == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    plan->topology.fdc_present = LIB_TRUE;
    plan->topology.fdc_drives = *drives;
    plan->topology.fdc = *config;
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_board_plan_configure_hdc(x86_nxvm_board_plan *plan,
    x86_nxvm_board_media_id media_id, x86_nxvm_board_media_id slave_media_id,
    const x86_nxvm_board_hdc_config *config)
{
    if (plan == LIB_NULL || config == LIB_NULL ||
        media_id == X86_NXVM_BOARD_MEDIA_ID_INVALID || slave_media_id == media_id) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    plan->topology.hdc_present = LIB_TRUE;
    plan->topology.hdc_media_id = media_id;
    plan->topology.hdc_slave_media_id = slave_media_id;
    plan->topology.hdc = *config;
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_board_plan_bind_profile_factory(x86_nxvm_board_plan *plan,
    x86_nxvm_board_profile_factory factory, void *context)
{
    if (plan == LIB_NULL || factory == LIB_NULL || plan->profile_factory != LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    plan->profile_factory = factory;
    plan->profile_factory_context = context;
    return LIB_STATUS_OK;
}

lib_i32 x86_nxvm_board_clock_plan_is_valid(
    const x86_nxvm_board_clock_plan *plan)
{
    return plan != LIB_NULL &&
        x86_core_machine_clock_ratio_is_valid(&plan->dma) &&
        x86_core_machine_clock_ratio_is_valid(&plan->pit) &&
        x86_core_machine_clock_ratio_is_valid(&plan->auxiliary_pit) &&
        x86_core_machine_clock_ratio_is_valid(&plan->rtc) &&
        x86_core_machine_clock_ratio_is_valid(&plan->vadp) &&
        x86_core_machine_clock_ratio_is_valid(&plan->kbc) &&
        x86_core_machine_clock_ratio_is_valid(&plan->provider);
}
