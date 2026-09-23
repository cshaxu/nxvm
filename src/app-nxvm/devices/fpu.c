#include "lib/types/types_interface.h"
#include "type.h"

#include "app-nxvm/devices/fpu.h"

#define CORE_MACHINE_FPU_CONTROL_DEFAULT 0x037fu
#define CORE_MACHINE_FPU_CONTROL_EXCEPTION_MASK 0x003fu
#define CORE_MACHINE_FPU_STATUS_IE 0x0001u
#define CORE_MACHINE_FPU_STATUS_ZE 0x0004u
#define CORE_MACHINE_FPU_STATUS_SF 0x0040u
#define CORE_MACHINE_FPU_STATUS_ES 0x0080u
#define CORE_MACHINE_FPU_STATUS_TOP 0x3800u

static lib_u8 core_machine_fpu_physical_index(const core_machine_fpu *fpu,
    lib_u8 logical_index)
{
    return (lib_u8)((fpu->top + logical_index) & 7u);
}

static C_VOID core_machine_fpu_sync_top(core_machine_fpu *fpu)
{
    fpu->status_word = (lib_u16)((fpu->status_word & ~CORE_MACHINE_FPU_STATUS_TOP) |
        ((lib_u16)fpu->top << 11u));
}

static C_VOID core_machine_fpu_raise(core_machine_fpu *fpu,
    lib_u16 status_bits, lib_u16 mask_bits)
{
    fpu->status_word = (lib_u16)(fpu->status_word | status_bits);
    if ((fpu->control_word & mask_bits) != mask_bits) {
        fpu->status_word = (lib_u16)(fpu->status_word | CORE_MACHINE_FPU_STATUS_ES);
        fpu->pending_unmasked_exception = LIB_TRUE;
    }
}

static C_INT core_machine_fpu_push(core_machine_fpu *fpu,
    const core_machine_fpu_value *value)
{
    lib_u8 index = (lib_u8)((fpu->top + 7u) & 7u);

    if (fpu->tags[index] != CORE_MACHINE_FPU_TAG_EMPTY) {
        core_machine_fpu_raise(fpu, CORE_MACHINE_FPU_STATUS_IE |
            CORE_MACHINE_FPU_STATUS_SF, CORE_MACHINE_FPU_STATUS_IE);
        return 0;
    }
    fpu->top = index;
    fpu->registers[index] = *value;
    fpu->tags[index] = CORE_MACHINE_FPU_TAG_VALID;
    core_machine_fpu_sync_top(fpu);
    return 1;
}

static C_INT core_machine_fpu_st(const core_machine_fpu *fpu, lib_u8 logical,
    core_machine_fpu_value *out_value, lib_u8 *out_physical)
{
    lib_u8 index;

    if (logical >= 8u) return 0;
    index = core_machine_fpu_physical_index(fpu, logical);
    if (fpu->tags[index] == CORE_MACHINE_FPU_TAG_EMPTY) return 0;
    if (out_value != LIB_NULL) *out_value = fpu->registers[index];
    if (out_physical != LIB_NULL) *out_physical = index;
    return 1;
}

static C_VOID core_machine_fpu_stack_fault(core_machine_fpu *fpu)
{
    core_machine_fpu_raise(fpu, CORE_MACHINE_FPU_STATUS_IE |
        CORE_MACHINE_FPU_STATUS_SF, CORE_MACHINE_FPU_STATUS_IE);
}

static C_INT core_machine_fpu_decode_m32(lib_u32 bits,
    core_machine_fpu_value *out_value)
{
    lib_u32 exponent = (bits >> 23u) & 0xffu;
    lib_u32 fraction = bits & 0x007fffffu;

    if (exponent == 0xffu || (exponent == 0u && fraction != 0u)) return 0;
    out_value->negative = (type_bool)((bits >> 31u) != 0u);
    if (exponent == 0u) {
        out_value->kind = CORE_MACHINE_FPU_VALUE_ZERO;
        out_value->exponent = 0;
        out_value->significand = 0u;
    } else {
        out_value->kind = CORE_MACHINE_FPU_VALUE_FINITE;
        out_value->exponent = (lib_i16)((lib_i16)exponent - 127);
        out_value->significand = 0x00800000u | fraction;
    }
    return 1;
}

