#include "lib/types/types_interface.h"
#include "x86/ibmpc-common/machine_board_interface.h"
#include "x86/core/device_support_interface.h"

#include "x86/ibmpc-common/machine_board_state.h"

#define CORE_MACHINE_BOARD_A20_BIT 0x02u

static lib_status core_machine_board_register_reset_rom_alias(void *owner);
static lib_status core_machine_board_memory_admission(void *owner,
    lib_size memory_bytes);

lib_status core_machine_board_bind_profile(core_machine_board_state *board,
    const core_machine_board_profile_binding *binding)
{
    if (board == LIB_NULL || !core_machine_configuration_is_open(board->core) ||
        board->profile_binding.context != LIB_NULL) return LIB_STATUS_INVALID_STATE;
    if (binding == LIB_NULL || binding->context == LIB_NULL ||
        binding->reset == LIB_NULL || binding->refresh_nmi == LIB_NULL ||
        binding->refresh_request == LIB_NULL || binding->refresh_complete == LIB_NULL ||
        binding->next_deadline == LIB_NULL || binding->finalize == LIB_NULL ||
        (binding->owns_refresh_output != LIB_FALSE && binding->owns_refresh_output != LIB_TRUE) ||
        (binding->shutdown_resets != LIB_FALSE && binding->shutdown_resets != LIB_TRUE))
        return LIB_STATUS_INVALID_ARGUMENT;
    board->profile_binding = *binding;
    return LIB_STATUS_OK;
}

/* Range-selected XT durations retain their existing L2 macro-axis values.
 * Quotient/remainder conversion avoids overflow before the final ceiling. */
static lib_u64 core_machine_xt_keyboard_duration(lib_u64 rate, lib_u32 us)
{
    lib_u64 whole = (rate / 1000000u) * us;
    lib_u64 fraction = (rate % 1000000u) * us;
    return whole + fraction / 1000000u + (fraction % 1000000u != 0u);
}

static lib_status core_machine_xt_keyboard_deliver(void *owner, lib_u8 byte)
{
    return core_machine_xt_ppi_keyboard_receive_device_byte(owner, byte);
}

static void core_machine_xt_keyboard_lines(void *owner,
    lib_u8 clock_held, lib_u8 clear_asserted)
{
    x86_xt_keyboard_set_lines(owner, clock_held, clear_asserted);
}

static void core_machine_xt_keyboard_released(void *owner)
{
    x86_xt_keyboard_receiver_ready(owner);
}

static lib_u8 core_machine_xt_ppi_request_nmi(void *owner)
{
    core_machine *machine = (core_machine *)owner;

    return core_machine_signal_nmi(machine);
}

static void core_machine_kbc_request_reset(void *owner)
{
    core_machine *machine = owner;

    core_machine_signal_processor_reset(machine);
}

static void core_machine_kbc_signal_a20(void *owner, lib_bool enabled)
{
    (void)core_machine_signal_a20(owner, enabled);
}

static void core_machine_kbc_irq_output(void *owner, lib_bool auxiliary,
    lib_bool asserted)
{
    core_machine_board_state *board = owner;
    core_machine_pic_irq_source *source = auxiliary ?
        board->keyboard_irq12_source : board->keyboard_irq1_source;
    if (asserted) core_machine_pic_irq_source_assert(source);
    else core_machine_pic_irq_source_deassert(source);
}

static void core_machine_xt_ppi_update_speaker(void *owner,
    lib_u8 timer_gate, lib_u8 data_enabled)
{
    core_machine_board_set_xt_ppi_speaker((core_machine_board_state *)owner, timer_gate,
        data_enabled);
}

lib_i32 core_machine_board_config_is_valid(
    const core_machine_config *config)
{
    return core_machine_clock_plan_is_valid(&config->clock_plan) &&
        (config->shared_pit_personality == X86_PIT_PERSONALITY_8254 ||
            config->shared_pit_personality == X86_PIT_PERSONALITY_8253) &&
        (config->auxiliary_pit_present == LIB_FALSE ||
         config->auxiliary_pit_present == LIB_TRUE) &&
        (config->pic_topology == CORE_MACHINE_PIC_TOPOLOGY_CASCADED ||
         config->pic_topology == CORE_MACHINE_PIC_TOPOLOGY_SINGLE) &&
        config->dma_controller_count <= CORE_MACHINE_DMA_CONTROLLER_COUNT &&
        (config->keyboard_topology == CORE_MACHINE_KEYBOARD_TOPOLOGY_8042 ||
         config->keyboard_topology == CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI) &&
        (config->keyboard_topology != CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI ||
         core_machine_xt_ppi_keyboard_config_is_valid(&config->xt_ppi_keyboard)) &&
        (!config->auxiliary_pit_present || config->auxiliary_pit_base_port <= 0xfffcu);
}

