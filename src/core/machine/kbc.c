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

static C_VOID core_machine_kbc_request_irq1(t_kbc *controller)
{
    if (controller->connect.pic_master != STD_NULL &&
        (controller->data.command_byte & CORE_MACHINE_KBC_COMMAND_IRQ1) != 0u) {
        core_machine_pic_set_irq(controller->connect.pic_master,
            controller->connect.pic_slave, 0x01u);
    }
}

static type_status core_machine_kbc_enqueue(t_kbc *controller, uint8_t value)
{
    type_unsigned_8 tail;

    if (controller == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (controller->data.fifo_count >= CORE_MACHINE_KBC_FIFO_CAPACITY) {
        return TYPE_STATUS_INVALID_STATE;
    }
    tail = (type_unsigned_8)((controller->data.fifo_head +
        controller->data.fifo_count) % CORE_MACHINE_KBC_FIFO_CAPACITY);
    controller->data.fifo[tail] = value;
    ++controller->data.fifo_count;
    core_machine_kbc_request_irq1(controller);
    return TYPE_STATUS_OK;
}

static uint8_t core_machine_kbc_dequeue(t_kbc *controller)
{
    uint8_t value = 0u;

    if (controller == STD_NULL || controller->data.fifo_count == 0u) return 0u;
    value = controller->data.fifo[controller->data.fifo_head];
    controller->data.fifo_head = (type_unsigned_8)((controller->data.fifo_head + 1u) %
        CORE_MACHINE_KBC_FIFO_CAPACITY);
    --controller->data.fifo_count;
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
    switch (command) {
    case 0xffu:
        (C_VOID)core_machine_kbc_enqueue(controller, CORE_MACHINE_KBC_ACK);
        (C_VOID)core_machine_kbc_enqueue(controller, CORE_MACHINE_KBC_BAT_OK);
        controller->data.scanning_enabled = TYPE_TRUE;
        break;
    case 0xf4u:
        controller->data.scanning_enabled = TYPE_TRUE;
        (C_VOID)core_machine_kbc_enqueue(controller, CORE_MACHINE_KBC_ACK);
        break;
    case 0xf5u:
        controller->data.scanning_enabled = TYPE_FALSE;
        (C_VOID)core_machine_kbc_enqueue(controller, CORE_MACHINE_KBC_ACK);
        break;
    case 0xf2u:
        (C_VOID)core_machine_kbc_enqueue(controller, CORE_MACHINE_KBC_ACK);
        (C_VOID)core_machine_kbc_enqueue(controller, CORE_MACHINE_KBC_IDENTIFY_0);
        (C_VOID)core_machine_kbc_enqueue(controller, CORE_MACHINE_KBC_IDENTIFY_1);
        break;
    default:
        (C_VOID)core_machine_kbc_enqueue(controller, CORE_MACHINE_KBC_RESEND);
        break;
    }
}

static C_VOID core_machine_kbc_read_data(t_port *port, type_unsigned_16 port_id,
    C_VOID *owner)
{
    t_kbc *controller = (t_kbc *)owner;

    (C_VOID)port_id;
    port->data.ioByte = core_machine_kbc_dequeue(controller);
    if (controller != STD_NULL && controller->data.fifo_count != 0u) {
        core_machine_kbc_request_irq1(controller);
    }
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
        controller->data.pending_write = CORE_MACHINE_KBC_PENDING_NONE;
        break;
    case CORE_MACHINE_KBC_PENDING_OUTPUT_PORT:
        core_machine_kbc_apply_output_port(controller, value);
        controller->data.pending_write = CORE_MACHINE_KBC_PENDING_NONE;
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
        (C_VOID)core_machine_kbc_enqueue(controller, controller->data.command_byte);
        break;
    case 0x60u:
        controller->data.pending_write = CORE_MACHINE_KBC_PENDING_COMMAND_BYTE;
        break;
    case 0xaau:
        controller->data.system_flag = TYPE_TRUE;
        (C_VOID)core_machine_kbc_enqueue(controller, 0x55u);
        break;
    case 0xabu:
        (C_VOID)core_machine_kbc_enqueue(controller, 0x00u);
        break;
    case 0xadu:
        controller->data.keyboard_enabled = TYPE_FALSE;
        controller->data.command_byte |= CORE_MACHINE_KBC_COMMAND_DISABLE_KEYBOARD;
        break;
    case 0xaeu:
        controller->data.keyboard_enabled = TYPE_TRUE;
        controller->data.command_byte &= ~CORE_MACHINE_KBC_COMMAND_DISABLE_KEYBOARD;
        break;
    case 0xd0u:
        (C_VOID)core_machine_kbc_enqueue(controller, controller->data.output_port);
        break;
    case 0xd1u:
        controller->data.pending_write = CORE_MACHINE_KBC_PENDING_OUTPUT_PORT;
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
    controller->connect.pic_master = pic_master;
    controller->connect.pic_slave = pic_slave;
    controller->connect.memory = memory;
    controller->connect.execution = execution;
}
C_VOID core_machine_kbc_reset(t_kbc *controller)
{
    if (controller == STD_NULL) return;
    STD_MEMSET(&controller->data, TYPE_ZERO_8, sizeof(controller->data));
    controller->data.command_byte = CORE_MACHINE_KBC_COMMAND_IRQ1 |
        CORE_MACHINE_KBC_COMMAND_SYSTEM;
    controller->data.output_port = CORE_MACHINE_KBC_OUTPUT_RESET;
    controller->data.keyboard_enabled = TYPE_TRUE;
    controller->data.scanning_enabled = TYPE_TRUE;
    controller->data.system_flag = TYPE_TRUE;
    core_machine_kbc_apply_output_port(controller, controller->data.output_port);
}
C_VOID core_machine_kbc_refresh(t_kbc *controller) { (C_VOID)controller; }
C_VOID core_machine_kbc_finalize(t_kbc *controller) { (C_VOID)controller; }
type_status core_machine_kbc_submit_scan_code(t_kbc *controller, uint8_t scan_code)
{
    if (controller == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (!controller->data.keyboard_enabled || !controller->data.scanning_enabled) {
        return TYPE_STATUS_INVALID_STATE;
    }
    return core_machine_kbc_enqueue(controller, scan_code);
}
