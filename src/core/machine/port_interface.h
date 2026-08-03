#ifndef NTVDM64_CORE_MACHINE_PORT_INTERFACE_H
#define NTVDM64_CORE_MACHINE_PORT_INTERFACE_H

#include <stdint.h>

#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct core_machine core_machine;

typedef nxvm_core_status (*core_machine_port_read_provider)(
    void *owner,
    uint16_t port,
    uint32_t *out_value);

typedef nxvm_core_status (*core_machine_port_write_provider)(
    void *owner,
    uint16_t port,
    uint32_t value);

typedef struct core_machine_port_provider {
    core_machine_port_read_provider read;
    core_machine_port_write_provider write;
} core_machine_port_provider;

nxvm_core_status core_machine_install_port_provider(
    core_machine *machine,
    uint16_t first,
    uint16_t last,
    const core_machine_port_provider *provider,
    void *owner);

nxvm_core_status core_machine_bus_read(
    core_machine *machine,
    uint16_t port,
    uint32_t *out_value);

nxvm_core_status core_machine_bus_write(
    core_machine *machine,
    uint16_t port,
    uint32_t value);

#ifdef __cplusplus
}
#endif

#endif