static C_INT core_machine_fpu_encode_m32(const core_machine_fpu_value *value,
    lib_u32 *out_bits)
{
    lib_u32 bits = value->negative ? 0x80000000u : 0u;
    C_INT exponent;

    if (value->kind == CORE_MACHINE_FPU_VALUE_ZERO) {
        *out_bits = bits;
        return 1;
    }
    if (value->kind == CORE_MACHINE_FPU_VALUE_INFINITY) {
        *out_bits = bits | 0x7f800000u;
        return 1;
    }
    exponent = (C_INT)value->exponent + 127;
    if (exponent <= 0 || exponent >= 0xff ||
        value->significand < 0x00800000u || value->significand >= 0x01000000u) {
        return 0;
    }
    *out_bits = bits | ((lib_u32)exponent << 23u) |
        (value->significand & 0x007fffffu);
    return 1;
}

static C_INT core_machine_fpu_add(const core_machine_fpu_value *left,
    const core_machine_fpu_value *right, core_machine_fpu_value *out_value)
{
    lib_i16 exponent;
    lib_i64 left_value;
    lib_i64 right_value;
    lib_i64 result;
    lib_u64 magnitude;
    lib_u32 shift;

    if (left->kind == CORE_MACHINE_FPU_VALUE_ZERO) {
        *out_value = *right;
        return 1;
    }
    if (right->kind == CORE_MACHINE_FPU_VALUE_ZERO) {
        *out_value = *left;
        return 1;
    }
    if (left->kind != CORE_MACHINE_FPU_VALUE_FINITE ||
        right->kind != CORE_MACHINE_FPU_VALUE_FINITE) return 0;
    exponent = left->exponent >= right->exponent ? left->exponent : right->exponent;
    shift = (lib_u32)(exponent - left->exponent);
    left_value = shift >= 32u ? 0 : (lib_i64)left->significand << 8u >> shift;
    shift = (lib_u32)(exponent - right->exponent);
    right_value = shift >= 32u ? 0 : (lib_i64)right->significand << 8u >> shift;
    if (left->negative) left_value = -left_value;
    if (right->negative) right_value = -right_value;
    result = left_value + right_value;
    if (result == 0) {
        out_value->kind = CORE_MACHINE_FPU_VALUE_ZERO;
        out_value->negative = LIB_FALSE;
        out_value->exponent = 0;
        out_value->significand = 0u;
        return 1;
    }
    out_value->negative = result < 0;
    magnitude = (lib_u64)(out_value->negative ? -result : result);
    while (magnitude >= 0x0000000100000000ull) {
        magnitude >>= 1u;
        ++exponent;
    }
    while (magnitude < 0x0000000080000000ull) {
        magnitude <<= 1u;
        --exponent;
    }
    out_value->kind = CORE_MACHINE_FPU_VALUE_FINITE;
    out_value->exponent = exponent;
    out_value->significand = (lib_u32)(magnitude >> 8u);
    return 1;
}

static C_INT core_machine_fpu_multiply(const core_machine_fpu_value *left,
    const core_machine_fpu_value *right, core_machine_fpu_value *out_value)
{
    lib_u64 product;
    lib_u32 shift;

    if (left->kind == CORE_MACHINE_FPU_VALUE_ZERO ||
        right->kind == CORE_MACHINE_FPU_VALUE_ZERO) {
        out_value->kind = CORE_MACHINE_FPU_VALUE_ZERO;
        out_value->negative = left->negative != right->negative;
        out_value->exponent = 0;
        out_value->significand = 0u;
        return 1;
    }
    if (left->kind != CORE_MACHINE_FPU_VALUE_FINITE ||
        right->kind != CORE_MACHINE_FPU_VALUE_FINITE) return 0;
    product = (lib_u64)left->significand * right->significand;
    shift = product >= 0x0000800000000000ull ? 24u : 23u;
    out_value->kind = CORE_MACHINE_FPU_VALUE_FINITE;
    out_value->negative = left->negative != right->negative;
    out_value->exponent = (lib_i16)(left->exponent + right->exponent +
        (shift == 24u ? 1 : 0));
    out_value->significand = (lib_u32)(product >> shift);
    return 1;
}

