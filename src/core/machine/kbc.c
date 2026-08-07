/* Copyright 2012-2014 Neko. */

#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/memory.h"
#include "core/machine/pic.h"
#include "core/machine/port.h"

#include "core/machine/kbc.h"

#define CORE_MACHINE_KBC_COMMAND_IRQ1 0x01u
#define CORE_MACHINE_KBC_COMMAND_SYSTEM 0x04u
#define CORE_MACHINE_KBC_COMMAND_DISABLE_KEYBOARD 0x10u
#define CORE_MACHINE_KBC_OUTPUT_RESET 0x01u
#define CORE_MACHINE_KBC_OUTPUT_A20 0x02u

#define CORE_MACHINE_KBC_ACK 0xfau
#define CORE_MACHINE_KBC_BAT_OK 0xaau
#define CORE_MACHINE_KBC_IDENTIFY_0 0xabu
#define CORE_MACHINE_KBC_IDENTIFY_1 0x83u
#define CORE_MACHINE_KBC_RESEND 0xfeu
#define CORE_MACHINE_KBC_ECHO 0xeeu
#define CORE_MACHINE_KBC_DEFAULT_TYPEMATIC 0x20u
#define CORE_MACHINE_KBC_AUX_DEFAULT_RESOLUTION 2u
#define CORE_MACHINE_KBC_AUX_DEFAULT_SAMPLE_RATE 100u
#define CORE_MACHINE_KBC_AUX_STATUS_REPORTING 0x20u
#define CORE_MACHINE_KBC_AUX_STATUS_SCALING_2_TO_1 0x10u

static C_VOID core_machine_kbc_deassert_irq1(t_kbc *controller)
{
    if (controller == STD_NULL || !controller->data.irq1_asserted) return;
    core_machine_pic_irq_source_deassert(&controller->connect.irq1_source);
    controller->data.irq1_asserted = TYPE_FALSE;
}

static C_VOID core_machine_kbc_deassert_irq12(t_kbc *controller)
{
    if (controller == STD_NULL || !controller->data.irq12_asserted) return;
    core_machine_pic_irq_source_deassert(&controller->connect.irq12_source);
    controller->data.irq12_asserted = TYPE_FALSE;
}

static C_VOID core_machine_kbc_refresh_current_irq(t_kbc *controller)
{
    core_machine_kbc_output_origin origin;

    if (controller == STD_NULL || controller->data.fifo_count == 0u) {
        core_machine_kbc_deassert_irq1(controller);
        core_machine_kbc_deassert_irq12(controller);
        return;
    }
    origin = controller->data.fifo_origin[controller->data.fifo_head];
    if (origin == CORE_MACHINE_KBC_OUTPUT_KEYBOARD) {
        core_machine_kbc_deassert_irq12(controller);
        if (!controller->data.irq1_asserted && controller->data.keyboard_enabled &&
            (controller->data.command_byte & CORE_MACHINE_KBC_COMMAND_IRQ1) != 0u) {
            core_machine_pic_irq_source_assert(&controller->connect.irq1_source);
            controller->data.irq1_asserted = TYPE_TRUE;
        }
    } else if (origin == CORE_MACHINE_KBC_OUTPUT_AUX) {
        core_machine_kbc_deassert_irq1(controller);
        if (!controller->data.irq12_asserted && controller->data.aux_enabled &&
            (controller->data.command_byte & CORE_MACHINE_KBC_COMMAND_IRQ12) != 0u &&
            (controller->data.command_byte & CORE_MACHINE_KBC_COMMAND_DISABLE_AUX) == 0u) {
            core_machine_pic_irq_source_assert(&controller->connect.irq12_source);
            controller->data.irq12_asserted = TYPE_TRUE;
        }
    } else {
        core_machine_kbc_deassert_irq1(controller);
        core_machine_kbc_deassert_irq12(controller);
    }
}

