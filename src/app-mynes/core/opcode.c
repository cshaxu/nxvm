#include "core/opcode_interface.h"

#define CORE_OPCODE_TEXT(value) ((const lib_u8 *)(value))

typedef struct core_opcode_info {
    const lib_u8 *mnemonic;
    const lib_u8 *mode;
    lib_u8 bytes;
} core_opcode_info;

/* This is the one immutable representation of MyNes's accepted RP2A03
 * opcode profile. CPU dispatch and copied App disassembly both consult it. */
static const core_opcode_info core_opcode_table[256] = {
    [0x00u] = { CORE_OPCODE_TEXT("BRK"), CORE_OPCODE_TEXT("imp"), 1u }, [0x01u] = { CORE_OPCODE_TEXT("ORA"), CORE_OPCODE_TEXT("indx"), 2u },
    [0x05u] = { CORE_OPCODE_TEXT("ORA"), CORE_OPCODE_TEXT("zp"), 2u }, [0x06u] = { CORE_OPCODE_TEXT("ASL"), CORE_OPCODE_TEXT("zp"), 2u },
    [0x08u] = { CORE_OPCODE_TEXT("PHP"), CORE_OPCODE_TEXT("imp"), 1u }, [0x09u] = { CORE_OPCODE_TEXT("ORA"), CORE_OPCODE_TEXT("imm"), 2u },
    [0x0au] = { CORE_OPCODE_TEXT("ASL"), CORE_OPCODE_TEXT("acc"), 1u }, [0x0du] = { CORE_OPCODE_TEXT("ORA"), CORE_OPCODE_TEXT("abs"), 3u },
    [0x0eu] = { CORE_OPCODE_TEXT("ASL"), CORE_OPCODE_TEXT("abs"), 3u }, [0x10u] = { CORE_OPCODE_TEXT("BPL"), CORE_OPCODE_TEXT("rel"), 2u },
    [0x11u] = { CORE_OPCODE_TEXT("ORA"), CORE_OPCODE_TEXT("indy"), 2u }, [0x15u] = { CORE_OPCODE_TEXT("ORA"), CORE_OPCODE_TEXT("zpx"), 2u },
    [0x16u] = { CORE_OPCODE_TEXT("ASL"), CORE_OPCODE_TEXT("zpx"), 2u }, [0x18u] = { CORE_OPCODE_TEXT("CLC"), CORE_OPCODE_TEXT("imp"), 1u },
    [0x19u] = { CORE_OPCODE_TEXT("ORA"), CORE_OPCODE_TEXT("absy"), 3u }, [0x1du] = { CORE_OPCODE_TEXT("ORA"), CORE_OPCODE_TEXT("absx"), 3u },
    [0x1eu] = { CORE_OPCODE_TEXT("ASL"), CORE_OPCODE_TEXT("absx"), 3u }, [0x20u] = { CORE_OPCODE_TEXT("JSR"), CORE_OPCODE_TEXT("abs"), 3u },
    [0x21u] = { CORE_OPCODE_TEXT("AND"), CORE_OPCODE_TEXT("indx"), 2u }, [0x24u] = { CORE_OPCODE_TEXT("BIT"), CORE_OPCODE_TEXT("zp"), 2u },
    [0x25u] = { CORE_OPCODE_TEXT("AND"), CORE_OPCODE_TEXT("zp"), 2u }, [0x26u] = { CORE_OPCODE_TEXT("ROL"), CORE_OPCODE_TEXT("zp"), 2u },
    [0x28u] = { CORE_OPCODE_TEXT("PLP"), CORE_OPCODE_TEXT("imp"), 1u }, [0x29u] = { CORE_OPCODE_TEXT("AND"), CORE_OPCODE_TEXT("imm"), 2u },
    [0x2au] = { CORE_OPCODE_TEXT("ROL"), CORE_OPCODE_TEXT("acc"), 1u }, [0x2cu] = { CORE_OPCODE_TEXT("BIT"), CORE_OPCODE_TEXT("abs"), 3u },
    [0x2du] = { CORE_OPCODE_TEXT("AND"), CORE_OPCODE_TEXT("abs"), 3u }, [0x2eu] = { CORE_OPCODE_TEXT("ROL"), CORE_OPCODE_TEXT("abs"), 3u },
    [0x30u] = { CORE_OPCODE_TEXT("BMI"), CORE_OPCODE_TEXT("rel"), 2u }, [0x31u] = { CORE_OPCODE_TEXT("AND"), CORE_OPCODE_TEXT("indy"), 2u },
    [0x35u] = { CORE_OPCODE_TEXT("AND"), CORE_OPCODE_TEXT("zpx"), 2u }, [0x36u] = { CORE_OPCODE_TEXT("ROL"), CORE_OPCODE_TEXT("zpx"), 2u },
    [0x38u] = { CORE_OPCODE_TEXT("SEC"), CORE_OPCODE_TEXT("imp"), 1u }, [0x39u] = { CORE_OPCODE_TEXT("AND"), CORE_OPCODE_TEXT("absy"), 3u },
    [0x3du] = { CORE_OPCODE_TEXT("AND"), CORE_OPCODE_TEXT("absx"), 3u }, [0x3eu] = { CORE_OPCODE_TEXT("ROL"), CORE_OPCODE_TEXT("absx"), 3u },
    [0x40u] = { CORE_OPCODE_TEXT("RTI"), CORE_OPCODE_TEXT("imp"), 1u }, [0x41u] = { CORE_OPCODE_TEXT("EOR"), CORE_OPCODE_TEXT("indx"), 2u },
    [0x45u] = { CORE_OPCODE_TEXT("EOR"), CORE_OPCODE_TEXT("zp"), 2u }, [0x46u] = { CORE_OPCODE_TEXT("LSR"), CORE_OPCODE_TEXT("zp"), 2u },
    [0x48u] = { CORE_OPCODE_TEXT("PHA"), CORE_OPCODE_TEXT("imp"), 1u }, [0x49u] = { CORE_OPCODE_TEXT("EOR"), CORE_OPCODE_TEXT("imm"), 2u },
    [0x4au] = { CORE_OPCODE_TEXT("LSR"), CORE_OPCODE_TEXT("acc"), 1u }, [0x4cu] = { CORE_OPCODE_TEXT("JMP"), CORE_OPCODE_TEXT("abs"), 3u },
    [0x4du] = { CORE_OPCODE_TEXT("EOR"), CORE_OPCODE_TEXT("abs"), 3u }, [0x4eu] = { CORE_OPCODE_TEXT("LSR"), CORE_OPCODE_TEXT("abs"), 3u },
    [0x50u] = { CORE_OPCODE_TEXT("BVC"), CORE_OPCODE_TEXT("rel"), 2u }, [0x51u] = { CORE_OPCODE_TEXT("EOR"), CORE_OPCODE_TEXT("indy"), 2u },
    [0x55u] = { CORE_OPCODE_TEXT("EOR"), CORE_OPCODE_TEXT("zpx"), 2u }, [0x56u] = { CORE_OPCODE_TEXT("LSR"), CORE_OPCODE_TEXT("zpx"), 2u },
    [0x58u] = { CORE_OPCODE_TEXT("CLI"), CORE_OPCODE_TEXT("imp"), 1u }, [0x59u] = { CORE_OPCODE_TEXT("EOR"), CORE_OPCODE_TEXT("absy"), 3u },
    [0x5du] = { CORE_OPCODE_TEXT("EOR"), CORE_OPCODE_TEXT("absx"), 3u }, [0x5eu] = { CORE_OPCODE_TEXT("LSR"), CORE_OPCODE_TEXT("absx"), 3u },
    [0x60u] = { CORE_OPCODE_TEXT("RTS"), CORE_OPCODE_TEXT("imp"), 1u }, [0x61u] = { CORE_OPCODE_TEXT("ADC"), CORE_OPCODE_TEXT("indx"), 2u },
    [0x65u] = { CORE_OPCODE_TEXT("ADC"), CORE_OPCODE_TEXT("zp"), 2u }, [0x66u] = { CORE_OPCODE_TEXT("ROR"), CORE_OPCODE_TEXT("zp"), 2u },
    [0x68u] = { CORE_OPCODE_TEXT("PLA"), CORE_OPCODE_TEXT("imp"), 1u }, [0x69u] = { CORE_OPCODE_TEXT("ADC"), CORE_OPCODE_TEXT("imm"), 2u },
    [0x6au] = { CORE_OPCODE_TEXT("ROR"), CORE_OPCODE_TEXT("acc"), 1u }, [0x6cu] = { CORE_OPCODE_TEXT("JMP"), CORE_OPCODE_TEXT("ind"), 3u },
    [0x6du] = { CORE_OPCODE_TEXT("ADC"), CORE_OPCODE_TEXT("abs"), 3u }, [0x6eu] = { CORE_OPCODE_TEXT("ROR"), CORE_OPCODE_TEXT("abs"), 3u },
    [0x70u] = { CORE_OPCODE_TEXT("BVS"), CORE_OPCODE_TEXT("rel"), 2u }, [0x71u] = { CORE_OPCODE_TEXT("ADC"), CORE_OPCODE_TEXT("indy"), 2u },
    [0x75u] = { CORE_OPCODE_TEXT("ADC"), CORE_OPCODE_TEXT("zpx"), 2u }, [0x76u] = { CORE_OPCODE_TEXT("ROR"), CORE_OPCODE_TEXT("zpx"), 2u },
    [0x78u] = { CORE_OPCODE_TEXT("SEI"), CORE_OPCODE_TEXT("imp"), 1u }, [0x79u] = { CORE_OPCODE_TEXT("ADC"), CORE_OPCODE_TEXT("absy"), 3u },
    [0x7du] = { CORE_OPCODE_TEXT("ADC"), CORE_OPCODE_TEXT("absx"), 3u }, [0x7eu] = { CORE_OPCODE_TEXT("ROR"), CORE_OPCODE_TEXT("absx"), 3u },
    [0x81u] = { CORE_OPCODE_TEXT("STA"), CORE_OPCODE_TEXT("indx"), 2u }, [0x84u] = { CORE_OPCODE_TEXT("STY"), CORE_OPCODE_TEXT("zp"), 2u },
    [0x85u] = { CORE_OPCODE_TEXT("STA"), CORE_OPCODE_TEXT("zp"), 2u }, [0x86u] = { CORE_OPCODE_TEXT("STX"), CORE_OPCODE_TEXT("zp"), 2u },
    [0x88u] = { CORE_OPCODE_TEXT("DEY"), CORE_OPCODE_TEXT("imp"), 1u }, [0x8au] = { CORE_OPCODE_TEXT("TXA"), CORE_OPCODE_TEXT("imp"), 1u },
    [0x8cu] = { CORE_OPCODE_TEXT("STY"), CORE_OPCODE_TEXT("abs"), 3u }, [0x8du] = { CORE_OPCODE_TEXT("STA"), CORE_OPCODE_TEXT("abs"), 3u },
    [0x8eu] = { CORE_OPCODE_TEXT("STX"), CORE_OPCODE_TEXT("abs"), 3u }, [0x90u] = { CORE_OPCODE_TEXT("BCC"), CORE_OPCODE_TEXT("rel"), 2u },
    [0x91u] = { CORE_OPCODE_TEXT("STA"), CORE_OPCODE_TEXT("indy"), 2u }, [0x94u] = { CORE_OPCODE_TEXT("STY"), CORE_OPCODE_TEXT("zpx"), 2u },
    [0x95u] = { CORE_OPCODE_TEXT("STA"), CORE_OPCODE_TEXT("zpx"), 2u }, [0x96u] = { CORE_OPCODE_TEXT("STX"), CORE_OPCODE_TEXT("zpy"), 2u },
    [0x98u] = { CORE_OPCODE_TEXT("TYA"), CORE_OPCODE_TEXT("imp"), 1u }, [0x99u] = { CORE_OPCODE_TEXT("STA"), CORE_OPCODE_TEXT("absy"), 3u },
    [0x9au] = { CORE_OPCODE_TEXT("TXS"), CORE_OPCODE_TEXT("imp"), 1u }, [0x9du] = { CORE_OPCODE_TEXT("STA"), CORE_OPCODE_TEXT("absx"), 3u },
    [0xa0u] = { CORE_OPCODE_TEXT("LDY"), CORE_OPCODE_TEXT("imm"), 2u }, [0xa1u] = { CORE_OPCODE_TEXT("LDA"), CORE_OPCODE_TEXT("indx"), 2u },
    [0xa2u] = { CORE_OPCODE_TEXT("LDX"), CORE_OPCODE_TEXT("imm"), 2u }, [0xa4u] = { CORE_OPCODE_TEXT("LDY"), CORE_OPCODE_TEXT("zp"), 2u },
    [0xa5u] = { CORE_OPCODE_TEXT("LDA"), CORE_OPCODE_TEXT("zp"), 2u }, [0xa6u] = { CORE_OPCODE_TEXT("LDX"), CORE_OPCODE_TEXT("zp"), 2u },
    [0xa8u] = { CORE_OPCODE_TEXT("TAY"), CORE_OPCODE_TEXT("imp"), 1u }, [0xa9u] = { CORE_OPCODE_TEXT("LDA"), CORE_OPCODE_TEXT("imm"), 2u },
    [0xaau] = { CORE_OPCODE_TEXT("TAX"), CORE_OPCODE_TEXT("imp"), 1u }, [0xacu] = { CORE_OPCODE_TEXT("LDY"), CORE_OPCODE_TEXT("abs"), 3u },
    [0xadu] = { CORE_OPCODE_TEXT("LDA"), CORE_OPCODE_TEXT("abs"), 3u }, [0xaeu] = { CORE_OPCODE_TEXT("LDX"), CORE_OPCODE_TEXT("abs"), 3u },
    [0xb0u] = { CORE_OPCODE_TEXT("BCS"), CORE_OPCODE_TEXT("rel"), 2u }, [0xb1u] = { CORE_OPCODE_TEXT("LDA"), CORE_OPCODE_TEXT("indy"), 2u },
    [0xb4u] = { CORE_OPCODE_TEXT("LDY"), CORE_OPCODE_TEXT("zpx"), 2u }, [0xb5u] = { CORE_OPCODE_TEXT("LDA"), CORE_OPCODE_TEXT("zpx"), 2u },
    [0xb6u] = { CORE_OPCODE_TEXT("LDX"), CORE_OPCODE_TEXT("zpy"), 2u }, [0xb8u] = { CORE_OPCODE_TEXT("CLV"), CORE_OPCODE_TEXT("imp"), 1u },
    [0xb9u] = { CORE_OPCODE_TEXT("LDA"), CORE_OPCODE_TEXT("absy"), 3u }, [0xbau] = { CORE_OPCODE_TEXT("TSX"), CORE_OPCODE_TEXT("imp"), 1u },
    [0xbcu] = { CORE_OPCODE_TEXT("LDY"), CORE_OPCODE_TEXT("absx"), 3u }, [0xbdu] = { CORE_OPCODE_TEXT("LDA"), CORE_OPCODE_TEXT("absx"), 3u },
    [0xbeu] = { CORE_OPCODE_TEXT("LDX"), CORE_OPCODE_TEXT("absy"), 3u }, [0xc0u] = { CORE_OPCODE_TEXT("CPY"), CORE_OPCODE_TEXT("imm"), 2u },
    [0xc1u] = { CORE_OPCODE_TEXT("CMP"), CORE_OPCODE_TEXT("indx"), 2u }, [0xc4u] = { CORE_OPCODE_TEXT("CPY"), CORE_OPCODE_TEXT("zp"), 2u },
    [0xc5u] = { CORE_OPCODE_TEXT("CMP"), CORE_OPCODE_TEXT("zp"), 2u }, [0xc6u] = { CORE_OPCODE_TEXT("DEC"), CORE_OPCODE_TEXT("zp"), 2u },
    [0xc8u] = { CORE_OPCODE_TEXT("INY"), CORE_OPCODE_TEXT("imp"), 1u }, [0xc9u] = { CORE_OPCODE_TEXT("CMP"), CORE_OPCODE_TEXT("imm"), 2u },
    [0xcau] = { CORE_OPCODE_TEXT("DEX"), CORE_OPCODE_TEXT("imp"), 1u }, [0xccu] = { CORE_OPCODE_TEXT("CPY"), CORE_OPCODE_TEXT("abs"), 3u },
    [0xcdu] = { CORE_OPCODE_TEXT("CMP"), CORE_OPCODE_TEXT("abs"), 3u }, [0xceu] = { CORE_OPCODE_TEXT("DEC"), CORE_OPCODE_TEXT("abs"), 3u },
    [0xd0u] = { CORE_OPCODE_TEXT("BNE"), CORE_OPCODE_TEXT("rel"), 2u }, [0xd1u] = { CORE_OPCODE_TEXT("CMP"), CORE_OPCODE_TEXT("indy"), 2u },
    [0xd5u] = { CORE_OPCODE_TEXT("CMP"), CORE_OPCODE_TEXT("zpx"), 2u }, [0xd6u] = { CORE_OPCODE_TEXT("DEC"), CORE_OPCODE_TEXT("zpx"), 2u },
    [0xd8u] = { CORE_OPCODE_TEXT("CLD"), CORE_OPCODE_TEXT("imp"), 1u }, [0xd9u] = { CORE_OPCODE_TEXT("CMP"), CORE_OPCODE_TEXT("absy"), 3u },
    [0xddu] = { CORE_OPCODE_TEXT("CMP"), CORE_OPCODE_TEXT("absx"), 3u }, [0xdeu] = { CORE_OPCODE_TEXT("DEC"), CORE_OPCODE_TEXT("absx"), 3u },
    [0xe0u] = { CORE_OPCODE_TEXT("CPX"), CORE_OPCODE_TEXT("imm"), 2u }, [0xe1u] = { CORE_OPCODE_TEXT("SBC"), CORE_OPCODE_TEXT("indx"), 2u },
    [0xe4u] = { CORE_OPCODE_TEXT("CPX"), CORE_OPCODE_TEXT("zp"), 2u }, [0xe5u] = { CORE_OPCODE_TEXT("SBC"), CORE_OPCODE_TEXT("zp"), 2u },
    [0xe6u] = { CORE_OPCODE_TEXT("INC"), CORE_OPCODE_TEXT("zp"), 2u }, [0xe8u] = { CORE_OPCODE_TEXT("INX"), CORE_OPCODE_TEXT("imp"), 1u },
    [0xe9u] = { CORE_OPCODE_TEXT("SBC"), CORE_OPCODE_TEXT("imm"), 2u }, [0xeau] = { CORE_OPCODE_TEXT("NOP"), CORE_OPCODE_TEXT("imp"), 1u },
    [0xecu] = { CORE_OPCODE_TEXT("CPX"), CORE_OPCODE_TEXT("abs"), 3u }, [0xedu] = { CORE_OPCODE_TEXT("SBC"), CORE_OPCODE_TEXT("abs"), 3u },
    [0xeeu] = { CORE_OPCODE_TEXT("INC"), CORE_OPCODE_TEXT("abs"), 3u }, [0xf0u] = { CORE_OPCODE_TEXT("BEQ"), CORE_OPCODE_TEXT("rel"), 2u },
    [0xf1u] = { CORE_OPCODE_TEXT("SBC"), CORE_OPCODE_TEXT("indy"), 2u }, [0xf5u] = { CORE_OPCODE_TEXT("SBC"), CORE_OPCODE_TEXT("zpx"), 2u },
    [0xf6u] = { CORE_OPCODE_TEXT("INC"), CORE_OPCODE_TEXT("zpx"), 2u }, [0xf8u] = { CORE_OPCODE_TEXT("SED"), CORE_OPCODE_TEXT("imp"), 1u },
    [0xf9u] = { CORE_OPCODE_TEXT("SBC"), CORE_OPCODE_TEXT("absy"), 3u }, [0xfdu] = { CORE_OPCODE_TEXT("SBC"), CORE_OPCODE_TEXT("absx"), 3u },
    [0xfeu] = { CORE_OPCODE_TEXT("INC"), CORE_OPCODE_TEXT("absx"), 3u }
};


