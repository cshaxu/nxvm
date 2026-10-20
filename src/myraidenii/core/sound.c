#include "myraidenii/core/sound.h"

enum {
    MYRAIDENII_CORE_SOUND_FIXED_ROM_BYTES = 0x2000u,
    MYRAIDENII_CORE_SOUND_RAM_START = 0x2000u,
    MYRAIDENII_CORE_SOUND_RAM_BYTES = 0x0800u,
    MYRAIDENII_CORE_SOUND_BANK_START = 0x8000u,
    MYRAIDENII_CORE_SOUND_BANK_BYTES = 0x8000u,
    MYRAIDENII_CORE_SOUND_COMMAND_ACKNOWLEDGE = 0x4000u,
    MYRAIDENII_CORE_SOUND_INTERRUPT_ACKNOWLEDGE = 0x4001u,
    MYRAIDENII_CORE_SOUND_RST10_ACKNOWLEDGE = 0x4002u,
    MYRAIDENII_CORE_SOUND_RST18_ACKNOWLEDGE = 0x4003u,
    MYRAIDENII_CORE_SOUND_COMMAND_0 = 0x4010u,
    MYRAIDENII_CORE_SOUND_COMMAND_1 = 0x4011u,
    MYRAIDENII_CORE_SOUND_COMMAND_PENDING = 0x4012u,
    MYRAIDENII_CORE_SOUND_COIN = 0x4013u,
    MYRAIDENII_CORE_SOUND_RESPONSE_0 = 0x4018u,
    MYRAIDENII_CORE_SOUND_RESPONSE_1 = 0x4019u,
    MYRAIDENII_CORE_SOUND_BANK = 0x401au,
    MYRAIDENII_CORE_SOUND_COIN_OUTPUT = 0x401bu,
    MYRAIDENII_CORE_SOUND_MAX_IMPLEMENTED_TICKS = 23u,
    MYRAIDENII_CORE_SOUND_FLAG_CARRY = 0x01u,
    MYRAIDENII_CORE_SOUND_FLAG_SUBTRACT = 0x02u,
    MYRAIDENII_CORE_SOUND_FLAG_PARITY_OVERFLOW = 0x04u,
    MYRAIDENII_CORE_SOUND_FLAG_HALF_CARRY = 0x10u,
    MYRAIDENII_CORE_SOUND_FLAG_ZERO = 0x40u,
    MYRAIDENII_CORE_SOUND_FLAG_SIGN = 0x80u
};

struct myraidenii_core_sound {
    myraidenii_core_sound_transport transport;
    lib_u8 *program;
    lib_size program_bytes;
    lib_u64 z80_ticks;
    lib_u64 z80_phase;
    lib_u64 available_ticks;
    lib_u16 pc;
    lib_u16 sp;
    lib_u8 a;
    lib_u8 b;
    lib_u8 c;
    lib_u8 d;
    lib_u8 e;
    lib_u8 h;
    lib_u8 l;
    lib_u8 f;
    lib_u8 alternate_a;
    lib_u8 alternate_b;
    lib_u8 alternate_c;
    lib_u8 alternate_d;
    lib_u8 alternate_e;
    lib_u8 alternate_h;
    lib_u8 alternate_l;
    lib_u8 alternate_f;
    lib_u16 ix;
    lib_u16 iy;
    lib_u8 bank;
    lib_u8 halted;
    lib_u8 ram[MYRAIDENII_CORE_SOUND_RAM_BYTES];
};

static lib_bool myraidenii_core_sound_transport_valid(
    const myraidenii_core_sound_transport *transport)
{
    return transport != LIB_NULL && transport->command_get != LIB_NULL &&
        transport->command_is_pending != LIB_NULL &&
        transport->command_acknowledge != LIB_NULL &&
        transport->interrupt_acknowledge != LIB_NULL &&
        transport->response_set != LIB_NULL && transport->coin_get != LIB_NULL;
}

static lib_u16 myraidenii_core_sound_hl(const myraidenii_core_sound *sound)
{
    return (lib_u16)((lib_u16)sound->h << 8u | sound->l);
}

static void myraidenii_core_sound_hl_set(myraidenii_core_sound *sound,
    lib_u16 value)
{
    sound->h = (lib_u8)(value >> 8u);
    sound->l = (lib_u8)value;
}

static lib_bool myraidenii_core_sound_even_parity(lib_u8 value)
{
    value ^= (lib_u8)(value >> 4u);
    value ^= (lib_u8)(value >> 2u);
    value ^= (lib_u8)(value >> 1u);
    return (value & 1u) == 0u;
}

