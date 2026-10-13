#include "app-mydeskpro386/machine/d4_platform.h"
#include "core/board-base/machine_board_interface.h"

static lib_status d4_port_read(void *owner, lib_u16 port, lib_u64 tick,
    lib_u32 *out_value);
static lib_status d4_port_write(void *owner, lib_u16 port, lib_u32 value);
static void d4_refresh_output(void *owner, lib_u8 asserted);
static void d4_failsafe_output(void *owner, lib_u8 asserted);

#define D4_KBC_COMMAND_ENABLE_SPEED 0xa3u
#define D4_KBC_COMMAND_TOGGLE_SPEED 0xa4u
#define D4_KBC_COMMAND_SPECIAL_READ 0xa5u
#define D4_KBC_COMMAND_IRQ1 0x01u
#define D4_KBC_SPECIAL_KEYBOARD_11_BIT 0x20u
#define D4_KBC_SPECIAL_OBF_IRQ_ENABLED 0x10u

lib_status core_machine_d4_platform_create(core_machine *core, x86_pit *pit,
    x86_pit *auxiliary_pit, const core_machine_d4_platform_config *config,
    core_machine_d4_speaker_output speaker, void *context,
    core_machine_d4_platform **out_platform)
{
    core_machine_d4_platform *platform;
    core_machine_port_route route;
    lib_status status;

    if (out_platform == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_platform = LIB_NULL;
    if (core == LIB_NULL || !core_machine_configuration_is_open(core))
        return LIB_STATUS_INVALID_STATE;
    if (pit == LIB_NULL || auxiliary_pit == LIB_NULL || config == LIB_NULL ||
        config->port != 0x61u || config->failsafe_pit_counter >= 3u ||
        config->speed_pit_counter >= 3u ||
        config->failsafe_pit_counter == config->speed_pit_counter ||
        config->speed_pit_count == 0u || config->speed_pit_count > 0x38u ||
        speaker == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    platform = lib_allocate(sizeof(*platform));
    if (platform == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    lib_memory_set(platform, 0, sizeof(*platform));
    platform->core = core;
    platform->shared_pit = pit;
    platform->auxiliary_pit = auxiliary_pit;
    platform->d4_platform_config = *config;
    platform->speaker = speaker;
    platform->speaker_context = context;
    route = (core_machine_port_route) {
        .address = config->port, .read = d4_port_read,
        .write = d4_port_write, .owner = platform
    };
    status = core_machine_install_port_routes(core, &route, 1u);
    if (status != LIB_STATUS_OK) {
        lib_release(platform);
        return status == LIB_STATUS_INVALID_STATE ? LIB_STATUS_INVALID_ARGUMENT : status;
    }
    core_machine_d4_platform_reset_latches(platform);
    *out_platform = platform;
    return LIB_STATUS_OK;
}

void core_machine_d4_platform_destroy(core_machine_d4_platform *platform)
{
    if (platform == LIB_NULL) return;
    if (platform->memory.configured)
        (void)core_machine_remove_memory_device_routes(platform->core, &platform->memory);
    if (platform->outputs_bound) {
        x86_pit_set_output(platform->shared_pit, 1u, LIB_NULL, LIB_NULL);
        x86_pit_set_output(platform->auxiliary_pit,
            platform->d4_platform_config.failsafe_pit_counter, LIB_NULL, LIB_NULL);
    }
    (void)core_machine_remove_port_routes(platform->core, platform);
    lib_release(platform);
}

lib_status core_machine_d4_platform_configure_memory(core_machine_d4_platform *platform,
    const core_machine_d4_memory_config *config)
{
    if (platform == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return core_machine_d4_memory_configure(platform->core, &platform->memory,
        config, core_machine_d4_platform_iochk_output, platform);
}

static void d4_board_reset(void *owner, core_machine_board_reset_phase phase)
{
    core_machine_d4_platform *platform = owner;

    switch (phase) {
    case CORE_MACHINE_BOARD_RESET_BEFORE_DEVICES:
        core_machine_d4_memory_reset(&platform->memory);
        break;
    case CORE_MACHINE_BOARD_RESET_PORT_LATCHES:
        core_machine_d4_platform_reset_latches(platform);
        break;
    case CORE_MACHINE_BOARD_RESET_AFTER_PIT:
        core_machine_d4_platform_after_pit_reset(platform);
        break;
    case CORE_MACHINE_BOARD_RESET_FINAL_REFRESH:
        core_machine_d4_platform_reset_refresh(platform);
        break;
    }
}

static void d4_board_refresh_nmi(void *owner)
{
    core_machine_d4_platform_refresh_nmi(owner);
}

static lib_bool d4_board_next_deadline(void *owner, lib_u64 now, lib_u64 *out_tick)
{
    lib_u8 address;

    if (!core_machine_d4_platform_refresh_request(owner, &address)) return LIB_FALSE;
    /* Existing L2 refresh service boundary, on the sole Core source axis. */
    *out_tick = now + 1u;
    return LIB_TRUE;
}

static void d4_board_finalize(void *owner)
{
    core_machine_d4_platform_destroy(owner);
}

core_machine_board_profile_binding core_machine_d4_platform_binding(
    core_machine_d4_platform *platform)
{
    return (core_machine_board_profile_binding) {
        .context = platform, .reset = d4_board_reset,
        .refresh_nmi = d4_board_refresh_nmi,
        .refresh_request = core_machine_d4_platform_refresh_request,
        .refresh_complete = core_machine_d4_platform_refresh_complete,
        .cpu_hold_asserted = core_machine_d4_platform_cpu_hold_asserted,
        .next_deadline = d4_board_next_deadline, .finalize = d4_board_finalize,
        .owns_refresh_output = LIB_TRUE, .shutdown_resets = LIB_TRUE
    };
}

void core_machine_d4_platform_reset_latches(core_machine_d4_platform *platform)
{
    if (platform == LIB_NULL) return;
    platform->d4_platform_port_b = 0x0fu;
    platform->d4_platform_iochk_latched = LIB_FALSE;
    platform->d4_platform_failsafe_latched = LIB_FALSE;
    platform->d4_platform_nmi_signaled = LIB_FALSE;
}

void core_machine_d4_platform_after_pit_reset(core_machine_d4_platform *platform)
{
    if (platform == LIB_NULL) return;
    (void)x86_pit_write_register(platform->shared_pit, 3u, 0x74u);
    (void)x86_pit_write_register(platform->shared_pit, 1u, 18u);
    (void)x86_pit_write_register(platform->shared_pit, 1u, 0u);
    (void)x86_pit_write_register(platform->auxiliary_pit, 3u, 0xb2u);
    (void)x86_pit_write_register(platform->auxiliary_pit,
        platform->d4_platform_config.speed_pit_counter,
        platform->d4_platform_config.speed_pit_count);
    (void)x86_pit_write_register(platform->auxiliary_pit,
        platform->d4_platform_config.speed_pit_counter, 0u);
    x86_pit_set_gate(platform->auxiliary_pit,
        platform->d4_platform_config.speed_pit_counter, LIB_FALSE);
    platform->speaker(platform->speaker_context, platform->d4_platform_port_b);
    x86_pit_set_output(platform->shared_pit, 1u, d4_refresh_output, platform);
    x86_pit_set_output(platform->auxiliary_pit,
        platform->d4_platform_config.failsafe_pit_counter,
        d4_failsafe_output, platform);
    platform->outputs_bound = LIB_TRUE;
}

void core_machine_d4_platform_reset_refresh(core_machine_d4_platform *platform)
{
    if (platform == LIB_NULL) return;
    platform->d4_refresh_hold_pending = LIB_FALSE;
    platform->d4_refresh_pulse_active = LIB_FALSE;
    platform->d4_refresh_address = 0u;
    platform->d4_speed_control_enabled = LIB_FALSE;
    platform->d4_speed_switch_selected = LIB_FALSE;
}

static lib_u8 d4_timer_status(
    const core_machine_d4_platform *platform, lib_u64 tick)
{
    lib_u8 value = 0u;
    (void)tick;

    if (platform == LIB_NULL) return 0u;
    if (x86_pit_get_output(platform->shared_pit, 1u)) value |= 0x10u;
    if (x86_pit_get_output(platform->shared_pit, 2u)) value |= 0x20u;
    return value;
}

static void d4_refresh_output(void *opaque, lib_u8 asserted)
{
    core_machine_d4_platform *platform = (core_machine_d4_platform *)opaque;
    /* The D4 counter-1 refresh pulse ends CPU-side locality.
     * D4 establishes this refresh topology, but not a physical page-retention
     * interval or any calibrated phase duration. */
    if (platform != LIB_NULL) {
        if (asserted) {
            platform->d4_refresh_pulse_active = LIB_FALSE;
        } else if (!platform->d4_refresh_pulse_active) {
            platform->d4_refresh_pulse_active = LIB_TRUE;
            core_machine_cpu_bus_refresh_pulse(platform->core);
            if (platform->d4_speed_control_enabled &&
                platform->d4_speed_switch_selected) {
                x86_pit_set_gate(platform->auxiliary_pit,
                    platform->d4_platform_config.speed_pit_counter, LIB_FALSE);
                x86_pit_set_gate(platform->auxiliary_pit,
                    platform->d4_platform_config.speed_pit_counter, LIB_TRUE);
            }
            platform->d4_refresh_hold_pending = LIB_TRUE;
        }
    }
}

lib_bool core_machine_d4_platform_cpu_hold_asserted(void *owner)
{
    const core_machine_d4_platform *platform = owner;

    return platform != LIB_NULL && platform->d4_speed_control_enabled &&
        platform->d4_speed_switch_selected &&
        !x86_pit_get_output(platform->auxiliary_pit,
            platform->d4_platform_config.speed_pit_counter);
}

x86_kbc8042_command_extension_result core_machine_d4_platform_kbc_command(
    void *owner, lib_u8 command, lib_u8 input_port, lib_u8 command_byte,
    lib_u8 *out_data)
{
    core_machine_d4_platform *platform = owner;

    if (platform == LIB_NULL || out_data == LIB_NULL)
        return X86_KBC8042_COMMAND_EXTENSION_UNHANDLED;
    switch (command) {
    case D4_KBC_COMMAND_ENABLE_SPEED:
        platform->d4_speed_control_enabled = LIB_TRUE;
        /* A3 selects software (COMMON) speed control.  A4 later chooses the
         * frozen switch-4 input.  This is the selected logical command model,
         * not a reconstructed 8042 firmware image. */
        platform->d4_speed_switch_selected = LIB_TRUE;
        return X86_KBC8042_COMMAND_EXTENSION_HANDLED;
    case D4_KBC_COMMAND_TOGGLE_SPEED:
        platform->d4_speed_switch_selected =
            platform->d4_speed_switch_selected ? LIB_FALSE : LIB_TRUE;
        if (platform->d4_speed_switch_selected)
            platform->d4_speed_control_enabled = (input_port & 0x10u) != 0u;
        return X86_KBC8042_COMMAND_EXTENSION_HANDLED;
    case D4_KBC_COMMAND_SPECIAL_READ:
        *out_data = (input_port & ~(D4_KBC_SPECIAL_KEYBOARD_11_BIT |
            D4_KBC_SPECIAL_OBF_IRQ_ENABLED)) |
            D4_KBC_SPECIAL_KEYBOARD_11_BIT |
            ((command_byte & D4_KBC_COMMAND_IRQ1) != 0u ?
                D4_KBC_SPECIAL_OBF_IRQ_ENABLED : 0u);
        return X86_KBC8042_COMMAND_EXTENSION_SPECIAL_READ;
    default:
        return X86_KBC8042_COMMAND_EXTENSION_UNHANDLED;
    }
}

lib_bool core_machine_d4_platform_refresh_request(void *owner, lib_u8 *out_address)
{
    const core_machine_d4_platform *platform = owner;
    if (platform == LIB_NULL || out_address == LIB_NULL ||
        !platform->d4_refresh_hold_pending) return LIB_FALSE;
    *out_address = platform->d4_refresh_address;
    return LIB_TRUE;
}

void core_machine_d4_platform_refresh_complete(void *owner)
{
    core_machine_d4_platform *platform = owner;
    if (platform == LIB_NULL || !platform->d4_refresh_hold_pending) return;
    platform->d4_refresh_address = (lib_u8)(platform->d4_refresh_address + 1u);
    platform->d4_refresh_hold_pending = LIB_FALSE;
}

void core_machine_d4_platform_refresh_nmi(core_machine_d4_platform *platform)
{
    lib_u8 pending;

    if (platform == LIB_NULL) return;
    pending = ((platform->d4_platform_port_b & 0x08u) == 0u &&
        platform->d4_platform_iochk_latched) ||
        ((platform->d4_platform_port_b & 0x04u) == 0u &&
        platform->d4_platform_failsafe_latched);
    if (pending && !platform->d4_platform_nmi_signaled &&
        core_machine_signal_nmi(platform->core)) {
        platform->d4_platform_nmi_signaled = LIB_TRUE;
    }
}

static void d4_failsafe_output(void *owner,
    lib_u8 asserted)
{
    core_machine_d4_platform *platform = (core_machine_d4_platform *)owner;

    if (platform == LIB_NULL || !asserted) return;
    platform->d4_platform_failsafe_latched = LIB_TRUE;
    core_machine_d4_platform_refresh_nmi(platform);
}

static lib_status d4_port_read(void *owner,
    lib_u16 port, lib_u64 tick,
    lib_u32 *out_value)
{
    core_machine_d4_platform *platform = (core_machine_d4_platform *)owner;

    if (platform == LIB_NULL || out_value == LIB_NULL ||
        port != platform->d4_platform_config.port) return LIB_STATUS_INVALID_ARGUMENT;
    *out_value = (lib_u32)(platform->d4_platform_port_b & 0x0fu) |
        d4_timer_status(platform, tick) |
        (platform->d4_platform_iochk_latched ? 0x40u : 0u) |
        (platform->d4_platform_failsafe_latched ? 0x80u : 0u);
    return LIB_STATUS_OK;
}

static lib_status d4_port_write(void *owner,
    lib_u16 port, lib_u32 value)
{
    core_machine_d4_platform *platform = (core_machine_d4_platform *)owner;

    if (platform == LIB_NULL ||
        port != platform->d4_platform_config.port) return LIB_STATUS_INVALID_ARGUMENT;
    platform->d4_platform_port_b = (lib_u8)value & 0x3fu;
    platform->speaker(platform->speaker_context, platform->d4_platform_port_b);
    /* DeskPro port 61h bits 3 and 2 disable IOCHK and RAM/fail-safe NMI.
     * A high pulse clears the corresponding latched status; this records the
     * bounded logical effect, not electrical pulse timing. */
    if ((platform->d4_platform_port_b & 0x08u) != 0u) {
        platform->d4_platform_iochk_latched = LIB_FALSE;
    }
    if ((platform->d4_platform_port_b & 0x04u) != 0u) {
        platform->d4_platform_failsafe_latched = LIB_FALSE;
    }
    if (!platform->d4_platform_iochk_latched &&
        !platform->d4_platform_failsafe_latched) {
        platform->d4_platform_nmi_signaled = LIB_FALSE;
    }
    core_machine_d4_platform_refresh_nmi(platform);
    return LIB_STATUS_OK;
}

lib_status core_machine_d4_platform_clear_iochk(core_machine_d4_platform *platform)
{
    if (platform == LIB_NULL || !core_machine_mutable_operation_is_allowed(platform->core))
        return LIB_STATUS_INVALID_STATE;
    platform->d4_platform_iochk_latched = LIB_FALSE;
    if (!platform->d4_platform_failsafe_latched) platform->d4_platform_nmi_signaled = LIB_FALSE;
    core_machine_d4_platform_refresh_nmi(platform);
    return LIB_STATUS_OK;
}

void core_machine_d4_platform_iochk_output(void *owner, lib_bool asserted)
{
    if (asserted) (void)core_machine_d4_platform_report_iochk(owner);
    else (void)core_machine_d4_platform_clear_iochk(owner);
}

lib_status core_machine_d4_platform_report_iochk(core_machine_d4_platform *platform)
{
    if (platform == LIB_NULL || !core_machine_mutable_operation_is_allowed(platform->core))
        return LIB_STATUS_INVALID_STATE;
    platform->d4_platform_iochk_latched = LIB_TRUE;
    core_machine_d4_platform_refresh_nmi(platform);
    return LIB_STATUS_OK;
}

lib_status core_machine_d4_platform_observe(const core_machine_d4_platform *platform,
    core_machine_d4_platform_observation *out_observation)
{
    if (out_observation == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (platform == LIB_NULL) {
        *out_observation = (core_machine_d4_platform_observation) {
            .iochk_enabled = LIB_TRUE, .failsafe_enabled = LIB_TRUE
        };
        return LIB_STATUS_OK;
    }
    out_observation->configured = LIB_TRUE;
    out_observation->iochk_enabled = (platform->d4_platform_port_b & 0x08u) == 0u;
    out_observation->failsafe_enabled =
        (platform->d4_platform_port_b & 0x04u) == 0u;
    out_observation->iochk_latched = platform->d4_platform_iochk_latched;
    out_observation->failsafe_latched = platform->d4_platform_failsafe_latched;
    out_observation->nmi_signaled = platform->d4_platform_nmi_signaled;
    out_observation->memory_configured = platform->memory.configured;
    out_observation->memory_control = platform->memory.control;
    out_observation->memory_ram_setup = platform->memory.ram_setup;
    return LIB_STATUS_OK;
}

typedef struct model40_d4_construction {
    const core_machine_d4_platform_config *config;
    core_machine_d4_platform *candidate;
} model40_d4_construction;

static lib_status model40_d4_factory(const core_machine_board_profile_services *services,
    void *context, core_machine_board_profile_binding *out_binding)
{
    model40_d4_construction *construction = context;
    lib_status status = core_machine_d4_platform_create(services->core, services->pit,
        services->auxiliary_pit, construction->config, services->speaker,
        services->speaker_context, &construction->candidate);

    if (status == LIB_STATUS_OK)
        *out_binding = core_machine_d4_platform_binding(construction->candidate);
    return status;
}

lib_status core_machine_d4_platform_attach(core_machine_board_state *board,
    const core_machine_d4_platform_config *config,
    core_machine_d4_platform **out_platform)
{
    model40_d4_construction construction = {config, LIB_NULL};
    lib_status status;

    if (out_platform == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_platform = LIB_NULL;
    status = core_machine_board_construct_profile(board, model40_d4_factory, &construction);
    if (status == LIB_STATUS_OK) *out_platform = construction.candidate;
    return status;
}
