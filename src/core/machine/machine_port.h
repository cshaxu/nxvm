#ifndef NXVM_CORE_MACHINE_PORT_H
#define NXVM_CORE_MACHINE_PORT_H

#include <stdint.h>

#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nxvm_core_machine nxvm_core_machine;

typedef nxvm_core_status (*nxvm_core_port_read_handler)(
    void *owner,
    uint16_t port,
    uint32_t *out_value);

typedef nxvm_core_status (*nxvm_core_port_write_handler)(
    void *owner,
    uint16_t port,
    uint32_t value);

typedef struct nxvm_core_port_ops {
    nxvm_core_port_read_handler read;
    nxvm_core_port_write_handler write;
} nxvm_core_port_ops;

nxvm_core_status nxvm_core_machine_install_port(
    nxvm_core_machine *machine,
    uint16_t first,
    uint16_t last,
    const nxvm_core_port_ops *ops,
    void *owner);

nxvm_core_status nxvm_core_machine_port_read(
    nxvm_core_machine *machine,
    uint16_t port,
    uint32_t *out_value);

nxvm_core_status nxvm_core_machine_port_write(
    nxvm_core_machine *machine,
    uint16_t port,
    uint32_t value);

#ifdef __cplusplus
}
#endif

#endif