lib_status core_machine_board_prepare_executor(
    const core_machine_config *config, core_machine_executor_config *out_executor)
{
    if (config == LIB_NULL || out_executor == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    const core_machine_executor_config executor = {
        .memory_bytes = config->memory_bytes,
        .cpu_profile = config->cpu_profile,
        .fpu_profile = config->fpu_profile,
        .cpu_80386_cr_mov_ignores_mod = config->cpu_80386_cr_mov_ignores_mod,
        .a20_wrap_policy = config->a20_wrap_policy,
        .ticks_per_instruction = config->ticks_per_instruction,
        .instruction_timing = config->instruction_timing,
        .transaction_contract = config->transaction_contract,
        .provider_clock = config->clock_plan.provider,
        .time_axis = config->time_axis,
        .l1_compatibility_policy = config->l1_compatibility_policy,
        .retirement_time_contract = config->retirement_time_contract,
        .retirement_qualification = config->retirement_qualification
    };
    if (!core_machine_neutral_config_is_valid(&executor) ||
        !core_machine_board_config_is_valid(config))
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_executor = executor;
    return LIB_STATUS_OK;
}

lib_status core_machine_create(const core_machine_config *config,
    core_machine **out_machine, core_machine_board_state **out_board)
{
    core_machine *machine;
    core_machine_board_state *board;
    core_machine_executor_config executor;
    lib_status status;

    if (out_board != LIB_NULL) *out_board = LIB_NULL;
    if (out_machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_machine = LIB_NULL;
    status = core_machine_board_prepare_executor(config, &executor);
    if (status != LIB_STATUS_OK) return status;
    status = core_machine_neutral_create(&executor, &machine);
    if (status != LIB_STATUS_OK) return status;
    status = core_machine_board_create(machine, config, &board);
    if (status != LIB_STATUS_OK) return status;
    *out_machine = machine;
    if (out_board != LIB_NULL) *out_board = board;
    return LIB_STATUS_OK;
}

lib_status core_machine_board_create(core_machine *machine,
    const core_machine_config *config, core_machine_board_state **out_board)
{
    core_machine_board_state *board;
    lib_status status;
    lib_u8 dma_controller_count;
    lib_size installed_bytes;
    core_machine_cpu_profile cpu_profile;

    *out_board = LIB_NULL;
    board = (core_machine_board_state *)lib_allocate_zero(1u,
        sizeof(*board));
    if (board == LIB_NULL) {
        core_machine_destroy(machine);
        return LIB_STATUS_NO_MEMORY;
    }
    board->core = machine;
    const core_machine_attachment attachment = {
        .memory_admission = core_machine_board_memory_admission,
        .deadline = core_machine_board_deadline_observe,
        .refresh_request = core_machine_board_refresh_request,
        .refresh_complete = core_machine_board_refresh_complete,
        .dma_ticks = core_machine_board_dma_ticks,
        .dma_request = core_machine_board_dma_request,
        .dma_advance = core_machine_board_dma_advance,
        .pit_ticks = core_machine_board_pit_ticks_advance,
        .pit_pic = core_machine_board_pit_pic_advance,
        .pic_pending = core_machine_board_pic_pending,
        .pic_acknowledge = core_machine_board_pic_acknowledge,
        .shutdown_reset = core_machine_board_shutdown_resets,
        .media = core_machine_board_media_advance,
        .rtc = core_machine_board_rtc_advance,
        .peripheral = core_machine_board_peripheral_advance,
        .reset_devices = core_machine_board_reset_devices,
        .reset_clocks = core_machine_board_reset_clocks,
        .refresh_nmi = core_machine_board_refresh_nmi,
        .finalize_devices = core_machine_board_finalize_devices,
        .firmware = core_machine_board_register_reset_rom_alias,
        .context = board
    };

    status = core_machine_bind_attachment(machine, &attachment);
    if (status != LIB_STATUS_OK) {
        core_machine_board_finalize_devices(board);
        core_machine_destroy(machine);
        return status;
    }
    if (core_machine_board_initialize_clocks(board,
            &config->clock_plan) != LIB_STATUS_OK) {
        core_machine_destroy(machine);
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    status = core_machine_fdc_create(&board->fdc);
    if (status == LIB_STATUS_OK) status = core_machine_hdc_create(&board->hdc);
    if (status != LIB_STATUS_OK) {
        core_machine_destroy(machine);
        return status;
    }
    board->dma_clock_explicit =
        config->clock_plan.dma.numerator != 0u &&
        config->clock_plan.dma.denominator != 0u;

    dma_controller_count = config->dma_controller_count == 0u ?
        CORE_MACHINE_DMA_CONTROLLER_COUNT : config->dma_controller_count;
    board->keyboard_topology = config->keyboard_topology;
    /* Zero is an explicit profile choice: without a calibrated guest-time
     * mapping, core-generated keyboard repeat must remain disabled. */
    board->kbc_typematic_initial_ticks = config->kbc_typematic_initial_ticks;
    board->kbc_typematic_repeat_ticks = config->kbc_typematic_repeat_ticks;
    board->kbc_command_response_ticks = config->kbc_command_response_ticks;
    board->kbc_command_response_status_polls =
        config->kbc_command_response_status_polls;
    board->kbc_serial_delivery_ticks = config->kbc_serial_delivery_ticks;
    board->kbc_input_port_configured = config->kbc_input_port_configured;
    board->kbc_input_port = config->kbc_input_port;
    /* Firmware-less fixtures may supply reset bytes from ordinary board RAM.
     * Firmware-backed machines install the reset-only ROM overlay later. */
    if (core_machine_get_memory_bytes(machine, &installed_bytes) != LIB_STATUS_OK ||
        core_machine_get_cpu_profile(machine, &cpu_profile) != LIB_STATUS_OK) {
        core_machine_destroy(machine);
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (installed_bytes >= 0x00100000u &&
        (cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 ||
         cpu_profile == CORE_MACHINE_CPU_PROFILE_80386)) {
        const core_machine_memory_alias_config alias = {
            cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 ? 0x00ff0000u : 0xffff0000u,
            0x000f0000u, 0x00010000u
        };
        lib_status status = core_machine_install_memory_aliases(machine,
            &alias, 1u, LIB_FALSE);
        if (status != LIB_STATUS_OK) {
            core_machine_destroy(machine);
            return status;
        }
    }
    {
        lib_status status = core_machine_board_register_a20_port(machine);
        if (status != LIB_STATUS_OK) {
            core_machine_destroy(machine);
            return status;
        }
    }
    {
        lib_status status = core_machine_vadp_create(machine, &board->shared_vadp);
        if (status != LIB_STATUS_OK) {
            core_machine_destroy(machine);
            return status;
        }
    }
    if (config->keyboard_topology == CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI) {
        lib_status status = core_machine_xt_ppi_keyboard_create(
            &config->xt_ppi_keyboard, machine, &board->xt_ppi_keyboard);
        if (status != LIB_STATUS_OK) {
            core_machine_destroy(machine);
            return status;
        }
        {
            lib_u64 rate = config->time_axis.ticks_per_second;
            x86_xt_keyboard_timing timing = {
                core_machine_xt_keyboard_duration(rate, 12500u),
                core_machine_xt_keyboard_duration(rate, 300000u),
                core_machine_xt_keyboard_duration(rate, 60u),
                core_machine_xt_keyboard_duration(rate, 25u)
            };
            status = x86_xt_keyboard_create(&timing,
                core_machine_xt_keyboard_deliver, board->xt_ppi_keyboard,
                &board->xt_keyboard);
            if (status != LIB_STATUS_OK) {
                core_machine_destroy(machine);
                return status;
            }
        }
    } else {
        lib_status status = core_machine_kbc_create(machine, &board->shared_kbc);
        if (status != LIB_STATUS_OK) {
            core_machine_destroy(machine);
            return status;
        }
    }
    {
        lib_status status = core_machine_dma_initialize(&board->shared_dma,
            machine, dma_controller_count);
        if (status == LIB_STATUS_OK) {
            status = core_machine_pic_initialize(&board->shared_pic_master,
                &board->shared_pic_slave, machine, config->pic_topology);
        }
        if (status != LIB_STATUS_OK) {
            core_machine_destroy(machine);
            return status;
        }
    }
    core_machine_pic_set_irq_timing(board->shared_pic_master,
        board->shared_pic_slave, &config->pic_irq_timing);
    {
        lib_status status = core_machine_pic_irq_source_bind(&board->shared_pit_irq0_source,
            board->shared_pic_master, board->shared_pic_slave, 0u);
        if (status == LIB_STATUS_OK)
            status = x86_pit_create(config->shared_pit_personality, &board->shared_pit);
        if (status == LIB_STATUS_OK) {
            status = core_machine_pit_install_ports(machine,
                board->shared_pit, 0x0040u);
        }
        if (status == LIB_STATUS_OK && config->auxiliary_pit_present) {
            status = x86_pit_create(X86_PIT_PERSONALITY_8254,
                &board->auxiliary_pit);
            if (status == LIB_STATUS_OK) {
                status = core_machine_pit_install_ports(machine,
                    board->auxiliary_pit, config->auxiliary_pit_base_port);
            }
            board->auxiliary_pit_configured = status == LIB_STATUS_OK;
        }
        if (status != LIB_STATUS_OK) {
            core_machine_destroy(machine);
            return status;
        }
    }
    x86_pit_set_output(board->shared_pit, 0,
        core_machine_pic_timer_output, board->shared_pit_irq0_source);
    if (config->keyboard_topology == CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI) {
        core_machine_board_configure_xt_ppi_speaker(board);
        lib_status status = core_machine_pic_irq_source_bind(&board->keyboard_irq1_source,
            board->shared_pic_master, board->shared_pic_slave, config->xt_ppi_keyboard.irq);
        if (status != LIB_STATUS_OK) {
            core_machine_destroy(machine);
            return status;
        }
        core_machine_xt_ppi_keyboard_bind_irq(board->xt_ppi_keyboard,
            core_machine_pic_timer_output, board->keyboard_irq1_source);
        core_machine_xt_ppi_keyboard_bind_nmi(board->xt_ppi_keyboard,
            core_machine_xt_ppi_request_nmi, machine);
        core_machine_xt_ppi_keyboard_bind_speaker(board->xt_ppi_keyboard,
            core_machine_xt_ppi_update_speaker, board);
        core_machine_xt_ppi_keyboard_bind_keyboard_observer(board->xt_ppi_keyboard,
            core_machine_xt_keyboard_lines, board->xt_keyboard,
            core_machine_xt_keyboard_released);
    } else {
        lib_status status = core_machine_pic_irq_source_bind(
            &board->keyboard_irq1_source, board->shared_pic_master,
            board->shared_pic_slave, 1u);
        if (status == LIB_STATUS_OK)
            status = core_machine_pic_irq_source_bind(&board->keyboard_irq12_source,
                board->shared_pic_master, board->shared_pic_slave, 12u);
        if (status != LIB_STATUS_OK) {
            core_machine_destroy(machine);
            return status;
        }
        core_machine_kbc_bind_core_services(board->shared_kbc,
            core_machine_kbc_irq_output, board, core_machine_kbc_signal_a20,
            machine, core_machine_kbc_request_reset, machine,
            !config->kbc_aux_absent);
        if (config->kbc_reset_output_port_configured) {
            core_machine_kbc_set_reset_output_port(board->shared_kbc,
                config->kbc_reset_output_port);
        }
        core_machine_kbc_set_typematic_timing(board->shared_kbc,
            board->kbc_typematic_initial_ticks,
            board->kbc_typematic_repeat_ticks);
        core_machine_kbc_set_command_response_timing(board->shared_kbc,
            board->kbc_command_response_ticks);
        core_machine_kbc_set_command_response_status_polls(board->shared_kbc,
            board->kbc_command_response_status_polls);
        core_machine_kbc_set_serial_delivery_timing(board->shared_kbc,
            board->kbc_serial_delivery_ticks);
    }
    x86_pit_set_output(board->shared_pit, 1, LIB_NULL, LIB_NULL);
    *out_board = board;
    return LIB_STATUS_OK;
}

lib_bool core_machine_board_shutdown_resets(void *owner)
{
    const core_machine_board_state *board = owner;
    return board != LIB_NULL && board->profile_binding.shutdown_resets;
}

lib_status core_machine_keyboard_receive_native_byte(core_machine_board_state *board,
    lib_u8 native_byte)
{
    core_machine_lifecycle lifecycle;

    if (board == LIB_NULL || !core_machine_mutable_operation_is_allowed(board->core) ||
        core_machine_get_lifecycle(board->core, &lifecycle) != LIB_STATUS_OK ||
        lifecycle == CORE_MACHINE_INITIALIZED ||
        lifecycle == CORE_MACHINE_FAULTED) {
        return LIB_STATUS_INVALID_STATE;
    }
    if (board->keyboard_topology ==
            CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI) {
        return x86_xt_keyboard_receive_native_bytes(board->xt_keyboard,
            &native_byte, 1u);
    }
    return core_machine_kbc_submit_native_byte(board->shared_kbc, native_byte);
}

lib_status core_machine_keyboard_get_native_scan_set(const core_machine_board_state *board,
    lib_u8 *out_scan_set)
{
    core_machine_lifecycle lifecycle;

    if (board == LIB_NULL || out_scan_set == LIB_NULL ||
        core_machine_get_lifecycle(board->core, &lifecycle) != LIB_STATUS_OK ||
        lifecycle == CORE_MACHINE_INITIALIZED) {
        return LIB_STATUS_INVALID_STATE;
    }
    *out_scan_set = board->keyboard_topology ==
        CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI ? CORE_MACHINE_KEYBOARD_SCAN_SET_1 :
        core_machine_kbc_get_native_scan_set(board->shared_kbc);
    return LIB_STATUS_OK;
}

lib_status core_machine_keyboard_receive_native_bytes(core_machine_board_state *board,
    const lib_u8 *native_bytes, lib_size count)
{
    core_machine_lifecycle lifecycle;

    if (board == LIB_NULL || !core_machine_mutable_operation_is_allowed(board->core) ||
        core_machine_get_lifecycle(board->core, &lifecycle) != LIB_STATUS_OK ||
        lifecycle == CORE_MACHINE_INITIALIZED ||
        lifecycle == CORE_MACHINE_FAULTED) {
        return LIB_STATUS_INVALID_STATE;
    }
    if (board->keyboard_topology ==
            CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI) {
        return x86_xt_keyboard_receive_native_bytes(board->xt_keyboard,
            native_bytes, count);
    }
    return core_machine_kbc_submit_native_bytes(board->shared_kbc, native_bytes, count);
}

lib_status core_machine_set_xt_ppi_fault_input(core_machine_board_state *board,
    core_machine_xt_ppi_fault_input input, lib_i32 asserted)
{
    if (board == LIB_NULL || !core_machine_mutable_operation_is_allowed(board->core) ||
        board->keyboard_topology != CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI) {
        return LIB_STATUS_INVALID_STATE;
    }
    return core_machine_xt_ppi_keyboard_set_fault_input(board->xt_ppi_keyboard,
        input, asserted);
}

lib_status core_machine_mouse_receive_relative(core_machine_board_state *board,
    lib_i16 delta_x, lib_i16 delta_y, lib_u8 buttons)
{
    core_machine_lifecycle lifecycle;

    if (board == LIB_NULL || !core_machine_mutable_operation_is_allowed(board->core) ||
        core_machine_get_lifecycle(board->core, &lifecycle) != LIB_STATUS_OK ||
        (lifecycle != CORE_MACHINE_RUNNING &&
        lifecycle != CORE_MACHINE_PAUSED &&
        lifecycle != CORE_MACHINE_STOPPED)) {
        return LIB_STATUS_INVALID_STATE;
    }
    if (board->keyboard_topology ==
            CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI) return LIB_STATUS_UNSUPPORTED;
    return core_machine_kbc_submit_aux_report(board->shared_kbc, delta_x, delta_y, buttons);
}

static lib_u32 core_machine_board_reset_rom_alias(
    core_machine_cpu_profile profile)
{
    switch (profile) {
    case CORE_MACHINE_CPU_PROFILE_80286:
        return 0x00ff0000u;
    case CORE_MACHINE_CPU_PROFILE_80386:
        return 0xffff0000u;
    case CORE_MACHINE_CPU_PROFILE_8086:
    case CORE_MACHINE_CPU_PROFILE_8088:
    case CORE_MACHINE_CPU_PROFILE_80186:
    case CORE_MACHINE_CPU_PROFILE_DEFAULT:
        return 0u;
    }
    return 0u;
}

static lib_status core_machine_board_register_reset_rom_alias(void *owner)
{
    core_machine_board_state *board = owner;
    core_machine *machine = board->core;
    core_machine_cpu_profile profile;
    lib_u32 reset_alias;
    lib_status status = core_machine_get_cpu_profile(machine, &profile);

    if (status != LIB_STATUS_OK) return status;
    reset_alias = core_machine_board_reset_rom_alias(profile);
    /* Keep the established source/target presence widths and explicit high
     * ROM precedence. PC address selection belongs only to this board. */
    if (reset_alias == 0u ||
        !core_machine_immutable_rom_mapping_contains(machine, 0x000ffff0u, 15u) ||
        core_machine_immutable_rom_mapping_contains(machine, reset_alias + 0xfff0u, 16u)) {
        return LIB_STATUS_OK;
    }
    return core_machine_register_immutable_rom_mapping_reset_window(machine,
        0x000f0000u, reset_alias, 0x00010000u);
}

static lib_status core_machine_board_memory_admission(void *owner,
    lib_size memory_bytes)
{
    const core_machine_board_state *board = owner;
    core_machine_planar_parity_observation parity = {0};

    (void)memory_bytes;
    if (board != LIB_NULL) core_machine_at_parity_observe(board->planar_parity, &parity);
    if (board == LIB_NULL || parity.configured) {
        return LIB_STATUS_INVALID_STATE;
    }
    return LIB_STATUS_OK;
}

static lib_status core_machine_board_read_a20(void *owner, lib_u16 port_id, lib_u64 tick,
    lib_u32 *out_value)
{
    (void)tick;
    lib_bool enabled;
    lib_status status;

    (void)port_id;
    if (out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = core_machine_observe_a20(owner, &enabled);
    if (status != LIB_STATUS_OK) return status;
    *out_value = enabled ? CORE_MACHINE_BOARD_A20_BIT : 0u;
    return LIB_STATUS_OK;
}

static lib_status core_machine_board_write_a20(void *owner, lib_u16 port_id,
    lib_u32 value)
{
    (void)port_id;
    return core_machine_signal_a20(owner,
        CORE_MACHINE_BIT_IS_SET(value, CORE_MACHINE_BOARD_A20_BIT));
}

lib_status core_machine_board_register_a20_port(core_machine *machine)
{
    core_machine_port_route route;

    if (machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    route = (core_machine_port_route) {0x0092u, core_machine_board_read_a20,
        core_machine_board_write_a20, machine, LIB_FALSE, 0u};
    return core_machine_install_port_routes(machine, &route, 1u);
}

static lib_i32 core_machine_rtc_cmos_config_is_valid(
    const core_machine_rtc_cmos_config *config)
{
    lib_size index;

    if (config == LIB_NULL || config->data_port !=
        (lib_u16)(config->index_port + 1u) || config->nmi_mask_bit == 0u ||
        config->ticks_per_second == 0u || config->default_count > CORE_MACHINE_RTC_DEFAULT_CAPACITY ||
        (config->timing.provenance != CORE_MACHINE_RTC_TIMING_L2_RATIO &&
         config->timing.provenance != CORE_MACHINE_RTC_TIMING_L3_SOURCE) ||
        (config->timing.provenance == CORE_MACHINE_RTC_TIMING_L3_SOURCE &&
         (config->timing.uip_lead_ticks == 0u || config->timing.update_ticks == 0u ||
          (lib_u64)config->timing.uip_lead_ticks +
              config->timing.update_ticks >= config->ticks_per_second))) {
        return LIB_FALSE;
    }
    for (index = 0u; index < config->default_count; ++index) {
        lib_u8 register_index = config->defaults[index].index;

        if (register_index >= X86_RTC_REGISTER_COUNT ||
            register_index == X86_RTC_REG_A ||
            register_index == X86_RTC_REG_B ||
            register_index == X86_RTC_REG_C ||
            register_index == X86_RTC_REG_D) {
            return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_status core_machine_rtc_cmos_port_read(void *owner,
    lib_u16 port, lib_u64 tick,
    lib_u32 *out_value)
{
    (void)tick;
    core_machine_board_state *board = (core_machine_board_state *)owner;

    if (board == LIB_NULL || out_value == LIB_NULL ||
        port != board->rtc_cmos_config.data_port) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_value = x86_rtc_read_register(board->shared_rtc,
        board->rtc_selected_register);
    return LIB_STATUS_OK;
}

static lib_status core_machine_rtc_cmos_port_write(void *owner,
    lib_u16 port, lib_u32 value)
{
    core_machine_board_state *board = (core_machine_board_state *)owner;

    if (board == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (port == board->rtc_cmos_config.index_port) {
        (void)core_machine_set_nmi_mask(board->core,
            (value & board->rtc_cmos_config.nmi_mask_bit) != 0u ?
            LIB_TRUE : LIB_FALSE);
        board->rtc_selected_register = (lib_u8)(value & 0x3fu);
        return LIB_STATUS_OK;
    }
    if (port == board->rtc_cmos_config.data_port) {
        x86_rtc_write_register(board->shared_rtc,
            board->rtc_selected_register, (lib_u8)value);
        return LIB_STATUS_OK;
    }
    return LIB_STATUS_INVALID_ARGUMENT;
}


static void core_machine_dma_refresh_pit_output(void *owner,
    lib_u8 asserted);

/* AT parity wiring programs the fixed counter-1 refresh divider on reset.
 * Firmware retains channel 0 and channel 2 programming. */
static void core_machine_pc_at_refresh_timer_program(core_machine_board_state *board)
{
    if (board == LIB_NULL) return;
    (void)x86_pit_write_register(board->shared_pit, 3u, 0x74u);
    (void)x86_pit_write_register(board->shared_pit, 1u, 18u);
    (void)x86_pit_write_register(board->shared_pit, 1u, 0u);
}

static lib_u8 core_machine_speaker_source_value(
    const core_machine_board_state *board)
{
    return board == LIB_NULL ? 0u : board->speaker_lines;
}

static void core_machine_speaker_refresh(core_machine_board_state *board)
{
    lib_u8 value;

    if (board == LIB_NULL) return;
    value = core_machine_speaker_source_value(board);
    board->speaker_output = (value & 0x02u) != 0u &&
        ((value & 0x01u) == 0u ||
        x86_pit_get_output(board->shared_pit, 2u));
}

static void core_machine_speaker_timer_output(void *owner,
    lib_u8 asserted)
{
    core_machine_board_state *board = (core_machine_board_state *)owner;

    (void)asserted;
    core_machine_speaker_refresh(board);
}

static void core_machine_speaker_set_gate(core_machine_board_state *board,
    lib_u8 value)
{
    if (board == LIB_NULL) return;
    board->speaker_lines = value & 0x03u;
    x86_pit_set_gate(board->shared_pit, 2u,
        (value & 0x01u) != 0u ? LIB_TRUE : LIB_FALSE);
    core_machine_speaker_refresh(board);
}

static void core_machine_at_speaker_output(void *owner, lib_u8 value)
{
    core_machine_speaker_set_gate(owner, value);
}

lib_status core_machine_board_construct_profile(core_machine_board_state *board,
    core_machine_board_profile_factory factory, void *context)
{
    core_machine_board_profile_binding binding = {0};
    lib_status status;

    if (board == LIB_NULL || !core_machine_configuration_is_open(board->core) ||
        board->profile_binding.context != LIB_NULL) return LIB_STATUS_INVALID_STATE;
    if (factory == LIB_NULL || board->planar_parity != LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    const core_machine_board_profile_services services = {
        board->core, board->shared_pit,
        board->auxiliary_pit_configured ? board->auxiliary_pit : LIB_NULL,
        core_machine_at_speaker_output, board
    };
    status = factory(&services, context, &binding);
    if (status != LIB_STATUS_OK) return status;
    status = core_machine_board_bind_profile(board, &binding);
    if (status != LIB_STATUS_OK) {
        if (binding.finalize != LIB_NULL) binding.finalize(binding.context);
        return status;
    }
    x86_pit_set_output(board->shared_pit, 2u,
        core_machine_speaker_timer_output, board);
    binding.reset(binding.context, CORE_MACHINE_BOARD_RESET_AFTER_PIT);
    return LIB_STATUS_OK;
}

void core_machine_board_reset_devices(void *owner)
{
    core_machine_board_state *board = owner;
    if (board->profile_binding.reset != LIB_NULL)
        board->profile_binding.reset(board->profile_binding.context,
            CORE_MACHINE_BOARD_RESET_BEFORE_DEVICES);
    if (board->keyboard_topology == CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI) {
        core_machine_xt_ppi_keyboard_reset(board->xt_ppi_keyboard);
        x86_xt_keyboard_reset(board->xt_keyboard);
    } else {
        core_machine_kbc_reset(board->shared_kbc);
        if (board->kbc_input_port_configured) {
            core_machine_kbc_set_input_port(board->shared_kbc,
                board->kbc_input_port);
        }
    }
    core_machine_dma_reset(board->shared_dma);
    if (board->rtc_cmos_configured) x86_rtc_reset(board->shared_rtc);
    core_machine_at_parity_reset(board->planar_parity);
    board->speaker_output = LIB_FALSE;
    board->speaker_lines = 0u;
    if (board->profile_binding.reset != LIB_NULL)
        board->profile_binding.reset(board->profile_binding.context,
            CORE_MACHINE_BOARD_RESET_PORT_LATCHES);
    core_machine_fdc_reset(board->fdc);
    core_machine_hdc_reset(board->hdc);
    core_machine_pic_reset(board->shared_pic_master,
        board->shared_pic_slave);
    x86_pit_reset(board->shared_pit);
    if (board->auxiliary_pit_configured) {
        x86_pit_reset(board->auxiliary_pit);
    }
    core_machine_board_after_pit_reset(board);
    if (board->profile_binding.reset != LIB_NULL)
        board->profile_binding.reset(board->profile_binding.context,
            CORE_MACHINE_BOARD_RESET_FINAL_REFRESH);
    core_machine_vadp_reset(board->shared_vadp);
}

void core_machine_board_finalize_devices(void *owner)
{
    core_machine_board_state *board = owner;
    if (board == LIB_NULL) return;
    core_machine_at_parity_destroy(board->planar_parity);
    if (board->profile_binding.finalize != LIB_NULL)
        board->profile_binding.finalize(board->profile_binding.context);
    /* Input finalizers may still drive PIT speaker gates and PIC IRQ lines. */
    if (board->keyboard_topology == CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI) {
        x86_xt_keyboard_destroy(board->xt_keyboard);
        core_machine_xt_ppi_keyboard_destroy(board->xt_ppi_keyboard);
    } else core_machine_kbc_destroy(board->shared_kbc);
    x86_pit_destroy(board->shared_pit);
    x86_pit_destroy(board->auxiliary_pit);
    core_machine_hdc_destroy(board->hdc);
    core_machine_fdc_destroy(board->fdc);
    core_machine_dma_finalize(board->shared_dma);
    x86_rtc_destroy(board->shared_rtc);
    core_machine_pic_finalize(board->shared_pic_master,
        board->shared_pic_slave);
    core_machine_vadp_destroy(board->shared_vadp);
    lib_release(board);
}

void core_machine_board_configure_xt_ppi_speaker(core_machine_board_state *board)
{
    if (board == LIB_NULL) return;
    board->xt_ppi_speaker_configured = LIB_TRUE;
    x86_pit_set_output(board->shared_pit, 2u,
        core_machine_speaker_timer_output, board);
    core_machine_board_set_xt_ppi_speaker(board, LIB_FALSE, LIB_FALSE);
}

void core_machine_board_set_xt_ppi_speaker(core_machine_board_state *board,
    lib_u8 timer_gate, lib_u8 data_enabled)
{
    if (board == LIB_NULL || !board->xt_ppi_speaker_configured) return;
    core_machine_speaker_set_gate(board,
        (timer_gate ? 0x01u : 0u) | (data_enabled ? 0x02u : 0u));
}

void core_machine_board_after_pit_reset(core_machine_board_state *board)
{
    if (board == LIB_NULL) return;
    if (board->dma_configured && !board->profile_binding.owns_refresh_output) {
        x86_pit_set_output(board->shared_pit, 1u,
            core_machine_dma_refresh_pit_output, board);
    }
    if (board->planar_parity != LIB_NULL) {
        core_machine_pc_at_refresh_timer_program(board);
    }
    if (board->planar_parity != LIB_NULL) {
        core_machine_speaker_set_gate(board,
            board->speaker_lines);
    }
    if (board->profile_binding.reset != LIB_NULL)
        board->profile_binding.reset(board->profile_binding.context,
            CORE_MACHINE_BOARD_RESET_AFTER_PIT);
}

void core_machine_board_refresh_nmi(void *owner)
{
    core_machine_board_state *board = owner;
    if (board == LIB_NULL) return;
    if (board->keyboard_topology ==
            CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI) {
        core_machine_xt_ppi_keyboard_refresh_nmi(board->xt_ppi_keyboard);
    }
    core_machine_at_parity_refresh_nmi(board->planar_parity);
    if (board->profile_binding.refresh_nmi != LIB_NULL)
        board->profile_binding.refresh_nmi(board->profile_binding.context);
}







static void core_machine_fdc_dma_request_assert(void *owner,
    const core_machine_dma_request_binding *binding)
{
    core_machine_board_state *board = owner;

    if (board == LIB_NULL || binding == LIB_NULL ||
        binding->core_token != board->fdc_dma_request.core_token ||
        binding->channel != board->fdc_dma_request.channel) return;
    core_machine_dma_request_assert(board->shared_dma, binding);
}

static void core_machine_fdc_dma_request_deassert(void *owner,
    const core_machine_dma_request_binding *binding)
{
    core_machine_board_state *board = owner;

    if (board == LIB_NULL || binding == LIB_NULL ||
        binding->core_token != board->fdc_dma_request.core_token ||
        binding->channel != board->fdc_dma_request.channel) return;
    core_machine_dma_request_deassert(board->shared_dma, binding);
}

static void core_machine_hdc_dma_request_assert(void *owner,
    const core_machine_dma_request_binding *binding)
{
    core_machine_board_state *board = owner;

    if (board == LIB_NULL || binding == LIB_NULL ||
        binding->core_token != board->hdc_dma_request.core_token ||
        binding->channel != board->hdc_dma_request.channel) return;
    core_machine_dma_request_assert(board->shared_dma, binding);
}

static void core_machine_hdc_dma_request_deassert(void *owner,
    const core_machine_dma_request_binding *binding)
{
    core_machine_board_state *board = owner;

    if (board == LIB_NULL || binding == LIB_NULL ||
        binding->core_token != board->hdc_dma_request.core_token ||
        binding->channel != board->hdc_dma_request.channel) return;
    core_machine_dma_request_deassert(board->shared_dma, binding);
}

static void core_machine_dma_refresh_pit_output(void *owner, lib_u8 asserted)
{
    core_machine_board_state *board = owner;

    if (board == LIB_NULL) return;
    if (asserted) {
        core_machine_dma_request_deassert(board->shared_dma, &board->refresh_dma_request);
    } else {
        core_machine_dma_request_assert(board->shared_dma, &board->refresh_dma_request);
    }
}

static const core_machine_dma_channel_provider core_machine_dma_refresh_provider = {
    LIB_NULL, LIB_NULL, LIB_NULL
};

static lib_u8 core_machine_dma_wiring_is_valid(
    const core_machine_dma_wiring *wiring)
{
    return wiring != LIB_NULL &&
        (wiring->fdc_channel == CORE_MACHINE_DMA_FDC_CHANNEL_UNBOUND ||
         (((wiring->controller_count == 1u && wiring->cascade_channel == 0u &&
            wiring->fdc_channel < 4u) ||
           (wiring->controller_count == CORE_MACHINE_DMA_CONTROLLER_COUNT &&
            wiring->cascade_channel == CORE_MACHINE_DMA_CASCADE_CHANNEL &&
            wiring->fdc_channel < 4u)) &&
          wiring->fdc_channel != 0u));
}

lib_status core_machine_configure_dma(core_machine_board_state *board,
    const core_machine_dma_wiring *wiring,
    core_machine_dma_request_binding *out_fdc_request)
{
    lib_status status;

    if (board == LIB_NULL || !core_machine_configuration_is_open(board->core) ||
        board->dma_configured) {
        return LIB_STATUS_INVALID_STATE;
    }
    if (!core_machine_dma_wiring_is_valid(wiring) || out_fdc_request == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    board->fdc_dma_request = (core_machine_dma_request_binding) {0};
    if (wiring->fdc_channel != CORE_MACHINE_DMA_FDC_CHANNEL_UNBOUND) {
        status = core_machine_dma_bind_channel(board->shared_dma,
            wiring->fdc_channel, core_machine_fdc_dma_provider(), board->fdc,
            &board->fdc_dma_request);
        if (status != LIB_STATUS_OK) return status;
    }
    status = core_machine_dma_bind_channel(board->shared_dma, 0u,
        &core_machine_dma_refresh_provider, board, &board->refresh_dma_request);
    if (status != LIB_STATUS_OK) return status;
    x86_pit_set_output(board->shared_pit, 1u,
        core_machine_dma_refresh_pit_output, board);
    board->dma_wiring = *wiring;
    board->dma_configured = LIB_TRUE;
    *out_fdc_request = board->fdc_dma_request;
    return LIB_STATUS_OK;
}

lib_status core_machine_get_fdc_dma_request_binding(const core_machine_board_state *board,
    core_machine_dma_request_binding *out_binding)
{
    if (board == LIB_NULL || out_binding == LIB_NULL || !board->dma_configured ||
        board->fdc_dma_request.core_token == 0u) {
        return LIB_STATUS_INVALID_STATE;
    }
    *out_binding = board->fdc_dma_request;
    return LIB_STATUS_OK;
}

static void core_machine_rtc_irq_output(void *context, lib_bool asserted)
{
    core_machine_pic_irq_source *source = context;
    if (asserted) core_machine_pic_irq_source_assert(source);
    else core_machine_pic_irq_source_deassert(source);
}

lib_status core_machine_configure_rtc_cmos(core_machine_board_state *board,
    const core_machine_rtc_cmos_config *config)
{
    x86_rtc_config rtc_config;
    core_machine_port_route routes[2];
    lib_status status;
    lib_size index;

    if (board == LIB_NULL || !core_machine_configuration_is_open(board->core) ||
        board->rtc_cmos_configured) {
        return LIB_STATUS_INVALID_STATE;
    }
    if (!core_machine_rtc_cmos_config_is_valid(config)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    rtc_config.ticks_per_second = config->ticks_per_second;
    rtc_config.uip_lead_ticks = config->timing.uip_lead_ticks;
    rtc_config.update_ticks = config->timing.update_ticks;
    status = core_machine_pic_irq_source_bind(&board->rtc_irq_source,
        board->shared_pic_master, board->shared_pic_slave, config->irq);
    if (status != LIB_STATUS_OK) return status;
    status = x86_rtc_create(&rtc_config, core_machine_rtc_irq_output,
        board->rtc_irq_source, &board->shared_rtc);
    if (status != LIB_STATUS_OK) return status;
    routes[0] = (core_machine_port_route) {
        .address = config->index_port,
        .write = core_machine_rtc_cmos_port_write, .owner = board
    };
    routes[1] = (core_machine_port_route) {
        .address = config->data_port,
        .read = core_machine_rtc_cmos_port_read,
        .write = core_machine_rtc_cmos_port_write, .owner = board
    };
    status = core_machine_install_port_routes(board->core, routes, 2u);
    if (status != LIB_STATUS_OK) {
        x86_rtc_destroy(board->shared_rtc);
        board->shared_rtc = LIB_NULL;
        return status;
    }
    for (index = 0u; index < config->default_count; ++index) {
        if (config->defaults[index].index <= X86_RTC_REG_D) continue;
        x86_rtc_write_register(board->shared_rtc,
            config->defaults[index].index, config->defaults[index].value);
    }
    if (config->derive_configuration_checksum) {
        lib_u16 checksum = 0u;

        /* The selected board owns a frozen CMOS image.  MC146818-compatible
         * firmware validates the complete configuration range, so derive its
         * checksum here after every configured byte has its sole owner value. */
        for (index = 0x10u; index < 0x2eu; ++index) {
            checksum = (lib_u16)(checksum +
                x86_rtc_read_register(board->shared_rtc, (lib_u8)index));
        }
        x86_rtc_write_register(board->shared_rtc, 0x2eu,
            CORE_MACHINE_MASK_U8(checksum >> 8u));
        x86_rtc_write_register(board->shared_rtc, 0x2fu,
            CORE_MACHINE_MASK_U8(checksum));
    }
    board->rtc_cmos_config = *config;
    board->rtc_cmos_configured = LIB_TRUE;
    return LIB_STATUS_OK;
}

lib_status core_machine_configure_planar_parity(core_machine_board_state *board,
    const core_machine_planar_parity_config *config)
{
    lib_status status;

    if (board == LIB_NULL || !core_machine_configuration_is_open(board->core) || board->planar_parity != LIB_NULL)
        return LIB_STATUS_INVALID_STATE;
    if (board->profile_binding.context != LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = core_machine_at_parity_create(board->core, board->shared_pit, config,
        core_machine_at_speaker_output, board, &board->planar_parity);
    if (status != LIB_STATUS_OK) return status;
    x86_pit_set_output(board->shared_pit, 2u,
        core_machine_speaker_timer_output, board);
    core_machine_pc_at_refresh_timer_program(board);
    core_machine_speaker_set_gate(board, board->speaker_lines);
    return LIB_STATUS_OK;
}

lib_status core_machine_report_planar_parity_fault(core_machine_board_state *board)
{
    return board == LIB_NULL ? LIB_STATUS_INVALID_STATE :
        core_machine_at_parity_report_fault(board->planar_parity);
}

static lib_status core_machine_absent_memory_read(void *owner,
    lib_u32 physical, lib_uptr destination,
    lib_uptr bytes, lib_bool observe_only)
{
    const core_machine_absent_memory *absent =
        (const core_machine_absent_memory *)owner;

    (void)physical;
    (void)observe_only;
    if (absent == LIB_NULL || !absent->configured || destination == 0u ||
        bytes == 0u) return LIB_STATUS_INTERNAL_ERROR;
    lib_memory_set((void *)destination, absent->config.read_value, bytes);
    return LIB_STATUS_OK;
}

static lib_status core_machine_absent_memory_write(void *owner,
    lib_u32 physical, lib_uptr source,
    lib_uptr bytes)
{
    const core_machine_absent_memory *absent =
        (const core_machine_absent_memory *)owner;

    (void)physical;
    if (absent == LIB_NULL || !absent->configured || source == 0u ||
        bytes == 0u) return LIB_STATUS_INTERNAL_ERROR;
    return LIB_STATUS_OK;
}

static lib_status core_machine_absent_memory_query(void *owner,
    lib_u32 physical, lib_uptr bytes,
    core_machine_memory_access access)
{
    const core_machine_absent_memory *absent =
        (const core_machine_absent_memory *)owner;

    (void)physical;
    if (absent == LIB_NULL || !absent->configured || bytes == 0u ||
        (access != CORE_MACHINE_MEMORY_ACCESS_READ &&
        access != CORE_MACHINE_MEMORY_ACCESS_WRITE)) return LIB_STATUS_INTERNAL_ERROR;
    return LIB_STATUS_OK;
}

lib_bool core_machine_board_refresh_request(void *owner, lib_u8 *out_address)
{
    const core_machine_board_state *board = owner;
    return board != LIB_NULL && board->profile_binding.refresh_request != LIB_NULL &&
        board->profile_binding.refresh_request(board->profile_binding.context, out_address);
}

void core_machine_board_refresh_complete(void *owner)
{
    core_machine_board_state *board = owner;
    if (board != LIB_NULL && board->profile_binding.refresh_complete != LIB_NULL)
        board->profile_binding.refresh_complete(board->profile_binding.context);
}









lib_status core_machine_get_speaker_observation(const core_machine_board_state *board,
    core_machine_speaker_observation *out_observation)
{
    lib_u8 value;

    if (board == LIB_NULL || out_observation == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    value = core_machine_speaker_source_value(board);
    out_observation->configured = board->xt_ppi_speaker_configured ||
        board->profile_binding.context != LIB_NULL || board->planar_parity != LIB_NULL;
    out_observation->timer_gate = (value & 0x01u) != 0u;
    out_observation->data_enabled = (value & 0x02u) != 0u;
    out_observation->timer_output = x86_pit_get_output(
        board->shared_pit, 2u);
    out_observation->output = board->speaker_output;
    return LIB_STATUS_OK;
}
lib_status core_machine_configure_absent_memory(core_machine_board_state *board,
    const core_machine_absent_memory_config *config)
{
    core_machine_absent_memory *absent;
    core_machine_memory_device_route route;
    lib_status status;
    lib_uptr index;

    if (board == LIB_NULL || !core_machine_configuration_is_open(board->core)) return LIB_STATUS_INVALID_STATE;
    if (config == LIB_NULL || config->bytes == 0u ||
        (lib_u64)config->physical_start + config->bytes >
            (lib_u64)LIB_UINT32_MAX + 1u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    absent = LIB_NULL;
    for (index = 0u; index < CORE_MACHINE_ABSENT_MEMORY_WINDOW_COUNT; ++index) {
        if (!board->absent_memory[index].configured) {
            absent = &board->absent_memory[index];
            break;
        }
    }
    if (absent == LIB_NULL) return LIB_STATUS_INVALID_STATE;
    absent->config = *config;
    absent->configured = LIB_TRUE;
    /* An unpopulated board window is a fallback, not an installed device:
     * a dynamically decoded video aperture may own part of the same physical
     * range while the remaining addresses still read as open bus. */
    route = (core_machine_memory_device_route) {
        config->physical_start, config->bytes,
        {core_machine_absent_memory_read, core_machine_absent_memory_write,
            core_machine_absent_memory_query},
        CORE_MACHINE_MEMORY_PROVIDER_FALLBACK
    };
    status = core_machine_install_memory_device_routes(board->core, &route, 1u,
        LIB_NULL, LIB_NULL, absent);
    if (status != LIB_STATUS_OK) {
        lib_memory_set(absent, 0, sizeof(*absent));
        return status;
    }
    return LIB_STATUS_OK;
}

lib_status core_machine_get_planar_parity_observation(const core_machine_board_state *board,
    core_machine_planar_parity_observation *out_observation)
{
    if (board == LIB_NULL || out_observation == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    core_machine_at_parity_observe(board->planar_parity, out_observation);
    return LIB_STATUS_OK;
}

static lib_i32 core_machine_fdc_topology_is_valid(
    const core_machine_fdc_topology *topology)
{
    lib_size first;
    lib_size second;

    if (topology == LIB_NULL || topology->media_registry == LIB_NULL ||
        topology->config.dma_channel != topology->dma_request.channel ||
        (topology->config.ready_mask & (lib_u8)~((1u <<
            CORE_MACHINE_FDC_DRIVE_COUNT) - 1u)) != 0u) {
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

static lib_size core_machine_hdc_port_addresses(
    const core_machine_hdc_config *config, lib_u16 ports[9])
{
    if (config->protocol == CORE_MACHINE_HDC_PROTOCOL_XEBEC_XT) {
        ports[0] = config->bus.xebec.data_port;
        ports[1] = config->bus.xebec.hardware_status_reset_port;
        ports[2] = config->bus.xebec.jumpers_select_port;
        ports[3] = config->bus.xebec.dma_irq_mask_port;
        return 4u;
    }
    ports[0] = config->bus.task_file.data_port;
    ports[1] = config->bus.task_file.error_features_port;
    ports[2] = config->bus.task_file.sector_count_port;
    ports[3] = config->bus.task_file.sector_number_port;
    ports[4] = config->bus.task_file.cylinder_low_port;
    ports[5] = config->bus.task_file.cylinder_high_port;
    ports[6] = config->bus.task_file.drive_head_port;
    ports[7] = config->bus.task_file.status_command_port;
    ports[8] = config->bus.task_file.alternate_status_device_control_port;
    return 9u;
}

static lib_i32 core_machine_hdc_topology_is_valid(
    const core_machine_hdc_topology *topology)
{
    const core_machine_hdc_config *config;
    lib_u16 ports[9];
    lib_size port_count;
    lib_size first;
    lib_size second;

    if (topology == LIB_NULL || topology->media_registry == LIB_NULL ||
        topology->media_id == CORE_MACHINE_MEDIA_ID_INVALID ||
        topology->slave_media_id == topology->media_id) return 0;
    config = &topology->config;
    port_count = core_machine_hdc_port_addresses(config, ports);
    if (config->protocol == CORE_MACHINE_HDC_PROTOCOL_XEBEC_XT) {
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
        if (config->bus.task_file.lba28_supported != LIB_FALSE &&
            config->bus.task_file.lba28_supported != LIB_TRUE) return 0;
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

lib_status core_machine_configure_fdc(core_machine_board_state *board,
    const core_machine_fdc_topology *topology)
{
    lib_status status;

    if (board == LIB_NULL || !core_machine_configuration_is_open(board->core) ||
        !board->dma_configured || board->fdc_configured) {
        return LIB_STATUS_INVALID_STATE;
    }
    if (!core_machine_fdc_topology_is_valid(topology) ||
        topology->dma_request.core_token != board->fdc_dma_request.core_token ||
        topology->dma_request.channel != board->fdc_dma_request.channel) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    board->fdc_topology = *topology;
    status = core_machine_fdc_configure(board->fdc, board->fdc_topology.media_registry,
        &board->fdc_topology.drives, &board->fdc_topology.dma_request,
        core_machine_fdc_dma_request_assert,
        core_machine_fdc_dma_request_deassert, board,
        board->shared_pic_master, board->shared_pic_slave,
        board->core, &board->fdc_topology.config,
        &board->fdc_topology.observation_provider);
    if (status != LIB_STATUS_OK) {
        lib_memory_set(&board->fdc_topology, 0u,
            sizeof(board->fdc_topology));
        return status;
    }
    board->fdc_configured = LIB_TRUE;
    return LIB_STATUS_OK;
}

lib_status core_machine_configure_hdc(core_machine_board_state *board,
    const core_machine_hdc_topology *topology)
{
    const core_machine_port_provider *provider;
    core_machine_port_route routes[10];
    lib_u16 ports[9];
    lib_size port_count;
    lib_size index;
    lib_status status;
    lib_bool xebec;

    if (board == LIB_NULL || !core_machine_configuration_is_open(board->core) ||
        board->hdc_configured) {
        return LIB_STATUS_INVALID_STATE;
    }
    if (!core_machine_hdc_topology_is_valid(topology)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (topology->config.protocol == CORE_MACHINE_HDC_PROTOCOL_COMPAQ_WD_40MB &&
        (!board->fdc_configured ||
            topology->config.bus.task_file.drive_address_port !=
                board->fdc_topology.config.direction_port)) return LIB_STATUS_INVALID_STATE;
    xebec = topology->config.protocol == CORE_MACHINE_HDC_PROTOCOL_XEBEC_XT;
    if (xebec && !board->dma_configured) return LIB_STATUS_INVALID_STATE;
    provider = core_machine_hdc_port_provider();
    if (provider == LIB_NULL) return LIB_STATUS_INTERNAL_ERROR;
    port_count = core_machine_hdc_port_addresses(&topology->config, ports);
    for (index = 0u; index < port_count; ++index) {
        routes[index] = (core_machine_port_route) {ports[index],
            xebec && index == 3u ? LIB_NULL : provider->read,
            provider->write, board->hdc, LIB_FALSE, 0u};
    }
    if (topology->config.protocol == CORE_MACHINE_HDC_PROTOCOL_COMPAQ_WD_40MB) {
        routes[port_count++] = (core_machine_port_route) {
            .address = topology->config.bus.task_file.drive_address_port,
            .read = provider->read, .owner = board->hdc,
            .wired_or_read = LIB_TRUE};
    }
    board->hdc_topology = *topology;
    status = core_machine_hdc_configure(board->hdc, board->hdc_topology.media_registry,
        board->hdc_topology.media_id, board->hdc_topology.slave_media_id,
        board->shared_pic_master,
        board->shared_pic_slave, &board->hdc_topology.config);
    if (status == LIB_STATUS_OK)
        status = core_machine_install_port_routes(board->core, routes, port_count);
    if (status == LIB_STATUS_OK && xebec) {
        status = core_machine_dma_bind_channel(board->shared_dma,
            board->hdc_topology.config.bus.xebec.dma_channel,
            core_machine_hdc_dma_provider(), board->hdc, &board->hdc_dma_request);
        if (status != LIB_STATUS_OK) {
            lib_status rollback = core_machine_remove_port_routes(board->core, board->hdc);

            if (rollback != LIB_STATUS_OK) status = rollback;
        }
    }
    if (status != LIB_STATUS_OK) {
        core_machine_hdc_clear_configuration(board->hdc);
        lib_memory_set(&board->hdc_topology, 0u,
            sizeof(board->hdc_topology));
        return status;
    }
    if (xebec) {
        core_machine_hdc_bind_dma_request(board->hdc, &board->hdc_dma_request,
            core_machine_hdc_dma_request_assert, core_machine_hdc_dma_request_deassert,
            board);
    }
    board->hdc_configured = LIB_TRUE;
    return LIB_STATUS_OK;
}