static type_status core_machine_kbc_enqueue(t_kbc *controller, uint8_t value,
    core_machine_kbc_output_origin origin)
{
    type_unsigned_8 tail;

    if (controller == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (controller->data.fifo_count >= CORE_MACHINE_KBC_FIFO_CAPACITY) {
        return TYPE_STATUS_INVALID_STATE;
    }
    tail = (type_unsigned_8)((controller->data.fifo_head +
        controller->data.fifo_count) % CORE_MACHINE_KBC_FIFO_CAPACITY);
    controller->data.fifo[tail] = value;
    controller->data.fifo_origin[tail] = origin;
    ++controller->data.fifo_count;
    controller->data.last_output_byte = value;
    core_machine_kbc_refresh_current_irq(controller);
    return TYPE_STATUS_OK;
}

static C_VOID core_machine_kbc_schedule_response(t_kbc *controller,
    const uint8_t *bytes, uint8_t count, core_machine_kbc_output_origin origin)
{
    uint8_t index;

    if (controller == STD_NULL || bytes == STD_NULL || count == 0u ||
        count > CORE_MACHINE_KBC_RESPONSE_CAPACITY ||
        controller->data.delayed_response_count != 0u) return;
    for (index = 0u; index < count; ++index) {
        controller->data.delayed_response[index] = bytes[index];
    }
    controller->data.delayed_response_count = count;
    controller->data.delayed_response_index = 0u;
    controller->data.delayed_response_origin = origin;
    controller->data.response_remaining_ticks =
        controller->data.command_response_ticks;

    /* The response bytes remain KBC-owned until the guest-visible FIFO has
     * room.  A full rapid-typeahead FIFO must delay a command reply, never
     * silently discard it.  A zero delay still drains synchronously for
     * controller commands that are observed in the same I/O sequence. */
    if (controller->data.command_response_ticks == 0u) {
        core_machine_kbc_advance(controller, 0u);
    }
}

static C_VOID core_machine_kbc_schedule_response_byte(t_kbc *controller,
    uint8_t value, core_machine_kbc_output_origin origin)
{
    core_machine_kbc_schedule_response(controller, &value, 1u, origin);
}

static C_VOID core_machine_kbc_set_typematic(t_kbc *controller, uint8_t value)
{
    if (controller == STD_NULL) return;
    controller->data.typematic = value;
}

static C_VOID core_machine_kbc_set_defaults(t_kbc *controller)
{
    if (controller == STD_NULL) return;
    controller->data.scan_set = CORE_MACHINE_KBC_SCAN_SET_1;
    controller->data.led_state = 0u;
    core_machine_kbc_set_typematic(controller, CORE_MACHINE_KBC_DEFAULT_TYPEMATIC);
    controller->data.scanning_enabled = TYPE_TRUE;
    controller->data.typematic_active = TYPE_FALSE;
    controller->data.typematic_remaining_ticks = 0u;
}

static type_bool core_machine_kbc_is_typematic_scan_code(uint8_t scan_code)
{
    switch (scan_code) {
    case 0x1du: case 0x2au: case 0x36u: case 0x38u:
    case 0x3au: case 0x45u: case 0x46u:
        return TYPE_FALSE;
    default:
        return scan_code != 0xe0u && scan_code != 0xe1u;
    }
}

static uint8_t core_machine_kbc_dequeue(t_kbc *controller)
{
    uint8_t value = 0u;
    core_machine_kbc_output_origin origin;

    if (controller == STD_NULL || controller->data.fifo_count == 0u) return 0u;
    value = controller->data.fifo[controller->data.fifo_head];
    origin = controller->data.fifo_origin[controller->data.fifo_head];
    controller->data.fifo_head = (type_unsigned_8)((controller->data.fifo_head + 1u) %
        CORE_MACHINE_KBC_FIFO_CAPACITY);
    --controller->data.fifo_count;
    /* Reading 60h acknowledges exactly the current origin. A queued successor
     * gets a fresh edge only after promotion through the one PIC boundary. */
    if (origin == CORE_MACHINE_KBC_OUTPUT_KEYBOARD) {
        core_machine_kbc_deassert_irq1(controller);
    } else if (origin == CORE_MACHINE_KBC_OUTPUT_AUX) {
        core_machine_kbc_deassert_irq12(controller);
    }
    core_machine_kbc_refresh_current_irq(controller);
    return value;
}

static uint8_t core_machine_kbc_status(const t_kbc *controller)
{
    uint8_t status = 0u;

    if (controller == STD_NULL) return status;
    if (controller->data.fifo_count != 0u) status |= VKBC_STATUS_OBF;
    if (controller->data.input_buffer_full) status |= VKBC_STATUS_IBF;
    if (controller->data.system_flag) status |= VKBC_STATUS_SYS;
    if (controller->data.last_write_command) status |= VKBC_STATUS_CD;
    if (controller->data.keyboard_enabled) status |= VKBC_STATUS_KE;
    if (controller->data.fifo_count != 0u &&
        controller->data.fifo_origin[controller->data.fifo_head] ==
            CORE_MACHINE_KBC_OUTPUT_AUX) status |= VKBC_STATUS_AUX;
    return status;
}

static C_VOID core_machine_kbc_apply_output_port(t_kbc *controller, uint8_t value)
{
    if (controller == STD_NULL) return;
    controller->data.output_port = value;
    if (controller->connect.memory != STD_NULL) {
        controller->connect.memory->data.flagA20 =
            (value & CORE_MACHINE_KBC_OUTPUT_A20) != 0u;
    }
    if ((value & CORE_MACHINE_KBC_OUTPUT_RESET) == 0u &&
        controller->connect.execution != STD_NULL) {
        core_machine_cpu_execution_request_reset(controller->connect.execution);
    }
}

static C_VOID core_machine_kbc_handle_keyboard_command(t_kbc *controller,
    uint8_t command)
{
    static const uint8_t identify[] = {
        CORE_MACHINE_KBC_ACK, CORE_MACHINE_KBC_IDENTIFY_0,
        CORE_MACHINE_KBC_IDENTIFY_1
    };
    static const uint8_t reset_ok[] = {
        CORE_MACHINE_KBC_ACK, CORE_MACHINE_KBC_BAT_OK
    };
    uint8_t resend;

    switch (command) {
    case 0xffu:
        core_machine_kbc_schedule_response(controller, reset_ok, sizeof(reset_ok),
            CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
        core_machine_kbc_set_defaults(controller);
        break;
    case 0xedu:
        core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_ACK,
            CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
        controller->data.pending_write = CORE_MACHINE_KBC_PENDING_KEYBOARD_LEDS;
        break;
    case 0xeeu:
        core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_ECHO,
            CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
        break;
    case 0xf0u:
        core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_ACK,
            CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
        controller->data.pending_write = CORE_MACHINE_KBC_PENDING_KEYBOARD_SCAN_SET;
        break;
    case 0xf3u:
        core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_ACK,
            CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
        controller->data.pending_write = CORE_MACHINE_KBC_PENDING_KEYBOARD_TYPEMATIC;
        break;
    case 0xf4u:
        controller->data.scanning_enabled = TYPE_TRUE;
        core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_ACK,
            CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
        break;
    case 0xf5u:
        controller->data.scanning_enabled = TYPE_FALSE;
        controller->data.typematic_active = TYPE_FALSE;
        core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_ACK,
            CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
        break;
    case 0xf6u:
        core_machine_kbc_set_defaults(controller);
        core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_ACK,
            CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
        break;
    case 0xf2u:
        core_machine_kbc_schedule_response(controller, identify, sizeof(identify),
            CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
        break;
    case 0xfeu:
        resend = controller->data.last_output_byte;
        core_machine_kbc_schedule_response_byte(controller, resend,
            CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
        break;
    default:
        core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_RESEND,
            CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
        break;
    }
}

static C_VOID core_machine_kbc_set_aux_defaults(t_kbc *controller)
{
    if (controller == STD_NULL) return;
    controller->data.aux_reporting_enabled = TYPE_FALSE;
    controller->data.aux_scaling_2_to_1 = TYPE_FALSE;
    controller->data.aux_button_state = 0u;
    controller->data.aux_resolution = CORE_MACHINE_KBC_AUX_DEFAULT_RESOLUTION;
    controller->data.aux_sample_rate = CORE_MACHINE_KBC_AUX_DEFAULT_SAMPLE_RATE;
    controller->data.aux_pending_parameter = CORE_MACHINE_KBC_AUX_PENDING_NONE;
}

static type_bool core_machine_kbc_is_aux_sample_rate(uint8_t value)
{
    switch (value) {
    case 10u: case 20u: case 40u: case 60u: case 80u: case 100u: case 200u:
        return TYPE_TRUE;
    default:
        return TYPE_FALSE;
    }
}

static uint8_t core_machine_kbc_aux_status(const t_kbc *controller)
{
    uint8_t status;

    if (controller == STD_NULL) return 0u;
    status = controller->data.aux_button_state & 0x07u;
    if (controller->data.aux_reporting_enabled) {
        status |= CORE_MACHINE_KBC_AUX_STATUS_REPORTING;
    }
    if (controller->data.aux_scaling_2_to_1) {
        status |= CORE_MACHINE_KBC_AUX_STATUS_SCALING_2_TO_1;
    }
    return status;
}

static C_VOID core_machine_kbc_handle_aux_command(t_kbc *controller,
    uint8_t command)
{
    static const uint8_t identify[] = { CORE_MACHINE_KBC_ACK, 0x00u };
    static const uint8_t reset_ok[] = {
        CORE_MACHINE_KBC_ACK, CORE_MACHINE_KBC_BAT_OK, 0x00u
    };
    uint8_t status_reply[4];

    if (controller == STD_NULL) return;
    if (controller->data.aux_pending_parameter ==
        CORE_MACHINE_KBC_AUX_PENDING_SAMPLE_RATE) {
        controller->data.aux_pending_parameter = CORE_MACHINE_KBC_AUX_PENDING_NONE;
        if (core_machine_kbc_is_aux_sample_rate(command)) {
            controller->data.aux_sample_rate = command;
            core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_ACK,
                CORE_MACHINE_KBC_OUTPUT_AUX);
        } else {
            core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_RESEND,
                CORE_MACHINE_KBC_OUTPUT_AUX);
        }
        return;
    }
    if (controller->data.aux_pending_parameter ==
        CORE_MACHINE_KBC_AUX_PENDING_RESOLUTION) {
        controller->data.aux_pending_parameter = CORE_MACHINE_KBC_AUX_PENDING_NONE;
        if (command <= 3u) {
            controller->data.aux_resolution = command;
            core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_ACK,
                CORE_MACHINE_KBC_OUTPUT_AUX);
        } else {
            core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_RESEND,
                CORE_MACHINE_KBC_OUTPUT_AUX);
        }
        return;
    }
    switch (command) {
    case 0xffu:
        core_machine_kbc_set_aux_defaults(controller);
        core_machine_kbc_schedule_response(controller, reset_ok, sizeof(reset_ok),
            CORE_MACHINE_KBC_OUTPUT_AUX);
        break;
    case 0xf6u:
        core_machine_kbc_set_aux_defaults(controller);
        core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_ACK,
            CORE_MACHINE_KBC_OUTPUT_AUX);
        break;
    case 0xf5u:
        controller->data.aux_reporting_enabled = TYPE_FALSE;
        core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_ACK,
            CORE_MACHINE_KBC_OUTPUT_AUX);
        break;
    case 0xf4u:
        controller->data.aux_reporting_enabled = TYPE_TRUE;
        core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_ACK,
            CORE_MACHINE_KBC_OUTPUT_AUX);
        break;
    case 0xf2u:
        core_machine_kbc_schedule_response(controller, identify, sizeof(identify),
            CORE_MACHINE_KBC_OUTPUT_AUX);
        break;
    case 0xf3u:
        core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_ACK,
            CORE_MACHINE_KBC_OUTPUT_AUX);
        controller->data.aux_pending_parameter =
            CORE_MACHINE_KBC_AUX_PENDING_SAMPLE_RATE;
        break;
    case 0xe8u:
        core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_ACK,
            CORE_MACHINE_KBC_OUTPUT_AUX);
        controller->data.aux_pending_parameter =
            CORE_MACHINE_KBC_AUX_PENDING_RESOLUTION;
        break;
    case 0xe9u:
        status_reply[0] = CORE_MACHINE_KBC_ACK;
        status_reply[1] = core_machine_kbc_aux_status(controller);
        status_reply[2] = controller->data.aux_resolution;
        status_reply[3] = controller->data.aux_sample_rate;
        core_machine_kbc_schedule_response(controller, status_reply,
            sizeof(status_reply), CORE_MACHINE_KBC_OUTPUT_AUX);
        break;
    default:
        core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_RESEND,
            CORE_MACHINE_KBC_OUTPUT_AUX);
        break;
    }
}