static void myraidenii_core_sound_flags_logic(myraidenii_core_sound *sound,
    lib_u8 value, lib_bool half_carry)
{
    sound->f = (value & MYRAIDENII_CORE_SOUND_FLAG_SIGN) |
        (value == 0u ? MYRAIDENII_CORE_SOUND_FLAG_ZERO : 0u) |
        (myraidenii_core_sound_even_parity(value) ?
            MYRAIDENII_CORE_SOUND_FLAG_PARITY_OVERFLOW : 0u) |
        (half_carry ? MYRAIDENII_CORE_SOUND_FLAG_HALF_CARRY : 0u);
}

static lib_u8 myraidenii_core_sound_add8(myraidenii_core_sound *sound,
    lib_u8 left, lib_u8 right, lib_u8 carry)
{
    const lib_u16 sum = (lib_u16)left + right + carry;
    const lib_u8 result = (lib_u8)sum;

    sound->f = (result & MYRAIDENII_CORE_SOUND_FLAG_SIGN) |
        (result == 0u ? MYRAIDENII_CORE_SOUND_FLAG_ZERO : 0u) |
        (((left ^ result) & (right ^ result) & 0x80u) != 0u ?
            MYRAIDENII_CORE_SOUND_FLAG_PARITY_OVERFLOW : 0u) |
        (((left & 0x0fu) + (right & 0x0fu) + carry) > 0x0fu ?
            MYRAIDENII_CORE_SOUND_FLAG_HALF_CARRY : 0u) |
        (sum > 0xffu ? MYRAIDENII_CORE_SOUND_FLAG_CARRY : 0u);
    return result;
}

static lib_u8 myraidenii_core_sound_sub8(myraidenii_core_sound *sound,
    lib_u8 left, lib_u8 right, lib_u8 carry)
{
    const lib_i16 difference = (lib_i16)left - right - carry;
    const lib_u8 result = (lib_u8)difference;

    sound->f = (result & MYRAIDENII_CORE_SOUND_FLAG_SIGN) |
        (result == 0u ? MYRAIDENII_CORE_SOUND_FLAG_ZERO : 0u) |
        (((left ^ right) & (left ^ result) & 0x80u) != 0u ?
            MYRAIDENII_CORE_SOUND_FLAG_PARITY_OVERFLOW : 0u) |
        ((lib_i16)(left & 0x0fu) - (lib_i16)(right & 0x0fu) - carry < 0 ?
            MYRAIDENII_CORE_SOUND_FLAG_HALF_CARRY : 0u) |
        MYRAIDENII_CORE_SOUND_FLAG_SUBTRACT |
        (difference < 0 ? MYRAIDENII_CORE_SOUND_FLAG_CARRY : 0u);
    return result;
}

static lib_status myraidenii_core_sound_read(myraidenii_core_sound *sound,
    lib_u16 address, lib_u8 *out_value)
{
    lib_size index;

    if (address < MYRAIDENII_CORE_SOUND_FIXED_ROM_BYTES) {
        *out_value = address < sound->program_bytes ? sound->program[address] : 0xffu;
        return LIB_STATUS_OK;
    }
    if (address >= MYRAIDENII_CORE_SOUND_RAM_START &&
        address - MYRAIDENII_CORE_SOUND_RAM_START < MYRAIDENII_CORE_SOUND_RAM_BYTES) {
        *out_value = sound->ram[address - MYRAIDENII_CORE_SOUND_RAM_START];
        return LIB_STATUS_OK;
    }
    if (address >= MYRAIDENII_CORE_SOUND_BANK_START) {
        index = MYRAIDENII_CORE_SOUND_FIXED_ROM_BYTES +
            (lib_size)sound->bank * MYRAIDENII_CORE_SOUND_BANK_BYTES +
            (address - MYRAIDENII_CORE_SOUND_BANK_START);
        *out_value = index < sound->program_bytes ? sound->program[index] : 0xffu;
        return LIB_STATUS_OK;
    }
    switch (address) {
    case MYRAIDENII_CORE_SOUND_COMMAND_0:
        return sound->transport.command_get(sound->transport.context, 0u, out_value);
    case MYRAIDENII_CORE_SOUND_COMMAND_1:
        return sound->transport.command_get(sound->transport.context, 1u, out_value);
    case MYRAIDENII_CORE_SOUND_COMMAND_PENDING: {
        lib_bool pending;
        lib_status status = sound->transport.command_is_pending(
            sound->transport.context, &pending);

        if (status != LIB_STATUS_OK) return status;
        *out_value = pending ? 1u : 0u;
        return LIB_STATUS_OK;
    }
    case MYRAIDENII_CORE_SOUND_COIN:
        return sound->transport.coin_get(sound->transport.context, out_value);
    default:
        return LIB_STATUS_UNSUPPORTED;
    }
}

