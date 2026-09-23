#include "core/opcode_interface.h"

typedef struct core_opcode_info {
    const char *mnemonic;
    const char *mode;
    lib_u8 bytes;
} core_opcode_info;

/* This is the one immutable representation of MyNes's accepted RP2A03
 * opcode profile. CPU dispatch and copied App disassembly both consult it. */
static const core_opcode_info core_opcode_table[256] = {
    [0x00u] = { "BRK", "imp", 1u }, [0x01u] = { "ORA", "indx", 2u },
    [0x05u] = { "ORA", "zp", 2u }, [0x06u] = { "ASL", "zp", 2u },
    [0x08u] = { "PHP", "imp", 1u }, [0x09u] = { "ORA", "imm", 2u },
    [0x0au] = { "ASL", "acc", 1u }, [0x0du] = { "ORA", "abs", 3u },
    [0x0eu] = { "ASL", "abs", 3u }, [0x10u] = { "BPL", "rel", 2u },
    [0x11u] = { "ORA", "indy", 2u }, [0x15u] = { "ORA", "zpx", 2u },
    [0x16u] = { "ASL", "zpx", 2u }, [0x18u] = { "CLC", "imp", 1u },
    [0x19u] = { "ORA", "absy", 3u }, [0x1du] = { "ORA", "absx", 3u },
    [0x1eu] = { "ASL", "absx", 3u }, [0x20u] = { "JSR", "abs", 3u },
    [0x21u] = { "AND", "indx", 2u }, [0x24u] = { "BIT", "zp", 2u },
    [0x25u] = { "AND", "zp", 2u }, [0x26u] = { "ROL", "zp", 2u },
    [0x28u] = { "PLP", "imp", 1u }, [0x29u] = { "AND", "imm", 2u },
    [0x2au] = { "ROL", "acc", 1u }, [0x2cu] = { "BIT", "abs", 3u },
    [0x2du] = { "AND", "abs", 3u }, [0x2eu] = { "ROL", "abs", 3u },
    [0x30u] = { "BMI", "rel", 2u }, [0x31u] = { "AND", "indy", 2u },
    [0x35u] = { "AND", "zpx", 2u }, [0x36u] = { "ROL", "zpx", 2u },
    [0x38u] = { "SEC", "imp", 1u }, [0x39u] = { "AND", "absy", 3u },
    [0x3du] = { "AND", "absx", 3u }, [0x3eu] = { "ROL", "absx", 3u },
    [0x40u] = { "RTI", "imp", 1u }, [0x41u] = { "EOR", "indx", 2u },
    [0x45u] = { "EOR", "zp", 2u }, [0x46u] = { "LSR", "zp", 2u },
    [0x48u] = { "PHA", "imp", 1u }, [0x49u] = { "EOR", "imm", 2u },
    [0x4au] = { "LSR", "acc", 1u }, [0x4cu] = { "JMP", "abs", 3u },
    [0x4du] = { "EOR", "abs", 3u }, [0x4eu] = { "LSR", "abs", 3u },
    [0x50u] = { "BVC", "rel", 2u }, [0x51u] = { "EOR", "indy", 2u },
    [0x55u] = { "EOR", "zpx", 2u }, [0x56u] = { "LSR", "zpx", 2u },
    [0x58u] = { "CLI", "imp", 1u }, [0x59u] = { "EOR", "absy", 3u },
    [0x5du] = { "EOR", "absx", 3u }, [0x5eu] = { "LSR", "absx", 3u },
    [0x60u] = { "RTS", "imp", 1u }, [0x61u] = { "ADC", "indx", 2u },
    [0x65u] = { "ADC", "zp", 2u }, [0x66u] = { "ROR", "zp", 2u },
    [0x68u] = { "PLA", "imp", 1u }, [0x69u] = { "ADC", "imm", 2u },
    [0x6au] = { "ROR", "acc", 1u }, [0x6cu] = { "JMP", "ind", 3u },
    [0x6du] = { "ADC", "abs", 3u }, [0x6eu] = { "ROR", "abs", 3u },
    [0x70u] = { "BVS", "rel", 2u }, [0x71u] = { "ADC", "indy", 2u },
    [0x75u] = { "ADC", "zpx", 2u }, [0x76u] = { "ROR", "zpx", 2u },
    [0x78u] = { "SEI", "imp", 1u }, [0x79u] = { "ADC", "absy", 3u },
    [0x7du] = { "ADC", "absx", 3u }, [0x7eu] = { "ROR", "absx", 3u },
    [0x81u] = { "STA", "indx", 2u }, [0x84u] = { "STY", "zp", 2u },
    [0x85u] = { "STA", "zp", 2u }, [0x86u] = { "STX", "zp", 2u },
    [0x88u] = { "DEY", "imp", 1u }, [0x8au] = { "TXA", "imp", 1u },
    [0x8cu] = { "STY", "abs", 3u }, [0x8du] = { "STA", "abs", 3u },
    [0x8eu] = { "STX", "abs", 3u }, [0x90u] = { "BCC", "rel", 2u },
    [0x91u] = { "STA", "indy", 2u }, [0x94u] = { "STY", "zpx", 2u },
    [0x95u] = { "STA", "zpx", 2u }, [0x96u] = { "STX", "zpy", 2u },
    [0x98u] = { "TYA", "imp", 1u }, [0x99u] = { "STA", "absy", 3u },
    [0x9au] = { "TXS", "imp", 1u }, [0x9du] = { "STA", "absx", 3u },
    [0xa0u] = { "LDY", "imm", 2u }, [0xa1u] = { "LDA", "indx", 2u },
    [0xa2u] = { "LDX", "imm", 2u }, [0xa4u] = { "LDY", "zp", 2u },
    [0xa5u] = { "LDA", "zp", 2u }, [0xa6u] = { "LDX", "zp", 2u },
    [0xa8u] = { "TAY", "imp", 1u }, [0xa9u] = { "LDA", "imm", 2u },
    [0xaau] = { "TAX", "imp", 1u }, [0xacu] = { "LDY", "abs", 3u },
    [0xadu] = { "LDA", "abs", 3u }, [0xaeu] = { "LDX", "abs", 3u },
    [0xb0u] = { "BCS", "rel", 2u }, [0xb1u] = { "LDA", "indy", 2u },
    [0xb4u] = { "LDY", "zpx", 2u }, [0xb5u] = { "LDA", "zpx", 2u },
    [0xb6u] = { "LDX", "zpy", 2u }, [0xb8u] = { "CLV", "imp", 1u },
    [0xb9u] = { "LDA", "absy", 3u }, [0xbau] = { "TSX", "imp", 1u },
    [0xbcu] = { "LDY", "absx", 3u }, [0xbdu] = { "LDA", "absx", 3u },
    [0xbeu] = { "LDX", "absy", 3u }, [0xc0u] = { "CPY", "imm", 2u },
    [0xc1u] = { "CMP", "indx", 2u }, [0xc4u] = { "CPY", "zp", 2u },
    [0xc5u] = { "CMP", "zp", 2u }, [0xc6u] = { "DEC", "zp", 2u },
    [0xc8u] = { "INY", "imp", 1u }, [0xc9u] = { "CMP", "imm", 2u },
    [0xcau] = { "DEX", "imp", 1u }, [0xccu] = { "CPY", "abs", 3u },
    [0xcdu] = { "CMP", "abs", 3u }, [0xceu] = { "DEC", "abs", 3u },
    [0xd0u] = { "BNE", "rel", 2u }, [0xd1u] = { "CMP", "indy", 2u },
    [0xd5u] = { "CMP", "zpx", 2u }, [0xd6u] = { "DEC", "zpx", 2u },
    [0xd8u] = { "CLD", "imp", 1u }, [0xd9u] = { "CMP", "absy", 3u },
    [0xddu] = { "CMP", "absx", 3u }, [0xdeu] = { "DEC", "absx", 3u },
    [0xe0u] = { "CPX", "imm", 2u }, [0xe1u] = { "SBC", "indx", 2u },
    [0xe4u] = { "CPX", "zp", 2u }, [0xe5u] = { "SBC", "zp", 2u },
    [0xe6u] = { "INC", "zp", 2u }, [0xe8u] = { "INX", "imp", 1u },
    [0xe9u] = { "SBC", "imm", 2u }, [0xeau] = { "NOP", "imp", 1u },
    [0xecu] = { "CPX", "abs", 3u }, [0xedu] = { "SBC", "abs", 3u },
    [0xeeu] = { "INC", "abs", 3u }, [0xf0u] = { "BEQ", "rel", 2u },
    [0xf1u] = { "SBC", "indy", 2u }, [0xf5u] = { "SBC", "zpx", 2u },
    [0xf6u] = { "INC", "zpx", 2u }, [0xf8u] = { "SED", "imp", 1u },
    [0xf9u] = { "SBC", "absy", 3u }, [0xfdu] = { "SBC", "absx", 3u },
    [0xfeu] = { "INC", "absx", 3u }
};


