#ifndef X86_VL82C420_INTERFACE_H
#define X86_VL82C420_INTERFACE_H

#include "lib/types/types_interface.h"

/* Address-neutral model of the PC110 VL82C420 configuration banks.
 * The board owns port decoding and any source-qualified reset image. */

typedef struct x86_vl82c420 x86_vl82c420;

typedef enum x86_vl82c420_bank {
    X86_VL82C420_BANK_SCAMP = 0,
    X86_VL82C420_BANK_BLOCK2,
    X86_VL82C420_BANK_ECED
} x86_vl82c420_bank;

typedef enum x86_vl82c420_unlock_read {
    X86_VL82C420_UNLOCK_READ_FC23 = 0,
    X86_VL82C420_UNLOCK_READ_F023,
    X86_VL82C420_UNLOCK_READ_C023,
    X86_VL82C420_UNLOCK_READ_0023
} x86_vl82c420_unlock_read;

lib_status x86_vl82c420_create(x86_vl82c420 **out_chip);
void x86_vl82c420_destroy(x86_vl82c420 *chip);

/* Reset only transient protocol state. Bank contents persist across a warm
 * reset; a later board binding supplies any source-qualified cold image. */
void x86_vl82c420_reset_protocol(x86_vl82c420 *chip);
void x86_vl82c420_observe_unlock_read(x86_vl82c420 *chip,
    x86_vl82c420_unlock_read read);
void x86_vl82c420_relock(x86_vl82c420 *chip);

lib_status x86_vl82c420_select(x86_vl82c420 *chip,
    x86_vl82c420_bank bank, lib_u8 index);
lib_status x86_vl82c420_read_selected(const x86_vl82c420 *chip,
    x86_vl82c420_bank bank, lib_u8 *out_value);
/* Observational board decode: preserve all selected indices and unlock state.
 * The same bank access rules and SCAMP index mask apply as to indexed I/O. */
lib_status x86_vl82c420_read_register(const x86_vl82c420 *chip,
    x86_vl82c420_bank bank, lib_u8 index, lib_u8 *out_value);
lib_status x86_vl82c420_write_selected(x86_vl82c420 *chip,
    x86_vl82c420_bank bank, lib_u8 value);

#endif
