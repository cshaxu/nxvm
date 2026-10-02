#include "lib/types/types_interface.h"

#include "app-nxvm/devices/xt_ppi_keyboard.h"

#define CORE_MACHINE_XT_PPI_PORT_B_CLOCK_NOT_HELD 0x40u
#define CORE_MACHINE_XT_PPI_PORT_B_CLEAR_KEYBOARD 0x80u
#define CORE_MACHINE_XT_PPI_PORT_B_DISABLE_RAM_PARITY 0x10u
#define CORE_MACHINE_XT_PPI_PORT_B_DISABLE_IO_CHECK 0x20u
#define CORE_MACHINE_XT_PPI_PORT_C_IO_CHECK 0x40u
#define CORE_MACHINE_XT_PPI_PORT_C_RAM_PARITY 0x80u

static x86_ppi8255_pins core_machine_xt_ppi_port_b(
    const core_machine_xt_ppi_keyboard *keyboard)
{
    x86_ppi8255_pins pins = {0u, 0u};
    if (keyboard != LIB_NULL) x86_ppi8255_output(keyboard->ppi, 1u, &pins);
    return pins;
}

static lib_u8 core_machine_xt_ppi_keyboard_port_c_input(
    const core_machine_xt_ppi_keyboard *keyboard)
{
    lib_u8 value = (core_machine_xt_ppi_port_b(keyboard).latch & 0x08u) != 0u ?
        keyboard->config.switches_high & 0x0fu : keyboard->config.switches_low & 0x0fu;
    if (keyboard->io_check_asserted) value |= CORE_MACHINE_XT_PPI_PORT_C_IO_CHECK;
    if (keyboard->ram_parity_asserted) value |= CORE_MACHINE_XT_PPI_PORT_C_RAM_PARITY;
    return value;
}

static lib_bool core_machine_xt_ppi_keyboard_nmi_pending(
    const core_machine_xt_ppi_keyboard *keyboard)
{
    x86_ppi8255_pins pins = core_machine_xt_ppi_port_b(keyboard);
    if (pins.output_mask != 0xffu) return LIB_FALSE;
    return (keyboard->io_check_asserted &&
        (pins.latch & CORE_MACHINE_XT_PPI_PORT_B_DISABLE_IO_CHECK) == 0u) ||
        (keyboard->ram_parity_asserted &&
        (pins.latch & CORE_MACHINE_XT_PPI_PORT_B_DISABLE_RAM_PARITY) == 0u);
}

void core_machine_xt_ppi_keyboard_refresh_nmi(core_machine_xt_ppi_keyboard *keyboard)
{
    if (keyboard == LIB_NULL) return;
    if (!core_machine_xt_ppi_keyboard_nmi_pending(keyboard)) {
        keyboard->nmi_signaled = LIB_FALSE;
    } else if (!keyboard->nmi_signaled && keyboard->nmi_request != LIB_NULL &&
        keyboard->nmi_request(keyboard->nmi_owner)) {
        keyboard->nmi_signaled = LIB_TRUE;
    }
}

static lib_bool core_machine_xt_ppi_keyboard_delivery_enabled(
    const core_machine_xt_ppi_keyboard *keyboard)
{
    x86_ppi8255_pins pins = core_machine_xt_ppi_port_b(keyboard);
    return pins.output_mask == 0xffu &&
        (pins.latch & (CORE_MACHINE_XT_PPI_PORT_B_CLOCK_NOT_HELD |
        CORE_MACHINE_XT_PPI_PORT_B_CLEAR_KEYBOARD)) ==
            CORE_MACHINE_XT_PPI_PORT_B_CLOCK_NOT_HELD;
}

static void core_machine_xt_ppi_keyboard_publish_speaker(
    core_machine_xt_ppi_keyboard *keyboard)
{
    x86_ppi8255_pins pins;
    if (keyboard == LIB_NULL || keyboard->speaker_update == LIB_NULL) return;
    pins = core_machine_xt_ppi_port_b(keyboard);
    keyboard->speaker_update(keyboard->speaker_owner,
        (pins.latch & 0x01u) != 0u, (pins.latch & 0x02u) != 0u);
}

static void core_machine_xt_ppi_keyboard_deassert_irq(
    core_machine_xt_ppi_keyboard *keyboard)
{
    if (keyboard == LIB_NULL || !keyboard->irq1_asserted) return;
    core_machine_pic_irq_source_deassert(&keyboard->irq1_source);
    keyboard->irq1_asserted = LIB_FALSE;
}