static C_INT core_machine_fpu_divide(core_machine_fpu *fpu,
    const core_machine_fpu_value *left, const core_machine_fpu_value *right,
    core_machine_fpu_value *out_value)
{
    lib_u64 quotient;

    if (right->kind == CORE_MACHINE_FPU_VALUE_ZERO) {
        core_machine_fpu_raise(fpu, CORE_MACHINE_FPU_STATUS_ZE,
            CORE_MACHINE_FPU_STATUS_ZE);
        out_value->kind = CORE_MACHINE_FPU_VALUE_INFINITY;
        out_value->negative = left->negative != right->negative;
        out_value->exponent = 0;
        out_value->significand = 0u;
        return 1;
    }
    if (left->kind == CORE_MACHINE_FPU_VALUE_ZERO) {
        out_value->kind = CORE_MACHINE_FPU_VALUE_ZERO;
        out_value->negative = left->negative != right->negative;
        out_value->exponent = 0;
        out_value->significand = 0u;
        return 1;
    }
    if (left->kind != CORE_MACHINE_FPU_VALUE_FINITE ||
        right->kind != CORE_MACHINE_FPU_VALUE_FINITE) return 0;
    quotient = ((lib_u64)left->significand << 24u) / right->significand;
    out_value->kind = CORE_MACHINE_FPU_VALUE_FINITE;
    out_value->negative = left->negative != right->negative;
    out_value->exponent = (lib_i16)(left->exponent - right->exponent - 1);
    if (quotient >= 0x01000000u) {
        quotient >>= 1u;
        ++out_value->exponent;
    }
    out_value->significand = (lib_u32)quotient;
    return 1;
}

C_VOID core_machine_fpu_initialize(core_machine_fpu *fpu,
    core_machine_fpu_profile profile)
{
    if (fpu == LIB_NULL) return;
    fpu->profile = profile;
    core_machine_fpu_reset(fpu);
}

C_VOID core_machine_fpu_reset(core_machine_fpu *fpu)
{
    lib_u8 index;

    if (fpu == LIB_NULL) return;
    fpu->control_word = CORE_MACHINE_FPU_CONTROL_DEFAULT;
    fpu->status_word = 0u;
    fpu->top = 0u;
    fpu->pending_unmasked_exception = LIB_FALSE;
    fpu->busy = LIB_FALSE;
    fpu->last_escape_opcode = 0u;
    fpu->last_escape_modrm = 0u;
    fpu->operation_ticks_min = 0u;
    fpu->operation_ticks_max = 0u;
    fpu->completion_remaining_ticks = 0u;
    fpu->last_wait_ticks = 0u;
    for (index = 0u; index < 8u; ++index) {
        fpu->tags[index] = CORE_MACHINE_FPU_TAG_EMPTY;
        fpu->registers[index].kind = CORE_MACHINE_FPU_VALUE_ZERO;
        fpu->registers[index].negative = LIB_FALSE;
        fpu->registers[index].exponent = 0;
        fpu->registers[index].significand = 0u;
    }
    core_machine_fpu_sync_top(fpu);
}

core_machine_fpu_operation_metadata core_machine_fpu_operation_metadata_get(
    lib_u8 escape_opcode, lib_u8 modrm)
{
    core_machine_fpu_operation_metadata metadata = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_FPU_PROFILE_8087,
        CORE_MACHINE_FPU_OPERATION_UNSUPPORTED, 0};
    lib_u8 mod = (lib_u8)(modrm >> 6u);
    lib_u8 reg = (lib_u8)((modrm >> 3u) & 7u);

    if (escape_opcode == 0xdbu && modrm == 0xe3u) {
        metadata.operation = CORE_MACHINE_FPU_OPERATION_FNINIT;
    } else if (escape_opcode == 0xd9u && mod != 3u && reg == 0u) {
        metadata.operation = CORE_MACHINE_FPU_OPERATION_FLD_M32;
    } else if (escape_opcode == 0xd9u && mod != 3u && reg == 3u) {
        metadata.operation = CORE_MACHINE_FPU_OPERATION_FSTP_M32;
    } else if (escape_opcode == 0xd9u && mod != 3u && reg == 5u) {
        metadata.operation = CORE_MACHINE_FPU_OPERATION_FLDCW_M16;
    } else if (escape_opcode == 0xd8u && mod == 3u) {
        switch (reg) {
        case 0u: metadata.operation = CORE_MACHINE_FPU_OPERATION_FADD_ST0_STI; break;
        case 1u: metadata.operation = CORE_MACHINE_FPU_OPERATION_FMUL_ST0_STI; break;
        case 4u: metadata.operation = CORE_MACHINE_FPU_OPERATION_FSUB_ST0_STI; break;
        case 6u: metadata.operation = CORE_MACHINE_FPU_OPERATION_FDIV_ST0_STI; break;
        default: break;
        }
    }
    metadata.valid = metadata.operation != CORE_MACHINE_FPU_OPERATION_UNSUPPORTED;
    return metadata;
}

