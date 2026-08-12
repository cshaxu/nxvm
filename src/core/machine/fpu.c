#include "type.h"

#include "core/machine/fpu.h"

#define CORE_MACHINE_FPU_CONTROL_DEFAULT 0x037fu
#define CORE_MACHINE_FPU_CONTROL_EXCEPTION_MASK 0x003fu
#define CORE_MACHINE_FPU_STATUS_IE 0x0001u
#define CORE_MACHINE_FPU_STATUS_ZE 0x0004u
#define CORE_MACHINE_FPU_STATUS_SF 0x0040u
#define CORE_MACHINE_FPU_STATUS_ES 0x0080u
#define CORE_MACHINE_FPU_STATUS_TOP 0x3800u

static type_unsigned_8 core_machine_fpu_physical_index(const core_machine_fpu *fpu,
    type_unsigned_8 logical_index)
{
    return (type_unsigned_8)((fpu->top + logical_index) & 7u);
}

static C_VOID core_machine_fpu_sync_top(core_machine_fpu *fpu)
{
    fpu->status_word = (type_unsigned_16)((fpu->status_word & ~CORE_MACHINE_FPU_STATUS_TOP) |
        ((type_unsigned_16)fpu->top << 11u));
}

static C_VOID core_machine_fpu_raise(core_machine_fpu *fpu,
    type_unsigned_16 status_bits, type_unsigned_16 mask_bits)
{
    fpu->status_word = (type_unsigned_16)(fpu->status_word | status_bits);
    if ((fpu->control_word & mask_bits) != mask_bits) {
        fpu->status_word = (type_unsigned_16)(fpu->status_word | CORE_MACHINE_FPU_STATUS_ES);
        fpu->pending_unmasked_exception = TYPE_TRUE;
    }
}

