#include "x86/devices/fpu/fpu.h"

typedef struct operation_case {
    lib_u8 opcode;
    lib_u8 modrm;
    x86_fpu_operation operation;
    lib_u64 legacy_ticks;
    lib_u64 i387_ticks;
    lib_u32 minimum;
    lib_u32 maximum;
} operation_case;

static lib_bool timing_cases(x86_fpu_profile profile)
{
    static const operation_case cases[] = {
        {0xdbu, 0xe3u, X86_FPU_OPERATION_FNINIT, 5u, 33u, 33u, 33u},
        {0xd9u, 0x00u, X86_FPU_OPERATION_FLD_M32, 47u, 14u, 9u, 18u},
        {0xd9u, 0x18u, X86_FPU_OPERATION_FSTP_M32, 87u, 34u, 25u, 43u},
        {0xd9u, 0x28u, X86_FPU_OPERATION_FLDCW_M16, 11u, 19u, 19u, 19u},
        {0xd8u, 0xc0u, X86_FPU_OPERATION_FADD_ST0_STI, 85u, 19u, 12u, 26u},
        {0xd8u, 0xc8u, X86_FPU_OPERATION_FMUL_ST0_STI, 117u, 34u, 17u, 50u},
        {0xd8u, 0xe0u, X86_FPU_OPERATION_FSUB_ST0_STI, 85u, 22u, 15u, 29u},
        {0xd8u, 0xf0u, X86_FPU_OPERATION_FDIV_ST0_STI, 198u, 79u, 77u, 80u},
        {0xdfu, 0xffu, X86_FPU_OPERATION_UNSUPPORTED, 85u, 28u, 0u, 0u}
    };
    x86_fpu *fpu = LIB_NULL;
    lib_bool failed = LIB_FALSE;
    lib_u64 remaining = 0u;

    if (x86_fpu_create(profile, &fpu) != LIB_STATUS_OK) return LIB_TRUE;
    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        const operation_case *item = &cases[index];
        const lib_u64 ticks = profile == X86_FPU_PROFILE_80387 ?
            item->i387_ticks : item->legacy_ticks;
        x86_fpu_operation_metadata metadata =
            x86_fpu_operation_metadata_get(item->opcode, item->modrm);
        failed |= metadata.operation != item->operation ||
            metadata.minimum_fpu != X86_FPU_PROFILE_8087 ||
            metadata.valid != (item->operation != X86_FPU_OPERATION_UNSUPPORTED);
        x86_fpu_begin_command(fpu, item->opcode, item->modrm);
        failed |= x86_fpu_ticks_until_completion(fpu, &remaining) != LIB_STATUS_OK ||
            remaining != ticks || !fpu->busy ||
            fpu->last_escape_opcode != item->opcode ||
            fpu->last_escape_modrm != item->modrm ||
            fpu->operation_ticks_min !=
                (profile == X86_FPU_PROFILE_80387 ? item->minimum : 0u) ||
            fpu->operation_ticks_max !=
                (profile == X86_FPU_PROFILE_80387 ? item->maximum : 0u);
        x86_fpu_advance(fpu, ticks - 3u);
        failed |= x86_fpu_ticks_until_completion(fpu, &remaining) != LIB_STATUS_OK ||
            remaining != 3u || x86_fpu_complete_wait(fpu) != 3u ||
            x86_fpu_last_wait_ticks(fpu) != 3u || fpu->busy ||
            x86_fpu_ticks_until_completion(fpu, &remaining) != LIB_STATUS_INVALID_STATE;
        x86_fpu_begin_command(fpu, item->opcode, item->modrm);
        x86_fpu_advance(fpu, ticks + 1u);
        failed |= fpu->busy || fpu->completion_remaining_ticks != 0u;
        failed |= x86_fpu_escape_dispatch(fpu, item->opcode, item->modrm) !=
            (profile == X86_FPU_PROFILE_8087 && metadata.valid ?
                X86_FPU_ESCAPE_EXECUTE_8087 : X86_FPU_ESCAPE_HANDOFF);
    }
    x86_fpu_destroy(fpu);
    return failed;
}