static lib_status myraidenii_core_sound_write(myraidenii_core_sound *sound,
    lib_u16 address, lib_u8 value)
{
    if (address >= MYRAIDENII_CORE_SOUND_RAM_START &&
        address - MYRAIDENII_CORE_SOUND_RAM_START < MYRAIDENII_CORE_SOUND_RAM_BYTES) {
        sound->ram[address - MYRAIDENII_CORE_SOUND_RAM_START] = value;
        return LIB_STATUS_OK;
    }
    switch (address) {
    case MYRAIDENII_CORE_SOUND_COMMAND_ACKNOWLEDGE:
        return sound->transport.command_acknowledge(sound->transport.context);
    case MYRAIDENII_CORE_SOUND_INTERRUPT_ACKNOWLEDGE:
    case MYRAIDENII_CORE_SOUND_RST10_ACKNOWLEDGE:
    case MYRAIDENII_CORE_SOUND_RST18_ACKNOWLEDGE:
        return sound->transport.interrupt_acknowledge(sound->transport.context);
    case MYRAIDENII_CORE_SOUND_RESPONSE_0:
        return sound->transport.response_set(sound->transport.context, 0u, value);
    case MYRAIDENII_CORE_SOUND_RESPONSE_1:
        return sound->transport.response_set(sound->transport.context, 1u, value);
    case MYRAIDENII_CORE_SOUND_BANK:
        /* The selected L2 map gives this byte one bank-select owner. */
        sound->bank = value;
        return LIB_STATUS_OK;
    case MYRAIDENII_CORE_SOUND_COIN_OUTPUT:
        return LIB_STATUS_OK;
    default:
        return LIB_STATUS_UNSUPPORTED;
    }
}

static lib_status myraidenii_core_sound_fetch(myraidenii_core_sound *sound,
    lib_u8 *out_value)
{
    lib_status status = myraidenii_core_sound_read(sound, sound->pc, out_value);

    if (status == LIB_STATUS_OK) ++sound->pc;
    return status;
}

static lib_status myraidenii_core_sound_fetch_word(myraidenii_core_sound *sound,
    lib_u16 *out_value)
{
    lib_u8 low;
    lib_u8 high;
    lib_status status = myraidenii_core_sound_fetch(sound, &low);

    if (status != LIB_STATUS_OK) return status;
    status = myraidenii_core_sound_fetch(sound, &high);
    if (status != LIB_STATUS_OK) return status;
    *out_value = (lib_u16)((lib_u16)high << 8u | low);
    return LIB_STATUS_OK;
}

static lib_status myraidenii_core_sound_register_get(myraidenii_core_sound *sound,
    lib_u8 code, lib_u8 *out_value)
{
    switch (code) {
    case 0u: *out_value = sound->b; return LIB_STATUS_OK;
    case 1u: *out_value = sound->c; return LIB_STATUS_OK;
    case 2u: *out_value = sound->d; return LIB_STATUS_OK;
    case 3u: *out_value = sound->e; return LIB_STATUS_OK;
    case 4u: *out_value = sound->h; return LIB_STATUS_OK;
    case 5u: *out_value = sound->l; return LIB_STATUS_OK;
    case 6u: return myraidenii_core_sound_read(sound,
        myraidenii_core_sound_hl(sound), out_value);
    default: *out_value = sound->a; return LIB_STATUS_OK;
    }
}

static lib_status myraidenii_core_sound_register_set(myraidenii_core_sound *sound,
    lib_u8 code, lib_u8 value)
{
    switch (code) {
    case 0u: sound->b = value; return LIB_STATUS_OK;
    case 1u: sound->c = value; return LIB_STATUS_OK;
    case 2u: sound->d = value; return LIB_STATUS_OK;
    case 3u: sound->e = value; return LIB_STATUS_OK;
    case 4u: sound->h = value; return LIB_STATUS_OK;
    case 5u: sound->l = value; return LIB_STATUS_OK;
    case 6u: return myraidenii_core_sound_write(sound,
        myraidenii_core_sound_hl(sound), value);
    default: sound->a = value; return LIB_STATUS_OK;
    }
}

static lib_u16 myraidenii_core_sound_pair_get(const myraidenii_core_sound *sound,
    lib_u8 code)
{
    switch (code) {
    case 0u: return (lib_u16)((lib_u16)sound->b << 8u | sound->c);
    case 1u: return (lib_u16)((lib_u16)sound->d << 8u | sound->e);
    case 2u: return myraidenii_core_sound_hl(sound);
    default: return sound->sp;
    }
}

static void myraidenii_core_sound_pair_set(myraidenii_core_sound *sound,
    lib_u8 code, lib_u16 value)
{
    switch (code) {
    case 0u: sound->b = (lib_u8)(value >> 8u); sound->c = (lib_u8)value; break;
    case 1u: sound->d = (lib_u8)(value >> 8u); sound->e = (lib_u8)value; break;
    case 2u: myraidenii_core_sound_hl_set(sound, value); break;
    default: sound->sp = value; break;
    }
}

static lib_u16 myraidenii_core_sound_stack_pair_get(const myraidenii_core_sound *sound,
    lib_u8 code)
{
    if (code != 3u) return myraidenii_core_sound_pair_get(sound, code);
    return (lib_u16)((lib_u16)sound->a << 8u | sound->f);
}