static lib_bool core_opcode_text_equal(const lib_u8 *left, const char *right)
{
    while (*right != '\0') {
        if (*left != (lib_u8)*right) return LIB_FALSE;
        ++left;
        ++right;
    }
    return *left == 0u;
}

static core_opcode_address_mode core_opcode_mode_from_text(const lib_u8 *mode)
{
    if (core_opcode_text_equal(mode, "imp")) return CORE_OPCODE_ADDRESS_IMPLIED;
    if (core_opcode_text_equal(mode, "acc")) return CORE_OPCODE_ADDRESS_ACCUMULATOR;
    if (core_opcode_text_equal(mode, "imm")) return CORE_OPCODE_ADDRESS_IMMEDIATE;
    if (core_opcode_text_equal(mode, "zp")) return CORE_OPCODE_ADDRESS_ZERO_PAGE;
    if (core_opcode_text_equal(mode, "zpx")) return CORE_OPCODE_ADDRESS_ZERO_PAGE_X;
    if (core_opcode_text_equal(mode, "zpy")) return CORE_OPCODE_ADDRESS_ZERO_PAGE_Y;
    if (core_opcode_text_equal(mode, "abs")) return CORE_OPCODE_ADDRESS_ABSOLUTE;
    if (core_opcode_text_equal(mode, "absx")) return CORE_OPCODE_ADDRESS_ABSOLUTE_X;
    if (core_opcode_text_equal(mode, "absy")) return CORE_OPCODE_ADDRESS_ABSOLUTE_Y;
    if (core_opcode_text_equal(mode, "ind")) return CORE_OPCODE_ADDRESS_INDIRECT;
    if (core_opcode_text_equal(mode, "indx")) return CORE_OPCODE_ADDRESS_INDIRECT_X;
    if (core_opcode_text_equal(mode, "indy")) return CORE_OPCODE_ADDRESS_INDIRECT_Y;
    return CORE_OPCODE_ADDRESS_RELATIVE;
}

lib_bool core_opcode_describe(lib_u8 opcode, core_opcode_metadata *out_metadata)
{
    const core_opcode_info *info = &core_opcode_table[opcode];

    if (out_metadata == LIB_NULL || info->mnemonic == LIB_NULL) return LIB_FALSE;
    out_metadata->mnemonic[0] = (lib_u8)info->mnemonic[0];
    out_metadata->mnemonic[1] = (lib_u8)info->mnemonic[1];
    out_metadata->mnemonic[2] = (lib_u8)info->mnemonic[2];
    out_metadata->mnemonic[3] = '\0';
    out_metadata->mode = core_opcode_mode_from_text(info->mode);
    out_metadata->bytes = info->bytes;
    return LIB_TRUE;
}
