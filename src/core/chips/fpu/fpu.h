#include "lib/types/types_interface.h"
#ifndef X86_FPU_H
#define X86_FPU_H

#include "core/chips/fpu/fpu_interface.h"

typedef enum x86_fpu_value_kind {
    X86_FPU_VALUE_ZERO,
    X86_FPU_VALUE_FINITE,
    X86_FPU_VALUE_INFINITY
} x86_fpu_value_kind;

typedef struct x86_fpu_value {
    x86_fpu_value_kind kind;
    lib_u8 negative;
    lib_i16 exponent;
    lib_u32 significand;
} x86_fpu_value;

struct x86_fpu {
    x86_fpu_profile profile;
    lib_u16 control_word;
    lib_u16 status_word;
    lib_u8 top;
    x86_fpu_tag tags[8];
    x86_fpu_value registers[8];
    lib_u8 pending_unmasked_exception;
    /* BUSY and ERROR are independent processor-extension signals.  FPU
     * completion is measured on the sole Core elapsed axis and is never
     * folded into CPU ESC retirement time. */
    lib_u8 busy;
    lib_u8 last_escape_opcode;
    lib_u8 last_escape_modrm;
    lib_u32 operation_ticks_min;
    lib_u32 operation_ticks_max;
    /* An External-L2 operation interval remains in the FPU owner until Core
     * time advances it.  FWAIT consumes only the remaining source ticks and
     * publishes that one wait contribution with its own CPU retirement. */
    lib_u64 completion_remaining_ticks;
};

#endif