static core_opcode_address_mode core_opcode_mode_from_text(const char *mode)
{
    if (lib_c_strcmp(mode, "imp") == 0) return CORE_OPCODE_ADDRESS_IMPLIED;
    if (lib_c_strcmp(mode, "acc") == 0) return CORE_OPCODE_ADDRESS_ACCUMULATOR;
    if (lib_c_strcmp(mode, "imm") == 0) return CORE_OPCODE_ADDRESS_IMMEDIATE;
    if (lib_c_strcmp(mode, "zp") == 0) return CORE_OPCODE_ADDRESS_ZERO_PAGE;
    if (lib_c_strcmp(mode, "zpx") == 0) return CORE_OPCODE_ADDRESS_ZERO_PAGE_X;
    if (lib_c_strcmp(mode, "zpy") == 0) return CORE_OPCODE_ADDRESS_ZERO_PAGE_Y;
    if (lib_c_strcmp(mode, "abs") == 0) return CORE_OPCODE_ADDRESS_ABSOLUTE;
    if (lib_c_strcmp(mode, "absx") == 0) return CORE_OPCODE_ADDRESS_ABSOLUTE_X;
    if (lib_c_strcmp(mode, "absy") == 0) return CORE_OPCODE_ADDRESS_ABSOLUTE_Y;
    if (lib_c_strcmp(mode, "ind") == 0) return CORE_OPCODE_ADDRESS_INDIRECT;
    if (lib_c_strcmp(mode, "indx") == 0) return CORE_OPCODE_ADDRESS_INDIRECT_X;
    if (lib_c_strcmp(mode, "indy") == 0) return CORE_OPCODE_ADDRESS_INDIRECT_Y;
    return CORE_OPCODE_ADDRESS_RELATIVE;
}

lib_bool core_opcode_describe(lib_u8 opcode, core_opcode_metadata *out_metadata)
{
    const core_opcode_info *info = &core_opcode_table[opcode];

    if (out_metadata == LIB_NULL || info->mnemonic == LIB_NULL) return LIB_FALSE;
    out_metadata->mnemonic[0] = info->mnemonic[0];
    out_metadata->mnemonic[1] = info->mnemonic[1];
    out_metadata->mnemonic[2] = info->mnemonic[2];
    out_metadata->mnemonic[3] = '\0';
    out_metadata->mode = core_opcode_mode_from_text(info->mode);
    out_metadata->bytes = info->bytes;
    return LIB_TRUE;
}
