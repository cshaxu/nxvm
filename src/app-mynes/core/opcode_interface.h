#ifndef CORE_OPCODE_INTERFACE_H
#define CORE_OPCODE_INTERFACE_H

#include "lib/types/types_interface.h"

typedef enum core_opcode_address_mode {
    CORE_OPCODE_ADDRESS_IMPLIED,
    CORE_OPCODE_ADDRESS_ACCUMULATOR,
    CORE_OPCODE_ADDRESS_IMMEDIATE,
    CORE_OPCODE_ADDRESS_ZERO_PAGE,
    CORE_OPCODE_ADDRESS_ZERO_PAGE_X,
    CORE_OPCODE_ADDRESS_ZERO_PAGE_Y,
    CORE_OPCODE_ADDRESS_ABSOLUTE,
    CORE_OPCODE_ADDRESS_ABSOLUTE_X,
    CORE_OPCODE_ADDRESS_ABSOLUTE_Y,
    CORE_OPCODE_ADDRESS_INDIRECT,
    CORE_OPCODE_ADDRESS_INDIRECT_X,
    CORE_OPCODE_ADDRESS_INDIRECT_Y,
    CORE_OPCODE_ADDRESS_RELATIVE
} core_opcode_address_mode;

typedef struct core_opcode_metadata {
    char mnemonic[4];
    core_opcode_address_mode mode;
    lib_u8 bytes;
} core_opcode_metadata;

lib_bool core_opcode_describe(lib_u8 opcode, core_opcode_metadata *out_metadata);

#endif