static lib_bool arithmetic_cases(void)
{
    static const struct {
        x86_fpu_operation operation;
        lib_u32 expected;
    } cases[] = {
        {X86_FPU_OPERATION_FADD_ST0_STI, 0x40400000u},
        {X86_FPU_OPERATION_FMUL_ST0_STI, 0x40000000u},
        {X86_FPU_OPERATION_FSUB_ST0_STI, 0x3f800000u},
        {X86_FPU_OPERATION_FDIV_ST0_STI, 0x40000000u}
    };
    x86_fpu *fpu = LIB_NULL;
    x86_fpu *other = LIB_NULL;
    x86_fpu_state state;
    lib_u32 bits = 0u;
    lib_bool failed = LIB_FALSE;

    if (x86_fpu_create(X86_FPU_PROFILE_8087, &fpu) != LIB_STATUS_OK) return LIB_TRUE;
    if (x86_fpu_create(X86_FPU_PROFILE_NONE, &other) != LIB_STATUS_OK) {
        x86_fpu_destroy(fpu);
        return LIB_TRUE;
    }
    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        x86_fpu_reset(fpu);
        failed |= x86_fpu_load_m32(fpu, 0x3f800000u) != X86_FPU_EXECUTE_COMPLETED ||
            x86_fpu_load_m32(fpu, 0x40000000u) != X86_FPU_EXECUTE_COMPLETED ||
            x86_fpu_binary_st0_sti(fpu, cases[index].operation, 1u) !=
                X86_FPU_EXECUTE_COMPLETED ||
            x86_fpu_store_m32(fpu, &bits) != X86_FPU_EXECUTE_COMPLETED ||
            bits != cases[index].expected;
    }
    x86_fpu_reset(fpu);
    failed |= x86_fpu_load_m32(fpu, 0x7fc00000u) != X86_FPU_EXECUTE_UNSUPPORTED ||
        x86_fpu_load_m32(fpu, 1u) != X86_FPU_EXECUTE_UNSUPPORTED;
    x86_fpu_load_control_word(fpu, 0x037eu);
    failed |= x86_fpu_store_m32(fpu, &bits) != X86_FPU_EXECUTE_COMPLETED ||
        !x86_fpu_wait_pending(fpu);
    x86_fpu_get_state(fpu, &state);
    failed |= (state.status_word & 0x00c1u) != 0x00c1u ||
        !state.pending_unmasked_exception;
    x86_fpu_get_state(other, &state);
    failed |= state.status_word != 0u || state.pending_unmasked_exception;
    x86_fpu_begin_command(other, 0xdbu, 0xe3u);
    failed |= other->busy || x86_fpu_escape_dispatch(other, 0xdbu, 0xe3u) !=
        X86_FPU_ESCAPE_CONSUME_NONE;
    x86_fpu_reset(fpu);
    x86_fpu_get_state(fpu, &state);
    failed |= state.control_word != 0x037fu || state.status_word != 0u ||
        x86_fpu_get_profile(fpu) != X86_FPU_PROFILE_8087;
    for (lib_size index = 0u; index < 8u; ++index) {
        failed |= state.tags[index] != X86_FPU_TAG_EMPTY;
    }
    x86_fpu_destroy(other);
    x86_fpu_destroy(fpu);
    return failed;
}

lib_i32 main(void)
{
    x86_fpu *fpu = LIB_NULL;
    lib_bool failed = x86_fpu_create(X86_FPU_PROFILE_8087, LIB_NULL) !=
        LIB_STATUS_INVALID_ARGUMENT;

    failed |= x86_fpu_create((x86_fpu_profile)99, &fpu) !=
        LIB_STATUS_INVALID_ARGUMENT || fpu != LIB_NULL;
    failed |= arithmetic_cases();
    failed |= timing_cases(X86_FPU_PROFILE_8087);
    failed |= timing_cases(X86_FPU_PROFILE_80287);
    failed |= timing_cases(X86_FPU_PROFILE_80387);
    return failed ? 1 : 0;
}