static void myraidenii_core_sound_stack_pair_set(myraidenii_core_sound *sound,
    lib_u8 code, lib_u16 value)
{
    if (code != 3u) {
        myraidenii_core_sound_pair_set(sound, code, value);
        return;
    }
    sound->a = (lib_u8)(value >> 8u);
    sound->f = (lib_u8)value;
}

static lib_bool myraidenii_core_sound_condition(const myraidenii_core_sound *sound,
    lib_u8 code)
{
    const lib_bool zero = (sound->f & MYRAIDENII_CORE_SOUND_FLAG_ZERO) != 0u;
    const lib_bool carry = (sound->f & MYRAIDENII_CORE_SOUND_FLAG_CARRY) != 0u;
    const lib_bool parity =
        (sound->f & MYRAIDENII_CORE_SOUND_FLAG_PARITY_OVERFLOW) != 0u;
    const lib_bool sign = (sound->f & MYRAIDENII_CORE_SOUND_FLAG_SIGN) != 0u;

    switch (code & 7u) {
    case 0u: return !zero;
    case 1u: return zero;
    case 2u: return !carry;
    case 3u: return carry;
    case 4u: return !parity;
    case 5u: return parity;
    case 6u: return !sign;
    default: return sign;
    }
}

static lib_status myraidenii_core_sound_push(myraidenii_core_sound *sound,
    lib_u16 value)
{
    lib_status status;

    sound->sp = (lib_u16)(sound->sp - 1u);
    status = myraidenii_core_sound_write(sound, sound->sp, (lib_u8)(value >> 8u));
    if (status != LIB_STATUS_OK) return status;
    sound->sp = (lib_u16)(sound->sp - 1u);
    return myraidenii_core_sound_write(sound, sound->sp, (lib_u8)value);
}

static lib_status myraidenii_core_sound_pop(myraidenii_core_sound *sound,
    lib_u16 *out_value)
{
    lib_u8 low;
    lib_u8 high;
    lib_status status = myraidenii_core_sound_read(sound, sound->sp, &low);

    if (status != LIB_STATUS_OK) return status;
    sound->sp = (lib_u16)(sound->sp + 1u);
    status = myraidenii_core_sound_read(sound, sound->sp, &high);
    if (status != LIB_STATUS_OK) return status;
    sound->sp = (lib_u16)(sound->sp + 1u);
    *out_value = (lib_u16)((lib_u16)high << 8u | low);
    return LIB_STATUS_OK;
}

static void myraidenii_core_sound_exchange_registers(myraidenii_core_sound *sound)
{
    lib_u8 value;

    value = sound->b; sound->b = sound->alternate_b; sound->alternate_b = value;
    value = sound->c; sound->c = sound->alternate_c; sound->alternate_c = value;
    value = sound->d; sound->d = sound->alternate_d; sound->alternate_d = value;
    value = sound->e; sound->e = sound->alternate_e; sound->alternate_e = value;
    value = sound->h; sound->h = sound->alternate_h; sound->alternate_h = value;
    value = sound->l; sound->l = sound->alternate_l; sound->alternate_l = value;
}

static lib_status myraidenii_core_sound_execute_indexed(myraidenii_core_sound *sound,
    lib_u16 *index, lib_u8 *out_ticks)
{
    lib_u8 opcode;
    lib_u16 word;
    lib_status status = myraidenii_core_sound_fetch(sound, &opcode);

    if (status != LIB_STATUS_OK) return status;
    switch (opcode) {
    case 0x09u:
    case 0x19u:
    case 0x29u:
    case 0x39u:
        word = myraidenii_core_sound_pair_get(sound, (lib_u8)((opcode >> 4u) & 3u));
        *index = (lib_u16)(*index + word);
        *out_ticks = 15u;
        return LIB_STATUS_OK;
    case 0x21u:
        status = myraidenii_core_sound_fetch_word(sound, index);
        *out_ticks = 14u;
        return status;
    case 0x22u:
        status = myraidenii_core_sound_fetch_word(sound, &word);
        if (status != LIB_STATUS_OK) return status;
        status = myraidenii_core_sound_write(sound, word, (lib_u8)*index);
        if (status == LIB_STATUS_OK)
            status = myraidenii_core_sound_write(sound, (lib_u16)(word + 1u),
                (lib_u8)(*index >> 8u));
        *out_ticks = 20u;
        return status;
    case 0x2au:
        status = myraidenii_core_sound_fetch_word(sound, &word);
        if (status != LIB_STATUS_OK) return status;
        {
            lib_u8 low;
            lib_u8 high;

            status = myraidenii_core_sound_read(sound, word, &low);
            if (status == LIB_STATUS_OK)
                status = myraidenii_core_sound_read(sound, (lib_u16)(word + 1u), &high);
            if (status == LIB_STATUS_OK)
                *index = (lib_u16)((lib_u16)high << 8u | low);
        }
        *out_ticks = 20u;
        return status;
    case 0x23u: ++*index; *out_ticks = 10u; return LIB_STATUS_OK;
    case 0x2bu: --*index; *out_ticks = 10u; return LIB_STATUS_OK;
    case 0xe1u:
        status = myraidenii_core_sound_pop(sound, index);
        *out_ticks = 14u;
        return status;
    case 0xe3u:
        status = myraidenii_core_sound_pop(sound, &word);
        if (status != LIB_STATUS_OK) return status;
        status = myraidenii_core_sound_push(sound, *index);
        if (status == LIB_STATUS_OK) *index = word;
        *out_ticks = 23u;
        return status;
    case 0xe5u:
        status = myraidenii_core_sound_push(sound, *index);
        *out_ticks = 15u;
        return status;
    case 0xe9u: sound->pc = *index; *out_ticks = 8u; return LIB_STATUS_OK;
    case 0xf9u: sound->sp = *index; *out_ticks = 10u; return LIB_STATUS_OK;
    default: return LIB_STATUS_UNSUPPORTED;
    }
}