static type_bool core_machine_fpu_profile_allows_cpu(core_machine_cpu_profile cpu,
    core_machine_fpu_profile fpu)
{
    if (fpu == CORE_MACHINE_FPU_PROFILE_NONE) return LIB_TRUE;
    if (fpu == CORE_MACHINE_FPU_PROFILE_8087) {
        return core_machine_cpu_profile_has_8086_semantics(cpu) ||
            cpu == CORE_MACHINE_CPU_PROFILE_80186;
    }
    if (fpu == CORE_MACHINE_FPU_PROFILE_80287) {
        return cpu == CORE_MACHINE_CPU_PROFILE_80286 ||
            cpu == CORE_MACHINE_CPU_PROFILE_80386;
    }
    return fpu == CORE_MACHINE_FPU_PROFILE_80387 &&
        cpu == CORE_MACHINE_CPU_PROFILE_80386;
}

static lib_u32 core_machine_fpu_external_l2_ticks(
    core_machine_fpu_profile profile, core_machine_fpu_operation operation)
{
    /* Intel's 80287 table supplies typical/range values; the 8087 and 80387
     * selections are corroborated by the same operation classes in 86Box.
     * These are intentionally L2 model choices on the existing Core axis. */
    if (profile == CORE_MACHINE_FPU_PROFILE_80387) {
        switch (operation) {
        case CORE_MACHINE_FPU_OPERATION_FNINIT: return 33u;
        case CORE_MACHINE_FPU_OPERATION_FLD_M32: return 14u;
        case CORE_MACHINE_FPU_OPERATION_FSTP_M32: return 34u;
        case CORE_MACHINE_FPU_OPERATION_FLDCW_M16: return 19u;
        case CORE_MACHINE_FPU_OPERATION_FADD_ST0_STI: return 19u;
        case CORE_MACHINE_FPU_OPERATION_FMUL_ST0_STI: return 34u;
        case CORE_MACHINE_FPU_OPERATION_FSUB_ST0_STI: return 22u;
        case CORE_MACHINE_FPU_OPERATION_FDIV_ST0_STI: return 79u;
        default: return 28u;
        }
    }
    switch (operation) {
    case CORE_MACHINE_FPU_OPERATION_FNINIT: return 5u;
    case CORE_MACHINE_FPU_OPERATION_FLD_M32: return 47u;
    case CORE_MACHINE_FPU_OPERATION_FSTP_M32: return 87u;
    case CORE_MACHINE_FPU_OPERATION_FLDCW_M16: return 11u;
    case CORE_MACHINE_FPU_OPERATION_FADD_ST0_STI:
    case CORE_MACHINE_FPU_OPERATION_FSUB_ST0_STI: return 85u;
    case CORE_MACHINE_FPU_OPERATION_FMUL_ST0_STI: return 117u;
    case CORE_MACHINE_FPU_OPERATION_FDIV_ST0_STI: return 198u;
    default: return 85u;
    }
}