static void core_machine_xt_ppi_keyboard_publish_lines(core_machine_xt_ppi_keyboard *keyboard)
{
    x86_ppi8255_pins pins;
    if (keyboard->line_observer == LIB_NULL) return;
    pins = core_machine_xt_ppi_port_b(keyboard);
    keyboard->line_observer(keyboard->line_observer_owner,
        (pins.latch & CORE_MACHINE_XT_PPI_PORT_B_CLOCK_NOT_HELD) == 0u,
        (pins.latch & CORE_MACHINE_XT_PPI_PORT_B_CLEAR_KEYBOARD) != 0u);
}

static void core_machine_xt_ppi_keyboard_clear_byte(
    core_machine_xt_ppi_keyboard *keyboard)
{
    if (keyboard == LIB_NULL) return;
    keyboard->byte_ready = LIB_FALSE;
    core_machine_xt_ppi_keyboard_deassert_irq(keyboard);
    if (keyboard->byte_released != LIB_NULL) {
        keyboard->byte_released(keyboard->byte_released_owner);
    }
}

static lib_status core_machine_xt_ppi_keyboard_read(void *owner,
    lib_u16 port, lib_u32 *out_value)
{
    core_machine_xt_ppi_keyboard *keyboard = owner;
    lib_u8 input = 0u, value = 0u;
    lib_status status;

    if (keyboard == LIB_NULL || out_value == LIB_NULL ||
        port < keyboard->config.port_a || port > keyboard->config.control_port) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (port == keyboard->config.port_a) input = keyboard->byte_ready ? keyboard->current_byte : 0u;
    else if (port == keyboard->config.port_c) input = core_machine_xt_ppi_keyboard_port_c_input(keyboard);
    status = x86_ppi8255_read(keyboard->ppi, (lib_u8)(port - keyboard->config.port_a), input, &value);
    if (status == LIB_STATUS_OK) *out_value = value;
    return status;
}

static lib_status core_machine_xt_ppi_keyboard_write(void *owner,
    lib_u16 port, lib_u32 value)
{
    core_machine_xt_ppi_keyboard *keyboard = owner;
    lib_u8 byte = (lib_u8)value;
    lib_status status;

    if (keyboard == LIB_NULL || port < keyboard->config.port_a ||
        port > keyboard->config.control_port) return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_ppi8255_write(keyboard->ppi, (lib_u8)(port - keyboard->config.port_a), byte);
    if (status != LIB_STATUS_OK) return status;
    if (port == keyboard->config.port_b) {
        core_machine_xt_ppi_keyboard_publish_speaker(keyboard);
        if ((byte & CORE_MACHINE_XT_PPI_PORT_B_CLEAR_KEYBOARD) != 0u) {
            core_machine_xt_ppi_keyboard_clear_byte(keyboard);
        }
        core_machine_xt_ppi_keyboard_publish_lines(keyboard);
    } else if (port == keyboard->config.control_port && !keyboard->byte_ready &&
        core_machine_xt_ppi_keyboard_delivery_enabled(keyboard)) {
        /* Direction changes can reopen delivery without another PB write. */
        core_machine_xt_ppi_keyboard_publish_lines(keyboard);
    }
    if (port == keyboard->config.port_b || port == keyboard->config.control_port) {
        core_machine_xt_ppi_keyboard_refresh_nmi(keyboard);
    }
    return LIB_STATUS_OK;
}

lib_i32 core_machine_xt_ppi_keyboard_config_is_valid(
    const core_machine_xt_ppi_keyboard_config *config)
{
    return config != LIB_NULL && config->port_a <= 0xfffcu &&
        config->port_b == config->port_a + 1u &&
        config->port_c == config->port_a + 2u &&
        config->control_port == config->port_a + 3u && config->irq < 16u;
}

