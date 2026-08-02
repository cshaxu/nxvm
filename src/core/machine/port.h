/* Copyright 2012-2014 Neko. */

#ifndef NXVM_CORE_PORT_H
#define NXVM_CORE_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "core/machine/vglobal.h"

#define NXVM_DEVICE_PORT "Unknown I/O Port"

#define VPORT_MAX_PORT_COUNT 0x10000

typedef struct t_port t_port;
typedef void (*core_machine_port_handler)(t_port *port, t_nubit16 port_id,
    void *owner);

typedef struct core_machine_port_provider_entry
    core_machine_port_provider_entry;

typedef struct {
    t_faddrcc legacy_read[VPORT_MAX_PORT_COUNT];
    t_faddrcc legacy_write[VPORT_MAX_PORT_COUNT];
    core_machine_port_provider_entry *providers;
} t_port_connect;

typedef struct {
    union {
        t_nubit8  ioByte;
        t_nubit16 ioWord;
        t_nubit32 ioDWord;
    };
} t_port_data;

struct t_port {
    t_port_data data;
    t_port_connect connect;
};

void core_machine_port_execute_read(t_port *port, t_nubit16 port_id);
void core_machine_port_execute_write(t_port *port, t_nubit16 port_id);
void core_machine_port_add_read(t_port *port, t_nubit16 port_id,
    core_machine_port_handler handler, void *owner);
void core_machine_port_add_write(t_port *port, t_nubit16 port_id,
    core_machine_port_handler handler, void *owner);
uint32_t core_machine_port_read(t_port *port, uint16_t port_id);
void core_machine_port_write(t_port *port, uint16_t port_id, uint32_t value);
void core_machine_port_initialize(t_port *port);
void core_machine_port_reset(t_port *port);
void core_machine_port_finalize(t_port *port);


#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
