#include "type.h"

#include "core/machine/machine.h"

static C_INT core_machine_rtc_cmos_config_is_valid(
    const core_machine_rtc_cmos_config *config)
{
    STD_SIZE_T index;

    if (config == STD_NULL || config->data_port !=
        (type_unsigned_16)(config->index_port + 1u) || config->nmi_mask_bit == 0u ||
        config->ticks_per_second == 0u || config->default_count > CORE_MACHINE_RTC_DEFAULT_COUNT ||
        (config->timing.provenance != CORE_MACHINE_RTC_TIMING_L2_RATIO &&
         config->timing.provenance != CORE_MACHINE_RTC_TIMING_L3_SOURCE) ||
        (config->timing.provenance == CORE_MACHINE_RTC_TIMING_L3_SOURCE &&
         (config->timing.uip_lead_ticks == 0u || config->timing.update_ticks == 0u ||
          (type_unsigned_64)config->timing.uip_lead_ticks +
              config->timing.update_ticks >= config->ticks_per_second))) {
        return TYPE_FALSE;
    }
    for (index = 0u; index < config->default_count; ++index) {
        type_unsigned_8 register_index = config->defaults[index].index;

        if (register_index >= CORE_MACHINE_RTC_REGISTER_COUNT ||
            register_index == CORE_MACHINE_RTC_REG_A ||
            register_index == CORE_MACHINE_RTC_REG_B ||
            register_index == CORE_MACHINE_RTC_REG_C ||
            register_index == CORE_MACHINE_RTC_REG_D) {
            return TYPE_FALSE;
        }
    }
    return TYPE_TRUE;
}

static type_status core_machine_rtc_cmos_port_read(C_VOID *owner,
    type_unsigned_16 port, type_unsigned_32 *out_value)
{
    core_machine *machine = (core_machine *)owner;

    if (machine == STD_NULL || out_value == STD_NULL ||
        port != machine->rtc_cmos_config.data_port) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    *out_value = core_machine_rtc_read_selected(&machine->shared_rtc);
    return TYPE_STATUS_OK;
}