C_VOID core_machine_fpu_begin_command(core_machine_fpu *fpu,
    C_UCHAR escape_opcode, C_UCHAR modrm)
{
    core_machine_fpu_operation_metadata metadata =
        core_machine_fpu_operation_metadata_get(escape_opcode, modrm);

    if (fpu == LIB_NULL || fpu->profile == CORE_MACHINE_FPU_PROFILE_NONE) return;
    fpu->busy = LIB_TRUE;
    fpu->last_escape_opcode = escape_opcode;
    fpu->last_escape_modrm = modrm;
    fpu->operation_ticks_min = 0u;
    fpu->operation_ticks_max = 0u;
    fpu->completion_remaining_ticks = core_machine_fpu_external_l2_ticks(
        fpu->profile, metadata.operation);
    if (fpu->profile != CORE_MACHINE_FPU_PROFILE_80387) return;
    switch (metadata.operation) {
    case CORE_MACHINE_FPU_OPERATION_FNINIT:
        fpu->operation_ticks_min = fpu->operation_ticks_max = 33u;
        break;
    case CORE_MACHINE_FPU_OPERATION_FLD_M32:
        fpu->operation_ticks_min = 9u; fpu->operation_ticks_max = 18u;
        break;
    case CORE_MACHINE_FPU_OPERATION_FSTP_M32:
        fpu->operation_ticks_min = 25u; fpu->operation_ticks_max = 43u;
        break;
    case CORE_MACHINE_FPU_OPERATION_FLDCW_M16:
        fpu->operation_ticks_min = fpu->operation_ticks_max = 19u;
        break;
    case CORE_MACHINE_FPU_OPERATION_FADD_ST0_STI:
        fpu->operation_ticks_min = 12u; fpu->operation_ticks_max = 26u;
        break;
    case CORE_MACHINE_FPU_OPERATION_FMUL_ST0_STI:
        fpu->operation_ticks_min = 17u; fpu->operation_ticks_max = 50u;
        break;
    case CORE_MACHINE_FPU_OPERATION_FSUB_ST0_STI:
        fpu->operation_ticks_min = 15u; fpu->operation_ticks_max = 29u;
        break;
    case CORE_MACHINE_FPU_OPERATION_FDIV_ST0_STI:
        fpu->operation_ticks_min = 77u; fpu->operation_ticks_max = 80u;
        break;
    default:
        break;
    }
}

core_machine_fpu_escape_action core_machine_fpu_escape_dispatch(
    core_machine_fpu *fpu, core_machine_cpu_profile cpu,
    C_UCHAR escape_opcode, C_UCHAR modrm)
{
    if (escape_opcode < 0xd8u || escape_opcode > 0xdfu) {
        return CORE_MACHINE_FPU_ESCAPE_UNSUPPORTED;
    }
    if (fpu == LIB_NULL || fpu->profile == CORE_MACHINE_FPU_PROFILE_NONE) {
        return CORE_MACHINE_FPU_ESCAPE_CONSUME_NONE;
    }
    if (!core_machine_fpu_profile_allows_cpu(cpu, fpu->profile)) {
        return CORE_MACHINE_FPU_ESCAPE_UNSUPPORTED;
    }
    return fpu->profile == CORE_MACHINE_FPU_PROFILE_8087 &&
        core_machine_fpu_operation_metadata_get(escape_opcode, modrm).valid ?
        CORE_MACHINE_FPU_ESCAPE_EXECUTE_8087 : CORE_MACHINE_FPU_ESCAPE_HANDOFF;
}

C_VOID core_machine_fpu_advance(core_machine_fpu *fpu,
    lib_u64 elapsed_ticks)
{
    if (fpu == LIB_NULL || !fpu->busy || elapsed_ticks == 0u) return;
    if (elapsed_ticks >= fpu->completion_remaining_ticks) {
        fpu->busy = LIB_FALSE;
        fpu->completion_remaining_ticks = 0u;
    } else {
        fpu->completion_remaining_ticks -= elapsed_ticks;
    }
}

type_status core_machine_fpu_ticks_until_completion(const core_machine_fpu *fpu,
    lib_u64 *out_ticks)
{
    if (fpu == LIB_NULL || out_ticks == LIB_NULL || !fpu->busy ||
        fpu->completion_remaining_ticks == 0u) {
        return TYPE_STATUS_INVALID_STATE;
    }
    *out_ticks = fpu->completion_remaining_ticks;
    return TYPE_STATUS_OK;
}

C_VOID core_machine_fpu_get_state(const core_machine_fpu *fpu,
    core_machine_fpu_state *out_state)
{
    lib_u8 index;

    if (fpu == LIB_NULL || out_state == LIB_NULL) return;
    out_state->control_word = fpu->control_word;
    out_state->status_word = fpu->status_word;
    out_state->top = fpu->top;
    out_state->pending_unmasked_exception = fpu->pending_unmasked_exception;
    for (index = 0u; index < 8u; ++index) out_state->tags[index] = fpu->tags[index];
}

core_machine_fpu_execute_result core_machine_fpu_load_m32(core_machine_fpu *fpu,
    lib_u32 bits)
{
    core_machine_fpu_value value;

    if (fpu == LIB_NULL || !core_machine_fpu_decode_m32(bits, &value)) {
        return CORE_MACHINE_FPU_EXECUTE_UNSUPPORTED;
    }
    (C_VOID)core_machine_fpu_push(fpu, &value);
    return CORE_MACHINE_FPU_EXECUTE_COMPLETED;
}