static C_INT core_machine_fpu_push(core_machine_fpu *fpu,
    const core_machine_fpu_value *value)
{
    type_unsigned_8 index = (type_unsigned_8)((fpu->top + 7u) & 7u);

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

static C_INT core_machine_fpu_st(const core_machine_fpu *fpu, type_unsigned_8 logical,
    core_machine_fpu_value *out_value, type_unsigned_8 *out_physical)
{
    type_unsigned_8 index;

    if (logical >= 8u) return 0;
    index = core_machine_fpu_physical_index(fpu, logical);
    if (fpu->tags[index] == CORE_MACHINE_FPU_TAG_EMPTY) return 0;
    if (out_value != STD_NULL) *out_value = fpu->registers[index];
    if (out_physical != STD_NULL) *out_physical = index;
    return 1;
}

static C_VOID core_machine_fpu_stack_fault(core_machine_fpu *fpu)
{
    core_machine_fpu_raise(fpu, CORE_MACHINE_FPU_STATUS_IE |
        CORE_MACHINE_FPU_STATUS_SF, CORE_MACHINE_FPU_STATUS_IE);
}

static C_INT core_machine_fpu_decode_m32(type_unsigned_32 bits,
    core_machine_fpu_value *out_value)
{
    type_unsigned_32 exponent = (bits >> 23u) & 0xffu;
    type_unsigned_32 fraction = bits & 0x007fffffu;

    if (exponent == 0xffu || (exponent == 0u && fraction != 0u)) return 0;
    out_value->negative = (type_bool)((bits >> 31u) != 0u);
    if (exponent == 0u) {
        out_value->kind = CORE_MACHINE_FPU_VALUE_ZERO;
        out_value->exponent = 0;
        out_value->significand = 0u;
    } else {
        out_value->kind = CORE_MACHINE_FPU_VALUE_FINITE;
        out_value->exponent = (type_signed_16)((type_signed_16)exponent - 127);
        out_value->significand = 0x00800000u | fraction;
    }
    return 1;
}

static C_INT core_machine_fpu_encode_m32(const core_machine_fpu_value *value,
    type_unsigned_32 *out_bits)
{
    type_unsigned_32 bits = value->negative ? 0x80000000u : 0u;
    int exponent;

    if (value->kind == CORE_MACHINE_FPU_VALUE_ZERO) {
        *out_bits = bits;
        return 1;
    }
    if (value->kind == CORE_MACHINE_FPU_VALUE_INFINITY) {
        *out_bits = bits | 0x7f800000u;
        return 1;
    }
    exponent = (int)value->exponent + 127;
    if (exponent <= 0 || exponent >= 0xff ||
        value->significand < 0x00800000u || value->significand >= 0x01000000u) {
        return 0;
    }
    *out_bits = bits | ((type_unsigned_32)exponent << 23u) |
        (value->significand & 0x007fffffu);
    return 1;
}

static C_INT core_machine_fpu_add(const core_machine_fpu_value *left,
    const core_machine_fpu_value *right, core_machine_fpu_value *out_value)
{
    type_signed_16 exponent;
    type_signed_64 left_value;
    type_signed_64 right_value;
    type_signed_64 result;
    type_unsigned_64 magnitude;
    type_unsigned_32 shift;

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
    shift = (type_unsigned_32)(exponent - left->exponent);
    left_value = shift >= 32u ? 0 : (type_signed_64)left->significand << 8u >> shift;
    shift = (type_unsigned_32)(exponent - right->exponent);
    right_value = shift >= 32u ? 0 : (type_signed_64)right->significand << 8u >> shift;
    if (left->negative) left_value = -left_value;
    if (right->negative) right_value = -right_value;
    result = left_value + right_value;
    if (result == 0) {
        out_value->kind = CORE_MACHINE_FPU_VALUE_ZERO;
        out_value->negative = TYPE_FALSE;
        out_value->exponent = 0;
        out_value->significand = 0u;
        return 1;
    }
    out_value->negative = result < 0;
    magnitude = (type_unsigned_64)(out_value->negative ? -result : result);
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
    out_value->significand = (type_unsigned_32)(magnitude >> 8u);
    return 1;
}

static C_INT core_machine_fpu_multiply(const core_machine_fpu_value *left,
    const core_machine_fpu_value *right, core_machine_fpu_value *out_value)
{
    type_unsigned_64 product;
    type_unsigned_32 shift;

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
    product = (type_unsigned_64)left->significand * right->significand;
    shift = product >= 0x0000800000000000ull ? 24u : 23u;
    out_value->kind = CORE_MACHINE_FPU_VALUE_FINITE;
    out_value->negative = left->negative != right->negative;
    out_value->exponent = (type_signed_16)(left->exponent + right->exponent +
        (shift == 24u ? 1 : 0));
    out_value->significand = (type_unsigned_32)(product >> shift);
    return 1;
}

static C_INT core_machine_fpu_divide(core_machine_fpu *fpu,
    const core_machine_fpu_value *left, const core_machine_fpu_value *right,
    core_machine_fpu_value *out_value)
{
    type_unsigned_64 quotient;

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
    quotient = ((type_unsigned_64)left->significand << 24u) / right->significand;
    out_value->kind = CORE_MACHINE_FPU_VALUE_FINITE;
    out_value->negative = left->negative != right->negative;
    out_value->exponent = (type_signed_16)(left->exponent - right->exponent - 1);
    if (quotient >= 0x01000000u) {
        quotient >>= 1u;
        ++out_value->exponent;
    }
    out_value->significand = (type_unsigned_32)quotient;
    return 1;
}

const C_CHAR *core_machine_fpu_profile_name(core_machine_fpu_profile profile)
{
    switch (profile) {
    case CORE_MACHINE_FPU_PROFILE_NONE: return "none";
    case CORE_MACHINE_FPU_PROFILE_8087: return "8087";
    case CORE_MACHINE_FPU_PROFILE_80287: return "80287";
    case CORE_MACHINE_FPU_PROFILE_80387: return "80387";
    }
    return "invalid";
}

C_VOID core_machine_fpu_initialize(core_machine_fpu *fpu,
    core_machine_fpu_profile profile)
{
    if (fpu == STD_NULL) return;
    fpu->profile = profile;
    core_machine_fpu_reset(fpu);
}

C_VOID core_machine_fpu_reset(core_machine_fpu *fpu)
{
    type_unsigned_8 index;

    if (fpu == STD_NULL) return;
    fpu->control_word = CORE_MACHINE_FPU_CONTROL_DEFAULT;
    fpu->status_word = 0u;
    fpu->top = 0u;
    fpu->pending_unmasked_exception = TYPE_FALSE;
    for (index = 0u; index < 8u; ++index) {
        fpu->tags[index] = CORE_MACHINE_FPU_TAG_EMPTY;
        fpu->registers[index].kind = CORE_MACHINE_FPU_VALUE_ZERO;
        fpu->registers[index].negative = TYPE_FALSE;
        fpu->registers[index].exponent = 0;
        fpu->registers[index].significand = 0u;
    }
    core_machine_fpu_sync_top(fpu);
}

core_machine_fpu_operation_metadata core_machine_fpu_operation_metadata_get(
    type_unsigned_8 escape_opcode, type_unsigned_8 modrm)
{
    core_machine_fpu_operation_metadata metadata = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_FPU_PROFILE_8087,
        CORE_MACHINE_FPU_OPERATION_UNSUPPORTED, 0};
    type_unsigned_8 mod = (type_unsigned_8)(modrm >> 6u);
    type_unsigned_8 reg = (type_unsigned_8)((modrm >> 3u) & 7u);

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

core_machine_fpu_escape_action core_machine_fpu_escape_dispatch(
    const core_machine_fpu *fpu, C_UCHAR escape_opcode, C_UCHAR modrm)
{
    if (escape_opcode < 0xd8u || escape_opcode > 0xdfu) {
        return CORE_MACHINE_FPU_ESCAPE_UNSUPPORTED;
    }
    if (fpu == STD_NULL || fpu->profile == CORE_MACHINE_FPU_PROFILE_NONE) {
        return CORE_MACHINE_FPU_ESCAPE_CONSUME_NONE;
    }
    return fpu->profile == CORE_MACHINE_FPU_PROFILE_8087 &&
        core_machine_fpu_operation_metadata_get(escape_opcode, modrm).valid ?
        CORE_MACHINE_FPU_ESCAPE_CONSUME_NONE : CORE_MACHINE_FPU_ESCAPE_UNSUPPORTED;
}

C_VOID core_machine_fpu_get_state(const core_machine_fpu *fpu,
    core_machine_fpu_state *out_state)
{
    type_unsigned_8 index;

    if (fpu == STD_NULL || out_state == STD_NULL) return;
    out_state->control_word = fpu->control_word;
    out_state->status_word = fpu->status_word;
    out_state->top = fpu->top;
    out_state->pending_unmasked_exception = fpu->pending_unmasked_exception;
    for (index = 0u; index < 8u; ++index) out_state->tags[index] = fpu->tags[index];
}

core_machine_fpu_execute_result core_machine_fpu_load_m32(core_machine_fpu *fpu,
    type_unsigned_32 bits)
{
    core_machine_fpu_value value;

    if (fpu == STD_NULL || !core_machine_fpu_decode_m32(bits, &value)) {
        return CORE_MACHINE_FPU_EXECUTE_UNSUPPORTED;
    }
    (C_VOID)core_machine_fpu_push(fpu, &value);
    return CORE_MACHINE_FPU_EXECUTE_COMPLETED;
}

core_machine_fpu_execute_result core_machine_fpu_store_m32(core_machine_fpu *fpu,
    type_unsigned_32 *out_bits)
{
    core_machine_fpu_value value;
    type_unsigned_8 index;

    if (fpu == STD_NULL || out_bits == STD_NULL ||
        !core_machine_fpu_st(fpu, 0u, &value, &index)) {
        if (fpu != STD_NULL) core_machine_fpu_stack_fault(fpu);
        if (out_bits != STD_NULL) *out_bits = 0u;
        return CORE_MACHINE_FPU_EXECUTE_COMPLETED;
    }
    if (!core_machine_fpu_encode_m32(&value, out_bits)) {
        return CORE_MACHINE_FPU_EXECUTE_UNSUPPORTED;
    }
    fpu->tags[index] = CORE_MACHINE_FPU_TAG_EMPTY;
    fpu->top = (type_unsigned_8)((fpu->top + 1u) & 7u);
    core_machine_fpu_sync_top(fpu);
    return CORE_MACHINE_FPU_EXECUTE_COMPLETED;
}

C_VOID core_machine_fpu_load_control_word(core_machine_fpu *fpu,
    type_unsigned_16 control_word)
{
    if (fpu == STD_NULL) return;
    fpu->control_word = (type_unsigned_16)((control_word & CORE_MACHINE_FPU_CONTROL_EXCEPTION_MASK) |
        (CORE_MACHINE_FPU_CONTROL_DEFAULT & ~CORE_MACHINE_FPU_CONTROL_EXCEPTION_MASK));
    if ((fpu->status_word & CORE_MACHINE_FPU_STATUS_ES) != 0u &&
        (fpu->control_word & (fpu->status_word & CORE_MACHINE_FPU_CONTROL_EXCEPTION_MASK)) !=
            (fpu->status_word & CORE_MACHINE_FPU_CONTROL_EXCEPTION_MASK)) {
        fpu->pending_unmasked_exception = TYPE_TRUE;
    }
}

core_machine_fpu_execute_result core_machine_fpu_binary_st0_sti(core_machine_fpu *fpu,
    core_machine_fpu_operation operation, type_unsigned_8 index)
{
    core_machine_fpu_value left;
    core_machine_fpu_value right;
    core_machine_fpu_value result;
    type_unsigned_8 destination;
    C_INT completed = 0;

    if (fpu == STD_NULL || !core_machine_fpu_st(fpu, 0u, &left, &destination) ||
        !core_machine_fpu_st(fpu, index, &right, STD_NULL)) {
        if (fpu != STD_NULL) core_machine_fpu_stack_fault(fpu);
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
    return fpu != STD_NULL && fpu->pending_unmasked_exception;
}
