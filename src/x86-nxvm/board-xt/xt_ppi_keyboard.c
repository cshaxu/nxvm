#include "lib/types/types_interface.h"

#include "x86-nxvm/board-xt/xt_ppi_keyboard.h"

lib_status x86_nxvm_board_xt_ppi_keyboard_create(
    const x86_nxvm_board_xt_ppi_keyboard_config *config, x86_core_machine *machine,
    x86_nxvm_board_xt_ppi_keyboard **out_keyboard)
{
    x86_nxvm_board_xt_ppi_keyboard *keyboard;
    lib_status status;
    if (out_keyboard == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_keyboard = LIB_NULL;
    if (machine == LIB_NULL || !x86_nxvm_board_xt_ppi_keyboard_config_is_valid(config))
        return LIB_STATUS_INVALID_ARGUMENT;
    keyboard = lib_allocate_zero(1u, sizeof(*keyboard));
    if (keyboard == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    status = x86_nxvm_board_xt_ppi_keyboard_initialize(keyboard, config, machine);
    if (status != LIB_STATUS_OK) {
        lib_release(keyboard);
        return status;
    }
    *out_keyboard = keyboard;
    return LIB_STATUS_OK;
}

void x86_nxvm_board_xt_ppi_keyboard_destroy(x86_nxvm_board_xt_ppi_keyboard *keyboard)
{
    x86_nxvm_board_xt_ppi_keyboard_finalize(keyboard);
    lib_release(keyboard);
}

#define X86_NXVM_BOARD_XT_PPI_PORT_B_CLOCK_NOT_HELD 0x40u
#define X86_NXVM_BOARD_XT_PPI_PORT_B_CLEAR_KEYBOARD 0x80u
#define X86_NXVM_BOARD_XT_PPI_PORT_B_DISABLE_RAM_PARITY 0x10u
#define X86_NXVM_BOARD_XT_PPI_PORT_B_DISABLE_IO_CHECK 0x20u
#define X86_NXVM_BOARD_XT_PPI_PORT_C_IO_CHECK 0x40u
#define X86_NXVM_BOARD_XT_PPI_PORT_C_TIMER_OUTPUT 0x20u
#define X86_NXVM_BOARD_XT_PPI_PORT_C_RAM_PARITY 0x80u

static x86_ppi8255_pins x86_nxvm_board_xt_ppi_port_b(
    const x86_nxvm_board_xt_ppi_keyboard *keyboard)
{
    x86_ppi8255_pins pins = {0u, 0u};
    if (keyboard != LIB_NULL) x86_ppi8255_output(keyboard->ppi, 1u, &pins);
    return pins;
}

static lib_u8 x86_nxvm_board_xt_ppi_keyboard_port_c_input(
    const x86_nxvm_board_xt_ppi_keyboard *keyboard)
{
    lib_u8 value = (x86_nxvm_board_xt_ppi_port_b(keyboard).latch & 0x08u) != 0u ?
        keyboard->config.switches_high & 0x0fu : keyboard->config.switches_low & 0x0fu;
    if (keyboard->timer_output != LIB_NULL &&
        keyboard->timer_output(keyboard->timer_output_owner)) {
        value |= X86_NXVM_BOARD_XT_PPI_PORT_C_TIMER_OUTPUT;
    }
    if (keyboard->io_check_asserted) value |= X86_NXVM_BOARD_XT_PPI_PORT_C_IO_CHECK;
    if (keyboard->ram_parity_asserted) value |= X86_NXVM_BOARD_XT_PPI_PORT_C_RAM_PARITY;
    return value;
}

static lib_bool x86_nxvm_board_xt_ppi_keyboard_nmi_pending(
    const x86_nxvm_board_xt_ppi_keyboard *keyboard)
{
    x86_ppi8255_pins pins = x86_nxvm_board_xt_ppi_port_b(keyboard);
    if (pins.output_mask != 0xffu) return LIB_FALSE;
    return (keyboard->io_check_asserted &&
        (pins.latch & X86_NXVM_BOARD_XT_PPI_PORT_B_DISABLE_IO_CHECK) == 0u) ||
        (keyboard->ram_parity_asserted &&
        (pins.latch & X86_NXVM_BOARD_XT_PPI_PORT_B_DISABLE_RAM_PARITY) == 0u);
}

void x86_nxvm_board_xt_ppi_keyboard_refresh_nmi(x86_nxvm_board_xt_ppi_keyboard *keyboard)
{
    if (keyboard == LIB_NULL) return;
    if (!x86_nxvm_board_xt_ppi_keyboard_nmi_pending(keyboard)) {
        keyboard->nmi_signaled = LIB_FALSE;
    } else if (!keyboard->nmi_signaled && keyboard->nmi_request != LIB_NULL &&
        keyboard->nmi_request(keyboard->nmi_owner)) {
        keyboard->nmi_signaled = LIB_TRUE;
    }
}

static lib_bool x86_nxvm_board_xt_ppi_keyboard_delivery_enabled(
    const x86_nxvm_board_xt_ppi_keyboard *keyboard)
{
    x86_ppi8255_pins pins = x86_nxvm_board_xt_ppi_port_b(keyboard);
    return pins.output_mask == 0xffu &&
        (pins.latch & (X86_NXVM_BOARD_XT_PPI_PORT_B_CLOCK_NOT_HELD |
        X86_NXVM_BOARD_XT_PPI_PORT_B_CLEAR_KEYBOARD)) ==
            X86_NXVM_BOARD_XT_PPI_PORT_B_CLOCK_NOT_HELD;
}

static void x86_nxvm_board_xt_ppi_keyboard_publish_speaker(
    x86_nxvm_board_xt_ppi_keyboard *keyboard)
{
    x86_ppi8255_pins pins;
    if (keyboard == LIB_NULL || keyboard->speaker_update == LIB_NULL) return;
    pins = x86_nxvm_board_xt_ppi_port_b(keyboard);
    pins.latch &= pins.output_mask;
    keyboard->speaker_update(keyboard->speaker_owner,
        (pins.latch & 0x01u) != 0u, (pins.latch & 0x02u) != 0u);
}

static void x86_nxvm_board_xt_ppi_keyboard_deassert_irq(
    x86_nxvm_board_xt_ppi_keyboard *keyboard)
{
    if (keyboard == LIB_NULL || !keyboard->irq1_asserted) return;
    if (keyboard->irq != LIB_NULL) keyboard->irq(keyboard->irq_owner, LIB_FALSE);
    keyboard->irq1_asserted = LIB_FALSE;
}

static void x86_nxvm_board_xt_ppi_keyboard_publish_lines(x86_nxvm_board_xt_ppi_keyboard *keyboard)
{
    x86_ppi8255_pins pins;
    if (keyboard->line_observer == LIB_NULL) return;
    pins = x86_nxvm_board_xt_ppi_port_b(keyboard);
    pins.latch &= pins.output_mask;
    keyboard->line_observer(keyboard->line_observer_owner,
        (pins.latch & X86_NXVM_BOARD_XT_PPI_PORT_B_CLOCK_NOT_HELD) == 0u,
        (pins.latch & X86_NXVM_BOARD_XT_PPI_PORT_B_CLEAR_KEYBOARD) != 0u);
}

static void x86_nxvm_board_xt_ppi_keyboard_clear_byte(
    x86_nxvm_board_xt_ppi_keyboard *keyboard)
{
    if (keyboard == LIB_NULL) return;
    keyboard->byte_ready = LIB_FALSE;
    x86_nxvm_board_xt_ppi_keyboard_deassert_irq(keyboard);
    if (keyboard->byte_released != LIB_NULL) {
        keyboard->byte_released(keyboard->byte_released_owner);
    }
}

static lib_status x86_nxvm_board_xt_ppi_keyboard_read(void *owner,
    lib_u16 port, lib_u64 tick,
    lib_u32 *out_value)
{
    (void)tick;
    x86_nxvm_board_xt_ppi_keyboard *keyboard = owner;
    lib_u8 input = 0u, value = 0u;
    lib_status status;

    if (keyboard == LIB_NULL || out_value == LIB_NULL ||
        port < keyboard->config.port_a || port > keyboard->config.control_port) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (port == keyboard->config.port_a) input = keyboard->byte_ready ? keyboard->current_byte : 0u;
    else if (port == keyboard->config.port_c) input = x86_nxvm_board_xt_ppi_keyboard_port_c_input(keyboard);
    status = x86_ppi8255_read(keyboard->ppi, (lib_u8)(port - keyboard->config.port_a), input, &value);
    if (status == LIB_STATUS_OK) *out_value = value;
    return status;
}

static lib_status x86_nxvm_board_xt_ppi_keyboard_write(void *owner,
    lib_u16 port, lib_u32 value)
{
    x86_nxvm_board_xt_ppi_keyboard *keyboard = owner;
    lib_u8 byte = (lib_u8)value;
    lib_status status;

    if (keyboard == LIB_NULL || port < keyboard->config.port_a ||
        port > keyboard->config.control_port) return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_ppi8255_write(keyboard->ppi, (lib_u8)(port - keyboard->config.port_a), byte);
    if (status != LIB_STATUS_OK) return status;
    if (port == keyboard->config.port_b || port == keyboard->config.control_port) {
        x86_ppi8255_pins pins = x86_nxvm_board_xt_ppi_port_b(keyboard);
        x86_nxvm_board_xt_ppi_keyboard_publish_speaker(keyboard);
        if (port == keyboard->config.port_b &&
            (pins.latch & pins.output_mask & X86_NXVM_BOARD_XT_PPI_PORT_B_CLEAR_KEYBOARD) != 0u) {
            x86_nxvm_board_xt_ppi_keyboard_clear_byte(keyboard);
        }
        x86_nxvm_board_xt_ppi_keyboard_publish_lines(keyboard);
        x86_nxvm_board_xt_ppi_keyboard_refresh_nmi(keyboard);
    }
    return LIB_STATUS_OK;
}

lib_i32 x86_nxvm_board_xt_ppi_keyboard_config_is_valid(
    const x86_nxvm_board_xt_ppi_keyboard_config *config)
{
    return config != LIB_NULL && config->port_a <= 0xfffcu &&
        config->port_b == config->port_a + 1u &&
        config->port_c == config->port_a + 2u &&
        config->control_port == config->port_a + 3u && config->irq < 16u;
}

lib_status x86_nxvm_board_xt_ppi_keyboard_initialize(
    x86_nxvm_board_xt_ppi_keyboard *keyboard,
    const x86_nxvm_board_xt_ppi_keyboard_config *config, x86_core_machine *machine)
{
    x86_core_machine_port_route routes[4];
    lib_size index;
    lib_status status;

    if (keyboard == LIB_NULL || machine == LIB_NULL ||
        !x86_nxvm_board_xt_ppi_keyboard_config_is_valid(config)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    status = x86_ppi8255_create(&keyboard->ppi);
    if (status != LIB_STATUS_OK) return status;
    for (index = 0u; index < 4u; ++index) {
        routes[index] = (x86_core_machine_port_route) {
            (lib_u16)(config->port_a + index),
            x86_nxvm_board_xt_ppi_keyboard_read,
            x86_nxvm_board_xt_ppi_keyboard_write, keyboard, LIB_FALSE, 0u};
    }
    status = x86_core_machine_install_port_routes(machine, routes, 4u);
    if (status != LIB_STATUS_OK) {
        x86_ppi8255_destroy(keyboard->ppi);
        keyboard->ppi = LIB_NULL;
        return status;
    }
    keyboard->config = *config;
    x86_nxvm_board_xt_ppi_keyboard_reset(keyboard);
    return LIB_STATUS_OK;
}

void x86_nxvm_board_xt_ppi_keyboard_bind_irq(x86_nxvm_board_xt_ppi_keyboard *keyboard,
    x86_nxvm_board_xt_ppi_irq_line irq, void *owner)
{
    if (keyboard == LIB_NULL) return;
    keyboard->irq = irq;
    keyboard->irq_owner = owner;
}

void x86_nxvm_board_xt_ppi_keyboard_bind_nmi(x86_nxvm_board_xt_ppi_keyboard *keyboard,
    x86_nxvm_board_xt_ppi_nmi_request request, void *owner)
{
    if (keyboard == LIB_NULL) return;
    keyboard->nmi_request = request;
    keyboard->nmi_owner = owner;
    x86_nxvm_board_xt_ppi_keyboard_refresh_nmi(keyboard);
}

void x86_nxvm_board_xt_ppi_keyboard_bind_speaker(x86_nxvm_board_xt_ppi_keyboard *keyboard,
    x86_nxvm_board_xt_ppi_speaker_update update, void *owner)
{
    if (keyboard == LIB_NULL) return;
    keyboard->speaker_update = update;
    keyboard->speaker_owner = owner;
    x86_nxvm_board_xt_ppi_keyboard_publish_speaker(keyboard);
}

void x86_nxvm_board_xt_ppi_keyboard_bind_keyboard_observer(
    x86_nxvm_board_xt_ppi_keyboard *keyboard, x86_nxvm_board_xt_ppi_line_observer observer,
    void *owner, x86_nxvm_board_xt_ppi_byte_released released)
{
    if (keyboard == LIB_NULL) return;
    keyboard->line_observer = observer;
    keyboard->line_observer_owner = owner;
    keyboard->byte_released = released;
    keyboard->byte_released_owner = owner;
    x86_nxvm_board_xt_ppi_keyboard_publish_lines(keyboard);
}

void x86_nxvm_board_xt_ppi_keyboard_bind_timer_output(x86_nxvm_board_xt_ppi_keyboard *keyboard,
    lib_bool (*output)(void *owner), void *owner)
{
    if (keyboard == LIB_NULL) return;
    keyboard->timer_output = output;
    keyboard->timer_output_owner = owner;
}

void x86_nxvm_board_xt_ppi_keyboard_reset(x86_nxvm_board_xt_ppi_keyboard *keyboard)
{
    if (keyboard == LIB_NULL) return;
    x86_nxvm_board_xt_ppi_keyboard_deassert_irq(keyboard);
    x86_ppi8255_reset(keyboard->ppi);
    keyboard->current_byte = 0u;
    keyboard->byte_ready = LIB_FALSE;
    keyboard->io_check_asserted = LIB_FALSE;
    keyboard->ram_parity_asserted = LIB_FALSE;
    keyboard->nmi_signaled = LIB_FALSE;
    x86_nxvm_board_xt_ppi_keyboard_publish_speaker(keyboard);
    x86_nxvm_board_xt_ppi_keyboard_publish_lines(keyboard);
}

void x86_nxvm_board_xt_ppi_keyboard_finalize(x86_nxvm_board_xt_ppi_keyboard *keyboard)
{
    if (keyboard == LIB_NULL) return;
    keyboard->line_observer = LIB_NULL;
    keyboard->line_observer_owner = LIB_NULL;
    x86_nxvm_board_xt_ppi_keyboard_reset(keyboard);
    x86_ppi8255_destroy(keyboard->ppi);
    keyboard->ppi = LIB_NULL;
    keyboard->nmi_request = LIB_NULL;
    keyboard->nmi_owner = LIB_NULL;
    keyboard->speaker_update = LIB_NULL;
    keyboard->speaker_owner = LIB_NULL;
    keyboard->timer_output = LIB_NULL;
    keyboard->timer_output_owner = LIB_NULL;
    keyboard->byte_released = LIB_NULL;
    keyboard->byte_released_owner = LIB_NULL;
}

lib_status x86_nxvm_board_xt_ppi_keyboard_set_fault_input(
    x86_nxvm_board_xt_ppi_keyboard *keyboard, x86_nxvm_board_xt_ppi_fault_input input,
    lib_i32 asserted)
{
    if (keyboard == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (input == X86_NXVM_BOARD_XT_PPI_FAULT_IO_CHECK) {
        keyboard->io_check_asserted = asserted ? LIB_TRUE : LIB_FALSE;
    } else if (input == X86_NXVM_BOARD_XT_PPI_FAULT_RAM_PARITY) {
        keyboard->ram_parity_asserted = asserted ? LIB_TRUE : LIB_FALSE;
    } else return LIB_STATUS_INVALID_ARGUMENT;
    x86_nxvm_board_xt_ppi_keyboard_refresh_nmi(keyboard);
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_board_xt_ppi_keyboard_receive_device_byte(
    x86_nxvm_board_xt_ppi_keyboard *keyboard, lib_u8 native_byte)
{
    if (keyboard == LIB_NULL || keyboard->byte_ready ||
        !x86_nxvm_board_xt_ppi_keyboard_delivery_enabled(keyboard)) {
        return LIB_STATUS_INVALID_STATE;
    }
    keyboard->current_byte = native_byte;
    keyboard->byte_ready = LIB_TRUE;
    if (keyboard->irq != LIB_NULL) keyboard->irq(keyboard->irq_owner, LIB_TRUE);
    keyboard->irq1_asserted = LIB_TRUE;
    return LIB_STATUS_OK;
}
