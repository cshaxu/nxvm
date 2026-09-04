/* Copyright 2012-2014 Neko. */

#ifndef CORE_MACHINE_PORT_H
#define CORE_MACHINE_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"
#include "core/machine/port_interface.h"

#define CORE_MACHINE_DEVICE_PORT "Unknown I/O Port"

#define VPORT_MAX_PORT_COUNT 0x10000

typedef struct t_port t_port;
typedef C_VOID (*core_machine_port_handler)(t_port *port, type_unsigned_16 port_id,
    C_VOID *owner);

typedef struct core_machine_port_provider_entry
    core_machine_port_provider_entry;

typedef struct core_machine_port_test_allocation {
    STD_SIZE_T fail_at;
    STD_SIZE_T attempts;
} core_machine_port_test_allocation;

typedef struct {
    core_machine_port_provider_entry *providers;
    core_machine_port_test_allocation *test_allocation;
    type_status registration_status;
} t_port_connect;

typedef struct {
    union {
        type_unsigned_8  ioByte;
        type_unsigned_16 ioWord;
        type_unsigned_32 ioDWord;
    };
    /* Width of the CPU transfer currently delivered to the registered
     * endpoint.  Ordinary debugger/bus access is one byte. */
    type_unsigned_8 access_bytes;
} t_port_data;

struct t_port {
    t_port_data data;
    t_port_connect connect;
};

type_status core_machine_port_execute_read(t_port *port, type_unsigned_16 port_id);
type_status core_machine_port_execute_write(t_port *port, type_unsigned_16 port_id);
/* CPU IN/OUT widths are one bus transaction at the addressed port.  The
 * provider remains the sole owner of the register value and receives one
 * read/write callback regardless of the CPU transfer width. */
type_status core_machine_port_execute_read_width(t_port *port,
    type_unsigned_16 port_id, type_unsigned_8 bytes);
type_status core_machine_port_execute_write_width(t_port *port,
    type_unsigned_16 port_id, type_unsigned_8 bytes);
type_status core_machine_port_add_read(t_port *port, type_unsigned_16 port_id,
    core_machine_port_handler handler, C_VOID *owner);
type_status core_machine_port_add_write(t_port *port, type_unsigned_16 port_id,
    core_machine_port_handler handler, C_VOID *owner);
type_status core_machine_port_add_read_provider(t_port *port,
    type_unsigned_16 port_id, core_machine_port_read_provider provider,
    C_VOID *owner);
type_status core_machine_port_add_read_wired_or_provider(t_port *port,
    type_unsigned_16 port_id, core_machine_port_read_provider provider,
    C_VOID *owner);
type_status core_machine_port_add_write_provider(t_port *port,
    type_unsigned_16 port_id, core_machine_port_write_provider provider,
    C_VOID *owner);
C_INT core_machine_port_has_read(const t_port *port, type_unsigned_16 port_id);
C_INT core_machine_port_has_write(const t_port *port, type_unsigned_16 port_id);
type_unsigned_32 core_machine_port_read(t_port *port, type_unsigned_16 port_id);
C_VOID core_machine_port_write(t_port *port, type_unsigned_16 port_id, type_unsigned_32 value);
C_VOID core_machine_port_initialize(t_port *port);
C_VOID core_machine_port_reset(t_port *port);
C_VOID core_machine_port_finalize(t_port *port);
core_machine_port_provider_entry *core_machine_port_registration_begin(t_port *port);
type_status core_machine_port_registration_status(const t_port *port);
C_VOID core_machine_port_rollback_registration(t_port *port,
    core_machine_port_provider_entry *checkpoint);
C_VOID core_machine_port_set_test_allocation(t_port *port,
    core_machine_port_test_allocation *test_allocation);


#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