static C_VOID core_machine_kbc_read_data(t_port *port, type_unsigned_16 port_id,
    C_VOID *owner)
{
    t_kbc *controller = (t_kbc *)owner;

    (C_VOID)port_id;
    port->data.ioByte = core_machine_kbc_dequeue(controller);
}

static C_VOID core_machine_kbc_read_status(t_port *port, type_unsigned_16 port_id,
    C_VOID *owner)
{
    (C_VOID)port_id;
    port->data.ioByte = core_machine_kbc_status((const t_kbc *)owner);
}

static C_VOID core_machine_kbc_write_data(t_port *port, type_unsigned_16 port_id,
    C_VOID *owner)
{
    t_kbc *controller = (t_kbc *)owner;
    uint8_t value = port->data.ioByte;

    (C_VOID)port_id;
    if (controller == STD_NULL) return;
    controller->data.input_buffer_full = TYPE_TRUE;
    controller->data.last_write_command = TYPE_FALSE;
    switch (controller->data.pending_write) {
    case CORE_MACHINE_KBC_PENDING_COMMAND_BYTE:
        controller->data.command_byte = value;
        controller->data.keyboard_enabled =
            (value & CORE_MACHINE_KBC_COMMAND_DISABLE_KEYBOARD) == 0u;
        controller->data.aux_enabled =
            (value & CORE_MACHINE_KBC_COMMAND_DISABLE_AUX) == 0u;
        controller->data.pending_write = CORE_MACHINE_KBC_PENDING_NONE;
        core_machine_kbc_refresh_current_irq(controller);
        break;
    case CORE_MACHINE_KBC_PENDING_OUTPUT_PORT:
        core_machine_kbc_apply_output_port(controller, value);
        controller->data.pending_write = CORE_MACHINE_KBC_PENDING_NONE;
        break;
    case CORE_MACHINE_KBC_PENDING_KEYBOARD_LEDS:
        controller->data.led_state = value & 0x07u;
        controller->data.pending_write = CORE_MACHINE_KBC_PENDING_NONE;
        core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_ACK,
            CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
        break;
    case CORE_MACHINE_KBC_PENDING_KEYBOARD_TYPEMATIC:
        core_machine_kbc_set_typematic(controller, value);
        controller->data.pending_write = CORE_MACHINE_KBC_PENDING_NONE;
        core_machine_kbc_schedule_response_byte(controller, CORE_MACHINE_KBC_ACK,
            CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
        break;
    case CORE_MACHINE_KBC_PENDING_KEYBOARD_SCAN_SET:
        controller->data.pending_write = CORE_MACHINE_KBC_PENDING_NONE;
        if (value == 0u) {
            uint8_t response[] = { CORE_MACHINE_KBC_ACK,
                controller->data.scan_set };
            core_machine_kbc_schedule_response(controller, response,
                sizeof(response), CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
        } else if (value == CORE_MACHINE_KBC_SCAN_SET_1) {
            controller->data.scan_set = value;
            core_machine_kbc_schedule_response_byte(controller,
                CORE_MACHINE_KBC_ACK, CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
        } else {
            core_machine_kbc_schedule_response_byte(controller,
                CORE_MACHINE_KBC_RESEND, CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
        }
        break;
    case CORE_MACHINE_KBC_PENDING_AUX_DEVICE:
        controller->data.pending_write = CORE_MACHINE_KBC_PENDING_NONE;
        core_machine_kbc_handle_aux_command(controller, value);
        break;
    default:
        core_machine_kbc_handle_keyboard_command(controller, value);
        break;
    }
    controller->data.input_buffer_full = TYPE_FALSE;
    controller->data.last_write_command = TYPE_FALSE;
}

static C_VOID core_machine_kbc_write_command(t_port *port,
    type_unsigned_16 port_id, C_VOID *owner)
{
    t_kbc *controller = (t_kbc *)owner;
    uint8_t command = port->data.ioByte;

    (C_VOID)port_id;
    if (controller == STD_NULL) return;
    controller->data.input_buffer_full = TYPE_TRUE;
    controller->data.last_write_command = TYPE_TRUE;
    switch (command) {
    case 0x20u:
        core_machine_kbc_schedule_response_byte(controller,
            controller->data.command_byte, CORE_MACHINE_KBC_OUTPUT_CONTROLLER);
        break;
    case 0x60u:
        controller->data.pending_write = CORE_MACHINE_KBC_PENDING_COMMAND_BYTE;
        break;
    case 0xaau:
        controller->data.system_flag = TYPE_TRUE;
        core_machine_kbc_schedule_response_byte(controller, 0x55u,
            CORE_MACHINE_KBC_OUTPUT_CONTROLLER);
        break;
    case 0xabu:
        core_machine_kbc_schedule_response_byte(controller, 0x00u,
            CORE_MACHINE_KBC_OUTPUT_CONTROLLER);
        break;
    case 0xadu:
        controller->data.keyboard_enabled = TYPE_FALSE;
        controller->data.command_byte |= CORE_MACHINE_KBC_COMMAND_DISABLE_KEYBOARD;
        core_machine_kbc_refresh_current_irq(controller);
        break;
    case 0xaeu:
        controller->data.keyboard_enabled = TYPE_TRUE;
        controller->data.command_byte &= ~CORE_MACHINE_KBC_COMMAND_DISABLE_KEYBOARD;
        core_machine_kbc_refresh_current_irq(controller);
        break;
    case 0xa7u:
        controller->data.aux_enabled = TYPE_FALSE;
        controller->data.command_byte |= CORE_MACHINE_KBC_COMMAND_DISABLE_AUX;
        core_machine_kbc_refresh_current_irq(controller);
        break;
    case 0xa8u:
        controller->data.aux_enabled = TYPE_TRUE;
        controller->data.command_byte &= ~CORE_MACHINE_KBC_COMMAND_DISABLE_AUX;
        core_machine_kbc_refresh_current_irq(controller);
        break;
    case 0xa9u:
        core_machine_kbc_schedule_response_byte(controller, 0x00u,
            CORE_MACHINE_KBC_OUTPUT_CONTROLLER);
        break;
    case 0xd0u:
        core_machine_kbc_schedule_response_byte(controller,
            controller->data.output_port, CORE_MACHINE_KBC_OUTPUT_CONTROLLER);
        break;
    case 0xd1u:
        controller->data.pending_write = CORE_MACHINE_KBC_PENDING_OUTPUT_PORT;
        break;
    case 0xd4u:
        controller->data.pending_write = CORE_MACHINE_KBC_PENDING_AUX_DEVICE;
        break;
    default:
        break;
    }
    controller->data.input_buffer_full = TYPE_FALSE;
    controller->data.last_write_command = TYPE_FALSE;
}

C_VOID core_machine_kbc_register_ports(t_kbc *controller, t_port *port)
{
    core_machine_port_add_read(port, 0x0060,
        core_machine_kbc_read_data, controller);
    core_machine_port_add_read(port, 0x0064,
        core_machine_kbc_read_status, controller);
    core_machine_port_add_write(port, 0x0060,
        core_machine_kbc_write_data, controller);
    core_machine_port_add_write(port, 0x0064,
        core_machine_kbc_write_command, controller);
}

C_VOID core_machine_kbc_initialize(t_kbc *controller, t_port *port) {
    if (controller == STD_NULL || port == STD_NULL) return;
    STD_MEMSET(controller, TYPE_ZERO_8, sizeof(*controller));
    core_machine_kbc_register_ports(controller, port);
    core_machine_kbc_reset(controller);
}
C_VOID core_machine_kbc_bind_core_services(t_kbc *controller, t_pic *pic_master,
    t_pic *pic_slave, t_ram *memory,
    core_machine_cpu_execution_context *execution)
{
    if (controller == STD_NULL) return;
    core_machine_pic_irq_source_bind(&controller->connect.irq1_source,
        pic_master, pic_slave, 1u);
    core_machine_pic_irq_source_bind(&controller->connect.irq12_source,
        pic_master, pic_slave, 12u);
    controller->connect.memory = memory;
    controller->connect.execution = execution;
}
C_VOID core_machine_kbc_reset(t_kbc *controller)
{
    uint32_t typematic_initial_ticks;
    uint32_t typematic_repeat_ticks;
    uint32_t command_response_ticks;

    if (controller == STD_NULL) return;
    typematic_initial_ticks = controller->data.typematic_initial_ticks;
    typematic_repeat_ticks = controller->data.typematic_repeat_ticks;
    command_response_ticks = controller->data.command_response_ticks;
    STD_MEMSET(&controller->data, TYPE_ZERO_8, sizeof(controller->data));
    controller->data.typematic_initial_ticks = typematic_initial_ticks;
    controller->data.typematic_repeat_ticks = typematic_repeat_ticks;
    controller->data.command_response_ticks = command_response_ticks;
    core_machine_pic_irq_source_deassert(&controller->connect.irq1_source);
    core_machine_pic_irq_source_deassert(&controller->connect.irq12_source);
    controller->data.command_byte = CORE_MACHINE_KBC_COMMAND_IRQ1 |
        CORE_MACHINE_KBC_COMMAND_IRQ12 |
        CORE_MACHINE_KBC_COMMAND_SYSTEM;
    controller->data.output_port = CORE_MACHINE_KBC_OUTPUT_RESET;
    controller->data.keyboard_enabled = TYPE_TRUE;
    controller->data.aux_enabled = TYPE_TRUE;
    core_machine_kbc_set_defaults(controller);
    core_machine_kbc_set_aux_defaults(controller);
    controller->data.system_flag = TYPE_TRUE;
    core_machine_kbc_apply_output_port(controller, controller->data.output_port);
}
C_VOID core_machine_kbc_refresh(t_kbc *controller) { (C_VOID)controller; }

C_VOID core_machine_kbc_advance(t_kbc *controller, uint64_t elapsed_ticks)
{
    if (controller == STD_NULL) return;
    if (controller->data.delayed_response_count != 0u) {
        if (elapsed_ticks < controller->data.response_remaining_ticks) {
            controller->data.response_remaining_ticks -= elapsed_ticks;
        } else {
            controller->data.response_remaining_ticks = 0u;
            if (controller->data.delayed_response_count <=
                CORE_MACHINE_KBC_FIFO_CAPACITY - controller->data.fifo_count) {
                while (controller->data.delayed_response_index <
                        controller->data.delayed_response_count) {
                    (C_VOID)core_machine_kbc_enqueue(controller,
                        controller->data.delayed_response[
                            controller->data.delayed_response_index],
                        controller->data.delayed_response_origin);
                    ++controller->data.delayed_response_index;
                }
                controller->data.delayed_response_count = 0u;
                controller->data.delayed_response_index = 0u;
            }
        }
    }
    if (elapsed_ticks == 0u) return;
    if (!controller->data.typematic_active) return;
    if (elapsed_ticks < controller->data.typematic_remaining_ticks) {
        controller->data.typematic_remaining_ticks -= elapsed_ticks;
        return;
    }
    elapsed_ticks -= controller->data.typematic_remaining_ticks;
    controller->data.typematic_remaining_ticks =
        controller->data.typematic_repeat_ticks;
    (C_VOID)core_machine_kbc_enqueue(controller,
        controller->data.typematic_scan_code, CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
    while (controller->data.typematic_repeat_ticks != 0u &&
        elapsed_ticks >= controller->data.typematic_repeat_ticks) {
        elapsed_ticks -= controller->data.typematic_repeat_ticks;
        (C_VOID)core_machine_kbc_enqueue(controller,
            controller->data.typematic_scan_code, CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
    }
    if (controller->data.typematic_repeat_ticks != 0u) {
        controller->data.typematic_remaining_ticks -= elapsed_ticks;
    }
}

C_VOID core_machine_kbc_set_typematic_timing(t_kbc *controller,
    uint32_t initial_ticks, uint32_t repeat_ticks)
{
    if (controller == STD_NULL) return;
    controller->data.typematic_initial_ticks = initial_ticks;
    controller->data.typematic_repeat_ticks = repeat_ticks;
}

C_VOID core_machine_kbc_set_command_response_timing(t_kbc *controller,
    uint32_t response_ticks)
{
    if (controller == STD_NULL) return;
    controller->data.command_response_ticks = response_ticks;
}
C_VOID core_machine_kbc_finalize(t_kbc *controller)
{
    if (controller != STD_NULL) {
        core_machine_pic_irq_source_deassert(&controller->connect.irq1_source);
        core_machine_pic_irq_source_deassert(&controller->connect.irq12_source);
        controller->data.irq1_asserted = TYPE_FALSE;
        controller->data.irq12_asserted = TYPE_FALSE;
    }
}
type_status core_machine_kbc_submit_scan_code(t_kbc *controller, uint8_t scan_code)
{
    if (controller == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (!controller->data.keyboard_enabled || !controller->data.scanning_enabled) {
        return TYPE_STATUS_INVALID_STATE;
    }
    if ((scan_code & 0x80u) != 0u &&
        (scan_code & 0x7fu) == controller->data.typematic_scan_code) {
        controller->data.typematic_active = TYPE_FALSE;
    } else if (controller->data.typematic_initial_ticks != 0u &&
        controller->data.typematic_repeat_ticks != 0u &&
        (scan_code & 0x80u) == 0u &&
        core_machine_kbc_is_typematic_scan_code(scan_code)) {
        controller->data.typematic_scan_code = scan_code;
        controller->data.typematic_remaining_ticks =
            controller->data.typematic_initial_ticks;
        controller->data.typematic_active = TYPE_TRUE;
    }
    return core_machine_kbc_enqueue(controller, scan_code,
        CORE_MACHINE_KBC_OUTPUT_KEYBOARD);
}

type_status core_machine_kbc_submit_scan_codes(t_kbc *controller,
    const uint8_t *scan_codes, STD_SIZE_T count)
{
    STD_SIZE_T index;

    if (controller == STD_NULL || (scan_codes == STD_NULL && count != 0u)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (!controller->data.keyboard_enabled || !controller->data.scanning_enabled) {
        return TYPE_STATUS_INVALID_STATE;
    }
    if (count > CORE_MACHINE_KBC_FIFO_CAPACITY - controller->data.fifo_count) {
        return TYPE_STATUS_INVALID_STATE;
    }
    for (index = 0u; index < count; ++index) {
        /* Capacity was reserved above; preserve per-byte typematic state. */
        (C_VOID)core_machine_kbc_submit_scan_code(controller, scan_codes[index]);
    }
    return TYPE_STATUS_OK;
}

static C_VOID core_machine_kbc_encode_aux_delta(int16_t delta, uint8_t sign_bit,
    uint8_t overflow_bit, uint8_t *packet_first, uint8_t *packet_data)
{
    if (delta > 255) {
        *packet_first |= overflow_bit;
        *packet_data = 0xffu;
    } else if (delta < -256) {
        *packet_first |= sign_bit | overflow_bit;
        *packet_data = 0x00u;
    } else {
        *packet_data = (uint8_t)delta;
        if (delta < 0) *packet_first |= sign_bit;
    }
}

type_status core_machine_kbc_submit_aux_report(t_kbc *controller,
    int16_t delta_x, int16_t delta_y, uint8_t buttons)
{
    uint8_t packet[3];

    if (controller == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    buttons &= 0x07u;
    if (!controller->data.aux_enabled || !controller->data.aux_reporting_enabled) {
        return TYPE_STATUS_INVALID_STATE;
    }
    if (delta_x == 0 && delta_y == 0 && buttons == controller->data.aux_button_state) {
        return TYPE_STATUS_OK;
    }
    if (controller->data.delayed_response_count != 0u ||
        CORE_MACHINE_KBC_FIFO_CAPACITY - controller->data.fifo_count <
            sizeof(packet)) return TYPE_STATUS_INVALID_STATE;
    packet[0] = (uint8_t)(0x08u | buttons);
    core_machine_kbc_encode_aux_delta(delta_x, 0x10u, 0x40u,
        &packet[0], &packet[1]);
    core_machine_kbc_encode_aux_delta(delta_y, 0x20u, 0x80u,
        &packet[0], &packet[2]);
    (C_VOID)core_machine_kbc_enqueue(controller, packet[0],
        CORE_MACHINE_KBC_OUTPUT_AUX);
    (C_VOID)core_machine_kbc_enqueue(controller, packet[1],
        CORE_MACHINE_KBC_OUTPUT_AUX);
    (C_VOID)core_machine_kbc_enqueue(controller, packet[2],
        CORE_MACHINE_KBC_OUTPUT_AUX);
    controller->data.aux_button_state = buttons;
    return TYPE_STATUS_OK;
}
