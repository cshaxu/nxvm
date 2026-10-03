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
    lib_u64 tick,
    lib_u32 *out_value);

typedef lib_status (*core_machine_port_write_provider)(
    void *owner,
    lib_u16 port,
    lib_u32 value);

typedef struct core_machine_port_provider {
    core_machine_port_read_provider read;
    core_machine_port_write_provider write;
} core_machine_port_provider;

/* Read tick is the Core guest time at dispatch, copied unchanged to each
 * byte lane and wired-OR contributor. A provider must not derive another
 * clock. Writes consume their settled device state.
 * The Core owns route storage. The caller retains each callback context until
 * its routes are removed or the machine is destroyed. */
typedef struct core_machine_port_route {
    lib_u16 address;
    core_machine_port_read_provider read;
    core_machine_port_write_provider write;
    void *owner;
    lib_bool wired_or_read;
    /* Exclusive end of a bank of eight-bit latches. A wide access wholly
     * inside this bank calls the same provider once per consecutive lane. */
    lib_u16 byte_lane_end;
} core_machine_port_route;

/* All routes are installed or none are. Route order is observable only for
 * explicit wired-OR contributors to an already installed read route. */
lib_status core_machine_install_port_routes(
    core_machine *machine,
    const core_machine_port_route *routes,
    lib_size count);

/* Construction rollback or teardown only, with execution stopped. Removes
 * only routes published with this owner token. */
lib_status core_machine_remove_port_routes(core_machine *machine,
    const void *owner);

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
