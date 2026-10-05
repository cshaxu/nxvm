#include "lib/types/types_interface.h"
#include "ibmpc/board-common/machine_board_interface.h"

#include "ibmpc/board-common/machine_board_state.h"

static lib_i32 core_machine_controller_timing_rule_is_valid(
    core_machine_controller_timing_rule rule)
{
    return rule == CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK ||
        rule == CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK ||
        rule == CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_DMA_SERVICE_PHASES;
}

static lib_i32 core_machine_clock_ratio_is_explicit(
    const core_machine_clock_ratio *ratio)
{
    return ratio != LIB_NULL && ratio->numerator != 0u &&
        ratio->denominator != 0u;
}

static core_machine_timing_disposition core_machine_controller_timing_disposition(
    const core_machine_plan *plan,
    core_machine_timing_capability capability)
{
    if (capability == CORE_MACHINE_TIMING_CAPABILITY_CTRL_PIT &&
        plan->controller_timing.pit_clock ==
            CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK) {
        return CORE_MACHINE_TIMING_DISPOSITION_L3_REQUIRED;
    }
    if (capability == CORE_MACHINE_TIMING_CAPABILITY_CTRL_DMA &&
        plan->controller_timing.dma_clock ==
            CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK &&
        plan->controller_timing.dma_service ==
            CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_DMA_SERVICE_PHASES) {
        return CORE_MACHINE_TIMING_DISPOSITION_L3_REQUIRED;
    }
    if (capability == CORE_MACHINE_TIMING_CAPABILITY_CTRL_RTC_CMOS &&
        plan->controller_timing.rtc_clock ==
            CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK) {
        return CORE_MACHINE_TIMING_DISPOSITION_L3_REQUIRED;
    }
    return CORE_MACHINE_TIMING_DISPOSITION_L2_FALLBACK;
}

static lib_i32 core_machine_controller_timing_rules_are_valid(
    const core_machine_plan *plan)
{
    const core_machine_controller_timing_rules *rules =
        &plan->controller_timing;

    if (!core_machine_controller_timing_rule_is_valid(rules->pic_visibility) ||
        !core_machine_controller_timing_rule_is_valid(rules->dma_clock) ||
        !core_machine_controller_timing_rule_is_valid(rules->dma_service) ||
        !core_machine_controller_timing_rule_is_valid(rules->pit_clock) ||
        !core_machine_controller_timing_rule_is_valid(rules->rtc_clock) ||
        rules->pic_visibility != CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK ||
        (rules->dma_clock != CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK &&
         rules->dma_clock !=
            CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK) ||
        (rules->dma_service != CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK &&
         rules->dma_service !=
            CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_DMA_SERVICE_PHASES) ||
        (rules->pit_clock != CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK &&
         rules->pit_clock !=
            CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK) ||
        (rules->rtc_clock != CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK &&
         rules->rtc_clock !=
            CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK)) {
        return 0;
    }
    if (rules->pit_clock ==
            CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK &&
        !core_machine_clock_ratio_is_explicit(&plan->configuration.clock_plan.pit)) {
        return 0;
    }
    if (rules->dma_clock ==
            CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK &&
        !core_machine_clock_ratio_is_explicit(&plan->configuration.clock_plan.dma)) {
        return 0;
    }
    if (rules->rtc_clock ==
            CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK &&
        (!core_machine_clock_ratio_is_explicit(&plan->configuration.clock_plan.rtc) ||
         !plan->topology.rtc_cmos_present ||
         plan->topology.rtc_cmos.timing.provenance !=
            CORE_MACHINE_RTC_TIMING_L3_SOURCE)) return 0;
    return rules->dma_service !=
            CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_DMA_SERVICE_PHASES ||
        rules->dma_clock ==
            CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK;
}