lib_status core_machine_xt_ppi_keyboard_initialize(
    core_machine_xt_ppi_keyboard *keyboard,
    const core_machine_xt_ppi_keyboard_config *config, core_machine *machine)
{
    core_machine_port_route routes[4];
    lib_size index;
    lib_status status;

    if (keyboard == LIB_NULL || machine == LIB_NULL ||
        !core_machine_xt_ppi_keyboard_config_is_valid(config)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    status = x86_ppi8255_create(&keyboard->ppi);
    if (status != LIB_STATUS_OK) return status;
    for (index = 0u; index < 4u; ++index) {
        routes[index] = (core_machine_port_route) {
            (lib_u16)(config->port_a + index),
            core_machine_xt_ppi_keyboard_read,
            core_machine_xt_ppi_keyboard_write, keyboard, LIB_FALSE, 0u};
    }
    status = core_machine_install_port_routes(machine, routes, 4u);
    if (status != LIB_STATUS_OK) {
        x86_ppi8255_destroy(keyboard->ppi);
        keyboard->ppi = LIB_NULL;
        return status;
    }
    keyboard->config = *config;
    core_machine_xt_ppi_keyboard_reset(keyboard);
    return LIB_STATUS_OK;
}

void core_machine_xt_ppi_keyboard_bind_pic(core_machine_xt_ppi_keyboard *keyboard,
    core_machine_pic_bus *master, core_machine_pic_bus *slave)
{
    if (keyboard == LIB_NULL) return;
    core_machine_pic_irq_source_bind(&keyboard->irq1_source, master, slave,
        keyboard->config.irq);
}

void core_machine_xt_ppi_keyboard_bind_nmi(core_machine_xt_ppi_keyboard *keyboard,
    core_machine_xt_ppi_nmi_request request, void *owner)
{
    if (keyboard == LIB_NULL) return;
    keyboard->nmi_request = request;
    keyboard->nmi_owner = owner;
    core_machine_xt_ppi_keyboard_refresh_nmi(keyboard);
}

void core_machine_xt_ppi_keyboard_bind_speaker(core_machine_xt_ppi_keyboard *keyboard,
    core_machine_xt_ppi_speaker_update update, void *owner)
{
    if (keyboard == LIB_NULL) return;
    keyboard->speaker_update = update;
    keyboard->speaker_owner = owner;
    core_machine_xt_ppi_keyboard_publish_speaker(keyboard);
}

void core_machine_xt_ppi_keyboard_bind_keyboard_observer(
    core_machine_xt_ppi_keyboard *keyboard, core_machine_xt_ppi_line_observer observer,
    void *owner, core_machine_xt_ppi_byte_released released)
{
    if (keyboard == LIB_NULL) return;
    keyboard->line_observer = observer;
    keyboard->line_observer_owner = owner;
    keyboard->byte_released = released;
    keyboard->byte_released_owner = owner;
    core_machine_xt_ppi_keyboard_publish_lines(keyboard);
}

void core_machine_xt_ppi_keyboard_reset(core_machine_xt_ppi_keyboard *keyboard)
{
    if (keyboard == LIB_NULL) return;
    core_machine_xt_ppi_keyboard_deassert_irq(keyboard);
    x86_ppi8255_reset(keyboard->ppi);
    keyboard->current_byte = 0u;
    keyboard->byte_ready = LIB_FALSE;
    keyboard->io_check_asserted = LIB_FALSE;
    keyboard->ram_parity_asserted = LIB_FALSE;
    keyboard->nmi_signaled = LIB_FALSE;
    core_machine_xt_ppi_keyboard_publish_speaker(keyboard);
}

void core_machine_xt_ppi_keyboard_finalize(core_machine_xt_ppi_keyboard *keyboard)
{
    if (keyboard == LIB_NULL) return;
    core_machine_xt_ppi_keyboard_reset(keyboard);
    x86_ppi8255_destroy(keyboard->ppi);
    keyboard->ppi = LIB_NULL;
    keyboard->nmi_request = LIB_NULL;
    keyboard->nmi_owner = LIB_NULL;
    keyboard->speaker_update = LIB_NULL;
    keyboard->speaker_owner = LIB_NULL;
    keyboard->line_observer = LIB_NULL;
    keyboard->line_observer_owner = LIB_NULL;
    keyboard->byte_released = LIB_NULL;
    keyboard->byte_released_owner = LIB_NULL;
}

lib_status core_machine_xt_ppi_keyboard_set_fault_input(
    core_machine_xt_ppi_keyboard *keyboard, core_machine_xt_ppi_fault_input input,
    lib_i32 asserted)
{
    if (keyboard == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (input == CORE_MACHINE_XT_PPI_FAULT_IO_CHECK) {
        keyboard->io_check_asserted = asserted ? LIB_TRUE : LIB_FALSE;
    } else if (input == CORE_MACHINE_XT_PPI_FAULT_RAM_PARITY) {
        keyboard->ram_parity_asserted = asserted ? LIB_TRUE : LIB_FALSE;
    } else return LIB_STATUS_INVALID_ARGUMENT;
    core_machine_xt_ppi_keyboard_refresh_nmi(keyboard);
    return LIB_STATUS_OK;
}

lib_status core_machine_xt_ppi_keyboard_receive_device_byte(
    core_machine_xt_ppi_keyboard *keyboard, lib_u8 native_byte)
{
    if (keyboard == LIB_NULL || keyboard->byte_ready ||
        !core_machine_xt_ppi_keyboard_delivery_enabled(keyboard)) {
        return LIB_STATUS_INVALID_STATE;
    }
    keyboard->current_byte = native_byte;
    keyboard->byte_ready = LIB_TRUE;
    core_machine_pic_irq_source_assert(&keyboard->irq1_source);
    keyboard->irq1_asserted = LIB_TRUE;
    return LIB_STATUS_OK;
}