static type_status core_machine_rtc_cmos_port_write(C_VOID *owner,
    type_unsigned_16 port, type_unsigned_32 value)
{
    core_machine *machine = (core_machine *)owner;

    if (machine == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (port == machine->rtc_cmos_config.index_port) {
        (C_VOID)core_machine_set_nmi_mask(machine,
            (value & machine->rtc_cmos_config.nmi_mask_bit) != 0u ?
            TYPE_TRUE : TYPE_FALSE);
        core_machine_rtc_select_register(&machine->shared_rtc, (type_unsigned_8)value);
        return TYPE_STATUS_OK;
    }
    if (port == machine->rtc_cmos_config.data_port) {
        core_machine_rtc_write_selected(&machine->shared_rtc, (type_unsigned_8)value);
        return TYPE_STATUS_OK;
    }
    return TYPE_STATUS_INVALID_ARGUMENT;
}

static C_VOID core_machine_planar_parity_refresh_nmi(core_machine *machine)
{
    if (machine != STD_NULL && machine->planar_parity_configured &&
        machine->planar_parity_latched &&
        (machine->planar_parity_port_b & 0x04u) != 0u &&
        !machine->executor_cpu.data.flagMaskNMI &&
        !machine->planar_parity_nmi_signaled) {
        machine->executor_cpu.data.flagNMI = TYPE_TRUE;
        machine->planar_parity_nmi_signaled = TYPE_TRUE;
    }
}

/* PC/AT-compatible port B exposes the system 8254's refresh and speaker
 * channel outputs independently of the board-specific NMI latches. */
static type_unsigned_8 core_machine_pc_at_port_b_timer_status(
    const core_machine *machine)
{
    type_unsigned_8 value = 0u;

    if (machine == STD_NULL) return 0u;
    if (core_machine_pit_get_output(&machine->shared_pit, 1u)) value |= 0x10u;
    if (core_machine_pit_get_output(&machine->shared_pit, 2u)) value |= 0x20u;
    return value;
}

/* PC/AT-compatible boards wire system PIT counter 1 to DRAM refresh and
 * expose its output at port 61h bit 4. The board programs mode 2 with the
 * fixed refresh divider on every cold reset; channel 0 and channel 2 remain
 * firmware-owned timer and speaker resources. */
static C_VOID core_machine_d4_kbc_output(C_VOID *opaque, type_unsigned_8 value)
{
    core_machine *machine = (core_machine *)opaque;
    if (machine != STD_NULL) machine->d4_slowdown_enabled =
        (value & 0x08u) == 0u ? TYPE_TRUE : TYPE_FALSE;
}

static C_VOID core_machine_d4_refresh_output(C_VOID *opaque, type_bool asserted)
{
    core_machine *machine = (core_machine *)opaque;
    /* Generic-AT policy: the counter-1 refresh pulse ends CPU-side locality.
     * D4 establishes this refresh topology, but not a physical page-retention
     * interval or any calibrated phase duration. */
    if (machine != STD_NULL) {
        if (asserted) {
            machine->d4_refresh_pulse_active = TYPE_FALSE;
        } else if (!machine->d4_refresh_pulse_active) {
            machine->d4_refresh_pulse_active = TYPE_TRUE;
            core_machine_external_cycle_invalidate(machine);
            if (machine->d4_slowdown_enabled) {
                core_machine_pit_set_gate(&machine->auxiliary_pit,
                    machine->d4_platform_config.slowdown_pit_counter, TYPE_FALSE);
                core_machine_pit_set_gate(&machine->auxiliary_pit,
                    machine->d4_platform_config.slowdown_pit_counter, TYPE_TRUE);
            }
            machine->d4_refresh_hold_pending = TYPE_TRUE;
        }
    }
}

static C_VOID core_machine_pc_at_refresh_timer_program(core_machine *machine)
{
    if (machine == STD_NULL) return;
    core_machine_port_write(&machine->executor_port, 0x0043u, 0x74u);
    core_machine_port_write(&machine->executor_port, 0x0041u, 18u);
    core_machine_port_write(&machine->executor_port, 0x0041u, 0u);
}

static type_unsigned_8 core_machine_pc_at_port_b_speaker_value(
    const core_machine *machine)
{
    if (machine == STD_NULL) return 0u;
    if (machine->xt_ppi_speaker_configured) return
        (machine->xt_ppi_speaker_gate ? 0x01u : 0u) |
        (machine->xt_ppi_speaker_data_enabled ? 0x02u : 0u);
    if (machine->d4_platform_configured) return machine->d4_platform_port_b;
    if (machine->planar_parity_configured) return machine->planar_parity_port_b;
    return 0u;
}

static C_VOID core_machine_pc_at_speaker_refresh(core_machine *machine)
{
    type_unsigned_8 value;

    if (machine == STD_NULL) return;
    value = core_machine_pc_at_port_b_speaker_value(machine);
    machine->speaker_output = (value & 0x02u) != 0u &&
        ((value & 0x01u) == 0u ||
        core_machine_pit_get_output(&machine->shared_pit, 2u));
}

static C_VOID core_machine_pc_at_speaker_timer_output(C_VOID *owner,
    type_bool asserted)
{
    core_machine *machine = (core_machine *)owner;

    (C_VOID)asserted;
    core_machine_pc_at_speaker_refresh(machine);
}

static C_VOID core_machine_pc_at_port_b_set_speaker_gate(core_machine *machine,
    type_unsigned_8 value)
{
    if (machine == STD_NULL) return;
    core_machine_pit_set_gate(&machine->shared_pit, 2u,
        (value & 0x01u) != 0u ? TYPE_TRUE : TYPE_FALSE);
    core_machine_pc_at_speaker_refresh(machine);
}

static C_VOID core_machine_planar_parity_memory_fault(C_VOID *owner,
    type_unsigned_32 physical)
{
    (C_VOID)physical;
    (C_VOID)core_machine_report_planar_parity_fault((core_machine *)owner);
}

static type_status core_machine_planar_parity_port_read(C_VOID *owner,
    type_unsigned_16 port, type_unsigned_32 *out_value)
{
    core_machine *machine = (core_machine *)owner;

    if (machine == STD_NULL || out_value == STD_NULL || !machine->planar_parity_configured ||
        port != machine->planar_parity_config.port) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_value = (type_unsigned_32)(machine->planar_parity_port_b & 0x4fu) |
        core_machine_pc_at_port_b_timer_status(machine) |
        (machine->planar_parity_latched ? 0x80u : 0u);
    return TYPE_STATUS_OK;
}

static type_status core_machine_planar_parity_port_write(C_VOID *owner,
    type_unsigned_16 port, type_unsigned_32 value)
{
    core_machine *machine = (core_machine *)owner;

    if (machine == STD_NULL || !machine->planar_parity_configured ||
        port != machine->planar_parity_config.port) return TYPE_STATUS_INVALID_ARGUMENT;
    machine->planar_parity_port_b = (type_unsigned_8)value;
    core_machine_pc_at_port_b_set_speaker_gate(machine, machine->planar_parity_port_b);
    if ((machine->planar_parity_port_b & 0x04u) == 0u) {
        machine->planar_parity_latched = TYPE_FALSE;
        machine->planar_parity_nmi_signaled = TYPE_FALSE;
    } else {
        core_machine_planar_parity_refresh_nmi(machine);
    }
    return TYPE_STATUS_OK;
}

static C_VOID core_machine_d4_platform_refresh_nmi(core_machine *machine)
{
    type_bool pending;

    if (machine == STD_NULL || !machine->d4_platform_configured) return;
    pending = ((machine->d4_platform_port_b & 0x08u) == 0u &&
        machine->d4_platform_iochk_latched) ||
        ((machine->d4_platform_port_b & 0x04u) == 0u &&
        machine->d4_platform_failsafe_latched);
    if (pending && !machine->executor_cpu.data.flagMaskNMI &&
        !machine->d4_platform_nmi_signaled) {
        machine->executor_cpu.data.flagNMI = TYPE_TRUE;
        machine->d4_platform_nmi_signaled = TYPE_TRUE;
    }
}

static C_VOID core_machine_d4_platform_failsafe_output(C_VOID *owner,
    type_bool asserted);

C_VOID core_machine_board_cold_reset(core_machine *machine)
{
    if (machine == STD_NULL) return;
    machine->planar_parity_port_b = machine->planar_parity_configured ? 0x04u : 0u;
    machine->planar_parity_latched = TYPE_FALSE;
    machine->planar_parity_nmi_signaled = TYPE_FALSE;
    machine->speaker_output = TYPE_FALSE;
    machine->xt_ppi_speaker_gate = TYPE_FALSE;
    machine->xt_ppi_speaker_data_enabled = TYPE_FALSE;
    machine->d4_platform_port_b = machine->d4_platform_configured ? 0x0fu : 0u;
    machine->d4_platform_iochk_latched = TYPE_FALSE;
    machine->d4_platform_failsafe_latched = TYPE_FALSE;
    machine->d4_platform_nmi_signaled = TYPE_FALSE;
}

C_VOID core_machine_board_configure_xt_ppi_speaker(core_machine *machine)
{
    if (machine == STD_NULL) return;
    machine->xt_ppi_speaker_configured = TYPE_TRUE;
    core_machine_pit_set_output(&machine->shared_pit, 2u,
        core_machine_pc_at_speaker_timer_output, machine);
    core_machine_board_set_xt_ppi_speaker(machine, TYPE_FALSE, TYPE_FALSE);
}

C_VOID core_machine_board_set_xt_ppi_speaker(core_machine *machine,
    type_bool timer_gate, type_bool data_enabled)
{
    if (machine == STD_NULL || !machine->xt_ppi_speaker_configured) return;
    machine->xt_ppi_speaker_gate = timer_gate;
    machine->xt_ppi_speaker_data_enabled = data_enabled;
    core_machine_pc_at_port_b_set_speaker_gate(machine,
        (timer_gate ? 0x01u : 0u) | (data_enabled ? 0x02u : 0u));
}

C_VOID core_machine_board_after_pit_reset(core_machine *machine)
{
    if (machine == STD_NULL) return;
    if (machine->planar_parity_configured || machine->d4_platform_configured) {
        core_machine_pc_at_refresh_timer_program(machine);
    }
    if (machine->planar_parity_configured) {
        core_machine_pc_at_port_b_set_speaker_gate(machine,
            machine->planar_parity_port_b);
    }
    if (machine->d4_platform_configured) {
        core_machine_pc_at_port_b_set_speaker_gate(machine,
            machine->d4_platform_port_b);
        core_machine_pit_set_output(&machine->shared_pit, 1u,
            core_machine_d4_refresh_output, machine);
        core_machine_pit_set_output(&machine->auxiliary_pit,
            machine->d4_platform_config.failsafe_pit_counter,
            core_machine_d4_platform_failsafe_output, machine);
    }
}

C_VOID core_machine_board_refresh_nmi(core_machine *machine)
{
    if (machine != STD_NULL && machine->keyboard_topology ==
            CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI) {
        core_machine_xt_ppi_keyboard_refresh_nmi(&machine->xt_ppi_keyboard);
    }
    core_machine_planar_parity_refresh_nmi(machine);
    core_machine_d4_platform_refresh_nmi(machine);
}

static C_VOID core_machine_d4_platform_failsafe_output(C_VOID *owner,
    type_bool asserted)
{
    core_machine *machine = (core_machine *)owner;

    if (machine == STD_NULL || !machine->d4_platform_configured || !asserted) return;
    machine->d4_platform_failsafe_latched = TYPE_TRUE;
    core_machine_d4_platform_refresh_nmi(machine);
}

static type_status core_machine_d4_platform_port_read(C_VOID *owner,
    type_unsigned_16 port, type_unsigned_32 *out_value)
{
    core_machine *machine = (core_machine *)owner;

    if (machine == STD_NULL || out_value == STD_NULL ||
        !machine->d4_platform_configured ||
        port != machine->d4_platform_config.port) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_value = (type_unsigned_32)(machine->d4_platform_port_b & 0x0fu) |
        core_machine_pc_at_port_b_timer_status(machine) |
        (machine->d4_platform_iochk_latched ? 0x40u : 0u) |
        (machine->d4_platform_failsafe_latched ? 0x80u : 0u);
    return TYPE_STATUS_OK;
}

static type_status core_machine_d4_platform_port_write(C_VOID *owner,
    type_unsigned_16 port, type_unsigned_32 value)
{
    core_machine *machine = (core_machine *)owner;

    if (machine == STD_NULL || !machine->d4_platform_configured ||
        port != machine->d4_platform_config.port) return TYPE_STATUS_INVALID_ARGUMENT;
    machine->d4_platform_port_b = (type_unsigned_8)value & 0x3fu;
    core_machine_pc_at_port_b_set_speaker_gate(machine, machine->d4_platform_port_b);
    /* DeskPro port 61h bits 3 and 2 disable IOCHK and RAM/fail-safe NMI.
     * A high pulse clears the corresponding latched status; this records the
     * bounded logical effect, not electrical pulse timing. */
    if ((machine->d4_platform_port_b & 0x08u) != 0u) {
        machine->d4_platform_iochk_latched = TYPE_FALSE;
    }
    if ((machine->d4_platform_port_b & 0x04u) != 0u) {
        machine->d4_platform_failsafe_latched = TYPE_FALSE;
    }
    if (!machine->d4_platform_iochk_latched &&
        !machine->d4_platform_failsafe_latched) {
        machine->d4_platform_nmi_signaled = TYPE_FALSE;
    }
    core_machine_d4_platform_refresh_nmi(machine);
    return TYPE_STATUS_OK;
}

static const core_machine_port_provider core_machine_d4_platform_port_provider = {
    core_machine_d4_platform_port_read,
    core_machine_d4_platform_port_write
};
static const core_machine_port_provider core_machine_rtc_cmos_port_provider = {
    core_machine_rtc_cmos_port_read,
    core_machine_rtc_cmos_port_write
};

static const core_machine_port_provider core_machine_rtc_cmos_index_port_provider = {
    STD_NULL,
    core_machine_rtc_cmos_port_write
};

static C_VOID core_machine_fdc_dma_request_assert(C_VOID *owner,
    const core_machine_dma_request_binding *binding)
{
    core_machine *machine = owner;

    if (machine == STD_NULL || binding == STD_NULL ||
        binding->core_token != machine->fdc_dma_request.core_token ||
        binding->channel != machine->fdc_dma_request.channel) return;
    core_machine_dma_request_assert(&machine->shared_dma_primary,
        &machine->shared_dma_secondary, binding);
}

static C_VOID core_machine_fdc_dma_request_deassert(C_VOID *owner,
    const core_machine_dma_request_binding *binding)
{
    core_machine *machine = owner;

    if (machine == STD_NULL || binding == STD_NULL ||
        binding->core_token != machine->fdc_dma_request.core_token ||
        binding->channel != machine->fdc_dma_request.channel) return;
    core_machine_dma_request_deassert(&machine->shared_dma_primary,
        &machine->shared_dma_secondary, binding);
}

static C_VOID core_machine_hdc_dma_request_assert(C_VOID *owner,
    const core_machine_dma_request_binding *binding)
{
    core_machine *machine = owner;

    if (machine == STD_NULL || binding == STD_NULL ||
        binding->core_token != machine->hdc_dma_request.core_token ||
        binding->channel != machine->hdc_dma_request.channel) return;
    core_machine_dma_request_assert(&machine->shared_dma_primary,
        &machine->shared_dma_secondary, binding);
}

static C_VOID core_machine_hdc_dma_request_deassert(C_VOID *owner,
    const core_machine_dma_request_binding *binding)
{
    core_machine *machine = owner;

    if (machine == STD_NULL || binding == STD_NULL ||
        binding->core_token != machine->hdc_dma_request.core_token ||
        binding->channel != machine->hdc_dma_request.channel) return;
    core_machine_dma_request_deassert(&machine->shared_dma_primary,
        &machine->shared_dma_secondary, binding);
}

static C_VOID core_machine_dma_refresh_pit_output(C_VOID *owner, type_bool asserted)
{
    core_machine *machine = owner;

    if (machine == STD_NULL) return;
    if (asserted) {
        core_machine_dma_request_deassert(&machine->shared_dma_primary,
            &machine->shared_dma_secondary, &machine->refresh_dma_request);
    } else {
        core_machine_dma_request_assert(&machine->shared_dma_primary,
            &machine->shared_dma_secondary, &machine->refresh_dma_request);
    }
}

static const core_machine_dma_channel_provider core_machine_dma_refresh_provider = {
    STD_NULL, STD_NULL, STD_NULL
};

static type_bool core_machine_dma_wiring_is_valid(
    const core_machine_dma_wiring *wiring)
{
    return wiring != STD_NULL &&
        (wiring->fdc_channel == CORE_MACHINE_DMA_FDC_CHANNEL_UNBOUND ||
         (((wiring->controller_count == 1u && wiring->cascade_channel == 0u &&
            wiring->fdc_channel < 4u) ||
           (wiring->controller_count == CORE_MACHINE_DMA_CONTROLLER_COUNT &&
            wiring->cascade_channel == CORE_MACHINE_DMA_CASCADE_CHANNEL &&
            wiring->fdc_channel < VDMA_CHANNEL_COUNT)) &&
          wiring->fdc_channel != 1u));
}

type_status core_machine_configure_dma(core_machine *machine,
    const core_machine_dma_wiring *wiring,
    core_machine_dma_request_binding *out_fdc_request)
{
    type_status status;

    if (!core_machine_configuration_is_open(machine) || machine->dma_configured) {
        return TYPE_STATUS_INVALID_STATE;
    }
    if (!core_machine_dma_wiring_is_valid(wiring) || out_fdc_request == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    machine->fdc_dma_request = (core_machine_dma_request_binding) {0};
    if (wiring->fdc_channel != CORE_MACHINE_DMA_FDC_CHANNEL_UNBOUND) {
        status = core_machine_dma_bind_channel(&machine->shared_dma_latch,
            &machine->shared_dma_primary, &machine->shared_dma_secondary,
            wiring->fdc_channel, core_machine_fdc_dma_provider(), &machine->fdc,
            &machine->fdc_dma_request);
        if (status != TYPE_STATUS_OK) return status;
    }
    status = core_machine_dma_bind_channel(&machine->shared_dma_latch,
        &machine->shared_dma_primary, &machine->shared_dma_secondary, 1u,
        &core_machine_dma_refresh_provider, machine, &machine->refresh_dma_request);
    if (status != TYPE_STATUS_OK) return status;
    core_machine_pit_set_output(&machine->shared_pit, 1u,
        core_machine_dma_refresh_pit_output, machine);
    machine->dma_wiring = *wiring;
    machine->dma_configured = TYPE_TRUE;
    *out_fdc_request = machine->fdc_dma_request;
    return TYPE_STATUS_OK;
}

type_status core_machine_get_fdc_dma_request_binding(const core_machine *machine,
    core_machine_dma_request_binding *out_binding)
{
    if (machine == STD_NULL || out_binding == STD_NULL || !machine->dma_configured ||
        machine->fdc_dma_request.core_token == 0u) {
        return TYPE_STATUS_INVALID_STATE;
    }
    *out_binding = machine->fdc_dma_request;
    return TYPE_STATUS_OK;
}

type_status core_machine_set_dma_bus_ready(core_machine *machine, C_INT ready)
{
    if (machine == STD_NULL || !core_machine_mutable_operation_is_allowed(machine) ||
        !machine->transaction_contract.dma_cycle_bus_ready_gate_enabled) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    machine->dma_cycle_bus_ready = ready ? TYPE_TRUE : TYPE_FALSE;
    return TYPE_STATUS_OK;
}
type_status core_machine_set_cpu_bus_ready(core_machine *machine, C_INT ready)
{
    if (machine == STD_NULL || !core_machine_mutable_operation_is_allowed(machine) ||
        !machine->transaction_contract.cpu_cycle_bus_ready_gate_enabled) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    machine->cpu_cycle_bus_ready = ready ? TYPE_TRUE : TYPE_FALSE;
    return TYPE_STATUS_OK;
}
type_status core_machine_configure_rtc_cmos(core_machine *machine,
    const core_machine_rtc_cmos_config *config)
{
    core_machine_rtc_config rtc_config;
    core_machine_port_provider_entry *port_checkpoint;
    type_status status;
    STD_SIZE_T index;

    if (!core_machine_configuration_is_open(machine) ||
        machine->rtc_cmos_configured) {
        return TYPE_STATUS_INVALID_STATE;
    }
    if (!core_machine_rtc_cmos_config_is_valid(config)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (core_machine_port_has_write(&machine->executor_port,
            config->index_port) || core_machine_port_has_read(
            &machine->executor_port, config->data_port) ||
        core_machine_port_has_write(&machine->executor_port,
            config->data_port)) {
        return TYPE_STATUS_INVALID_STATE;
    }
    port_checkpoint = core_machine_port_registration_begin(&machine->executor_port);
    status = core_machine_install_port_provider(machine, config->index_port,
        config->index_port, &core_machine_rtc_cmos_index_port_provider, machine);
    if (status != TYPE_STATUS_OK) {
        core_machine_port_rollback_registration(&machine->executor_port,
            port_checkpoint);
        return status;
    }
    status = core_machine_install_port_provider(machine, config->data_port,
        config->data_port, &core_machine_rtc_cmos_port_provider, machine);
    if (status != TYPE_STATUS_OK) {
        core_machine_port_rollback_registration(&machine->executor_port,
            port_checkpoint);
        return status;
    }
    rtc_config.irq = config->irq;
    rtc_config.ticks_per_second = config->ticks_per_second;
    rtc_config.timing = config->timing;
    core_machine_rtc_initialize(&machine->shared_rtc, &machine->shared_pic_master,
        &machine->shared_pic_slave, &rtc_config);
    for (index = 0u; index < config->default_count; ++index) {
        core_machine_rtc_write_nvram(&machine->shared_rtc,
            config->defaults[index].index, config->defaults[index].value);
    }
    machine->rtc_cmos_config = *config;
    machine->rtc_cmos_configured = TYPE_TRUE;
    return TYPE_STATUS_OK;
}

type_status core_machine_enable_memory_parity(core_machine *machine,
    STD_SIZE_T bytes, core_machine_memory_parity_fault_observer fault, C_VOID *owner)
{
    if (!core_machine_configuration_is_open(machine)) return TYPE_STATUS_INVALID_STATE;
    return core_machine_memory_enable_parity(&machine->executor_memory, bytes,
        fault, owner);
}
type_status core_machine_configure_planar_parity(core_machine *machine,
    const core_machine_planar_parity_config *config)
{
    core_machine_port_provider provider = { core_machine_planar_parity_port_read,
        core_machine_planar_parity_port_write };
    core_machine_port_provider_entry *checkpoint;
    type_status status;

    if (!core_machine_configuration_is_open(machine) || machine->planar_parity_configured)
        return TYPE_STATUS_INVALID_STATE;
    if (config == STD_NULL || config->port != CORE_MACHINE_PC_AT_PORT_B ||
        config->memory_bytes == 0u || config->memory_bytes >
            machine->executor_memory.connect.installed_bytes ||
        core_machine_port_has_read(&machine->executor_port,
            config->port) || core_machine_port_has_write(&machine->executor_port,
            config->port)) return TYPE_STATUS_INVALID_ARGUMENT;
    checkpoint = core_machine_port_registration_begin(&machine->executor_port);
    status = core_machine_install_port_provider(machine, config->port, config->port,
        &provider, machine);
    if (status != TYPE_STATUS_OK) {
        core_machine_port_rollback_registration(&machine->executor_port, checkpoint);
        return status;
    }
    machine->planar_parity_config = *config;
    machine->planar_parity_port_b = 0x04u;
    machine->planar_parity_configured = TYPE_TRUE;
    core_machine_pit_set_output(&machine->shared_pit, 2u,
        core_machine_pc_at_speaker_timer_output, machine);
    core_machine_pc_at_refresh_timer_program(machine);
    core_machine_pc_at_port_b_set_speaker_gate(machine, machine->planar_parity_port_b);
    status = core_machine_memory_enable_parity(&machine->executor_memory,
        config->memory_bytes, core_machine_planar_parity_memory_fault, machine);
    if (status != TYPE_STATUS_OK) {
        machine->planar_parity_configured = TYPE_FALSE;
        core_machine_port_rollback_registration(&machine->executor_port, checkpoint);
        return status;
    }
    return TYPE_STATUS_OK;
}

type_status core_machine_configure_d4_platform(core_machine *machine,
    const core_machine_d4_platform_config *config)
{
    core_machine_port_provider_entry *checkpoint;
    type_status status;

    if (!core_machine_configuration_is_open(machine) ||
        machine->d4_platform_configured) return TYPE_STATUS_INVALID_STATE;
    if (config == STD_NULL || config->port != CORE_MACHINE_PC_AT_PORT_B ||
        config->failsafe_pit_counter >= 3u || config->slowdown_pit_counter >= 3u ||
        config->failsafe_pit_counter == config->slowdown_pit_counter || !machine->auxiliary_pit_configured ||
        machine->auxiliary_pit.connect.output[config->failsafe_pit_counter] != STD_NULL ||
        machine->auxiliary_pit.connect.output[config->slowdown_pit_counter] != STD_NULL ||
        machine->shared_kbc.connect.output_port != STD_NULL ||
        core_machine_port_has_read(&machine->executor_port, config->port) ||
        core_machine_port_has_write(&machine->executor_port, config->port)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    checkpoint = core_machine_port_registration_begin(&machine->executor_port);
    status = core_machine_install_port_provider(machine, config->port, config->port,
        &core_machine_d4_platform_port_provider, machine);
    if (status != TYPE_STATUS_OK) {
        core_machine_port_rollback_registration(&machine->executor_port, checkpoint);
        return status;
    }
    machine->d4_platform_config = *config;
    machine->d4_platform_port_b = 0x0fu;
    machine->d4_platform_configured = TYPE_TRUE;
    core_machine_pit_set_output(&machine->shared_pit, 2u,
        core_machine_pc_at_speaker_timer_output, machine);
    core_machine_pc_at_refresh_timer_program(machine);
    core_machine_pit_set_output(&machine->shared_pit, 1u,
        core_machine_d4_refresh_output, machine);
    core_machine_pc_at_port_b_set_speaker_gate(machine, machine->d4_platform_port_b);
    core_machine_pit_set_output(&machine->auxiliary_pit,
        config->failsafe_pit_counter, core_machine_d4_platform_failsafe_output,
        machine);
    if (!core_machine_kbc_bind_output_port(&machine->shared_kbc,
            core_machine_d4_kbc_output, machine)) return TYPE_STATUS_INVALID_ARGUMENT;
    return TYPE_STATUS_OK;
}
type_status core_machine_report_planar_parity_fault(core_machine *machine)
{
    if (machine == STD_NULL || !core_machine_mutable_operation_is_allowed(machine) ||
        !machine->planar_parity_configured) return TYPE_STATUS_INVALID_STATE;
    machine->planar_parity_latched = TYPE_TRUE;
    core_machine_planar_parity_refresh_nmi(machine);
    return TYPE_STATUS_OK;
}

static type_status core_machine_absent_memory_read(C_VOID *owner,
    type_unsigned_32 physical, type_virtual_address destination,
    type_native_unsigned bytes)
{
    const core_machine_absent_memory *absent =
        (const core_machine_absent_memory *)owner;

    (C_VOID)physical;
    if (absent == STD_NULL || !absent->configured || destination == 0u ||
        bytes == 0u) return TYPE_STATUS_FAULT;
    STD_MEMSET((C_VOID *)destination, absent->config.read_value, bytes);
    return TYPE_STATUS_OK;
}

static type_status core_machine_absent_memory_write(C_VOID *owner,
    type_unsigned_32 physical, type_virtual_address source,
    type_native_unsigned bytes)
{
    const core_machine_absent_memory *absent =
        (const core_machine_absent_memory *)owner;

    (C_VOID)physical;
    if (absent == STD_NULL || !absent->configured || source == 0u ||
        bytes == 0u) return TYPE_STATUS_FAULT;
    return TYPE_STATUS_OK;
}

static type_status core_machine_absent_memory_query(C_VOID *owner,
    type_unsigned_32 physical, type_native_unsigned bytes,
    core_machine_memory_access access)
{
    const core_machine_absent_memory *absent =
        (const core_machine_absent_memory *)owner;

    (C_VOID)physical;
    if (absent == STD_NULL || !absent->configured || bytes == 0u ||
        (access != CORE_MACHINE_MEMORY_ACCESS_READ &&
        access != CORE_MACHINE_MEMORY_ACCESS_WRITE)) return TYPE_STATUS_FAULT;
    return TYPE_STATUS_OK;
}

type_status core_machine_clear_d4_iochk_fault(core_machine *machine)
{
    if (machine == STD_NULL || !core_machine_mutable_operation_is_allowed(machine) ||
        !machine->d4_platform_configured) return TYPE_STATUS_INVALID_STATE;
    machine->d4_platform_iochk_latched = TYPE_FALSE;
    if (!machine->d4_platform_failsafe_latched) machine->d4_platform_nmi_signaled = TYPE_FALSE;
    core_machine_d4_platform_refresh_nmi(machine);
    return TYPE_STATUS_OK;
}

type_status core_machine_report_d4_iochk_fault(core_machine *machine)
{
    if (machine == STD_NULL || !core_machine_mutable_operation_is_allowed(machine) ||
        !machine->d4_platform_configured) return TYPE_STATUS_INVALID_STATE;
    machine->d4_platform_iochk_latched = TYPE_TRUE;
    core_machine_d4_platform_refresh_nmi(machine);
    return TYPE_STATUS_OK;
}

type_status core_machine_get_d4_platform_observation(const core_machine *machine,
    core_machine_d4_platform_observation *out_observation)
{
    if (machine == STD_NULL || out_observation == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    out_observation->configured = machine->d4_platform_configured;
    out_observation->iochk_enabled = (machine->d4_platform_port_b & 0x08u) == 0u;
    out_observation->failsafe_enabled =
        (machine->d4_platform_port_b & 0x04u) == 0u;
    out_observation->iochk_latched = machine->d4_platform_iochk_latched;
    out_observation->failsafe_latched = machine->d4_platform_failsafe_latched;
    out_observation->nmi_signaled = machine->d4_platform_nmi_signaled;
    return TYPE_STATUS_OK;
}
type_status core_machine_get_speaker_observation(const core_machine *machine,
    core_machine_speaker_observation *out_observation)
{
    type_unsigned_8 value;

    if (machine == STD_NULL || out_observation == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    value = core_machine_pc_at_port_b_speaker_value(machine);
    out_observation->configured = machine->xt_ppi_speaker_configured ||
        machine->d4_platform_configured || machine->planar_parity_configured;
    out_observation->timer_gate = (value & 0x01u) != 0u;
    out_observation->data_enabled = (value & 0x02u) != 0u;
    out_observation->timer_output = core_machine_pit_get_output(
        &machine->shared_pit, 2u);
    out_observation->output = machine->speaker_output;
    return TYPE_STATUS_OK;
}
type_status core_machine_configure_absent_memory(core_machine *machine,
    const core_machine_absent_memory_config *config)
{
    type_status status;

    if (!core_machine_configuration_is_open(machine) ||
        machine->absent_memory.configured) return TYPE_STATUS_INVALID_STATE;
    if (config == STD_NULL || config->bytes == 0u ||
        (type_unsigned_64)config->physical_start + config->bytes >
            (type_unsigned_64)TYPE_MAX_UNSIGNED_32 + 1u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    machine->absent_memory.config = *config;
    machine->absent_memory.configured = TYPE_TRUE;
    status = core_machine_memory_register_device_provider(&machine->executor_memory,
        config->physical_start, config->bytes, core_machine_absent_memory_read,
        core_machine_absent_memory_write, core_machine_absent_memory_query,
        &machine->absent_memory);
    if (status != TYPE_STATUS_OK) {
        STD_MEMSET(&machine->absent_memory, 0, sizeof(machine->absent_memory));
        return status;
    }
    return TYPE_STATUS_OK;
}

type_status core_machine_get_planar_parity_observation(const core_machine *machine,
    core_machine_planar_parity_observation *out_observation)
{
    if (machine == STD_NULL || out_observation == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    out_observation->configured = machine->planar_parity_configured;
    out_observation->enabled = (machine->planar_parity_port_b & 0x04u) != 0u;
    out_observation->latched = machine->planar_parity_latched;
    out_observation->nmi_signaled = machine->planar_parity_nmi_signaled;
    return TYPE_STATUS_OK;
}

static C_INT core_machine_fdc_topology_is_valid(
    const core_machine_fdc_topology *topology)
{
    STD_SIZE_T first;
    STD_SIZE_T second;

    if (topology == STD_NULL || topology->media_registry == STD_NULL ||
        topology->config.dma_channel != topology->dma_request.channel ||
        topology->config.unready_read_policy >
            CORE_MACHINE_FDC_UNREADY_READ_DESKPRO_REFERENCE) {
        return 0;
    }
    for (first = 0u; first < CORE_MACHINE_FDC_DRIVE_COUNT; ++first) {
        if (topology->drives.media_id[first] == CORE_MACHINE_MEDIA_ID_INVALID) {
            continue;
        }
        for (second = first + 1u; second < CORE_MACHINE_FDC_DRIVE_COUNT; ++second) {
            if (topology->drives.media_id[first] == topology->drives.media_id[second]) {
                return 0;
            }
        }
    }
    return 1;
}

static C_INT core_machine_hdc_topology_is_valid(
    const core_machine_hdc_topology *topology)
{
    const core_machine_hdc_config *config;
    const type_unsigned_16 *ports;
    type_unsigned_16 task_file_ports[9];
    type_unsigned_16 xebec_ports[4];
    STD_SIZE_T port_count;
    STD_SIZE_T first;
    STD_SIZE_T second;

    if (topology == STD_NULL || topology->media_registry == STD_NULL ||
        topology->media_id == CORE_MACHINE_MEDIA_ID_INVALID ||
        topology->slave_media_id == topology->media_id) return 0;
    config = &topology->config;
    if (config->protocol == CORE_MACHINE_HDC_PROTOCOL_XEBEC_XT) {
        xebec_ports[0] = config->bus.xebec.data_port;
        xebec_ports[1] = config->bus.xebec.hardware_status_reset_port;
        xebec_ports[2] = config->bus.xebec.jumpers_select_port;
        xebec_ports[3] = config->bus.xebec.dma_irq_mask_port;
        ports = xebec_ports;
        port_count = sizeof(xebec_ports) / sizeof(xebec_ports[0]);
        if (config->irq != 5u || config->bus.xebec.dma_channel != 3u ||
            config->bus.xebec.drive_type != CORE_MACHINE_XEBEC_DRIVE_TYPE_2 ||
            config->bus.xebec.expected_media_geometry.logical_sector_count !=
                CORE_MACHINE_XEBEC_TYPE_2_LOGICAL_SECTOR_COUNT ||
            config->bus.xebec.expected_media_geometry.bytes_per_sector !=
                CORE_MACHINE_XEBEC_TYPE_2_BYTES_PER_SECTOR ||
            config->bus.xebec.expected_media_geometry.cylinders !=
                CORE_MACHINE_XEBEC_TYPE_2_CYLINDERS ||
            config->bus.xebec.expected_media_geometry.heads !=
                CORE_MACHINE_XEBEC_TYPE_2_HEADS ||
            config->bus.xebec.expected_media_geometry.sectors_per_track !=
                CORE_MACHINE_XEBEC_TYPE_2_SECTORS_PER_TRACK) return 0;
    } else {
        task_file_ports[0] = config->bus.task_file.data_port;
        task_file_ports[1] = config->bus.task_file.error_features_port;
        task_file_ports[2] = config->bus.task_file.sector_count_port;
        task_file_ports[3] = config->bus.task_file.sector_number_port;
        task_file_ports[4] = config->bus.task_file.cylinder_low_port;
        task_file_ports[5] = config->bus.task_file.cylinder_high_port;
        task_file_ports[6] = config->bus.task_file.drive_head_port;
        task_file_ports[7] = config->bus.task_file.status_command_port;
        task_file_ports[8] = config->bus.task_file.alternate_status_device_control_port;
        ports = task_file_ports;
        port_count = sizeof(task_file_ports) / sizeof(task_file_ports[0]);
        if (config->bus.task_file.lba28_supported != TYPE_FALSE &&
            config->bus.task_file.lba28_supported != TYPE_TRUE) return 0;
        if (config->protocol != CORE_MACHINE_HDC_PROTOCOL_ATA_PIO &&
            config->protocol != CORE_MACHINE_HDC_PROTOCOL_COMPAQ_WD_40MB &&
            config->protocol != CORE_MACHINE_HDC_PROTOCOL_IBM_WD1003_ST506) return 0;
        if ((config->protocol == CORE_MACHINE_HDC_PROTOCOL_ATA_PIO &&
                config->bus.task_file.drive_address_port != 0u) ||
            (config->protocol == CORE_MACHINE_HDC_PROTOCOL_COMPAQ_WD_40MB &&
                (config->bus.task_file.lba28_supported ||
                    config->bus.task_file.drive_address_port == 0u))) return 0;
        if (config->protocol == CORE_MACHINE_HDC_PROTOCOL_IBM_WD1003_ST506 &&
            (config->bus.task_file.lba28_supported ||
                config->bus.task_file.drive_address_port != 0u ||
                config->bus.task_file.clock_ticks_per_second == 0u ||
                config->bus.task_file.clock_ticks_per_second % 1000000u != 0u)) return 0;
    }
    for (first = 0u; first < port_count; ++first) {
        if (ports[first] == 0u) return 0;
        for (second = first + 1u; second < port_count; ++second) {
            if (ports[first] == ports[second]) return 0;
        }
    }
    return 1;
}

typedef struct core_machine_port_direction_requirement {
    type_unsigned_16 port;
    type_bool read;
    type_bool write;
} core_machine_port_direction_requirement;

static C_INT core_machine_controller_ports_are_available(
    const core_machine *machine,
    const core_machine_port_direction_requirement *requirements,
    STD_SIZE_T count)
{
    STD_SIZE_T index;

    if (machine == STD_NULL || requirements == STD_NULL) return 0;
    for (index = 0u; index < count; ++index) {
        if ((requirements[index].read && core_machine_port_has_read(
                &machine->executor_port, requirements[index].port)) ||
            (requirements[index].write && core_machine_port_has_write(
                &machine->executor_port, requirements[index].port))) {
            return 0;
        }
    }
    return 1;
}

type_status core_machine_configure_fdc(core_machine *machine,
    const core_machine_fdc_topology *topology)
{
    const core_machine_port_direction_requirement ports[] = {
        {topology == STD_NULL ? 0u : topology->config.dor_port,
            TYPE_FALSE, TYPE_TRUE},
        {topology == STD_NULL ? 0u : topology->config.status_port,
            TYPE_TRUE, TYPE_FALSE},
        {topology == STD_NULL ? 0u : topology->config.data_port,
            TYPE_TRUE, TYPE_TRUE},
        {topology == STD_NULL ? 0u : topology->config.direction_port,
            topology != STD_NULL && topology->config.direction_port != 0u,
            TYPE_FALSE},
        {topology == STD_NULL ? 0u : topology->config.control_port,
            TYPE_FALSE, topology != STD_NULL && topology->config.control_port != 0u}
    };
    core_machine_port_provider_entry *port_checkpoint;
    type_status status;

    if (!core_machine_configuration_is_open(machine) || !machine->dma_configured ||
        machine->fdc_configured) {
        return TYPE_STATUS_INVALID_STATE;
    }
    if (!core_machine_fdc_topology_is_valid(topology) ||
        topology->dma_request.core_token != machine->fdc_dma_request.core_token ||
        topology->dma_request.channel != machine->fdc_dma_request.channel) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (!core_machine_controller_ports_are_available(machine, ports,
            sizeof(ports) / sizeof(ports[0]))) return TYPE_STATUS_INVALID_STATE;
    port_checkpoint = core_machine_port_registration_begin(&machine->executor_port);
    machine->fdc_topology = *topology;
    core_machine_fdc_connect(&machine->fdc, machine->fdc_topology.media_registry,
        &machine->fdc_topology.drives, &machine->fdc_topology.dma_request,
        core_machine_fdc_dma_request_assert,
        core_machine_fdc_dma_request_deassert, machine,
        &machine->shared_pic_master, &machine->shared_pic_slave,
        &machine->executor_port, &machine->fdc_topology.config,
        &machine->fdc_topology.observation_provider);
    core_machine_fdc_initialize(&machine->fdc);
    status = core_machine_port_registration_status(&machine->executor_port);
    if (status != TYPE_STATUS_OK) {
        core_machine_port_rollback_registration(&machine->executor_port,
            port_checkpoint);
        core_machine_fdc_finalize(&machine->fdc);
        STD_MEMSET(&machine->fdc_topology, TYPE_ZERO_8,
            sizeof(machine->fdc_topology));
        return status;
    }
    machine->fdc_configured = TYPE_TRUE;
    return TYPE_STATUS_OK;
}

type_status core_machine_configure_hdc(core_machine *machine,
    const core_machine_hdc_topology *topology)
{
    const core_machine_port_provider *provider;
    core_machine_port_direction_requirement ports[9];
    STD_SIZE_T port_count;
    STD_SIZE_T index;
    type_status status;
    core_machine_port_provider_entry *port_checkpoint;

    if (!core_machine_configuration_is_open(machine) || machine->hdc_configured) {
        return TYPE_STATUS_INVALID_STATE;
    }
    if (!core_machine_hdc_topology_is_valid(topology)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (topology->config.protocol == CORE_MACHINE_HDC_PROTOCOL_COMPAQ_WD_40MB &&
        (!machine->fdc_configured ||
            topology->config.bus.task_file.drive_address_port !=
                machine->fdc_topology.config.direction_port ||
            !core_machine_port_has_read(&machine->executor_port,
                topology->config.bus.task_file.drive_address_port))) return TYPE_STATUS_INVALID_STATE;
    if (topology->config.protocol == CORE_MACHINE_HDC_PROTOCOL_XEBEC_XT) {
        if (!machine->dma_configured) return TYPE_STATUS_INVALID_STATE;
        ports[0] = (core_machine_port_direction_requirement) {
            topology->config.bus.xebec.data_port, TYPE_TRUE, TYPE_TRUE};
        ports[1] = (core_machine_port_direction_requirement) {
            topology->config.bus.xebec.hardware_status_reset_port, TYPE_TRUE, TYPE_TRUE};
        ports[2] = (core_machine_port_direction_requirement) {
            topology->config.bus.xebec.jumpers_select_port, TYPE_TRUE, TYPE_TRUE};
        ports[3] = (core_machine_port_direction_requirement) {
            topology->config.bus.xebec.dma_irq_mask_port, TYPE_FALSE, TYPE_TRUE};
        port_count = 4u;
    } else {
        const core_machine_hdc_task_file_config *task_file =
            &topology->config.bus.task_file;

        ports[0] = (core_machine_port_direction_requirement) {task_file->data_port,
            TYPE_TRUE, TYPE_TRUE};
        ports[1] = (core_machine_port_direction_requirement) {
            task_file->error_features_port, TYPE_TRUE, TYPE_TRUE};
        ports[2] = (core_machine_port_direction_requirement) {task_file->sector_count_port,
            TYPE_TRUE, TYPE_TRUE};
        ports[3] = (core_machine_port_direction_requirement) {task_file->sector_number_port,
            TYPE_TRUE, TYPE_TRUE};
        ports[4] = (core_machine_port_direction_requirement) {task_file->cylinder_low_port,
            TYPE_TRUE, TYPE_TRUE};
        ports[5] = (core_machine_port_direction_requirement) {task_file->cylinder_high_port,
            TYPE_TRUE, TYPE_TRUE};
        ports[6] = (core_machine_port_direction_requirement) {task_file->drive_head_port,
            TYPE_TRUE, TYPE_TRUE};
        ports[7] = (core_machine_port_direction_requirement) {task_file->status_command_port,
            TYPE_TRUE, TYPE_TRUE};
        ports[8] = (core_machine_port_direction_requirement) {
            task_file->alternate_status_device_control_port, TYPE_TRUE, TYPE_TRUE};
        port_count = 9u;
    }
    if (!core_machine_controller_ports_are_available(machine, ports, port_count)) {
        return TYPE_STATUS_INVALID_STATE;
    }
    provider = core_machine_hdc_port_provider();
    if (provider == STD_NULL) return TYPE_STATUS_FAULT;
    port_checkpoint = core_machine_port_registration_begin(&machine->executor_port);
    machine->hdc_topology = *topology;
    core_machine_hdc_connect(&machine->hdc, machine->hdc_topology.media_registry,
        machine->hdc_topology.media_id, machine->hdc_topology.slave_media_id,
        &machine->shared_pic_master,
        &machine->shared_pic_slave, &machine->hdc_topology.config);
    core_machine_hdc_initialize(&machine->hdc);
    for (index = 0u; index < port_count; ++index) {
        if (ports[index].read && (status = core_machine_port_add_read_provider(
                &machine->executor_port, ports[index].port, provider->read,
                &machine->hdc)) != TYPE_STATUS_OK) break;
        if (ports[index].write && (status = core_machine_port_add_write_provider(
                &machine->executor_port, ports[index].port, provider->write,
                &machine->hdc)) != TYPE_STATUS_OK) break;
    }
    if (status != TYPE_STATUS_OK) {
        core_machine_port_rollback_registration(&machine->executor_port,
            port_checkpoint);
        core_machine_hdc_finalize(&machine->hdc);
        STD_MEMSET(&machine->hdc_topology, TYPE_ZERO_8,
            sizeof(machine->hdc_topology));
        return status;
    }
    if (machine->hdc_topology.config.protocol == CORE_MACHINE_HDC_PROTOCOL_COMPAQ_WD_40MB) {
        status = core_machine_port_add_read_wired_or_provider(&machine->executor_port,
            machine->hdc_topology.config.bus.task_file.drive_address_port,
            provider->read, &machine->hdc);
        if (status != TYPE_STATUS_OK) {
            core_machine_port_rollback_registration(&machine->executor_port,
                port_checkpoint);
            core_machine_hdc_finalize(&machine->hdc);
            STD_MEMSET(&machine->hdc_topology, TYPE_ZERO_8,
                sizeof(machine->hdc_topology));
            return status;
        }
    }
    if (machine->hdc_topology.config.protocol == CORE_MACHINE_HDC_PROTOCOL_XEBEC_XT) {
        status = core_machine_dma_bind_channel(&machine->shared_dma_latch,
            &machine->shared_dma_primary, &machine->shared_dma_secondary,
            machine->hdc_topology.config.bus.xebec.dma_channel,
            core_machine_hdc_dma_provider(), &machine->hdc, &machine->hdc_dma_request);
        if (status != TYPE_STATUS_OK) {
            core_machine_port_rollback_registration(&machine->executor_port,
                port_checkpoint);
            core_machine_hdc_finalize(&machine->hdc);
            STD_MEMSET(&machine->hdc_topology, TYPE_ZERO_8,
                sizeof(machine->hdc_topology));
            return status;
        }
        core_machine_hdc_bind_dma_request(&machine->hdc, &machine->hdc_dma_request,
            core_machine_hdc_dma_request_assert, core_machine_hdc_dma_request_deassert,
            machine);
    }
    machine->hdc_configured = TYPE_TRUE;
    return TYPE_STATUS_OK;
}