core_machine_fpu_execute_result core_machine_fpu_store_m32(core_machine_fpu *fpu,
    lib_u32 *out_bits)
{
    core_machine_fpu_value value;
    lib_u8 index;

    if (fpu == LIB_NULL || out_bits == LIB_NULL ||
        !core_machine_fpu_st(fpu, 0u, &value, &index)) {
        if (fpu != LIB_NULL) core_machine_fpu_stack_fault(fpu);
        if (out_bits != LIB_NULL) *out_bits = 0u;
        return CORE_MACHINE_FPU_EXECUTE_COMPLETED;
    }
    if (!core_machine_fpu_encode_m32(&value, out_bits)) {
        return CORE_MACHINE_FPU_EXECUTE_UNSUPPORTED;
    }
    fpu->tags[index] = CORE_MACHINE_FPU_TAG_EMPTY;
    fpu->top = (lib_u8)((fpu->top + 1u) & 7u);
    core_machine_fpu_sync_top(fpu);
    return CORE_MACHINE_FPU_EXECUTE_COMPLETED;
}

C_VOID core_machine_fpu_load_control_word(core_machine_fpu *fpu,
    lib_u16 control_word)
{
    if (fpu == LIB_NULL) return;
    fpu->control_word = (lib_u16)((control_word & CORE_MACHINE_FPU_CONTROL_EXCEPTION_MASK) |
        (CORE_MACHINE_FPU_CONTROL_DEFAULT & ~CORE_MACHINE_FPU_CONTROL_EXCEPTION_MASK));
    if ((fpu->status_word & CORE_MACHINE_FPU_STATUS_ES) != 0u &&
        (fpu->control_word & (fpu->status_word & CORE_MACHINE_FPU_CONTROL_EXCEPTION_MASK)) !=
            (fpu->status_word & CORE_MACHINE_FPU_CONTROL_EXCEPTION_MASK)) {
        fpu->pending_unmasked_exception = LIB_TRUE;
    }
}

core_machine_fpu_execute_result core_machine_fpu_binary_st0_sti(core_machine_fpu *fpu,
    core_machine_fpu_operation operation, lib_u8 index)
{
    core_machine_fpu_value left;
    core_machine_fpu_value right;
    core_machine_fpu_value result;
    lib_u8 destination;
    C_INT completed = 0;

    if (fpu == LIB_NULL || !core_machine_fpu_st(fpu, 0u, &left, &destination) ||
        !core_machine_fpu_st(fpu, index, &right, LIB_NULL)) {
        if (fpu != LIB_NULL) core_machine_fpu_stack_fault(fpu);
        return CORE_MACHINE_FPU_EXECUTE_COMPLETED;
    }
    if (operation == CORE_MACHINE_FPU_OPERATION_FSUB_ST0_STI) {
        right.negative = !right.negative;
        completed = core_machine_fpu_add(&left, &right, &result);
    } else if (operation == CORE_MACHINE_FPU_OPERATION_FADD_ST0_STI) {
        completed = core_machine_fpu_add(&left, &right, &result);
    } else if (operation == CORE_MACHINE_FPU_OPERATION_FMUL_ST0_STI) {
        completed = core_machine_fpu_multiply(&left, &right, &result);
    } else if (operation == CORE_MACHINE_FPU_OPERATION_FDIV_ST0_STI) {
        completed = core_machine_fpu_divide(fpu, &left, &right, &result);
    }
    if (!completed) return CORE_MACHINE_FPU_EXECUTE_UNSUPPORTED;
    fpu->registers[destination] = result;
    return CORE_MACHINE_FPU_EXECUTE_COMPLETED;
}

type_bool core_machine_fpu_wait_pending(const core_machine_fpu *fpu)
{
    return fpu != LIB_NULL && fpu->pending_unmasked_exception;
}

lib_u64 core_machine_fpu_complete_wait(core_machine_fpu *fpu)
{
    lib_u64 ticks;

    if (fpu == LIB_NULL) return 0u;
    ticks = fpu->completion_remaining_ticks;
    fpu->busy = LIB_FALSE;
    fpu->completion_remaining_ticks = 0u;
    fpu->last_wait_ticks = ticks;
    return ticks;
}

lib_u64 core_machine_fpu_last_wait_ticks(const core_machine_fpu *fpu)
{
    return fpu == LIB_NULL ? 0u : fpu->last_wait_ticks;
}
