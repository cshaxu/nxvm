#include "type.h"

#include "core/machine/machine.h"

C_INT core_machine_retirement_time_contract_is_valid(
    core_machine_retirement_time_contract contract)
{
    return contract == CORE_MACHINE_RETIREMENT_TIME_DETERMINISTIC ||
        contract == CORE_MACHINE_RETIREMENT_TIME_PHYSICAL;
}

C_INT core_machine_timing_capability_is_valid(
    core_machine_timing_capability capability)
{
    return capability >= CORE_MACHINE_TIMING_CAPABILITY_CPU_EXEC &&
        capability <= CORE_MACHINE_TIMING_CAPABILITY_PRODUCT_DEBUG;
}

static C_INT core_machine_timing_disposition_is_valid(
    core_machine_timing_disposition disposition)
{
    return disposition == CORE_MACHINE_TIMING_DISPOSITION_L2_FALLBACK ||
        disposition == CORE_MACHINE_TIMING_DISPOSITION_NON_GUEST_TIME ||
        disposition == CORE_MACHINE_TIMING_DISPOSITION_L3_REQUIRED;
}

static C_INT core_machine_controller_timing_rule_is_valid(
    core_machine_controller_timing_rule rule)
{
    return rule == CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK ||
        rule == CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK ||
        rule == CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_DMA_SERVICE_PHASES;
}

