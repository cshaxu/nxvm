#ifndef CORE_MACHINE_ROM_MAPPING_INTERFACE_H
#define CORE_MACHINE_ROM_MAPPING_INTERFACE_H
#include "lib/types/types_interface.h"

#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct core_machine core_machine;

/* ROM backing is immutable. In-range physical writes are accepted and
 * discarded, matching a non-writable ROM bus target. */
type_status core_machine_register_immutable_rom_mapping(
    core_machine *machine,
    lib_u32 physical_start,
    const lib_u8 *image,
    lib_size bytes);

/* Configuration-only alias of an existing core-owned immutable ROM subrange.
 * The source bytes remain owned by their original mapping. Ordinary routes
 * retain their provider order; reset-only fetches select an alias first. */
type_status core_machine_register_immutable_rom_mapping_alias(
    core_machine *machine, lib_u32 source_start,
    lib_u32 physical_start, lib_size bytes);
/* CPU reset aliases alone decode before ordinary board A20 routing. */
type_status core_machine_register_immutable_rom_mapping_reset_alias(
    core_machine *machine, lib_u32 source_start,
    lib_u32 physical_start, lib_size bytes);

#ifdef __cplusplus
}
#endif

#endif
