#ifndef CORE_MACHINE_ROM_MAPPING_INTERFACE_H
#define CORE_MACHINE_ROM_MAPPING_INTERFACE_H

#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct core_machine core_machine;

type_status core_machine_register_immutable_rom_mapping(
    core_machine *machine,
    type_unsigned_32 physical_start,
    const type_unsigned_8 *image,
    STD_SIZE_T bytes);

#ifdef __cplusplus
}
#endif

#endif