static lib_status myraidenii_core_sound_execute(myraidenii_core_sound *sound,
    lib_u8 *out_ticks)
{
    lib_u8 opcode;
    lib_u8 value;
    lib_u16 word;
    lib_status status;

    if (sound->halted) {
        *out_ticks = 4u;
        return LIB_STATUS_OK;
    }
    status = myraidenii_core_sound_fetch(sound, &opcode);
    if (status != LIB_STATUS_OK) return status;
    if (opcode == 0xddu) {
        return myraidenii_core_sound_execute_indexed(sound, &sound->ix, out_ticks);
    }
    if (opcode == 0xfdu) {
        return myraidenii_core_sound_execute_indexed(sound, &sound->iy, out_ticks);
    }
    if ((opcode & 0xc0u) == 0x40u) {
        if (opcode == 0x76u) {
            sound->halted = LIB_TRUE;
            *out_ticks = 4u;
            return LIB_STATUS_OK;
        }
        status = myraidenii_core_sound_register_get(sound, (lib_u8)(opcode & 7u), &value);
        if (status != LIB_STATUS_OK) return status;
        status = myraidenii_core_sound_register_set(sound,
            (lib_u8)((opcode >> 3u) & 7u), value);
        *out_ticks = ((opcode & 7u) == 6u || ((opcode >> 3u) & 7u) == 6u) ? 7u : 4u;
        return status;
    }
    if ((opcode & 0xc7u) == 0x06u) {
        status = myraidenii_core_sound_fetch(sound, &value);
        if (status != LIB_STATUS_OK) return status;
        status = myraidenii_core_sound_register_set(sound,
            (lib_u8)((opcode >> 3u) & 7u), value);
        *out_ticks = ((opcode >> 3u) & 7u) == 6u ? 10u : 7u;
        return status;
    }
    if ((opcode & 0xcfu) == 0x01u) {
        status = myraidenii_core_sound_fetch_word(sound, &word);
        if (status == LIB_STATUS_OK)
            myraidenii_core_sound_pair_set(sound, (lib_u8)((opcode >> 4u) & 3u), word);
        *out_ticks = 10u;
        return status;
    }
    if ((opcode & 0xcfu) == 0x03u) {
        const lib_u8 pair = (lib_u8)((opcode >> 4u) & 3u);

        myraidenii_core_sound_pair_set(sound, pair,
            (lib_u16)(myraidenii_core_sound_pair_get(sound, pair) + 1u));
        *out_ticks = 6u;
        return LIB_STATUS_OK;
    }
    if ((opcode & 0xcfu) == 0x0bu) {
        const lib_u8 pair = (lib_u8)((opcode >> 4u) & 3u);

        myraidenii_core_sound_pair_set(sound, pair,
            (lib_u16)(myraidenii_core_sound_pair_get(sound, pair) - 1u));
        *out_ticks = 6u;
        return LIB_STATUS_OK;
    }
    if ((opcode & 0xc7u) == 0x04u || (opcode & 0xc7u) == 0x05u) {
        const lib_u8 code = (lib_u8)((opcode >> 3u) & 7u);
        const lib_bool subtract = (opcode & 1u) != 0u;
        const lib_u8 carry = sound->f & MYRAIDENII_CORE_SOUND_FLAG_CARRY;

        status = myraidenii_core_sound_register_get(sound, code, &value);
        if (status != LIB_STATUS_OK) return status;
        value = subtract ? myraidenii_core_sound_sub8(sound, value, 1u, 0u) :
            myraidenii_core_sound_add8(sound, value, 1u, 0u);
        sound->f = (sound->f & (lib_u8)~MYRAIDENII_CORE_SOUND_FLAG_CARRY) | carry;
        status = myraidenii_core_sound_register_set(sound, code, value);
        *out_ticks = code == 6u ? 11u : 4u;
        return status;
    }
    if ((opcode & 0xc0u) == 0x80u) {
        const lib_u8 operation = (lib_u8)((opcode >> 3u) & 7u);
        const lib_u8 carry = (sound->f & MYRAIDENII_CORE_SOUND_FLAG_CARRY) != 0u;

        status = myraidenii_core_sound_register_get(sound, (lib_u8)(opcode & 7u), &value);
        if (status != LIB_STATUS_OK) return status;
        switch (operation) {
        case 0u: sound->a = myraidenii_core_sound_add8(sound, sound->a, value, 0u); break;
        case 1u: sound->a = myraidenii_core_sound_add8(sound, sound->a, value, carry); break;
        case 2u: sound->a = myraidenii_core_sound_sub8(sound, sound->a, value, 0u); break;
        case 3u: sound->a = myraidenii_core_sound_sub8(sound, sound->a, value, carry); break;
        case 4u: sound->a &= value; myraidenii_core_sound_flags_logic(sound, sound->a, LIB_TRUE); break;
        case 5u: sound->a ^= value; myraidenii_core_sound_flags_logic(sound, sound->a, LIB_FALSE); break;
        case 6u: sound->a |= value; myraidenii_core_sound_flags_logic(sound, sound->a, LIB_FALSE); break;
        default: (void)myraidenii_core_sound_sub8(sound, sound->a, value, 0u); break;
        }
        *out_ticks = (opcode & 7u) == 6u ? 7u : 4u;
        return LIB_STATUS_OK;
    }
    if ((opcode & 0xcfu) == 0xc1u) {
        status = myraidenii_core_sound_pop(sound, &word);
        if (status == LIB_STATUS_OK)
            myraidenii_core_sound_stack_pair_set(sound,
                (lib_u8)((opcode >> 4u) & 3u), word);
        *out_ticks = 10u;
        return status;
    }
    if ((opcode & 0xcfu) == 0xc5u) {
        status = myraidenii_core_sound_push(sound,
            myraidenii_core_sound_stack_pair_get(sound,
                (lib_u8)((opcode >> 4u) & 3u)));
        *out_ticks = 11u;
        return status;
    }
    if ((opcode & 0xc7u) == 0xc2u) {
        status = myraidenii_core_sound_fetch_word(sound, &word);
        if (status == LIB_STATUS_OK &&
            myraidenii_core_sound_condition(sound, (lib_u8)((opcode >> 3u) & 7u)))
            sound->pc = word;
        *out_ticks = 10u;
        return status;
    }
    if ((opcode & 0xc7u) == 0xc4u) {
        status = myraidenii_core_sound_fetch_word(sound, &word);
        if (status != LIB_STATUS_OK) return status;
        if (!myraidenii_core_sound_condition(sound, (lib_u8)((opcode >> 3u) & 7u))) {
            *out_ticks = 10u;
            return LIB_STATUS_OK;
        }
        status = myraidenii_core_sound_push(sound, sound->pc);
        if (status == LIB_STATUS_OK) sound->pc = word;
        *out_ticks = 17u;
        return status;
    }
    switch (opcode) {
    case 0x00u: *out_ticks = 4u; return LIB_STATUS_OK;
    case 0x08u:
        value = sound->a; sound->a = sound->alternate_a; sound->alternate_a = value;
        value = sound->f; sound->f = sound->alternate_f; sound->alternate_f = value;
        *out_ticks = 4u;
        return LIB_STATUS_OK;
    case 0x01u:
        status = myraidenii_core_sound_fetch_word(sound, &word);
        sound->b = (lib_u8)(word >> 8u); sound->c = (lib_u8)word;
        *out_ticks = 10u;
        return status;
    case 0x06u:
        status = myraidenii_core_sound_fetch(sound, &sound->b);
        *out_ticks = 7u;
        return status;
    case 0x0eu:
        status = myraidenii_core_sound_fetch(sound, &sound->c);
        *out_ticks = 7u;
        return status;
    case 0x11u:
        status = myraidenii_core_sound_fetch_word(sound, &word);
        sound->d = (lib_u8)(word >> 8u); sound->e = (lib_u8)word;
        *out_ticks = 10u;
        return status;
    case 0x16u:
        status = myraidenii_core_sound_fetch(sound, &sound->d);
        *out_ticks = 7u;
        return status;
    case 0x1eu:
        status = myraidenii_core_sound_fetch(sound, &sound->e);
        *out_ticks = 7u;
        return status;
    case 0x18u:
        status = myraidenii_core_sound_fetch(sound, &value);
        if (status == LIB_STATUS_OK) sound->pc = (lib_u16)(sound->pc + (lib_i8)value);
        *out_ticks = 12u;
        return status;
    case 0x20u:
    case 0x28u:
    case 0x30u:
    case 0x38u:
        status = myraidenii_core_sound_fetch(sound, &value);
        if (status != LIB_STATUS_OK) return status;
        if (myraidenii_core_sound_condition(sound, (lib_u8)((opcode >> 3u) & 3u))) {
            sound->pc = (lib_u16)(sound->pc + (lib_i8)value);
            *out_ticks = 12u;
        } else {
            *out_ticks = 7u;
        }
        return LIB_STATUS_OK;
    case 0x21u:
        status = myraidenii_core_sound_fetch_word(sound, &word);
        if (status == LIB_STATUS_OK) myraidenii_core_sound_hl_set(sound, word);
        *out_ticks = 10u;
        return status;
    case 0x26u:
        status = myraidenii_core_sound_fetch(sound, &sound->h);
        *out_ticks = 7u;
        return status;
    case 0x2eu:
        status = myraidenii_core_sound_fetch(sound, &sound->l);
        *out_ticks = 7u;
        return status;
    case 0x31u:
        status = myraidenii_core_sound_fetch_word(sound, &sound->sp);
        *out_ticks = 10u;
        return status;
    case 0x32u:
        status = myraidenii_core_sound_fetch_word(sound, &word);
        if (status == LIB_STATUS_OK)
            status = myraidenii_core_sound_write(sound, word, sound->a);
        *out_ticks = 13u;
        return status;
    case 0x36u:
        status = myraidenii_core_sound_fetch(sound, &value);
        if (status == LIB_STATUS_OK)
            status = myraidenii_core_sound_write(sound, myraidenii_core_sound_hl(sound), value);
        *out_ticks = 10u;
        return status;
    case 0x3au:
        status = myraidenii_core_sound_fetch_word(sound, &word);
        if (status == LIB_STATUS_OK)
            status = myraidenii_core_sound_read(sound, word, &sound->a);
        *out_ticks = 13u;
        return status;
    case 0x3eu:
        status = myraidenii_core_sound_fetch(sound, &sound->a);
        *out_ticks = 7u;
        return status;
    case 0x76u:
        sound->halted = LIB_TRUE;
        *out_ticks = 4u;
        return LIB_STATUS_OK;
    case 0x7eu:
        status = myraidenii_core_sound_read(sound, myraidenii_core_sound_hl(sound), &sound->a);
        *out_ticks = 7u;
        return status;
    case 0xafu:
        sound->a = 0u;
        myraidenii_core_sound_flags_logic(sound, sound->a, LIB_FALSE);
        *out_ticks = 4u;
        return LIB_STATUS_OK;
    case 0xc0u:
    case 0xc8u:
    case 0xd0u:
    case 0xd8u:
    case 0xe0u:
    case 0xe8u:
    case 0xf0u:
    case 0xf8u:
        if (!myraidenii_core_sound_condition(sound, (lib_u8)((opcode >> 3u) & 7u))) {
            *out_ticks = 5u;
            return LIB_STATUS_OK;
        }
        status = myraidenii_core_sound_pop(sound, &sound->pc);
        *out_ticks = 11u;
        return status;
    case 0xc3u:
        status = myraidenii_core_sound_fetch_word(sound, &sound->pc);
        *out_ticks = 10u;
        return status;
    case 0xc9u:
        status = myraidenii_core_sound_pop(sound, &sound->pc);
        *out_ticks = 10u;
        return status;
    case 0xcdu:
        status = myraidenii_core_sound_fetch_word(sound, &word);
        if (status != LIB_STATUS_OK) return status;
        status = myraidenii_core_sound_push(sound, sound->pc);
        if (status == LIB_STATUS_OK) sound->pc = word;
        *out_ticks = 17u;
        return status;
    case 0xd9u:
        myraidenii_core_sound_exchange_registers(sound);
        *out_ticks = 4u;
        return LIB_STATUS_OK;
    case 0xe3u:
        status = myraidenii_core_sound_pop(sound, &word);
        if (status != LIB_STATUS_OK) return status;
        status = myraidenii_core_sound_push(sound, myraidenii_core_sound_hl(sound));
        if (status == LIB_STATUS_OK) myraidenii_core_sound_hl_set(sound, word);
        *out_ticks = 19u;
        return status;
    case 0xebu:
        word = myraidenii_core_sound_hl(sound);
        myraidenii_core_sound_hl_set(sound,
            (lib_u16)((lib_u16)sound->d << 8u | sound->e));
        sound->d = (lib_u8)(word >> 8u);
        sound->e = (lib_u8)word;
        *out_ticks = 4u;
        return LIB_STATUS_OK;
    case 0xe9u:
        sound->pc = myraidenii_core_sound_hl(sound);
        *out_ticks = 4u;
        return LIB_STATUS_OK;
    case 0xf9u:
        sound->sp = myraidenii_core_sound_hl(sound);
        *out_ticks = 6u;
        return LIB_STATUS_OK;
    case 0xd3u:
        status = myraidenii_core_sound_fetch(sound, &value);
        if (status == LIB_STATUS_OK)
            status = myraidenii_core_sound_write(sound,
                (lib_u16)((lib_u16)sound->a << 8u | value), sound->a);
        *out_ticks = 11u;
        return status;
    case 0xdbu:
        status = myraidenii_core_sound_fetch(sound, &value);
        if (status == LIB_STATUS_OK)
            status = myraidenii_core_sound_read(sound,
                (lib_u16)((lib_u16)sound->a << 8u | value), &sound->a);
        *out_ticks = 11u;
        return status;
    default:
        return LIB_STATUS_UNSUPPORTED;
    }
}

