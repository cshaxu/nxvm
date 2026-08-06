#ifndef CORE_MACHINE_ROM_MAPPING_INTERFACE_H
#define CORE_MACHINE_ROM_MAPPING_INTERFACE_H

#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct core_machine core_machine;

type_status core_machine_register_immutable_rom_mapping(
    core_machine *machine,
    uint32_t physical_start,
    const uint8_t *image,
    STD_SIZE_T bytes);

#ifdef __cplusplus
}
#endif

#endif
