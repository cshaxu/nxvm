/* Copyright 2012-2026 Neko. */
#ifndef X86_RB5C396_INTERFACE_H
#define X86_RB5C396_INTERFACE_H

#include "lib/types/types_interface.h"

typedef struct x86_rb5c396 x86_rb5c396;

#define X86_RB5C396_INDEX_PORT 0x03e0u
#define X86_RB5C396_DATA_PORT 0x03e1u
#define X86_RB5C396_SLOT_COUNT 2u
#define X86_RB5C396_REGISTER_COUNT 60u
#define X86_RB5C396_IDENTIFICATION_REGISTER 0x00u
#define X86_RB5C396_INTERFACE_STATUS_REGISTER 0x01u
#define X86_RB5C396_CHIP_IDENTIFICATION_REGISTER 0x3au
#define X86_RB5C396_IDENTIFICATION_VALUE 0x83u
#define X86_RB5C396_CHIP_IDENTIFICATION_VALUE 0xb2u

/* L2 controller model from Ricoh EA-028-9804, printed pages 27 and 43--45.
 * The controller receives physical socket levels from its board.  It does
 * not create a PC Card, map card storage or retain a host-side medium. */
typedef struct x86_rb5c396_slot_inputs {
    lib_bool card_present;
    lib_bool ready;
    lib_bool write_protected;
    lib_bool battery_detect_2;
    lib_bool battery_detect_1;
} x86_rb5c396_slot_inputs;

lib_status x86_rb5c396_create(x86_rb5c396 **out_chip);
void x86_rb5c396_destroy(x86_rb5c396 *chip);
/* Controller reset clears internal register state but preserves physical
 * socket levels supplied by its board. */
void x86_rb5c396_reset(x86_rb5c396 *chip);
lib_status x86_rb5c396_set_slot_inputs(x86_rb5c396 *chip, lib_u8 slot,
    const x86_rb5c396_slot_inputs *inputs);
lib_status x86_rb5c396_read(x86_rb5c396 *chip, lib_u16 port,
    lib_u8 *out_value);
lib_status x86_rb5c396_write(x86_rb5c396 *chip, lib_u16 port,
    lib_u8 value);

#endif