static C_INT core_machine_clock_ratio_is_explicit(
    const core_machine_clock_ratio *ratio)
{
    return ratio != STD_NULL && ratio->numerator != 0u &&
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

static C_INT core_machine_controller_timing_rules_are_valid(
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

static C_INT core_machine_timing_capability_is_non_guest_time(
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

type_status core_machine_plan_validate(const core_machine_plan *plan)
{
    type_bool seen[CORE_MACHINE_TIMING_CAPABILITY_COUNT] = {0};
    STD_SIZE_T index;

    if (plan == STD_NULL || plan->declaration_count !=
        CORE_MACHINE_TIMING_CAPABILITY_COUNT ||
        (plan->configuration.keyboard_topology != CORE_MACHINE_KEYBOARD_TOPOLOGY_8042 &&
        plan->configuration.keyboard_topology != CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI) ||
        (plan->configuration.keyboard_topology == CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI &&
        !core_machine_xt_ppi_keyboard_config_is_valid(
            &plan->configuration.xt_ppi_keyboard)) ||
        !core_machine_transaction_contract_is_valid(
            &plan->configuration.transaction_contract) ||
        !core_machine_controller_timing_rules_are_valid(plan)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (plan->topology.absent_memory_count > CORE_MACHINE_ABSENT_MEMORY_WINDOW_COUNT ||
        (plan->topology.planar_parity_present != TYPE_FALSE &&
         plan->topology.planar_parity_present != TYPE_TRUE) ||
        (plan->topology.d4_platform_present != TYPE_FALSE &&
         plan->topology.d4_platform_present != TYPE_TRUE) ||
        (plan->topology.display_present != TYPE_FALSE &&
         plan->topology.display_present != TYPE_TRUE) ||
        (plan->topology.dma_present != TYPE_FALSE &&
         plan->topology.dma_present != TYPE_TRUE) ||
        (plan->topology.rtc_cmos_present != TYPE_FALSE &&
         plan->topology.rtc_cmos_present != TYPE_TRUE) ||
        (plan->topology.fdc_present != TYPE_FALSE &&
         plan->topology.fdc_present != TYPE_TRUE) ||
        (plan->topology.hdc_present != TYPE_FALSE &&
         plan->topology.hdc_present != TYPE_TRUE) ||
        (plan->topology.fdc_present && (!plan->topology.dma_present ||
         plan->topology.dma.fdc_channel == CORE_MACHINE_DMA_FDC_CHANNEL_UNBOUND)) ||
        (plan->topology.hdc_present && !plan->topology.fdc_present &&
         plan->topology.hdc.protocol ==
             CORE_MACHINE_HDC_PROTOCOL_COMPAQ_WD_40MB) ||
        plan->memory_device_count > CORE_MACHINE_PLAN_MEMORY_DEVICE_COUNT ||
        (plan->topology.dma_present && plan->topology.dma.controller_count !=
            (plan->configuration.dma_controller_count == 0u ?
                CORE_MACHINE_DMA_CONTROLLER_COUNT :
                plan->configuration.dma_controller_count)) ||
        (plan->d4_memory_parity_mask != STD_NULL &&
         !plan->topology.d4_platform_present) ||
        ((plan->topology.fdc_present || plan->topology.hdc_present) &&
         plan->media_registry == STD_NULL)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    for (index = 0u; index < plan->declaration_count; ++index) {
        const core_machine_timing_declaration *declaration =
            &plan->declarations[index];

        if (!core_machine_timing_capability_is_valid(declaration->capability) ||
            !core_machine_timing_disposition_is_valid(declaration->disposition) ||
            declaration->seam != core_machine_timing_capability_seam(
                declaration->capability) || seen[declaration->capability]) {
            return TYPE_STATUS_INVALID_ARGUMENT;
        }
        if (core_machine_timing_capability_is_non_guest_time(
                declaration->capability)) {
            if (declaration->disposition !=
                CORE_MACHINE_TIMING_DISPOSITION_NON_GUEST_TIME) {
                return TYPE_STATUS_INVALID_ARGUMENT;
            }
        } else if (declaration->disposition !=
            core_machine_controller_timing_disposition(plan,
                declaration->capability)) {
            return TYPE_STATUS_INVALID_ARGUMENT;
        }
        seen[declaration->capability] = TYPE_TRUE;
    }
    for (index = 0u; index < CORE_MACHINE_TIMING_CAPABILITY_COUNT; ++index) {
        if (!seen[index]) return TYPE_STATUS_INVALID_ARGUMENT;
    }
    return TYPE_STATUS_OK;
}

static C_VOID core_machine_plan_d4_parity_fault(C_VOID *owner,
    type_unsigned_32 physical)
{
    core_machine *machine = (core_machine *)owner;

    if (machine == STD_NULL || machine->d4_plan_parity_mask == STD_NULL) return;
    *machine->d4_plan_parity_mask |= (type_unsigned_8)(1u << (physical & 3u));
    (C_VOID)core_machine_report_d4_iochk_fault(machine);
}

static C_VOID core_machine_plan_d4_memory_write(C_VOID *owner,
    type_unsigned_32 physical, type_native_unsigned bytes)
{
    (C_VOID)physical;
    (C_VOID)bytes;
    (C_VOID)core_machine_clear_d4_iochk_fault((core_machine *)owner);
}

type_status core_machine_plan_apply_topology(core_machine *machine,
    const core_machine_plan *plan)
{
    core_machine_fdc_topology fdc;
    core_machine_hdc_topology hdc;
    core_machine_display_config display;
    const core_machine_plan_topology *topology;
    type_status status;
    STD_SIZE_T index;

    if (machine == STD_NULL || plan == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    topology = &plan->topology;
    for (index = 0u; index < topology->absent_memory_count; ++index) {
        status = core_machine_configure_absent_memory(machine,
            &topology->absent_memory[index]);
        if (status != TYPE_STATUS_OK) return status;
    }
    for (index = 0u; index < plan->memory_device_count; ++index) {
        const core_machine_plan_memory_device *device = &plan->memory_devices[index];

        if ((status = core_machine_register_memory_device(machine,
                device->physical_start, device->bytes, &device->callbacks,
                device->owner)) != TYPE_STATUS_OK) return status;
    }
    if (topology->planar_parity_present && (status = core_machine_configure_planar_parity(
            machine, &topology->planar_parity)) != TYPE_STATUS_OK) return status;
    if (topology->d4_platform_present && (status = core_machine_configure_d4_platform(
            machine, &topology->d4_platform)) != TYPE_STATUS_OK) return status;
    if (plan->d4_memory_parity_mask != STD_NULL) {
        machine->d4_plan_parity_mask = plan->d4_memory_parity_mask;
        status = core_machine_enable_memory_parity(machine, 1024u * 1024u,
            core_machine_plan_d4_parity_fault, machine);
        if (status != TYPE_STATUS_OK) return status;
        status = core_machine_register_memory_write_observer(machine,
            core_machine_plan_d4_memory_write, machine);
        if (status != TYPE_STATUS_OK) return status;
    }
    if (topology->display_present) {
        display = topology->display;
        if ((status = core_machine_configure_display(machine, &display)) !=
                TYPE_STATUS_OK) return status;
        core_machine_display_provider_slot_freeze(plan->display_provider);
    }
    if (topology->dma_present && (status = core_machine_configure_dma(machine,
            &topology->dma, &machine->fdc_dma_request)) != TYPE_STATUS_OK) return status;
    if (topology->rtc_cmos_present && (status = core_machine_configure_rtc_cmos(
            machine, &topology->rtc_cmos)) != TYPE_STATUS_OK) return status;
    if (topology->fdc_present) {
        fdc.media_registry = plan->media_registry;
        fdc.drives = topology->fdc_drives;
        fdc.config = topology->fdc;
        fdc.observation_provider = plan->fdc_observation_provider;
        fdc.dma_request = machine->fdc_dma_request;
        if ((status = core_machine_configure_fdc(machine, &fdc)) != TYPE_STATUS_OK) {
            return status;
        }
    }
    if (topology->hdc_present) {
        hdc.media_registry = plan->media_registry;
        hdc.media_id = topology->hdc_media_id;
        hdc.slave_media_id = topology->hdc_slave_media_id;
        hdc.config = topology->hdc;
        if ((status = core_machine_configure_hdc(machine, &hdc)) != TYPE_STATUS_OK) {
            return status;
        }
    }
    return TYPE_STATUS_OK;
}

const core_machine_timing_declaration *
core_machine_plan_declaration_find(const core_machine_plan *plan,
    core_machine_timing_capability capability)
{
    STD_SIZE_T index;

    if (plan == STD_NULL) return STD_NULL;
    for (index = 0u; index < plan->declaration_count; ++index) {
        if (plan->declarations[index].capability == capability) {
            return &plan->declarations[index];
        }
    }
    return STD_NULL;
}

type_status core_machine_plan_create(const core_machine_config *configuration,
    core_machine_plan **out_plan)
{
    STD_SIZE_T index;
    core_machine_plan *plan;

    if (out_plan == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_plan = STD_NULL;
    plan = (core_machine_plan *)STD_CALLOC(1u, sizeof(*plan));
    if (plan == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    if (configuration != STD_NULL) plan->configuration = *configuration;
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
    return TYPE_STATUS_OK;
}

C_VOID core_machine_plan_destroy(core_machine_plan *plan)
{
    STD_FREE(plan);
}

type_status core_machine_plan_set_topology(core_machine_plan *plan,
    const core_machine_plan_topology *topology)
{
    if (plan == STD_NULL || topology == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    plan->topology = *topology;
    return TYPE_STATUS_OK;
}

type_status core_machine_plan_set_controller_timing_rules(core_machine_plan *plan,
    const core_machine_controller_timing_rules *rules)
{
    if (plan == STD_NULL || rules == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
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
    return TYPE_STATUS_OK;
}

type_status core_machine_plan_bind_media_registry(core_machine_plan *plan,
    const core_machine_media_registry *registry)
{
    if (plan == STD_NULL || registry == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    plan->media_registry = registry;
    return TYPE_STATUS_OK;
}

type_status core_machine_plan_bind_display_provider(core_machine_plan *plan,
    core_machine_display_provider_slot *provider)
{
    if (plan == STD_NULL || provider == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    plan->display_provider = provider;
    return TYPE_STATUS_OK;
}

type_status core_machine_plan_bind_fdc_terminal_observation(core_machine_plan *plan,
    core_machine_fdc_terminal_observation_provider provider)
{
    if (plan == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    plan->fdc_observation_provider = provider;
    return TYPE_STATUS_OK;
}

type_status core_machine_plan_configure_fdc(core_machine_plan *plan,
    const core_machine_fdc_drive_bindings *drives,
    const core_machine_fdc_config *config)
{
    if (plan == STD_NULL || drives == STD_NULL || config == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    plan->topology.fdc_present = TYPE_TRUE;
    plan->topology.fdc_drives = *drives;
    plan->topology.fdc = *config;
    return TYPE_STATUS_OK;
}

type_status core_machine_plan_configure_hdc(core_machine_plan *plan,
    core_machine_media_id media_id, core_machine_media_id slave_media_id,
    const core_machine_hdc_config *config)
{
    if (plan == STD_NULL || config == STD_NULL ||
        media_id == CORE_MACHINE_MEDIA_ID_INVALID || slave_media_id == media_id) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    plan->topology.hdc_present = TYPE_TRUE;
    plan->topology.hdc_media_id = media_id;
    plan->topology.hdc_slave_media_id = slave_media_id;
    plan->topology.hdc = *config;
    return TYPE_STATUS_OK;
}

type_status core_machine_plan_register_memory_device(core_machine_plan *plan,
    type_unsigned_32 physical_start, STD_SIZE_T bytes,
    const core_machine_memory_device_callbacks *callbacks, C_VOID *owner)
{
    core_machine_plan_memory_device *device;

    if (plan == STD_NULL || callbacks == STD_NULL || bytes == 0u ||
        plan->memory_device_count >= CORE_MACHINE_PLAN_MEMORY_DEVICE_COUNT) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    device = &plan->memory_devices[plan->memory_device_count++];
    device->physical_start = physical_start;
    device->bytes = bytes;
    device->callbacks = *callbacks;
    device->owner = owner;
    return TYPE_STATUS_OK;
}

type_status core_machine_plan_enable_d4_memory_parity(core_machine_plan *plan,
    type_unsigned_8 *mask)
{
    if (plan == STD_NULL || mask == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    plan->d4_memory_parity_mask = mask;
    return TYPE_STATUS_OK;
}
C_INT core_machine_external_cycle_timing_is_valid(
    const core_machine_external_cycle_timing *timing)
{
    if (timing == STD_NULL || (timing->overlap_policy !=
        CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_DISABLED && timing->overlap_policy !=
        CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_EXPLICIT_SEQUENTIAL)) return 0;
    if (timing->page_bytes == 0u) {
        return timing->page_miss_ticks == 0u && timing->page_hit_ticks == 0u &&
            timing->overlap_policy == CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_DISABLED;
    }
    return (timing->page_bytes & (timing->page_bytes - 1u)) == 0u &&
        ((timing->first_eligible_address == 0u &&
          timing->last_eligible_address == 0u) ||
         timing->first_eligible_address <= timing->last_eligible_address);
}
C_INT core_machine_external_access_wait_windows_are_valid(
    const core_machine_external_access_wait_window *windows)
{
    STD_SIZE_T index;

    if (windows == STD_NULL) return 0;
    for (index = 0u; index < CORE_MACHINE_EXTERNAL_ACCESS_WAIT_WINDOW_CAPACITY;
            ++index) {
        const core_machine_external_access_wait_window *window = &windows[index];
        if (window->wait_ticks == 0u) continue;
        if ((window->space != CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY &&
                window->space != CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT) ||
            window->first_address > window->last_address ||
            (window->space == CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT &&
                window->last_address > 0xffffu)) return 0;
    }
    return 1;
}

C_INT core_machine_transaction_contract_is_valid(
    const core_machine_transaction_contract *contract)
{
    return contract != STD_NULL &&
        core_machine_external_cycle_timing_is_valid(
            &contract->external_cycle_timing) &&
        core_machine_external_access_wait_windows_are_valid(
            contract->external_access_wait_windows) &&
        (contract->dma_cycle_bus_ready_gate_enabled == TYPE_FALSE ||
         contract->dma_cycle_bus_ready_gate_enabled == TYPE_TRUE) &&
        (contract->cpu_cycle_bus_ready_gate_enabled == TYPE_FALSE ||
         contract->cpu_cycle_bus_ready_gate_enabled == TYPE_TRUE) &&
        (contract->cpu_prefetch_reservation_enabled == TYPE_FALSE ||
         contract->cpu_prefetch_reservation_enabled == TYPE_TRUE);
}

C_INT core_machine_clock_plan_is_valid(
    const core_machine_clock_plan *plan)
{
    return plan != STD_NULL &&
        core_machine_clock_ratio_is_valid(&plan->dma) &&
        core_machine_clock_ratio_is_valid(&plan->pit) &&
        core_machine_clock_ratio_is_valid(&plan->rtc) &&
        core_machine_clock_ratio_is_valid(&plan->vadp) &&
        core_machine_clock_ratio_is_valid(&plan->kbc) &&
        core_machine_clock_ratio_is_valid(&plan->provider);
}

/*
 * PIC/PIT/DMA share one immediate arbitration boundary. The callback owns one
 * machine tick; scheduling the next tick from the callback preserves both
 * deterministic due-time order and the existing one-grant DMA semantics.
 */
