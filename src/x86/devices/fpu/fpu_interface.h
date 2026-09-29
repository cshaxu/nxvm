#ifndef X86_FPU_INTERFACE_H
#define X86_FPU_INTERFACE_H
#include "lib/types/types_interface.h"


/* Partial 8087 arithmetic and 8087/287/387 extension timing; not full x87. */

typedef struct x86_fpu x86_fpu;

typedef enum x86_fpu_tag {
    X86_FPU_TAG_EMPTY = 3,
    X86_FPU_TAG_VALID = 0
} x86_fpu_tag;

typedef enum x86_fpu_profile {
    X86_FPU_PROFILE_NONE = 0,
    X86_FPU_PROFILE_8087,
    X86_FPU_PROFILE_80287,
    X86_FPU_PROFILE_80387
} x86_fpu_profile;

typedef enum x86_fpu_operation {
    X86_FPU_OPERATION_CONSUME_NONE = 0,
    X86_FPU_OPERATION_FNINIT,
    X86_FPU_OPERATION_FLD_M32,
    X86_FPU_OPERATION_FSTP_M32,
    X86_FPU_OPERATION_FLDCW_M16,
    X86_FPU_OPERATION_FADD_ST0_STI,
    X86_FPU_OPERATION_FMUL_ST0_STI,
    X86_FPU_OPERATION_FSUB_ST0_STI,
    X86_FPU_OPERATION_FDIV_ST0_STI,
    X86_FPU_OPERATION_UNSUPPORTED
} x86_fpu_operation;

typedef struct x86_fpu_operation_metadata {
    x86_fpu_profile minimum_fpu;
    x86_fpu_operation operation;
    lib_i32 valid;
} x86_fpu_operation_metadata;

typedef struct x86_fpu_state {
    lib_u16 control_word;
    lib_u16 status_word;
    lib_u8 top;
    lib_u8 tags[8];
    lib_i32 pending_unmasked_exception;
} x86_fpu_state;

x86_fpu_operation_metadata x86_fpu_operation_metadata_get(
    lib_u8 escape_opcode, lib_u8 modrm);

typedef enum x86_fpu_escape_action {
    X86_FPU_ESCAPE_CONSUME_NONE,
    X86_FPU_ESCAPE_HANDOFF,
    X86_FPU_ESCAPE_EXECUTE_8087,
    X86_FPU_ESCAPE_UNSUPPORTED
} x86_fpu_escape_action;

typedef enum x86_fpu_execute_result {
    X86_FPU_EXECUTE_COMPLETED,
    X86_FPU_EXECUTE_UNSUPPORTED
} x86_fpu_execute_result;

/* One execution owner serializes operations. Stop using the instance before
 * destroy. Creation failure clears out_fpu; reset retains its variant. */
lib_status x86_fpu_create(x86_fpu_profile profile, x86_fpu **out_fpu);
void x86_fpu_destroy(x86_fpu *fpu);
x86_fpu_profile x86_fpu_get_profile(const x86_fpu *fpu);
void x86_fpu_reset(x86_fpu *fpu);
x86_fpu_escape_action x86_fpu_escape_dispatch(
    const x86_fpu *fpu,
    lib_u8 escape_opcode, lib_u8 modrm);
/* Begin a validated ESC command after any supported local semantic update.
 * The selected interval is a source-axis External-L2 model; it contains no
 * board callback or host-time dependency. */
void x86_fpu_begin_command(x86_fpu *fpu,
    lib_u8 escape_opcode, lib_u8 modrm);
void x86_fpu_advance(x86_fpu *fpu,
    lib_u64 elapsed_ticks);
lib_status x86_fpu_ticks_until_completion(const x86_fpu *fpu,
    lib_u64 *out_ticks);
void x86_fpu_get_state(const x86_fpu *fpu,
    x86_fpu_state *out_state);
x86_fpu_execute_result x86_fpu_load_m32(x86_fpu *fpu,
    lib_u32 bits);
x86_fpu_execute_result x86_fpu_store_m32(x86_fpu *fpu,
    lib_u32 *out_bits);
void x86_fpu_load_control_word(x86_fpu *fpu,
    lib_u16 control_word);
x86_fpu_execute_result x86_fpu_binary_st0_sti(x86_fpu *fpu,
    x86_fpu_operation operation, lib_u8 index);
lib_u8 x86_fpu_wait_pending(const x86_fpu *fpu);
lib_u64 x86_fpu_complete_wait(x86_fpu *fpu);
lib_u64 x86_fpu_last_wait_ticks(const x86_fpu *fpu);

#endif