lib_status myraidenii_core_sound_create(
    const myraidenii_core_sound_config *config, myraidenii_core_sound **out_sound)
{
    myraidenii_core_sound *sound;

    if (out_sound == LIB_NULL || config == LIB_NULL ||
        !myraidenii_core_sound_transport_valid(&config->transport) ||
        (config->program == LIB_NULL && config->program_bytes != 0u) ||
        (config->program != LIB_NULL &&
            config->program_bytes < MYRAIDENII_CORE_SOUND_FIXED_ROM_BYTES))
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_sound = LIB_NULL;
    sound = lib_allocate_zero(1u, sizeof(*sound));
    if (sound == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    sound->transport = config->transport;
    sound->program_bytes = config->program_bytes;
    if (sound->program_bytes != 0u) {
        sound->program = lib_allocate(sound->program_bytes);
        if (sound->program == LIB_NULL) {
            lib_release(sound);
            return LIB_STATUS_NO_MEMORY;
        }
        lib_memory_copy(sound->program, config->program, sound->program_bytes);
    }
    myraidenii_core_sound_reset(sound);
    *out_sound = sound;
    return LIB_STATUS_OK;
}

void myraidenii_core_sound_destroy(myraidenii_core_sound *sound)
{
    if (sound == LIB_NULL) return;
    lib_release(sound->program);
    lib_release(sound);
}

void myraidenii_core_sound_reset(myraidenii_core_sound *sound)
{
    if (sound != LIB_NULL) {
        sound->z80_ticks = 0u;
        sound->z80_phase = 0u;
        sound->available_ticks = 0u;
        sound->pc = 0u;
        sound->sp = 0xffffu;
        sound->a = 0u;
        sound->b = 0u;
        sound->c = 0u;
        sound->d = 0u;
        sound->e = 0u;
        sound->h = 0u;
        sound->l = 0u;
        sound->bank = 0u;
        sound->halted = LIB_FALSE;
        lib_memory_set(sound->ram, 0, sizeof(sound->ram));
    }
}

lib_status myraidenii_core_sound_advance(myraidenii_core_sound *sound,
    lib_u64 board_ticks)
{
    lib_u64 source_whole;
    lib_u64 source_remainder;
    lib_u64 delivered;
    lib_u64 phase;
    lib_status status;

    if (sound == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    source_whole = board_ticks / MYRAIDENII_CORE_SOUND_Z80_CLOCK_DENOMINATOR;
    source_remainder = board_ticks % MYRAIDENII_CORE_SOUND_Z80_CLOCK_DENOMINATOR;
    if (source_whole > LIB_UINT64_MAX / MYRAIDENII_CORE_SOUND_Z80_CLOCK_NUMERATOR)
        return LIB_STATUS_LIMIT_EXCEEDED;
    delivered = source_whole * MYRAIDENII_CORE_SOUND_Z80_CLOCK_NUMERATOR;
    phase = source_remainder * MYRAIDENII_CORE_SOUND_Z80_CLOCK_NUMERATOR + sound->z80_phase;
    if (delivered > LIB_UINT64_MAX -
            phase / MYRAIDENII_CORE_SOUND_Z80_CLOCK_DENOMINATOR)
        return LIB_STATUS_LIMIT_EXCEEDED;
    delivered += phase / MYRAIDENII_CORE_SOUND_Z80_CLOCK_DENOMINATOR;
    if (sound->z80_ticks > LIB_UINT64_MAX - delivered ||
        sound->available_ticks > LIB_UINT64_MAX - delivered)
        return LIB_STATUS_LIMIT_EXCEEDED;
    sound->z80_ticks += delivered;
    sound->available_ticks += delivered;
    sound->z80_phase = phase % MYRAIDENII_CORE_SOUND_Z80_CLOCK_DENOMINATOR;
    if (sound->program == LIB_NULL) return LIB_STATUS_OK;
    while (sound->available_ticks >= MYRAIDENII_CORE_SOUND_MAX_IMPLEMENTED_TICKS) {
        lib_u8 ticks;

        status = myraidenii_core_sound_execute(sound, &ticks);
        if (status != LIB_STATUS_OK) return status;
        sound->available_ticks -= ticks;
    }
    return LIB_STATUS_OK;
}
