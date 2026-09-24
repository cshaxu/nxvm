#ifndef CORE_MACHINE_PORT_INTERFACE_H
#define CORE_MACHINE_PORT_INTERFACE_H
#include "lib/types/types_interface.h"



#ifdef __cplusplus
extern "C" {
#endif

typedef struct core_machine core_machine;

typedef lib_status (*core_machine_port_read_provider)(
    void *owner,
    lib_u16 port,
    lib_u32 *out_value);

typedef lib_status (*core_machine_port_write_provider)(
    void *owner,
    lib_u16 port,
    lib_u32 value);

typedef struct core_machine_port_provider {
    core_machine_port_read_provider read;
    core_machine_port_write_provider write;
} core_machine_port_provider;

lib_status core_machine_install_port_provider(
    core_machine *machine,
    lib_u16 first,
    lib_u16 last,
    const core_machine_port_provider *provider,
    void *owner);

lib_status core_machine_bus_read(
    core_machine *machine,
    lib_u16 port,
    lib_u32 *out_value);

lib_status core_machine_bus_write(
    core_machine *machine,
    lib_u16 port,
    lib_u32 value);

#ifdef __cplusplus
}
#endif

#endif
