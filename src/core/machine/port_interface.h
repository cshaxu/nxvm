#ifndef CORE_MACHINE_PORT_INTERFACE_H
#define CORE_MACHINE_PORT_INTERFACE_H


#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct core_machine core_machine;

typedef type_status (*core_machine_port_read_provider)(
    C_VOID *owner,
    type_unsigned_16 port,
    type_unsigned_32 *out_value);

typedef type_status (*core_machine_port_write_provider)(
    C_VOID *owner,
    type_unsigned_16 port,
    type_unsigned_32 value);

typedef struct core_machine_port_provider {
    core_machine_port_read_provider read;
    core_machine_port_write_provider write;
} core_machine_port_provider;

type_status core_machine_install_port_provider(
    core_machine *machine,
    type_unsigned_16 first,
    type_unsigned_16 last,
    const core_machine_port_provider *provider,
    C_VOID *owner);

type_status core_machine_bus_read(
    core_machine *machine,
    type_unsigned_16 port,
    type_unsigned_32 *out_value);

type_status core_machine_bus_write(
    core_machine *machine,
    type_unsigned_16 port,
    type_unsigned_32 value);

#ifdef __cplusplus
}
#endif

#endif
