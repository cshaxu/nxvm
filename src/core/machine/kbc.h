/* Copyright 2012-2014 Neko. */

#ifndef CORE_MACHINE_KBC_H
#define CORE_MACHINE_KBC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"
#include "core/machine/port.h"

#define CORE_MACHINE_DEVICE_KBC "Intel 8042"

typedef struct t_pic t_pic;
typedef struct t_ram t_ram;
typedef struct core_machine_cpu_execution_context
    core_machine_cpu_execution_context;

#define CORE_MACHINE_KBC_FIFO_CAPACITY 16u

#define VKBC_STATUS_OBF 0x01 /* output buffer contains a byte */
#define VKBC_STATUS_IBF 0x02 /* synchronous command/data processing */
#define VKBC_STATUS_SYS 0x04 /* controller self test/system flag */
#define VKBC_STATUS_CD  0x08 /* last write selected the command port */
#define VKBC_STATUS_KE  0x10 /* keyboard interface enabled */

typedef enum core_machine_kbc_pending_write {
    CORE_MACHINE_KBC_PENDING_NONE,
    CORE_MACHINE_KBC_PENDING_COMMAND_BYTE,
    CORE_MACHINE_KBC_PENDING_OUTPUT_PORT
} core_machine_kbc_pending_write;

typedef struct t_kbc_data {
    type_unsigned_8 command_byte;
    type_unsigned_8 output_port;
    type_unsigned_8 fifo[CORE_MACHINE_KBC_FIFO_CAPACITY];
    type_unsigned_8 fifo_head;
    type_unsigned_8 fifo_count;
    core_machine_kbc_pending_write pending_write;
    type_bool keyboard_enabled;
    type_bool scanning_enabled;
    type_bool system_flag;
    type_bool input_buffer_full;
    type_bool last_write_command;
} t_kbc_data;

typedef struct t_kbc_connect {
    t_pic *pic_master;
    t_pic *pic_slave;
    t_ram *memory;
    core_machine_cpu_execution_context *execution;
} t_kbc_connect;

typedef struct t_kbc {
    t_kbc_data data;
    t_kbc_connect connect;
} t_kbc;

C_VOID core_machine_kbc_register_ports(t_kbc *controller, t_port *port);
C_VOID core_machine_kbc_initialize(t_kbc *controller, t_port *port);
C_VOID core_machine_kbc_bind_core_services(t_kbc *controller, t_pic *pic_master,
    t_pic *pic_slave, t_ram *memory,
    core_machine_cpu_execution_context *execution);
C_VOID core_machine_kbc_reset(t_kbc *controller);
C_VOID core_machine_kbc_refresh(t_kbc *controller);
C_VOID core_machine_kbc_finalize(t_kbc *controller);
type_status core_machine_kbc_submit_scan_code(t_kbc *controller, uint8_t scan_code);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