static lib_i32 core_machine_timing_capability_is_non_guest_time(
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

static core_machine_timing_seam core_machine_timing_capability_seam(
    core_machine_timing_capability capability)
{
    if (capability == CORE_MACHINE_TIMING_CAPABILITY_CPU_RETIRE) {
        return CORE_MACHINE_TIMING_SEAM_RETIREMENT;
    }
    if (capability <= CORE_MACHINE_TIMING_CAPABILITY_CPU_FPU) {
        return CORE_MACHINE_TIMING_SEAM_CPU_PROGRAM;
    }
    if (capability == CORE_MACHINE_TIMING_CAPABILITY_TIME_CLOCK) {
        return CORE_MACHINE_TIMING_SEAM_CLOCK;
    }
    if (capability == CORE_MACHINE_TIMING_CAPABILITY_TIME_LIFECYCLE) {
        return CORE_MACHINE_TIMING_SEAM_LIFECYCLE;
    }
    if (capability <= CORE_MACHINE_TIMING_CAPABILITY_TXN_ARBITRATION) {
        return CORE_MACHINE_TIMING_SEAM_TRANSACTION;
    }
    if (capability <= CORE_MACHINE_TIMING_CAPABILITY_MEM_ROM_FIRMWARE) {
        return CORE_MACHINE_TIMING_SEAM_MEMORY;
    }
    if (capability == CORE_MACHINE_TIMING_CAPABILITY_MACHINE_CONFIG) {
        return CORE_MACHINE_TIMING_SEAM_CONFIGURATION;
    }
    if (capability <= CORE_MACHINE_TIMING_CAPABILITY_DISPLAY_VADP) {
        return CORE_MACHINE_TIMING_SEAM_DEVICE;
    }
    return CORE_MACHINE_TIMING_SEAM_OBSERVATION;
}

lib_status core_machine_plan_validate(const core_machine_plan *plan)
{
    lib_size index;

    if (plan == LIB_NULL || core_machine_validate_timing_declarations(
            plan->declarations, plan->declaration_count) != LIB_STATUS_OK ||
        (plan->configuration.keyboard_topology != CORE_MACHINE_KEYBOARD_TOPOLOGY_8042 &&
        plan->configuration.keyboard_topology != CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI) ||
        (plan->configuration.keyboard_topology == CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI &&
        !core_machine_xt_ppi_keyboard_config_is_valid(
            &plan->configuration.xt_ppi_keyboard)) ||
        !core_machine_controller_timing_rules_are_valid(plan)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (plan->topology.absent_memory_count > CORE_MACHINE_ABSENT_MEMORY_WINDOW_COUNT ||
        plan->topology.memory_alias_count > CORE_MACHINE_MEMORY_ALIAS_COUNT ||
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
         plan->topology.dma.fdc_channel == CORE_MACHINE_DMA_FDC_CHANNEL_UNBOUND)) ||
        (plan->topology.hdc_present && !plan->topology.fdc_present &&
         plan->topology.hdc.protocol ==
             CORE_MACHINE_HDC_PROTOCOL_COMPAQ_WD_40MB) ||
        (plan->topology.dma_present && plan->topology.dma.controller_count !=
            (plan->configuration.dma_controller_count == 0u ?
                CORE_MACHINE_DMA_CONTROLLER_COUNT :
                plan->configuration.dma_controller_count)) ||
        ((plan->topology.fdc_present || plan->topology.hdc_present) &&
         plan->media_registry == LIB_NULL)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    for (index = 0u; index < plan->declaration_count; ++index) {
        const core_machine_timing_declaration *declaration =
            &plan->declarations[index];

        if (declaration->seam != core_machine_timing_capability_seam(
                declaration->capability)) {
            return LIB_STATUS_INVALID_ARGUMENT;
        }
        if (core_machine_timing_capability_is_non_guest_time(
                declaration->capability)) {
            if (declaration->disposition !=
                CORE_MACHINE_TIMING_DISPOSITION_NON_GUEST_TIME) {
                return LIB_STATUS_INVALID_ARGUMENT;
            }
        } else if (declaration->disposition !=
            core_machine_controller_timing_disposition(plan,
                declaration->capability)) {
            return LIB_STATUS_INVALID_ARGUMENT;
        }
    }
    return LIB_STATUS_OK;
}

lib_status core_machine_plan_apply_topology(core_machine *machine,
    core_machine_board_state *board,
    const core_machine_plan *plan)
{
    core_machine_fdc_topology fdc;
    core_machine_hdc_topology hdc;
    core_machine_display_config display;
    const core_machine_plan_topology *topology;
    lib_status status;
    lib_size index;

    if (machine == LIB_NULL || board == LIB_NULL || plan == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    topology = &plan->topology;
    status = core_machine_install_memory_aliases(machine, topology->memory_alias,
        topology->memory_alias_count, LIB_TRUE);
    if (status != LIB_STATUS_OK) return status;
    for (index = 0u; index < topology->absent_memory_count; ++index) {
        status = core_machine_configure_absent_memory(board,
            &topology->absent_memory[index]);
        if (status != LIB_STATUS_OK) return status;
    }
    if (topology->planar_parity_present && (status = core_machine_configure_planar_parity(
            board, &topology->planar_parity)) != LIB_STATUS_OK) return status;
    if (plan->profile_factory != LIB_NULL && (status = core_machine_board_construct_profile(
            board, plan->profile_factory, plan->profile_factory_context)) != LIB_STATUS_OK)
        return status;
    if (topology->display_present) {
        display = topology->display;
        if ((status = core_machine_configure_display(board, &display)) !=
                LIB_STATUS_OK) return status;
        core_machine_display_provider_slot_freeze(plan->display_provider);
    }
    if (topology->dma_present && (status = core_machine_configure_dma(board,
            &topology->dma, &board->fdc_dma_request)) != LIB_STATUS_OK) return status;
    if (topology->rtc_cmos_present && (status = core_machine_configure_rtc_cmos(
            board, &topology->rtc_cmos)) != LIB_STATUS_OK) return status;
    if (topology->fdc_present) {
        fdc.media_registry = plan->media_registry;
        fdc.drives = topology->fdc_drives;
        fdc.config = topology->fdc;
        fdc.observation_provider = plan->fdc_observation_provider;
        fdc.dma_request = board->fdc_dma_request;
        if ((status = core_machine_configure_fdc(board, &fdc)) != LIB_STATUS_OK) {
            return status;
        }
    }
    if (topology->hdc_present) {
        hdc.media_registry = plan->media_registry;
        hdc.media_id = topology->hdc_media_id;
        hdc.slave_media_id = topology->hdc_slave_media_id;
        hdc.config = topology->hdc;
        if ((status = core_machine_configure_hdc(board, &hdc)) != LIB_STATUS_OK) {
            return status;
        }
    }
    return LIB_STATUS_OK;
}

lib_status core_machine_create_from_plan(const core_machine_plan *plan,
    core_machine **out_machine, core_machine_board_state **out_board)
{
    core_machine *machine;
    core_machine_board_state *board;
    lib_status status;

    if (out_machine != LIB_NULL) *out_machine = LIB_NULL;
    if (out_board != LIB_NULL) *out_board = LIB_NULL;
    if (out_machine == LIB_NULL || out_board == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (core_machine_plan_validate(plan) != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    status = core_machine_create(&plan->configuration, &machine, &board);
    if (status != LIB_STATUS_OK) return status;
    status = core_machine_plan_apply_topology(machine, board, plan);
    if (status == LIB_STATUS_OK)
        status = core_machine_install_timing_declarations(machine,
            plan->declarations, plan->declaration_count);
    if (status != LIB_STATUS_OK) {
        core_machine_destroy(machine);
        return status;
    }
    board->controller_timing = plan->controller_timing;
    *out_machine = machine;
    *out_board = board;
    return LIB_STATUS_OK;
}

lib_status core_machine_plan_create(const core_machine_config *configuration,
    core_machine_plan **out_plan)
{
    lib_size index;
    core_machine_plan *plan;

    if (out_plan == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_plan = LIB_NULL;
    plan = (core_machine_plan *)lib_allocate_zero(1u, sizeof(*plan));
    if (plan == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    if (configuration != LIB_NULL) plan->configuration = *configuration;
    plan->declaration_count = CORE_MACHINE_TIMING_CAPABILITY_COUNT;
    for (index = 0u; index < plan->declaration_count; ++index) {
        core_machine_timing_capability capability =
            (core_machine_timing_capability)index;

        plan->declarations[index].capability = capability;
        plan->declarations[index].seam =
            core_machine_timing_capability_seam(capability);
        plan->declarations[index].disposition =
            core_machine_timing_capability_is_non_guest_time(capability) ?
            CORE_MACHINE_TIMING_DISPOSITION_NON_GUEST_TIME :
            CORE_MACHINE_TIMING_DISPOSITION_L2_FALLBACK;
    }
    *out_plan = plan;
    return LIB_STATUS_OK;
}

void core_machine_plan_destroy(core_machine_plan *plan)
{
    lib_release(plan);
}

lib_status core_machine_plan_set_topology(core_machine_plan *plan,
    const core_machine_plan_topology *topology)
{
    if (plan == LIB_NULL || topology == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    plan->topology = *topology;
    return LIB_STATUS_OK;
}

lib_status core_machine_plan_set_controller_timing_rules(core_machine_plan *plan,
    const core_machine_controller_timing_rules *rules)
{
    if (plan == LIB_NULL || rules == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    plan->controller_timing = *rules;
    plan->declarations[CORE_MACHINE_TIMING_CAPABILITY_CTRL_PIC].disposition =
        core_machine_controller_timing_disposition(plan,
            CORE_MACHINE_TIMING_CAPABILITY_CTRL_PIC);
    plan->declarations[CORE_MACHINE_TIMING_CAPABILITY_CTRL_DMA].disposition =
        core_machine_controller_timing_disposition(plan,
            CORE_MACHINE_TIMING_CAPABILITY_CTRL_DMA);
    plan->declarations[CORE_MACHINE_TIMING_CAPABILITY_CTRL_PIT].disposition =
        core_machine_controller_timing_disposition(plan,
            CORE_MACHINE_TIMING_CAPABILITY_CTRL_PIT);
    plan->declarations[CORE_MACHINE_TIMING_CAPABILITY_CTRL_RTC_CMOS].disposition =
        core_machine_controller_timing_disposition(plan,
            CORE_MACHINE_TIMING_CAPABILITY_CTRL_RTC_CMOS);
    return LIB_STATUS_OK;
}

lib_status core_machine_plan_bind_media_registry(core_machine_plan *plan,
    const core_machine_media_registry *registry)
{
    if (plan == LIB_NULL || registry == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    plan->media_registry = registry;
    return LIB_STATUS_OK;
}

lib_status core_machine_plan_bind_display_provider(core_machine_plan *plan,
    core_machine_display_provider_slot *provider)
{
    if (plan == LIB_NULL || provider == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    plan->display_provider = provider;
    return LIB_STATUS_OK;
}

lib_status core_machine_plan_bind_fdc_terminal_observation(core_machine_plan *plan,
    core_machine_fdc_terminal_observation_provider provider)
{
    if (plan == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    plan->fdc_observation_provider = provider;
    return LIB_STATUS_OK;
}

lib_status core_machine_plan_configure_fdc(core_machine_plan *plan,
    const core_machine_fdc_drive_bindings *drives,
    const core_machine_fdc_config *config)
{
    if (plan == LIB_NULL || drives == LIB_NULL || config == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    plan->topology.fdc_present = LIB_TRUE;
    plan->topology.fdc_drives = *drives;
    plan->topology.fdc = *config;
    return LIB_STATUS_OK;
}

lib_status core_machine_plan_configure_hdc(core_machine_plan *plan,
    core_machine_media_id media_id, core_machine_media_id slave_media_id,
    const core_machine_hdc_config *config)
{
    if (plan == LIB_NULL || config == LIB_NULL ||
        media_id == CORE_MACHINE_MEDIA_ID_INVALID || slave_media_id == media_id) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    plan->topology.hdc_present = LIB_TRUE;
    plan->topology.hdc_media_id = media_id;
    plan->topology.hdc_slave_media_id = slave_media_id;
    plan->topology.hdc = *config;
    return LIB_STATUS_OK;
}

lib_status core_machine_plan_bind_profile_factory(core_machine_plan *plan,
    core_machine_board_profile_factory factory, void *context)
{
    if (plan == LIB_NULL || factory == LIB_NULL || plan->profile_factory != LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    plan->profile_factory = factory;
    plan->profile_factory_context = context;
    return LIB_STATUS_OK;
}

lib_i32 core_machine_clock_plan_is_valid(
    const core_machine_clock_plan *plan)
{
    return plan != LIB_NULL &&
        core_machine_clock_ratio_is_valid(&plan->dma) &&
        core_machine_clock_ratio_is_valid(&plan->pit) &&
        core_machine_clock_ratio_is_valid(&plan->auxiliary_pit) &&
        core_machine_clock_ratio_is_valid(&plan->rtc) &&
        core_machine_clock_ratio_is_valid(&plan->vadp) &&
        core_machine_clock_ratio_is_valid(&plan->kbc) &&
        core_machine_clock_ratio_is_valid(&plan->provider);
}
