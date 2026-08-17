#ifndef CORE_MACHINE_ROM_MAPPING_INTERFACE_H
#define CORE_MACHINE_ROM_MAPPING_INTERFACE_H

#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct core_machine core_machine;

/* ROM backing is immutable. In-range physical writes are accepted and
 * discarded, matching a non-writable ROM bus target. */
type_status core_machine_register_immutable_rom_mapping(
    core_machine *machine,
    type_unsigned_32 physical_start,
    const type_unsigned_8 *image,
    STD_SIZE_T bytes);

/* Configuration-only alias of an existing core-owned immutable ROM subrange.
 * The source bytes remain owned by their original mapping. Earlier providers
 * retain route priority where the alias target overlaps them. */
type_status core_machine_register_immutable_rom_mapping_alias(
    core_machine *machine, type_unsigned_32 source_start,
    type_unsigned_32 physical_start, STD_SIZE_T bytes);

#ifdef __cplusplus
}
#endif

#endif
