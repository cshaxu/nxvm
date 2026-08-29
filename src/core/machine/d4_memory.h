#ifndef CORE_MACHINE_D4_MEMORY_H
#define CORE_MACHINE_D4_MEMORY_H

#include "type.h"
#include "core/machine/memory_interface.h"

typedef struct core_machine core_machine;
typedef struct core_machine_d4_memory_config core_machine_d4_memory_config;

#define CORE_MACHINE_D4_MEMORY_BYTES (128u * 1024u)
#define CORE_MACHINE_D4_MEMORY_ROM_CHIP_BYTES (16u * 1024u)

typedef struct core_machine_d4_memory {
    type_unsigned_8 compatibility[CORE_MACHINE_D4_MEMORY_BYTES];
    type_unsigned_8 control;
    type_unsigned_8 diagnostic_high;
    type_unsigned_16 reset_ram_setup;
    type_unsigned_16 ram_setup;
    type_unsigned_8 parity_fault_mask;
    type_bool configured;
} core_machine_d4_memory;

C_INT core_machine_d4_memory_config_is_valid(
    const core_machine_d4_memory_config *config);
type_status core_machine_d4_memory_configure(core_machine *machine,
    const core_machine_d4_memory_config *config);
C_VOID core_machine_d4_memory_reset(core_machine *machine);

#endif
