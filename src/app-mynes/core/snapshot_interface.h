#ifndef CORE_SNAPSHOT_INTERFACE_H
#define CORE_SNAPSHOT_INTERFACE_H

#include "core/machine_interface.h"

typedef lib_status (*core_snapshot_write_callback)(void *context,
    const lib_u8 *bytes, lib_size byte_count);
typedef lib_status (*core_snapshot_read_callback)(void *context,
    lib_u8 *bytes, lib_size byte_count);

/* Product-private, portable-by-definition state stream.  The caller supplies
 * a currently loaded matching cartridge; this API never opens paths or embeds
 * ROM content.  Read validates the complete candidate before touching live
 * machine state. */
lib_status core_snapshot_write(const core_machine *machine,
    core_snapshot_write_callback write, void *context);
lib_status core_snapshot_read(core_machine *machine,
    core_snapshot_read_callback read, void *context);

#endif
