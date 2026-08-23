#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/cpu_instructions.h"
#include "../support/core_machine_cpu_fixture.h"

#define PREVIEW_RESET_PHYSICAL 0x000ffff0u

static C_INT preview_expect(const type_unsigned_8 *bytes,
    type_unsigned_8 available_bytes, core_machine_cpu_profile profile,
    type_bool code_32, type_unsigned_8 expected_bytes,
    type_unsigned_8 expected_components)
{
    core_machine_cpu_instruction_lexeme lexeme;

    return core_machine_cpu_instruction_lexeme_scan(bytes, available_bytes,
        profile, code_32, &lexeme) && lexeme.available &&
        lexeme.byte_count == expected_bytes &&
        lexeme.component_count == expected_components;
}

static C_INT preview_test_layouts(C_VOID)
{
    static const type_unsigned_8 nop[] = { 0x90u };
    static const type_unsigned_8 mov_imm32[] = {
        0x66u, 0xb8u, 0x78u, 0x56u, 0x34u, 0x12u
    };
    static const type_unsigned_8 mov_sib_disp[] = {
        0x67u, 0x8bu, 0x84u, 0x88u, 0x78u, 0x56u, 0x34u, 0x12u
    };
    static const type_unsigned_8 near_jcc[] = {
        0x0fu, 0x84u, 0x78u, 0x56u, 0x34u, 0x12u
    };
    static const type_unsigned_8 group1_alias[] = { 0x82u, 0xc0u, 0xffu };
    static const type_unsigned_8 clts[] = { 0x0fu, 0x06u };
    static const type_unsigned_8 shld_imm8[] = {
        0x0fu, 0xa4u, 0xc1u, 0x05u
    };
    static const type_unsigned_8 rep_movsb[] = { 0xf3u, 0xa4u };
    static const type_unsigned_8 repeated_operand_prefix[] = {
        0x66u, 0x66u, 0xb8u, 0x78u, 0x56u, 0x34u, 0x12u
    };
    static const type_unsigned_8 repeated_address_prefix[] = {
        0x67u, 0x67u, 0x8au, 0x06u
    };

    return preview_expect(nop, sizeof(nop), CORE_MACHINE_CPU_PROFILE_8086,
        TYPE_FALSE, 1u, 1u) &&
        preview_expect(mov_imm32, sizeof(mov_imm32),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 6u, 3u) &&
        preview_expect(mov_sib_disp, sizeof(mov_sib_disp),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 8u, 5u) &&
        preview_expect(near_jcc, sizeof(near_jcc),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_TRUE, 6u, 3u) &&
        preview_expect(group1_alias, sizeof(group1_alias),
            CORE_MACHINE_CPU_PROFILE_8086, TYPE_FALSE, 3u, 3u) &&
        preview_expect(group1_alias, sizeof(group1_alias),
            CORE_MACHINE_CPU_PROFILE_80186, TYPE_FALSE, 3u, 3u) &&
        preview_expect(group1_alias, sizeof(group1_alias),
            CORE_MACHINE_CPU_PROFILE_80286, TYPE_FALSE, 3u, 3u) &&
        preview_expect(group1_alias, sizeof(group1_alias),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_TRUE, 3u, 3u) &&
        preview_expect(clts, sizeof(clts), CORE_MACHINE_CPU_PROFILE_80386,
            TYPE_TRUE, 2u, 2u) &&
        preview_expect(shld_imm8, sizeof(shld_imm8),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_TRUE, 4u, 4u) &&
        preview_expect(rep_movsb, sizeof(rep_movsb),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_TRUE, 2u, 2u) &&
        preview_expect(repeated_operand_prefix, sizeof(repeated_operand_prefix),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 7u, 4u) &&
        preview_expect(repeated_address_prefix, sizeof(repeated_address_prefix),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 4u, 4u);
}

static C_INT preview_test_accumulator_xchg_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    type_unsigned_8 profile;
    type_unsigned_8 opcode;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (opcode = 0x90u; opcode != 0x98u; ++opcode) {
        const type_unsigned_8 primary[] = {opcode};
        if (!preview_expect(primary, sizeof(primary), profiles[profile], TYPE_FALSE,
            1u, 1u)) return 0;
    }
    return preview_expect((const type_unsigned_8[]){0x66u,0x91u}, 2u,
        CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 2u, 2u) &&
        preview_expect((const type_unsigned_8[]){0x66u,0x97u}, 2u,
        CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 2u, 2u);
}
static C_INT preview_test_primary_inc_dec_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    type_unsigned_8 profile;
    type_unsigned_8 opcode;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (opcode = 0x40u; opcode != 0x50u; ++opcode) {
        const type_unsigned_8 primary[] = {opcode};
        if (!preview_expect(primary, sizeof(primary), profiles[profile], TYPE_FALSE,
            1u, 1u)) return 0;
    }
    return preview_expect((const type_unsigned_8[]){0x66u,0x40u}, 2u,
        CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 2u, 2u) &&
        preview_expect((const type_unsigned_8[]){0x66u,0x48u}, 2u,
        CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 2u, 2u);
}
static C_INT preview_test_immediate_register_mov_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    type_unsigned_8 profile;
    type_unsigned_8 opcode;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile) {
        for (opcode = 0xb0u; opcode != 0xb8u; ++opcode) {
            const type_unsigned_8 byte_immediate[] = {opcode, 0xa5u};
            if (!preview_expect(byte_immediate, sizeof(byte_immediate),
                profiles[profile], TYPE_FALSE, 2u, 2u)) return 0;
        }
        for (opcode = 0xb8u; opcode != 0xc0u; ++opcode) {
            const type_unsigned_8 word_immediate[] = {opcode, 0x34u, 0x12u};
            if (!preview_expect(word_immediate, sizeof(word_immediate),
                profiles[profile], TYPE_FALSE, 3u, 2u)) return 0;
        }
    }
    for (opcode = 0xb0u; opcode != 0xb8u; ++opcode) {
        const type_unsigned_8 byte_immediate[] = {0x66u, opcode, 0xa5u};
        if (!preview_expect(byte_immediate, sizeof(byte_immediate),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 3u, 3u)) return 0;
    }
    for (opcode = 0xb8u; opcode != 0xc0u; ++opcode) {
        const type_unsigned_8 dword_immediate[] = {0x66u, opcode,
            0x78u, 0x56u, 0x34u, 0x12u};
        if (!preview_expect(dword_immediate, sizeof(dword_immediate),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 6u, 3u)) return 0;
    }
    return 1;
}

static C_INT preview_test_accumulator_test_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 byte[] = {0xa8u, 0x80u};
    static const type_unsigned_8 word[] = {0xa9u, 0u, 0x80u};
    static const type_unsigned_8 dword[] = {0x66u, 0xa9u, 0u, 0u, 0u, 0x80u};
    type_unsigned_8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
        if (!preview_expect(byte, sizeof(byte), profiles[profile], TYPE_FALSE,
            2u, 2u) || !preview_expect(word, sizeof(word), profiles[profile],
            TYPE_FALSE, 3u, 2u)) return 0;
    return preview_expect(dword, sizeof(dword), CORE_MACHINE_CPU_PROFILE_80386,
        TYPE_FALSE, 6u, 3u);
}
static C_INT preview_test_scas_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    type_unsigned_8 profile;
    type_unsigned_8 opcode;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (opcode = 0xaeu; opcode != 0xb0u; ++opcode) {
        const type_unsigned_8 primary[] = {opcode};
        const type_unsigned_8 repe[] = {0xf3u, opcode};
        const type_unsigned_8 repne[] = {0xf2u, opcode};
        if (!preview_expect(primary, sizeof(primary), profiles[profile],
            TYPE_FALSE, 1u, 1u) || !preview_expect(repe, sizeof(repe),
            profiles[profile], TYPE_FALSE, 2u, 2u) ||
            !preview_expect(repne, sizeof(repne), profiles[profile],
            TYPE_FALSE, 2u, 2u)) return 0;
    }
    for (opcode = 0xaeu; opcode != 0xb0u; ++opcode) {
        const type_unsigned_8 operand[] = {0x66u, opcode};
        const type_unsigned_8 address[] = {0x67u, opcode};
        const type_unsigned_8 combined[] = {0x66u, 0x67u, opcode};
        const type_unsigned_8 repeated[] = {0xf2u, 0x66u, 0x67u, opcode};
        if (!preview_expect(operand, sizeof(operand),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 2u, 2u) ||
            !preview_expect(address, sizeof(address),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 2u, 2u) ||
            !preview_expect(combined, sizeof(combined),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 3u, 3u) ||
            !preview_expect(repeated, sizeof(repeated),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 4u, 4u)) return 0;
    }
    return 1;
}
static C_INT preview_test_lods_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    type_unsigned_8 profile;
    type_unsigned_8 opcode;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (opcode = 0xacu; opcode != 0xaeu; ++opcode) {
        const type_unsigned_8 primary[] = {opcode};
        const type_unsigned_8 repeated[] = {0xf3u, opcode};
        if (!preview_expect(primary, sizeof(primary), profiles[profile],
            TYPE_FALSE, 1u, 1u) || !preview_expect(repeated, sizeof(repeated),
            profiles[profile], TYPE_FALSE, 2u, 2u)) return 0;
    }
    for (opcode = 0xacu; opcode != 0xaeu; ++opcode) {
        const type_unsigned_8 operand[] = {0x66u, opcode};
        const type_unsigned_8 address[] = {0x67u, opcode};
        const type_unsigned_8 combined[] = {0x66u, 0x67u, opcode};
        const type_unsigned_8 repeated[] = {0xf3u, 0x66u, 0x67u, opcode};
        if (!preview_expect(operand, sizeof(operand),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 2u, 2u) ||
            !preview_expect(address, sizeof(address),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 2u, 2u) ||
            !preview_expect(combined, sizeof(combined),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 3u, 3u) ||
            !preview_expect(repeated, sizeof(repeated),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 4u, 4u)) return 0;
    }
    return 1;
}
static C_INT preview_test_stos_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    type_unsigned_8 profile;
    type_unsigned_8 opcode;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (opcode = 0xaau; opcode != 0xacu; ++opcode) {
        const type_unsigned_8 primary[] = {opcode};
        const type_unsigned_8 repeated[] = {0xf3u, opcode};
        if (!preview_expect(primary, sizeof(primary), profiles[profile],
            TYPE_FALSE, 1u, 1u) || !preview_expect(repeated, sizeof(repeated),
            profiles[profile], TYPE_FALSE, 2u, 2u)) return 0;
    }
    for (opcode = 0xaau; opcode != 0xacu; ++opcode) {
        const type_unsigned_8 operand[] = {0x66u, opcode};
        const type_unsigned_8 address[] = {0x67u, opcode};
        const type_unsigned_8 combined[] = {0x66u, 0x67u, opcode};
        const type_unsigned_8 repeated[] = {0xf3u, 0x66u, 0x67u, opcode};
        if (!preview_expect(operand, sizeof(operand),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 2u, 2u) ||
            !preview_expect(address, sizeof(address),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 2u, 2u) ||
            !preview_expect(combined, sizeof(combined),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 3u, 3u) ||
            !preview_expect(repeated, sizeof(repeated),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 4u, 4u)) return 0;
    }
    return 1;
}
static C_INT preview_test_cmps_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    type_unsigned_8 profile;
    type_unsigned_8 opcode;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (opcode = 0xa6u; opcode != 0xa8u; ++opcode) {
        const type_unsigned_8 primary[] = {opcode};
        const type_unsigned_8 repe[] = {0xf3u, opcode};
        const type_unsigned_8 repne[] = {0xf2u, opcode};
        if (!preview_expect(primary, sizeof(primary), profiles[profile],
            TYPE_FALSE, 1u, 1u) || !preview_expect(repe, sizeof(repe),
            profiles[profile], TYPE_FALSE, 2u, 2u) ||
            !preview_expect(repne, sizeof(repne), profiles[profile],
            TYPE_FALSE, 2u, 2u)) return 0;
    }
    for (opcode = 0xa6u; opcode != 0xa8u; ++opcode) {
        const type_unsigned_8 operand[] = {0x66u, opcode};
        const type_unsigned_8 address[] = {0x67u, opcode};
        const type_unsigned_8 combined[] = {0x66u, 0x67u, opcode};
        const type_unsigned_8 repeated[] = {0xf2u, 0x66u, 0x67u, opcode};
        if (!preview_expect(operand, sizeof(operand),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 2u, 2u) ||
            !preview_expect(address, sizeof(address),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 2u, 2u) ||
            !preview_expect(combined, sizeof(combined),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 3u, 3u) ||
            !preview_expect(repeated, sizeof(repeated),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 4u, 4u)) return 0;
    }
    return 1;
}

static C_INT preview_test_movs_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    type_unsigned_8 profile;
    type_unsigned_8 opcode;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (opcode = 0xa4u; opcode != 0xa6u; ++opcode) {
        const type_unsigned_8 primary[] = {opcode};
        const type_unsigned_8 repeated[] = {0xf3u, opcode};
        if (!preview_expect(primary, sizeof(primary), profiles[profile],
            TYPE_FALSE, 1u, 1u) || !preview_expect(repeated, sizeof(repeated),
            profiles[profile], TYPE_FALSE, 2u, 2u)) return 0;
    }
    for (opcode = 0xa4u; opcode != 0xa6u; ++opcode) {
        const type_unsigned_8 operand[] = {0x66u, opcode};
        const type_unsigned_8 address[] = {0x67u, opcode};
        const type_unsigned_8 combined[] = {0x66u, 0x67u, opcode};
        const type_unsigned_8 repeated[] = {0xf3u, 0x66u, 0x67u, opcode};
        if (!preview_expect(operand, sizeof(operand),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 2u, 2u) ||
            !preview_expect(address, sizeof(address),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 2u, 2u) ||
            !preview_expect(combined, sizeof(combined),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 3u, 3u) ||
            !preview_expect(repeated, sizeof(repeated),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 4u, 4u)) return 0;
    }
    return 1;
}

static C_INT preview_test_moffs_mov_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    type_unsigned_8 profile;
    type_unsigned_8 opcode;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (opcode = 0xa0u; opcode != 0xa4u; ++opcode) {
        const type_unsigned_8 moffs16[] = {opcode, 0x34u, 0x12u};
        if (!preview_expect(moffs16, sizeof(moffs16), profiles[profile],
            TYPE_FALSE, 3u, 2u)) return 0;
    }
    for (opcode = 0xa0u; opcode != 0xa4u; ++opcode) {
        const type_unsigned_8 operand16[] = {0x66u, opcode, 0x34u, 0x12u};
        const type_unsigned_8 address32[] = {0x67u, opcode,
            0x78u, 0x56u, 0x34u, 0x12u};
        const type_unsigned_8 combined[] = {0x66u, 0x67u, opcode,
            0x78u, 0x56u, 0x34u, 0x12u};
        if (!preview_expect(operand16, sizeof(operand16),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 4u, 3u) ||
            !preview_expect(address32, sizeof(address32),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 6u, 3u) ||
            !preview_expect(combined, sizeof(combined),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 7u, 4u)) return 0;
    }
    return 1;
}

static C_INT preview_test_enter_leave_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 enter[] = {0xc8u, 4u, 0u, 2u};
    static const type_unsigned_8 leave[] = {0xc9u};
   type_unsigned_8 profile;


    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
        if (!preview_expect(enter, sizeof(enter), profiles[profile], TYPE_FALSE,
            4u, 2u) || !preview_expect(leave, sizeof(leave), profiles[profile],
            TYPE_FALSE, 1u, 1u)) return 0;
    return preview_expect((const type_unsigned_8[]){0x66u,0xc8u,4u,0u,1u},
        5u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 5u, 3u) &&
        preview_expect((const type_unsigned_8[]){0x66u,0xc9u}, 2u,
        CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 2u, 2u);
}
static C_INT preview_test_far_return_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 ret[] = {0xcbu};
    static const type_unsigned_8 cleanup[] = {0xcau, 4u, 0u};
    type_unsigned_8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
        if (!preview_expect(ret, sizeof(ret), profiles[profile], TYPE_FALSE,
            1u, 1u) || !preview_expect(cleanup, sizeof(cleanup),
            profiles[profile], TYPE_FALSE, 3u, 2u)) return 0;
    return preview_expect((const type_unsigned_8[]){0x66u, 0xcbu}, 2u,
        CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 2u, 2u) &&
        preview_expect((const type_unsigned_8[]){0x66u, 0xcau, 4u, 0u}, 4u,
        CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 4u, 3u);
}
static C_INT preview_test_near_return_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 ret[] = {0xc3u};
    static const type_unsigned_8 cleanup[] = {0xc2u, 4u, 0u};
    type_unsigned_8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
        if (!preview_expect(ret, sizeof(ret), profiles[profile], TYPE_FALSE,
            1u, 1u) || !preview_expect(cleanup, sizeof(cleanup),
            profiles[profile], TYPE_FALSE, 3u, 2u)) return 0;
    return preview_expect((const type_unsigned_8[]){0x66u, 0xc3u}, 2u,
        CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 2u, 2u) &&
        preview_expect((const type_unsigned_8[]){0x66u, 0xc2u, 4u, 0u}, 4u,
        CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 4u, 3u);
}
static C_INT preview_test_group2_immediate_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    type_unsigned_8 profile;
    type_unsigned_8 opcode;
    type_unsigned_8 extension;

    for (opcode = 0xc0u; opcode != 0xc2u; ++opcode)
    for (extension = 0u; extension != 8u; ++extension) {
        const type_unsigned_8 code[] = {opcode,
            (type_unsigned_8)((extension << 3u) | 0xc0u), 1u};
        core_machine_cpu_instruction_lexeme lexeme;
        if (core_machine_cpu_instruction_lexeme_scan(code, sizeof(code),
            CORE_MACHINE_CPU_PROFILE_8086, TYPE_FALSE, &lexeme) ||
            lexeme.available) return 0;
        for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
            if (!preview_expect(code, sizeof(code), profiles[profile],
                TYPE_FALSE, 3u, 3u)) return 0;
    }
    return preview_expect((const type_unsigned_8[]){0x66u, 0xc1u, 0xc0u, 1u},
        4u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 4u, 4u);
}
static C_INT preview_test_les_lds_profiles(C_VOID)
{
    static const type_unsigned_8 les[] = {0xc4u,0x06u,0u,0x20u};
    static const type_unsigned_8 lds[] = {0xc5u,0x06u,0u,0x20u};
    static const core_machine_cpu_profile profiles[] = { CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386 };
    type_unsigned_8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
        if (!preview_expect(les,4u,profiles[profile],TYPE_FALSE,4u,3u) || !preview_expect(lds,4u,profiles[profile],TYPE_FALSE,4u,3u)) return 0;
    return preview_expect((const type_unsigned_8[]){0x66u,0xc4u,0x06u,0u,0x20u},5u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_FALSE,5u,4u);
}

static C_INT preview_test_sreg_push_pop_profiles(C_VOID)
{
    static const type_unsigned_8 legacy[] = {0x06u,0x07u,0x0eu,0x16u,0x17u,0x1eu,0x1fu}; static const type_unsigned_8 extended[] = {0xa0u,0xa1u,0xa8u,0xa9u}; static const core_machine_cpu_profile p[] = {CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386}; type_unsigned_8 i,j;
    for(i=0u;i!=sizeof(p)/sizeof(p[0]);++i) for(j=0u;j!=sizeof(legacy);++j) if(!preview_expect(&legacy[j],1u,p[i],TYPE_FALSE,1u,1u)) return 0;
    for(j=0u;j!=sizeof(extended);++j) { if(!preview_expect((const type_unsigned_8[]){0x0fu,extended[j]},2u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_FALSE,2u,2u)) return 0; } return 1;
}

static C_INT preview_test_lea_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = { CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386 };
    static const type_unsigned_8 lea[] = {0x8du, 0x40u, 0x10u};
    type_unsigned_8 profile;
    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
        if (!preview_expect(lea, sizeof(lea), profiles[profile], TYPE_FALSE, 3u, 3u)) return 0;
    return preview_expect((const type_unsigned_8[]){0x66u,0x8du,0x40u,0x10u},4u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_FALSE,4u,4u) && preview_expect((const type_unsigned_8[]){0x67u,0x8du,0x46u,0x10u},4u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_FALSE,4u,4u);
}

static C_INT preview_test_loop_jcxz_profiles(C_VOID)
{
    static const type_unsigned_8 opcodes[] = {0xe0u, 0xe1u, 0xe2u, 0xe3u};
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    type_unsigned_8 profile;
    type_unsigned_8 opcode;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
        for (opcode = 0u; opcode != sizeof(opcodes); ++opcode)
            if (!preview_expect((const type_unsigned_8[]){opcodes[opcode], 0u},
                2u, profiles[profile], TYPE_FALSE, 2u, 2u)) return 0;
    return preview_expect((const type_unsigned_8[]){0x66u, 0xe2u, 0u}, 3u,
        CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 3u, 3u) &&
        preview_expect((const type_unsigned_8[]){0x67u, 0xe3u, 0u}, 3u,
        CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 3u, 3u);
}
static C_INT preview_test_direct_flags_profiles(C_VOID)
{
    static const type_unsigned_8 opcodes[] = {0xf5u, 0xf8u, 0xf9u, 0xfcu, 0xfdu};
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    type_unsigned_8 profile;
    type_unsigned_8 opcode;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
        for (opcode = 0u; opcode != sizeof(opcodes); ++opcode)
            if (!preview_expect(&opcodes[opcode], 1u, profiles[profile],
                TYPE_FALSE, 1u, 1u)) return 0;
    return preview_expect((const type_unsigned_8[]){0x66u, 0xf8u}, 2u,
        CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 2u, 2u) &&
        preview_expect((const type_unsigned_8[]){0x67u, 0xfdu}, 2u,
        CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 2u, 2u);
}
static C_INT preview_test_modrm_data_move_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 xchg[] = { 0x86u, 0xc1u };
    static const type_unsigned_8 xchg_memory[] = { 0x87u, 0x06u, 0u, 0x20u };
    static const type_unsigned_8 mov[] = { 0x88u, 0xc1u, 0x89u, 0xc1u,
        0x8au, 0xc1u, 0x8bu, 0xc1u };
    static const type_unsigned_8 mov_memory[] = { 0x8bu, 0x06u, 0u, 0x20u };
    static const type_unsigned_8 sreg_store[] = { 0x8cu, 0xc1u };
    static const type_unsigned_8 sreg_load[] = { 0x8eu, 0xd9u };
    type_unsigned_8 profile;
    type_unsigned_8 opcode;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile) {
        if (!preview_expect(xchg, sizeof(xchg), profiles[profile], TYPE_FALSE,
                2u, 2u) ||
            !preview_expect(xchg_memory, sizeof(xchg_memory), profiles[profile],
                TYPE_FALSE, 4u, 3u) ||
            !preview_expect(mov_memory, sizeof(mov_memory), profiles[profile],
                TYPE_FALSE, 4u, 3u) ||
            !preview_expect(sreg_store, sizeof(sreg_store), profiles[profile],
                TYPE_FALSE, 2u, 2u) ||
            !preview_expect(sreg_load, sizeof(sreg_load), profiles[profile],
                TYPE_FALSE, 2u, 2u)) return 0;
        for (opcode = 0u; opcode != sizeof(mov); opcode += 2u)
            if (!preview_expect(&mov[opcode], 2u, profiles[profile], TYPE_FALSE,
                    2u, 2u)) return 0;
    }
    return preview_expect((const type_unsigned_8[]){0x66u, 0x87u, 0xc1u},
        3u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 3u, 3u) &&
        preview_expect((const type_unsigned_8[]){0x67u, 0x87u, 0x46u, 0x10u},
        4u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 4u, 4u) &&
        preview_expect((const type_unsigned_8[]){0x66u, 0x8bu, 0xc1u},
        3u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 3u, 3u) &&
        preview_expect((const type_unsigned_8[]){0x67u, 0x8bu, 0x46u, 0x10u},
        4u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 4u, 4u) &&
        preview_expect((const type_unsigned_8[]){0x66u, 0x8cu, 0xc1u},
        3u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 3u, 3u) &&
        preview_expect((const type_unsigned_8[]){0x67u, 0x8eu, 0x5eu, 0x10u},
        4u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 4u, 4u) &&
        preview_expect((const type_unsigned_8[]){0x8cu, 0xe1u}, 2u,
        CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 2u, 2u) &&
        preview_expect((const type_unsigned_8[]){0x8eu, 0xe1u}, 2u,
        CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 2u, 2u);
}
static C_INT preview_test_rm_immediate_mov_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 mov8_memory[] = {0xc6u, 0x06u, 0u, 0x20u, 0x5au};
    static const type_unsigned_8 mov16_memory[] = {0xc7u, 0x06u, 0u, 0x20u, 0x34u, 0x12u};
    core_machine_cpu_instruction_lexeme lexeme;
    type_unsigned_8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile) {
        if (!preview_expect(mov8_memory, sizeof(mov8_memory), profiles[profile],
                TYPE_FALSE, 5u, 4u) ||
            !preview_expect(mov16_memory, sizeof(mov16_memory), profiles[profile],
                TYPE_FALSE, 6u, 4u) ||
            !preview_expect((const type_unsigned_8[]){0xc6u, 0xc0u, 0x5au}, 3u,
                profiles[profile], TYPE_FALSE, 3u, 3u) ||
            !preview_expect((const type_unsigned_8[]){0xc7u, 0xc0u, 0x34u, 0x12u},
                4u, profiles[profile], TYPE_FALSE, 4u, 3u) ||
            core_machine_cpu_instruction_lexeme_scan(
                (const type_unsigned_8[]){0xc6u, 0xc8u, 0x5au}, 3u,
                profiles[profile], TYPE_FALSE, &lexeme) || lexeme.available) return 0;
    }
    return preview_expect((const type_unsigned_8[]){0x66u, 0xc7u, 0x06u, 0u,
        0x20u, 0x78u, 0x56u, 0x34u, 0x12u}, 9u,
        CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 9u, 5u) &&
        preview_expect((const type_unsigned_8[]){0x67u, 0xc7u, 0x46u, 0x10u,
        0x34u, 0x12u}, 6u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 6u, 5u);
}
static C_INT preview_test_shared_prefix_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 segment_prefixes[] = {0x26u, 0x2eu, 0x36u, 0x3eu};
    static const type_unsigned_8 repeat_prefixes[] = {0xf2u, 0xf3u};
    static const type_unsigned_8 size_prefixes[] = {0x66u, 0x67u};
    static const type_unsigned_8 locked_add[] = {0xf0u, 0x01u, 0x06u, 0u, 0x20u};
    core_machine_cpu_instruction_lexeme lexeme;
    type_unsigned_8 profile;
    type_unsigned_8 prefix;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile) {
        for (prefix = 0u; prefix != sizeof(segment_prefixes); ++prefix)
            if (!preview_expect((const type_unsigned_8[]){segment_prefixes[prefix],
                    0x8au, 0x06u, 0u, 0x20u}, 5u, profiles[profile],
                    TYPE_FALSE, 5u, 4u)) return 0;
        for (prefix = 0u; prefix != sizeof(repeat_prefixes); ++prefix)
            if (!preview_expect((const type_unsigned_8[]){repeat_prefixes[prefix],
                    0xa4u}, 2u, profiles[profile], TYPE_FALSE, 2u, 2u)) return 0;
    }
    for (profile = 0u; profile != 3u; ++profile) {
        for (prefix = 0u; prefix != sizeof(size_prefixes); ++prefix)
            if (core_machine_cpu_instruction_lexeme_scan(
                    (const type_unsigned_8[]){size_prefixes[prefix], 0x8bu, 0xc1u},
                    3u, profiles[profile], TYPE_FALSE, &lexeme) || lexeme.available)
                return 0;
        if (core_machine_cpu_instruction_lexeme_scan(locked_add, sizeof(locked_add),
                profiles[profile], TYPE_FALSE, &lexeme) || lexeme.available) return 0;
    }
    return preview_expect((const type_unsigned_8[]){0x64u, 0x8au, 0x06u, 0u, 0x20u},
        5u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 5u, 4u) &&
        preview_expect((const type_unsigned_8[]){0x65u, 0x8au, 0x06u, 0u, 0x20u},
        5u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 5u, 4u) &&
        preview_expect((const type_unsigned_8[]){0x66u, 0x8bu, 0xc1u}, 3u,
        CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 3u, 3u) &&
        preview_expect((const type_unsigned_8[]){0x67u, 0x8bu, 0x46u, 0x10u}, 4u,
        CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 4u, 4u) &&
        !core_machine_cpu_instruction_lexeme_scan(locked_add, sizeof(locked_add),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, &lexeme) && !lexeme.available;
}
static C_INT preview_test_primary_alu_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 base[] = {
        0x00u, 0x08u, 0x10u, 0x18u, 0x20u, 0x28u, 0x30u, 0x38u
    };
    type_unsigned_8 profile;
    type_unsigned_8 operation;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (operation = 0u; operation != sizeof(base); ++operation) {
        const type_unsigned_8 opcode = base[operation];
        if (!preview_expect((const type_unsigned_8[]){opcode, 0xc1u}, 2u,
                profiles[profile], TYPE_FALSE, 2u, 2u) ||
            !preview_expect((const type_unsigned_8[]){(type_unsigned_8)(opcode + 1u),
                0xc1u}, 2u, profiles[profile], TYPE_FALSE, 2u, 2u) ||
            !preview_expect((const type_unsigned_8[]){(type_unsigned_8)(opcode + 2u),
                0xc1u}, 2u, profiles[profile], TYPE_FALSE, 2u, 2u) ||
            !preview_expect((const type_unsigned_8[]){(type_unsigned_8)(opcode + 3u),
                0xc1u}, 2u, profiles[profile], TYPE_FALSE, 2u, 2u) ||
            !preview_expect((const type_unsigned_8[]){(type_unsigned_8)(opcode + 4u),
                0x5au}, 2u, profiles[profile], TYPE_FALSE, 2u, 2u) ||
            !preview_expect((const type_unsigned_8[]){(type_unsigned_8)(opcode + 5u),
                0x5au, 0u}, 3u, profiles[profile], TYPE_FALSE, 3u, 2u)) return 0;
    }
    return preview_expect((const type_unsigned_8[]){0x66u, 0x01u, 0xc8u}, 3u,
        CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 3u, 3u) &&
        preview_expect((const type_unsigned_8[]){0x67u, 0x01u, 0x46u, 0x10u},
        4u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 4u, 4u) &&
        preview_expect((const type_unsigned_8[]){0x66u, 0x05u, 0x5au, 0u,
        0u, 0u}, 6u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 6u, 3u) &&
        preview_expect((const type_unsigned_8[]){0x66u, 0x67u, 0x03u, 0x46u,
        0x10u}, 5u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 5u, 5u);
}
static C_INT preview_test_direct_far_control_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 call_far[] = {0x9au, 0x34u, 0x12u, 0x78u, 0x56u};
    static const type_unsigned_8 jump_far[] = {0xeau, 0x34u, 0x12u, 0x78u, 0x56u};
    type_unsigned_8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
        if (!preview_expect(call_far, sizeof(call_far), profiles[profile],
                TYPE_FALSE, 5u, 2u) ||
            !preview_expect(jump_far, sizeof(jump_far), profiles[profile],
                TYPE_FALSE, 5u, 2u)) return 0;
    return preview_expect((const type_unsigned_8[]){0x9au, 0x78u, 0x56u,
            0x34u, 0x12u, 0xbcu, 0x9au}, 7u,
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_TRUE, 7u, 2u) &&
        preview_expect((const type_unsigned_8[]){0xeau, 0x78u, 0x56u,
            0x34u, 0x12u, 0xbcu, 0x9au}, 7u,
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_TRUE, 7u, 2u) &&
        preview_expect((const type_unsigned_8[]){0x66u, 0x9au, 0x34u,
            0x12u, 0x78u, 0x56u}, 6u,
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_TRUE, 6u, 3u) &&
        preview_expect((const type_unsigned_8[]){0x66u, 0xeau, 0x78u,
            0x56u, 0x34u, 0x12u, 0xbcu, 0x9au}, 8u,
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 8u, 3u) &&
        preview_expect((const type_unsigned_8[]){0x66u, 0x67u, 0x9au,
            0x78u, 0x56u, 0x34u, 0x12u, 0xbcu, 0x9au}, 9u,
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 9u, 4u);
}
static C_INT preview_test_direct_near_control_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 call_near[] = {0xe8u, 0x34u, 0x12u};
    static const type_unsigned_8 jump_near[] = {0xe9u, 0x34u, 0x12u};
    static const type_unsigned_8 jump_short[] = {0xebu, 0x80u};
    type_unsigned_8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
        if (!preview_expect(call_near, sizeof(call_near), profiles[profile],
                TYPE_FALSE, 3u, 2u) ||
            !preview_expect(jump_near, sizeof(jump_near), profiles[profile],
                TYPE_FALSE, 3u, 2u) ||
            !preview_expect(jump_short, sizeof(jump_short), profiles[profile],
                TYPE_FALSE, 2u, 2u)) return 0;
    return preview_expect((const type_unsigned_8[]){0x66u, 0xe8u,
            0x78u, 0x56u, 0x34u, 0x12u}, 6u,
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 6u, 3u) &&
        preview_expect((const type_unsigned_8[]){0x66u, 0xe9u,
            0x78u, 0x56u, 0x34u, 0x12u}, 6u,
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 6u, 3u) &&
        preview_expect((const type_unsigned_8[]){0x67u, 0xe9u, 0x34u, 0x12u},
            4u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 4u, 3u) &&
        preview_expect((const type_unsigned_8[]){0x66u, 0xebu, 0x80u}, 3u,
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 3u, 3u) &&
        preview_expect((const type_unsigned_8[]){0x66u, 0x67u, 0xe8u,
            0x78u, 0x56u, 0x34u, 0x12u}, 7u,
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 7u, 4u);
}
static C_INT preview_test_near_jcc_profiles(C_VOID)
{
    static const core_machine_cpu_profile unavailable_profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    core_machine_cpu_instruction_lexeme lexeme;
    type_unsigned_8 profile;
    type_unsigned_8 opcode;

    for (profile = 0u; profile < sizeof(unavailable_profiles) /
        sizeof(unavailable_profiles[0]); ++profile)
    for (opcode = 0x80u; opcode != 0x90u; ++opcode)
        if (core_machine_cpu_instruction_lexeme_scan((const type_unsigned_8[]){
                0x0fu, opcode, 0u, 0u }, 4u, unavailable_profiles[profile],
                TYPE_FALSE, &lexeme) || lexeme.available) return 0;
    for (opcode = 0x80u; opcode != 0x90u; ++opcode) {
        if (!preview_expect((const type_unsigned_8[]){0x0fu, opcode, 0x34u, 0x12u},
                4u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 4u, 3u) ||
            !preview_expect((const type_unsigned_8[]){0x0fu, opcode,
                0x78u, 0x56u, 0x34u, 0x12u}, 6u,
                CORE_MACHINE_CPU_PROFILE_80386, TYPE_TRUE, 6u, 3u)) return 0;
    }
    return preview_expect((const type_unsigned_8[]){0x66u, 0x0fu, 0x84u,
            0x78u, 0x56u, 0x34u, 0x12u}, 7u,
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 7u, 4u) &&
        preview_expect((const type_unsigned_8[]){0x67u, 0x0fu, 0x85u,
            0x34u, 0x12u}, 5u, CORE_MACHINE_CPU_PROFILE_80386,
            TYPE_FALSE, 5u, 4u) &&
        preview_expect((const type_unsigned_8[]){0x66u, 0x67u, 0x0fu,
            0x8cu, 0x78u, 0x56u, 0x34u, 0x12u}, 8u,
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 8u, 5u);
}
static C_INT preview_test_setcc_profiles(C_VOID)
{
    static const core_machine_cpu_profile unavailable_profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286
    };
    core_machine_cpu_instruction_lexeme lexeme;
    type_unsigned_8 profile;
    type_unsigned_8 opcode;

    for (profile = 0u; profile < sizeof(unavailable_profiles) /
        sizeof(unavailable_profiles[0]); ++profile)
    for (opcode = 0x90u; opcode != 0xa0u; ++opcode)
        if (core_machine_cpu_instruction_lexeme_scan((const type_unsigned_8[]){
                0x0fu, opcode, 0xc0u }, 3u, unavailable_profiles[profile],
                TYPE_FALSE, &lexeme) || lexeme.available) return 0;
    for (opcode = 0x90u; opcode != 0xa0u; ++opcode)
        if (!preview_expect((const type_unsigned_8[]){0x0fu, opcode, 0xc0u},
                3u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 3u, 3u) ||
            !preview_expect((const type_unsigned_8[]){0x0fu, opcode, 0x06u,
                0x34u, 0x12u}, 5u, CORE_MACHINE_CPU_PROFILE_80386,
                TYPE_FALSE, 5u, 4u)) return 0;
    return preview_expect((const type_unsigned_8[]){0x66u, 0x0fu, 0x94u, 0xc0u},
            4u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 4u, 4u) &&
        preview_expect((const type_unsigned_8[]){0x67u, 0x0fu, 0x95u, 0x05u,
            0x78u, 0x56u, 0x34u, 0x12u}, 8u,
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 8u, 5u) &&
        preview_expect((const type_unsigned_8[]){0x66u, 0x67u, 0x0fu, 0x9cu,
            0x05u, 0x78u, 0x56u, 0x34u, 0x12u}, 9u,
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 9u, 6u);
}
static C_INT preview_test_bit_test_profiles(C_VOID)
{
    static const core_machine_cpu_profile unavailable_profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286
    };
    static const type_unsigned_8 opcodes[] = {0xa3u, 0xabu, 0xb3u, 0xbbu};
    core_machine_cpu_instruction_lexeme lexeme;
    type_unsigned_8 profile;
    type_unsigned_8 index;
    type_unsigned_8 extension;

    for (profile = 0u; profile < sizeof(unavailable_profiles) /
        sizeof(unavailable_profiles[0]); ++profile)
    for (index = 0u; index < sizeof(opcodes); ++index)
        if (core_machine_cpu_instruction_lexeme_scan((const type_unsigned_8[]){
                0x0fu, opcodes[index], 0xc8u }, 3u, unavailable_profiles[profile],
                TYPE_FALSE, &lexeme) || lexeme.available) return 0;
    for (index = 0u; index < sizeof(opcodes); ++index)
        if (!preview_expect((const type_unsigned_8[]){0x0fu, opcodes[index], 0xc8u},
                3u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 3u, 3u) ||
            !preview_expect((const type_unsigned_8[]){0x0fu, opcodes[index], 0x0eu,
                0x34u, 0x12u}, 5u, CORE_MACHINE_CPU_PROFILE_80386,
                TYPE_FALSE, 5u, 4u)) return 0;
    for (extension = 0u; extension != 8u; ++extension) {
        const type_unsigned_8 ba[] = {0x0fu, 0xbau,
            (type_unsigned_8)(0xc0u | (extension << 3u)), 0x1fu};
        const type_bool valid = extension >= 4u;
        if (valid != (core_machine_cpu_instruction_lexeme_scan(ba, sizeof(ba),
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, &lexeme) && lexeme.available))
            return 0;
        if (valid && !preview_expect(ba, sizeof(ba), CORE_MACHINE_CPU_PROFILE_80386,
            TYPE_FALSE, 4u, 4u)) return 0;
    }
    return preview_expect((const type_unsigned_8[]){0x0fu, 0xbau, 0x2eu,
            0x34u, 0x12u, 0x1fu}, 6u, CORE_MACHINE_CPU_PROFILE_80386,
            TYPE_FALSE, 6u, 5u) &&
        preview_expect((const type_unsigned_8[]){0x66u, 0x0fu, 0xabu, 0xc8u},
            4u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 4u, 4u) &&
        preview_expect((const type_unsigned_8[]){0x67u, 0x0fu, 0xbau, 0x2du,
            0x78u, 0x56u, 0x34u, 0x12u, 0x1fu}, 9u,
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 9u, 6u) &&
        preview_expect((const type_unsigned_8[]){0x66u, 0x67u, 0x0fu, 0xbbu,
            0x05u, 0x78u, 0x56u, 0x34u, 0x12u}, 9u,
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 9u, 6u);
}
static C_INT preview_test_double_shift_profiles(C_VOID)
{
    static const core_machine_cpu_profile unavailable[] = {
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286
    };
    static const type_unsigned_8 immediate[] = {0xa4u, 0xacu};
    static const type_unsigned_8 cl[] = {0xa5u, 0xadu};
    core_machine_cpu_instruction_lexeme lexeme;
    type_unsigned_8 profile;
    type_unsigned_8 index;

    for (profile = 0u; profile < sizeof(unavailable) / sizeof(unavailable[0]); ++profile)
    for (index = 0u; index < sizeof(immediate); ++index)
        if (core_machine_cpu_instruction_lexeme_scan((const type_unsigned_8[]){
                0x0fu, immediate[index], 0xc8u, 1u }, 4u, unavailable[profile],
                TYPE_FALSE, &lexeme) || lexeme.available) return 0;
    for (index = 0u; index < sizeof(immediate); ++index)
        if (!preview_expect((const type_unsigned_8[]){0x0fu, immediate[index],
                0xc8u, 1u}, 4u, CORE_MACHINE_CPU_PROFILE_80386,
                TYPE_FALSE, 4u, 4u) || !preview_expect((const type_unsigned_8[]){
                0x0fu, immediate[index], 0x0eu, 0x34u, 0x12u, 1u}, 6u,
                CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 6u, 5u) ||
            !preview_expect((const type_unsigned_8[]){0x0fu, cl[index], 0xc8u},
                3u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 3u, 3u)) return 0;
    return preview_expect((const type_unsigned_8[]){0x66u,0x0fu,0xa4u,0xc8u,1u},
            5u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 5u, 5u) &&
        preview_expect((const type_unsigned_8[]){0x67u,0x0fu,0xadu,0x05u,
            0x78u,0x56u,0x34u,0x12u}, 8u, CORE_MACHINE_CPU_PROFILE_80386,
            TYPE_FALSE, 8u, 5u) && preview_expect((const type_unsigned_8[]){
            0x66u,0x67u,0x0fu,0xacu,0x05u,0x78u,0x56u,0x34u,0x12u,1u},
            10u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 10u, 7u);
}
static C_INT preview_test_bit_scan_profiles(C_VOID)
{
    static const core_machine_cpu_profile unavailable[] = {
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286
    };
    static const type_unsigned_8 opcodes[] = {0xbcu, 0xbdu};
    core_machine_cpu_instruction_lexeme lexeme;
    type_unsigned_8 profile;
    type_unsigned_8 index;

    for (profile = 0u; profile < sizeof(unavailable) / sizeof(unavailable[0]); ++profile)
    for (index = 0u; index < sizeof(opcodes); ++index)
        if (core_machine_cpu_instruction_lexeme_scan((const type_unsigned_8[]){
                0x0fu, opcodes[index], 0xc8u }, 3u, unavailable[profile],
                TYPE_FALSE, &lexeme) || lexeme.available) return 0;
    for (index = 0u; index < sizeof(opcodes); ++index)
        if (!preview_expect((const type_unsigned_8[]){0x0fu, opcodes[index], 0xc8u},
                3u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 3u, 3u) ||
            !preview_expect((const type_unsigned_8[]){0x0fu, opcodes[index], 0x0eu,
                0x34u, 0x12u}, 5u, CORE_MACHINE_CPU_PROFILE_80386,
                TYPE_FALSE, 5u, 4u)) return 0;
    return preview_expect((const type_unsigned_8[]){0x66u,0x0fu,0xbcu,0xc8u},
            4u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 4u, 4u) &&
        preview_expect((const type_unsigned_8[]){0x67u,0x0fu,0xbdu,0x05u,
            0x78u,0x56u,0x34u,0x12u}, 8u, CORE_MACHINE_CPU_PROFILE_80386,
            TYPE_FALSE, 8u, 5u) && preview_expect((const type_unsigned_8[]){
            0x66u,0x67u,0x0fu,0xbcu,0x05u,0x78u,0x56u,0x34u,0x12u},
            9u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 9u, 6u);
}
static C_INT preview_test_movx_profiles(C_VOID)
{
    static const core_machine_cpu_profile unavailable[] = {CORE_MACHINE_CPU_PROFILE_80186,CORE_MACHINE_CPU_PROFILE_80286};
    static const type_unsigned_8 opcodes[] = {0xb6u,0xb7u,0xbeu,0xbfu};
    core_machine_cpu_instruction_lexeme lexeme; type_unsigned_8 profile,index;
    for(profile=0u;profile<sizeof(unavailable)/sizeof(unavailable[0]);++profile)
    for(index=0u;index<sizeof(opcodes);++index)
        if(core_machine_cpu_instruction_lexeme_scan((const type_unsigned_8[]){0x0fu,opcodes[index],0xc8u},3u,unavailable[profile],TYPE_FALSE,&lexeme)||lexeme.available)return 0;
    for(index=0u;index<sizeof(opcodes);++index)
        if(!preview_expect((const type_unsigned_8[]){0x0fu,opcodes[index],0xc8u},3u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_FALSE,3u,3u)||!preview_expect((const type_unsigned_8[]){0x0fu,opcodes[index],0x0eu,0x34u,0x12u},5u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_FALSE,5u,4u))return 0;
    return preview_expect((const type_unsigned_8[]){0x66u,0x0fu,0xb7u,0xc8u},4u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_FALSE,4u,4u)&&preview_expect((const type_unsigned_8[]){0x67u,0x0fu,0xbeu,0x05u,0x78u,0x56u,0x34u,0x12u},8u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_FALSE,8u,5u)&&preview_expect((const type_unsigned_8[]){0x66u,0x67u,0x0fu,0xbfu,0x05u,0x78u,0x56u,0x34u,0x12u},9u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_FALSE,9u,6u);
}
static C_INT preview_test_imul2_profiles(C_VOID)
{
    static const core_machine_cpu_profile unavailable[] = {CORE_MACHINE_CPU_PROFILE_80186,CORE_MACHINE_CPU_PROFILE_80286};
    core_machine_cpu_instruction_lexeme lexeme; type_unsigned_8 profile;
    for(profile=0u;profile<sizeof(unavailable)/sizeof(unavailable[0]);++profile)
        if(core_machine_cpu_instruction_lexeme_scan((const type_unsigned_8[]){0x0fu,0xafu,0xc8u},3u,unavailable[profile],TYPE_FALSE,&lexeme)||lexeme.available)return 0;
    return preview_expect((const type_unsigned_8[]){0x0fu,0xafu,0xc8u},3u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_FALSE,3u,3u)&&preview_expect((const type_unsigned_8[]){0x0fu,0xafu,0x0eu,0x34u,0x12u},5u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_FALSE,5u,4u)&&preview_expect((const type_unsigned_8[]){0x66u,0x0fu,0xafu,0xc8u},4u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_FALSE,4u,4u)&&preview_expect((const type_unsigned_8[]){0x67u,0x0fu,0xafu,0x05u,0x78u,0x56u,0x34u,0x12u},8u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_FALSE,8u,5u)&&preview_expect((const type_unsigned_8[]){0x66u,0x67u,0x0fu,0xafu,0x05u,0x78u,0x56u,0x34u,0x12u},9u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_FALSE,9u,6u);
}
static C_INT preview_test_system_selector_group_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    core_machine_cpu_instruction_lexeme lexeme;
    type_unsigned_8 profile;
    type_unsigned_8 extension;

    for (profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]); ++profile) {
        for (extension = 0u; extension < 6u; ++extension)
            if (!preview_expect((const type_unsigned_8[]){0x0fu, 0x00u,
                    (type_unsigned_8)(0xc0u | (extension << 3u))}, 3u,
                    profiles[profile], TYPE_FALSE, 3u, 3u)) return 0;
        for (extension = 6u; extension < 8u; ++extension)
            if (core_machine_cpu_instruction_lexeme_scan((const type_unsigned_8[]){0x0fu,
                    0x00u, (type_unsigned_8)(extension << 3u)}, 3u,
                    profiles[profile], TYPE_FALSE, &lexeme) || lexeme.available ||
                core_machine_cpu_instruction_lexeme_scan((const type_unsigned_8[]){0x0fu,
                    0x00u, (type_unsigned_8)((extension << 3u) | 6u), 0u, 0u}, 5u,
                    profiles[profile], TYPE_FALSE, &lexeme) || lexeme.available) return 0;
    }
    return core_machine_cpu_instruction_lexeme_scan((const type_unsigned_8[]){0x0fu,
            0x00u, 0xc0u}, 3u, CORE_MACHINE_CPU_PROFILE_80186, TYPE_FALSE,
            &lexeme) == TYPE_FALSE && !lexeme.available;
}
static C_INT preview_test_system_group_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 legal_memory[] = {0x0fu, 0x01u, 0x06u, 0u, 0x20u};
    static const type_unsigned_8 legal_register[] = {0x0fu, 0x01u, 0xe0u};
    core_machine_cpu_instruction_lexeme lexeme;
    type_unsigned_8 profile;
    type_unsigned_8 extension;

    if (!preview_expect((const type_unsigned_8[]){0x0fu}, 1u,
            CORE_MACHINE_CPU_PROFILE_8086, TYPE_FALSE, 1u, 1u) ||
        core_machine_cpu_instruction_lexeme_scan(legal_memory, sizeof(legal_memory),
            CORE_MACHINE_CPU_PROFILE_80186, TYPE_FALSE, &lexeme) || lexeme.available)
        return 0;
    for (profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]); ++profile)
        if (!preview_expect(legal_memory, sizeof(legal_memory), profiles[profile],
                TYPE_FALSE, 5u, 4u) || !preview_expect(legal_register,
                sizeof(legal_register), profiles[profile], TYPE_FALSE, 3u, 3u) ||
                !preview_expect((const type_unsigned_8[]){0x0fu, 0x01u, 0xf0u},
                3u, profiles[profile], TYPE_FALSE, 3u, 3u)) return 0;
    for (extension = 5u; extension < 8u; extension += 2u)
        for (profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]); ++profile)
            if (core_machine_cpu_instruction_lexeme_scan((const type_unsigned_8[]){0x0fu,
                    0x01u, (type_unsigned_8)(extension << 3u)}, 3u,
                    profiles[profile], TYPE_FALSE, &lexeme) || lexeme.available ||
                core_machine_cpu_instruction_lexeme_scan((const type_unsigned_8[]){0x0fu,
                    0x01u, (type_unsigned_8)((extension << 3u) | 6u), 0u, 0u}, 5u,
                    profiles[profile], TYPE_FALSE, &lexeme) || lexeme.available) return 0;
    return preview_expect((const type_unsigned_8[]){0x66u, 0x0fu, 0x01u,
            0x06u, 0u, 0x20u}, 6u, CORE_MACHINE_CPU_PROFILE_80386,
            TYPE_FALSE, 6u, 5u) && preview_expect((const type_unsigned_8[]){0x67u,
            0x0fu, 0x01u, 0x05u, 0u, 0x20u, 0u, 0u}, 8u,
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 8u, 5u);
}
static C_INT preview_test_test_mov_profiles(C_VOID)
{
    static const core_machine_cpu_profile unavailable[] = {
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286
    };
    static const type_unsigned_8 opcodes[] = {0x24u, 0x26u};
    core_machine_cpu_instruction_lexeme lexeme;
    type_unsigned_8 profile;
    type_unsigned_8 opcode;
    type_unsigned_8 index;

    for (profile = 0u; profile < sizeof(unavailable) / sizeof(unavailable[0]); ++profile)
        for (opcode = 0u; opcode < sizeof(opcodes); ++opcode)
            if (core_machine_cpu_instruction_lexeme_scan((const type_unsigned_8[]){0x0fu,
                    opcodes[opcode], 0xf0u}, 3u, unavailable[profile], TYPE_TRUE,
                    &lexeme) || lexeme.available) return 0;
    for (opcode = 0u; opcode < sizeof(opcodes); ++opcode) {
        for (index = 6u; index < 8u; ++index)
            if (!preview_expect((const type_unsigned_8[]){0x0fu, opcodes[opcode],
                    (type_unsigned_8)(0xc0u | (index << 3u))}, 3u,
                    CORE_MACHINE_CPU_PROFILE_80386, TYPE_TRUE, 3u, 3u)) return 0;
        for (index = 0u; index < 6u; ++index)
            if (core_machine_cpu_instruction_lexeme_scan((const type_unsigned_8[]){0x0fu,
                    opcodes[opcode], (type_unsigned_8)(0xc0u | (index << 3u))},
                    3u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_TRUE, &lexeme) ||
                    lexeme.available) return 0;
        if (core_machine_cpu_instruction_lexeme_scan((const type_unsigned_8[]){0x0fu,
                opcodes[opcode], 0x06u, 0u, 0x20u}, 5u,
                CORE_MACHINE_CPU_PROFILE_80386, TYPE_TRUE, &lexeme) || lexeme.available)
            return 0;
    }
    return preview_expect((const type_unsigned_8[]){0x66u, 0x0fu, 0x24u,
            0xf0u}, 4u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_TRUE, 4u, 4u) &&
        preview_expect((const type_unsigned_8[]){0x67u, 0x0fu, 0x26u,
            0xf8u}, 4u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_TRUE, 4u, 4u) &&
        preview_expect((const type_unsigned_8[]){0x66u, 0x67u, 0x0fu, 0x26u,
            0xf0u}, 5u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_TRUE, 5u, 5u);
}
static C_INT preview_test_lss_lfs_lgs_profiles(C_VOID)
{
    static const core_machine_cpu_profile unavailable[] = {
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286
    };
    static const type_unsigned_8 opcodes[] = {0xb2u, 0xb4u, 0xb5u};
    core_machine_cpu_instruction_lexeme lexeme;
    type_unsigned_8 profile;
    type_unsigned_8 opcode;

    for (profile = 0u; profile < sizeof(unavailable) / sizeof(unavailable[0]); ++profile)
        for (opcode = 0u; opcode < sizeof(opcodes); ++opcode)
            if (core_machine_cpu_instruction_lexeme_scan((const type_unsigned_8[]){0x0fu,
                    opcodes[opcode], 0x06u, 0u, 0x20u}, 5u,
                    unavailable[profile], TYPE_FALSE, &lexeme) || lexeme.available)
                return 0;
    for (opcode = 0u; opcode < sizeof(opcodes); ++opcode)
        if (!preview_expect((const type_unsigned_8[]){0x0fu, opcodes[opcode],
                    0x06u, 0u, 0x20u}, 5u, CORE_MACHINE_CPU_PROFILE_80386,
                    TYPE_FALSE, 5u, 4u) ||
                core_machine_cpu_instruction_lexeme_scan((const type_unsigned_8[]){0x0fu,
                    opcodes[opcode], 0xc0u}, 3u, CORE_MACHINE_CPU_PROFILE_80386,
                    TYPE_FALSE, &lexeme) || lexeme.available)
            return 0;
    return preview_expect((const type_unsigned_8[]){0x66u, 0x0fu, 0xb2u,
            0x06u, 0u, 0x20u}, 6u, CORE_MACHINE_CPU_PROFILE_80386,
            TYPE_FALSE, 6u, 5u) &&
        preview_expect((const type_unsigned_8[]){0x67u, 0x0fu, 0xb4u,
            0x05u, 0x78u, 0x56u, 0x34u, 0x12u}, 8u,
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 8u, 5u) &&
        preview_expect((const type_unsigned_8[]){0x66u, 0x67u, 0x0fu, 0xb5u,
            0x05u, 0x78u, 0x56u, 0x34u, 0x12u}, 9u,
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 9u, 6u);
}
static C_INT preview_test_debug_mov_profiles(C_VOID)
{
    static const core_machine_cpu_profile unavailable[]={CORE_MACHINE_CPU_PROFILE_80186,CORE_MACHINE_CPU_PROFILE_80286};
    static const type_unsigned_8 opcodes[]={0x21u,0x23u},valid[]={0u,1u,2u,3u,6u,7u};
    core_machine_cpu_instruction_lexeme lexeme;type_unsigned_8 p,o,i;
    for(p=0u;p<sizeof(unavailable)/sizeof(unavailable[0]);++p)for(o=0u;o<sizeof(opcodes);++o)if(core_machine_cpu_instruction_lexeme_scan((const type_unsigned_8[]){0x0fu,opcodes[o],0xc0u},3u,unavailable[p],TYPE_FALSE,&lexeme)||lexeme.available)return 0;
    for(o=0u;o<sizeof(opcodes);++o){for(i=0u;i<sizeof(valid);++i)if(!preview_expect((const type_unsigned_8[]){0x0fu,opcodes[o],(type_unsigned_8)(0xc0u|(valid[i]<<3u))},3u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_TRUE,3u,3u))return 0;for(i=4u;i<6u;++i)if(core_machine_cpu_instruction_lexeme_scan((const type_unsigned_8[]){0x0fu,opcodes[o],(type_unsigned_8)(0xc0u|(i<<3u))},3u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_TRUE,&lexeme)||lexeme.available)return 0;}
    return !(core_machine_cpu_instruction_lexeme_scan((const type_unsigned_8[]){0x0fu,0x21u,0x00u},3u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_TRUE,&lexeme)||lexeme.available);
}
static C_INT preview_test_short_jcc_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    type_unsigned_8 profile;
    type_unsigned_8 opcode;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (opcode = 0x70u; opcode != 0x80u; ++opcode)
        if (!preview_expect((const type_unsigned_8[]){opcode, 0x80u}, 2u,
                profiles[profile], TYPE_FALSE, 2u, 2u)) return 0;
    return preview_expect((const type_unsigned_8[]){0x66u, 0x74u, 0x80u}, 3u,
        CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 3u, 3u) &&
        preview_expect((const type_unsigned_8[]){0x67u, 0x75u, 0x7fu}, 3u,
        CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 3u, 3u) &&
        preview_expect((const type_unsigned_8[]){0x66u, 0x67u, 0x7cu, 0u}, 4u,
        CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 4u, 4u);
}
static C_INT preview_test_scalar_io_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 immediate[] = { 0xe4u, 0xe5u, 0xe6u, 0xe7u };
    static const type_unsigned_8 dx[] = { 0xecu, 0xedu, 0xeeu, 0xefu };
    type_unsigned_8 profile;
    type_unsigned_8 opcode;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile) {
        for (opcode = 0u; opcode != sizeof(immediate); ++opcode)
            if (!preview_expect((const type_unsigned_8[]){immediate[opcode], 0x5au},
                    2u, profiles[profile], TYPE_FALSE, 2u, 2u)) return 0;
        for (opcode = 0u; opcode != sizeof(dx); ++opcode)
            if (!preview_expect(&dx[opcode], 1u, profiles[profile], TYPE_FALSE,
                    1u, 1u)) return 0;
    }
    return preview_expect((const type_unsigned_8[]){0x66u, 0xe5u, 0x5au},
        3u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 3u, 3u) &&
        preview_expect((const type_unsigned_8[]){0x66u, 0xe7u, 0x5au}, 3u,
        CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 3u, 3u) &&
        preview_expect((const type_unsigned_8[]){0x66u, 0xedu}, 2u,
        CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 2u, 2u) &&
        preview_expect((const type_unsigned_8[]){0x66u, 0xefu}, 2u,
        CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 2u, 2u);
}
static C_INT preview_test_string_io_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    core_machine_cpu_instruction_lexeme lexeme;
    type_unsigned_8 profile;
    type_unsigned_8 opcode;

    for (opcode = 0x6cu; opcode != 0x70u; ++opcode)
        if (core_machine_cpu_instruction_lexeme_scan(&opcode, 1u,
                CORE_MACHINE_CPU_PROFILE_8086, TYPE_FALSE, &lexeme) ||
            lexeme.available) return 0;
    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (opcode = 0x6cu; opcode != 0x70u; ++opcode)
        if (!preview_expect(&opcode, 1u, profiles[profile], TYPE_FALSE,
                1u, 1u) || !preview_expect((const type_unsigned_8[]){0xf3u,
                opcode}, 2u, profiles[profile], TYPE_FALSE, 2u, 2u)) return 0;
    return preview_expect((const type_unsigned_8[]){0x66u, 0x6du}, 2u,
        CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 2u, 2u) &&
        preview_expect((const type_unsigned_8[]){0x67u, 0x6cu}, 2u,
        CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 2u, 2u) &&
        preview_expect((const type_unsigned_8[]){0x66u, 0x67u, 0x6fu}, 3u,
        CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 3u, 3u) &&
        preview_expect((const type_unsigned_8[]){0xf3u, 0x66u, 0x67u, 0x6du},
        4u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 4u, 4u);
}
static C_INT preview_test_push_immediate_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 push_iw[] = { 0x68u, 0x34u, 0x12u };
    static const type_unsigned_8 push_ib[] = { 0x6au, 0x80u };
    core_machine_cpu_instruction_lexeme lexeme;
    type_unsigned_8 profile;

    if (core_machine_cpu_instruction_lexeme_scan(push_iw, sizeof(push_iw),
            CORE_MACHINE_CPU_PROFILE_8086, TYPE_FALSE, &lexeme) ||
        lexeme.available || core_machine_cpu_instruction_lexeme_scan(push_ib,
            sizeof(push_ib), CORE_MACHINE_CPU_PROFILE_8086, TYPE_FALSE,
            &lexeme) || lexeme.available) return 0;
    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
        if (!preview_expect(push_iw, sizeof(push_iw), profiles[profile],
                TYPE_FALSE, 3u, 2u) || !preview_expect(push_ib,
                sizeof(push_ib), profiles[profile], TYPE_FALSE, 2u, 2u)) return 0;
    return preview_expect((const type_unsigned_8[]){0x66u, 0x68u, 0x78u,
        0x56u, 0x34u, 0x12u}, 6u, CORE_MACHINE_CPU_PROFILE_80386,
        TYPE_FALSE, 6u, 3u) && preview_expect((const type_unsigned_8[]){0x66u,
        0x6au, 0x80u}, 3u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE,
        3u, 3u) && preview_expect((const type_unsigned_8[]){0x67u, 0x68u,
        0x34u, 0x12u}, 4u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE,
        4u, 3u) && preview_expect((const type_unsigned_8[]){0x66u, 0x67u,
        0x6au, 0x80u}, 4u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE,
        4u, 4u);
}
static C_INT preview_test_gpr_push_pop_profiles(C_VOID)
{
    static const core_machine_cpu_profile p[] = {CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386}; type_unsigned_8 i;
    for (i=0u;i!=sizeof(p)/sizeof(p[0]);++i) if (!preview_expect((const type_unsigned_8[]){0x50u},1u,p[i],TYPE_FALSE,1u,1u)||!preview_expect((const type_unsigned_8[]){0x58u},1u,p[i],TYPE_FALSE,1u,1u)) return 0;
    return preview_expect((const type_unsigned_8[]){0x66u,0x50u},2u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_FALSE,2u,2u)&&preview_expect((const type_unsigned_8[]){0x67u,0x58u},2u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_FALSE,2u,2u);
}

static C_INT preview_test_pushf_popf_profiles(C_VOID)
{
    static const core_machine_cpu_profile p[] = {CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386}; type_unsigned_8 i;
    for (i=0u;i!=sizeof(p)/sizeof(p[0]);++i) if (!preview_expect((const type_unsigned_8[]){0x9cu},1u,p[i],TYPE_FALSE,1u,1u)||!preview_expect((const type_unsigned_8[]){0x9du},1u,p[i],TYPE_FALSE,1u,1u)) return 0;
    return preview_expect((const type_unsigned_8[]){0x66u,0x9cu},2u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_FALSE,2u,2u)&&preview_expect((const type_unsigned_8[]){0x67u,0x9du},2u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_FALSE,2u,2u);
}

static C_INT preview_test_hlt_profiles(C_VOID)
{
    static const core_machine_cpu_profile p[] = {CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386}; type_unsigned_8 i;
    for (i=0u;i!=sizeof(p)/sizeof(p[0]);++i) if (!preview_expect((const type_unsigned_8[]){0xf4u},1u,p[i],TYPE_FALSE,1u,1u)) return 0;
    return preview_expect((const type_unsigned_8[]){0x66u,0xf4u},2u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_FALSE,2u,2u)&&preview_expect((const type_unsigned_8[]){0x67u,0xf4u},2u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_FALSE,2u,2u);
}

static C_INT preview_test_cli_sti_profiles(C_VOID)
{
    static const core_machine_cpu_profile p[] = {CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386}; type_unsigned_8 i;
    for (i=0u;i!=sizeof(p)/sizeof(p[0]);++i) if (!preview_expect((const type_unsigned_8[]){0xfau},1u,p[i],TYPE_FALSE,1u,1u)||!preview_expect((const type_unsigned_8[]){0xfbu},1u,p[i],TYPE_FALSE,1u,1u)) return 0;
    return preview_expect((const type_unsigned_8[]){0x66u,0xfau},2u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_FALSE,2u,2u)&&preview_expect((const type_unsigned_8[]){0x67u,0xfbu},2u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_FALSE,2u,2u);
}

static C_INT preview_test_lahf_sahf_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386};
    type_unsigned_8 profile;
    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
        if (!preview_expect((const type_unsigned_8[]){0x9fu},1u,profiles[profile],TYPE_FALSE,1u,1u) || !preview_expect((const type_unsigned_8[]){0x9eu},1u,profiles[profile],TYPE_FALSE,1u,1u)) return 0;
    return preview_expect((const type_unsigned_8[]){0x66u,0x9fu},2u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_FALSE,2u,2u) && preview_expect((const type_unsigned_8[]){0x67u,0x9eu},2u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_FALSE,2u,2u);
}

static C_INT preview_test_xlat_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386};
    type_unsigned_8 profile;
    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
        if (!preview_expect((const type_unsigned_8[]){0xd7u},1u,profiles[profile],TYPE_FALSE,1u,1u)) return 0;
    return preview_expect((const type_unsigned_8[]){0x67u,0xd7u},2u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_FALSE,2u,2u) && preview_expect((const type_unsigned_8[]){0x26u,0xd7u},2u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_FALSE,2u,2u);
}

static C_INT preview_test_decimal_adjust_profiles(C_VOID)
{
    static const type_unsigned_8 simple[] = {0x27u,0x2fu,0x37u,0x3fu};
    static const core_machine_cpu_profile profiles[] = {CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386};
    type_unsigned_8 profile; type_unsigned_8 opcode;
    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile) {
        for (opcode = 0u; opcode != sizeof(simple); ++opcode)
            if (!preview_expect(&simple[opcode],1u,profiles[profile],TYPE_FALSE,1u,1u)) return 0;
        if (!preview_expect((const type_unsigned_8[]){0xd4u,10u},2u,profiles[profile],TYPE_FALSE,2u,2u) || !preview_expect((const type_unsigned_8[]){0xd5u,10u},2u,profiles[profile],TYPE_FALSE,2u,2u)) return 0;
    }
    return preview_expect((const type_unsigned_8[]){0x66u,0xd4u,10u},3u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_FALSE,3u,3u) && preview_expect((const type_unsigned_8[]){0x67u,0xd5u,10u},3u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_FALSE,3u,3u);
}

static C_INT preview_test_lar_lsl_profiles(C_VOID)
{
    static const type_unsigned_8 lar[] = {0x0fu,0x02u,0xc1u};
    static const type_unsigned_8 lsl[] = {0x0fu,0x03u,0xc1u};
    core_machine_cpu_instruction_lexeme lexeme;

    if (core_machine_cpu_instruction_lexeme_scan(lar,3u,
        CORE_MACHINE_CPU_PROFILE_80186,TYPE_FALSE,&lexeme) || lexeme.available)
        return 0;
    return preview_expect(lar,3u,CORE_MACHINE_CPU_PROFILE_80286,TYPE_FALSE,3u,3u) &&
        preview_expect(lsl,3u,CORE_MACHINE_CPU_PROFILE_80286,TYPE_FALSE,3u,3u) &&
        preview_expect(lar,3u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_FALSE,3u,3u);
}

static C_INT preview_test_pusha_popa_profiles(C_VOID)
{
    static const type_unsigned_8 pusha[] = {0x60u};
    static const type_unsigned_8 popa[] = {0x61u};
    core_machine_cpu_instruction_lexeme lexeme;

    if (core_machine_cpu_instruction_lexeme_scan(pusha,1u,CORE_MACHINE_CPU_PROFILE_8086,TYPE_FALSE,&lexeme)||lexeme.available||core_machine_cpu_instruction_lexeme_scan(popa,1u,CORE_MACHINE_CPU_PROFILE_8086,TYPE_FALSE,&lexeme)||lexeme.available) return 0;
    return preview_expect(pusha,1u,CORE_MACHINE_CPU_PROFILE_80186,TYPE_FALSE,1u,1u)&&preview_expect(popa,1u,CORE_MACHINE_CPU_PROFILE_80186,TYPE_FALSE,1u,1u)&&preview_expect(pusha,1u,CORE_MACHINE_CPU_PROFILE_80286,TYPE_FALSE,1u,1u)&&preview_expect(popa,1u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_FALSE,1u,1u)&&preview_expect((const type_unsigned_8[]){0x66u,0x60u},2u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_FALSE,2u,2u)&&preview_expect((const type_unsigned_8[]){0x66u,0x61u},2u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_FALSE,2u,2u);
}

static C_INT preview_test_imul_immediate_profiles(C_VOID)
{
    static const type_unsigned_8 iw[] = {0x69u,0xc1u,0xfeu,0xffu};
    static const type_unsigned_8 ib[] = {0x6bu,0xc1u,0xfeu};
    core_machine_cpu_instruction_lexeme lexeme;

    if (core_machine_cpu_instruction_lexeme_scan(iw,sizeof(iw),CORE_MACHINE_CPU_PROFILE_8086,TYPE_FALSE,&lexeme)||lexeme.available||core_machine_cpu_instruction_lexeme_scan(ib,sizeof(ib),CORE_MACHINE_CPU_PROFILE_8086,TYPE_FALSE,&lexeme)||lexeme.available) return 0;
    return preview_expect(iw,sizeof(iw),CORE_MACHINE_CPU_PROFILE_80186,TYPE_FALSE,4u,3u)&&preview_expect(ib,sizeof(ib),CORE_MACHINE_CPU_PROFILE_80186,TYPE_FALSE,3u,3u)&&preview_expect(iw,sizeof(iw),CORE_MACHINE_CPU_PROFILE_80286,TYPE_FALSE,4u,3u)&&preview_expect(ib,sizeof(ib),CORE_MACHINE_CPU_PROFILE_80386,TYPE_FALSE,3u,3u)&&preview_expect((const type_unsigned_8[]){0x66u,0x69u,0xc1u,0xfeu,0xffu,0xffu,0xffu},7u,CORE_MACHINE_CPU_PROFILE_80386,TYPE_FALSE,7u,4u);
}

static C_INT preview_test_bound_profiles(C_VOID)
{
    static const type_unsigned_8 bound[] = {0x62u, 0x06u, 0u, 0x20u};
    core_machine_cpu_instruction_lexeme lexeme;

    if (core_machine_cpu_instruction_lexeme_scan(bound, sizeof(bound),
        CORE_MACHINE_CPU_PROFILE_8086, TYPE_FALSE, &lexeme) || lexeme.available)
        return 0;
    return preview_expect(bound, sizeof(bound), CORE_MACHINE_CPU_PROFILE_80186,
        TYPE_FALSE, 4u, 3u) && preview_expect(bound, sizeof(bound),
        CORE_MACHINE_CPU_PROFILE_80286, TYPE_FALSE, 4u, 3u) &&
        preview_expect(bound, sizeof(bound), CORE_MACHINE_CPU_PROFILE_80386,
        TYPE_FALSE, 4u, 3u) && preview_expect((const type_unsigned_8[]){0x66u,0x62u,0x06u,0u,0x20u},5u,
        CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 5u, 4u);
}

static C_INT preview_test_arpl_profiles(C_VOID)
{
    static const type_unsigned_8 arpl[] = {0x63u, 0xc0u};
    core_machine_cpu_instruction_lexeme lexeme;

    if (core_machine_cpu_instruction_lexeme_scan(arpl, sizeof(arpl),
        CORE_MACHINE_CPU_PROFILE_8086, TYPE_FALSE, &lexeme) || lexeme.available ||
        core_machine_cpu_instruction_lexeme_scan(arpl, sizeof(arpl),
        CORE_MACHINE_CPU_PROFILE_80186, TYPE_FALSE, &lexeme) || lexeme.available)
        return 0;
    return preview_expect(arpl, sizeof(arpl), CORE_MACHINE_CPU_PROFILE_80286,
        TYPE_FALSE, 2u, 2u) && preview_expect(arpl, sizeof(arpl),
        CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 2u, 2u);
}

static C_INT preview_test_iret_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 iret[] = {0xcfu};
    type_unsigned_8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
        if (!preview_expect(iret, sizeof(iret), profiles[profile],
            TYPE_FALSE, 1u, 1u)) return 0;
    return preview_expect((const type_unsigned_8[]){0x66u, 0xcfu}, 2u,
        CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 2u, 2u) &&
        preview_expect((const type_unsigned_8[]){0x66u, 0x67u, 0xcfu}, 3u,
        CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 3u, 3u);
}

static C_INT preview_test_int3_into_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 int3[] = {0xccu};
    static const type_unsigned_8 into[] = {0xceu};
    type_unsigned_8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
        if (!preview_expect(int3, sizeof(int3), profiles[profile],
            TYPE_FALSE, 1u, 1u) || !preview_expect(into, sizeof(into),
            profiles[profile], TYPE_FALSE, 1u, 1u)) return 0;
    return preview_expect((const type_unsigned_8[]){0x66u, 0xccu}, 2u,
        CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 2u, 2u) &&
        preview_expect((const type_unsigned_8[]){0x66u, 0x67u, 0xceu}, 3u,
        CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 3u, 3u);
}

static C_INT preview_test_int_immediate_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 interrupt[] = {0xcdu, 0x31u};
    type_unsigned_8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
        if (!preview_expect(interrupt, sizeof(interrupt), profiles[profile],
            TYPE_FALSE, 2u, 2u)) return 0;
    return preview_expect((const type_unsigned_8[]){0x66u, 0xcdu, 0x31u},
        3u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 3u, 3u) &&
        preview_expect((const type_unsigned_8[]){0x67u, 0xcdu, 0x31u},
        3u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 3u, 3u) &&
        preview_expect((const type_unsigned_8[]){0x66u, 0x67u, 0xcdu, 0x31u},
        4u, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, 4u, 4u);
}
static C_INT preview_test_group3_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    type_unsigned_8 profile_index;
    type_unsigned_8 extension;

    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (extension = 0u; extension != 8u; ++extension) {
        type_unsigned_8 f6[] = { 0xf6u,
            (type_unsigned_8)((extension << 3u) | 0xc0u), 0xa5u };
        type_unsigned_8 f7[] = { 0xf7u,
            (type_unsigned_8)((extension << 3u) | 0xc0u),
            0x78u, 0x56u, 0x34u, 0x12u };
        core_machine_cpu_instruction_lexeme lexeme;
        const type_unsigned_8 operand_bytes = profiles[profile_index] ==
            CORE_MACHINE_CPU_PROFILE_80386 ? 4u : 2u;
        const type_bool code_32 = profiles[profile_index] ==
            CORE_MACHINE_CPU_PROFILE_80386 ? TYPE_TRUE : TYPE_FALSE;
        const type_bool invalid = extension == 1u;

        if (invalid) {
            if (core_machine_cpu_instruction_lexeme_scan(f6, sizeof(f6),
                profiles[profile_index], code_32, &lexeme) || lexeme.available ||
                core_machine_cpu_instruction_lexeme_scan(f7, sizeof(f7),
                profiles[profile_index], code_32, &lexeme) || lexeme.available)
                return 0;
        } else if (!preview_expect(f6, sizeof(f6), profiles[profile_index],
            code_32, extension == 0u ? 3u : 2u,
            extension == 0u ? 3u : 2u) || !preview_expect(f7, sizeof(f7),
            profiles[profile_index], code_32, extension == 0u ?
            (type_unsigned_8)(2u + operand_bytes) : 2u,
            extension == 0u ? 3u : 2u)) return 0;
    }
    return 1;
}
static C_INT preview_test_group45_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    type_unsigned_8 profile_index;
    type_unsigned_8 extension;

    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (extension = 0u; extension != 8u; ++extension) {
        type_unsigned_8 fe[] = { 0xfeu, (type_unsigned_8)((extension << 3u) | 0xc0u) };
        type_unsigned_8 ff_register[] = { 0xffu,
            (type_unsigned_8)((extension << 3u) | 0xc0u) };
        type_unsigned_8 ff_memory[] = { 0xffu,
            (type_unsigned_8)((extension << 3u) | 0x06u), 0u, 0x40u };
        core_machine_cpu_instruction_lexeme lexeme;
        const type_bool fe_valid = extension <= 1u;
        const type_bool ff_register_valid = extension <= 2u || extension == 4u ||
            extension == 6u;
        const type_bool ff_memory_valid = extension <= 6u;

        if (fe_valid != (core_machine_cpu_instruction_lexeme_scan(fe, sizeof(fe),
            profiles[profile_index], TYPE_FALSE, &lexeme) && lexeme.available) ||
            ff_register_valid != (core_machine_cpu_instruction_lexeme_scan(ff_register,
            sizeof(ff_register), profiles[profile_index], TYPE_FALSE, &lexeme) &&
            lexeme.available) || ff_memory_valid !=
            (core_machine_cpu_instruction_lexeme_scan(ff_memory, sizeof(ff_memory),
            profiles[profile_index], TYPE_FALSE, &lexeme) && lexeme.available)) return 0;
        if (fe_valid && !preview_expect(fe, sizeof(fe), profiles[profile_index],
            TYPE_FALSE, 2u, 2u)) return 0;
        if (ff_register_valid && !preview_expect(ff_register, sizeof(ff_register),
            profiles[profile_index], TYPE_FALSE, 2u, 2u)) return 0;
        if (ff_memory_valid && !preview_expect(ff_memory, sizeof(ff_memory),
            profiles[profile_index], TYPE_FALSE, 4u, 3u)) return 0;
    }
    return 1;
}
static C_INT preview_test_unavailable(C_VOID)
{
    static const type_unsigned_8 truncated[] = { 0x8bu };
    static const type_unsigned_8 legacy_operand_prefix[] = { 0x66u, 0x90u };
    static const type_unsigned_8 invalid_opcode[] = { 0xd6u };
    static const type_unsigned_8 locked_nop[] = { 0xf0u, 0x90u };
    core_machine_cpu_instruction_lexeme lexeme;

    return !core_machine_cpu_instruction_lexeme_scan(truncated,
            sizeof(truncated), CORE_MACHINE_CPU_PROFILE_80386, TYPE_TRUE,
            &lexeme) && !lexeme.available &&
        !core_machine_cpu_instruction_lexeme_scan(legacy_operand_prefix,
            sizeof(legacy_operand_prefix), CORE_MACHINE_CPU_PROFILE_80286,
            TYPE_FALSE, &lexeme) && !lexeme.available &&
        !core_machine_cpu_instruction_lexeme_scan(invalid_opcode,
            sizeof(invalid_opcode), CORE_MACHINE_CPU_PROFILE_80386,
            TYPE_TRUE, &lexeme) && !lexeme.available &&
        !core_machine_cpu_instruction_lexeme_scan(locked_nop,
            sizeof(locked_nop), CORE_MACHINE_CPU_PROFILE_80386,
            TYPE_TRUE, &lexeme) && !lexeme.available;
}

static C_INT preview_test_cpu_fetch_nonpublication(C_VOID)
{
    static const type_unsigned_8 program[] = { 0x0fu, 0x84u, 0x78u, 0x56u };
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386
    };
    core_machine_cpu_instruction_lexeme lexeme;
    core_machine_observation before = { 0 };
    core_machine_observation after = { 0 };
    core_machine *machine = STD_NULL;
    type_unsigned_64 committed = 0u;
    type_unsigned_64 cancelled = 0u;
    STD_SIZE_T trace_count = 0u;
    C_INT failed = core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
        core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, PREVIEW_RESET_PHYSICAL, program,
            sizeof(program)) != TYPE_STATUS_OK ||
        core_machine_capture_observation(machine, &before) != TYPE_STATUS_OK;

    if (!failed) {
        committed = machine->transaction.committed_count;
        cancelled = machine->transaction.cancelled_count;
        trace_count = machine->trace.count;
        failed |= !core_machine_cpu_execution_preview_lexeme(
            &machine->executor_cpu_execution, &lexeme) || !lexeme.available ||
            lexeme.byte_count != sizeof(program) || lexeme.component_count != 3u ||
            core_machine_capture_observation(machine, &after) != TYPE_STATUS_OK ||
            STD_MEMCMP(&before, &after, sizeof(before)) != 0 ||
            machine->transaction.committed_count != committed ||
            machine->transaction.cancelled_count != cancelled ||
            machine->trace.count != trace_count;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT preview_test_limited_fetch_nonpublication(C_VOID)
{
    static const type_unsigned_8 program[] = { 0x0fu, 0x84u, 0x78u, 0x56u };
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386
    };
    core_machine_cpu_instruction_lexeme lexeme;
    core_machine_observation before = { 0 };
    core_machine_observation after = { 0 };
    core_machine *machine = STD_NULL;
    type_unsigned_64 committed = 0u;
    type_unsigned_64 cancelled = 0u;
    STD_SIZE_T trace_count = 0u;
    C_INT failed = core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
        core_machine_reset(machine) != TYPE_STATUS_OK ||
        !test_core_machine_fixture_prepare_real_mode_execution(machine, 0u) ||
        ((machine->executor_cpu.data.cs.base = 0x00fffffcu),
            core_machine_memory_write(machine, 0x00fffffcu, program,
                sizeof(program)) != TYPE_STATUS_OK) ||
        core_machine_capture_observation(machine, &before) != TYPE_STATUS_OK;

    if (!failed) {
        committed = machine->transaction.committed_count;
        cancelled = machine->transaction.cancelled_count;
        trace_count = machine->trace.count;
        failed |= core_machine_cpu_execution_preview_lexeme(
            &machine->executor_cpu_execution, &lexeme) || lexeme.available ||
            core_machine_capture_observation(machine, &after) != TYPE_STATUS_OK ||
            STD_MEMCMP(&before, &after, sizeof(before)) != 0 ||
            machine->transaction.committed_count != committed ||
            machine->transaction.cancelled_count != cancelled ||
            machine->trace.count != trace_count;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT preview_test_taken_jcc_target(C_VOID)
{
    static const type_unsigned_8 program[] = { 0x75u, 0xfeu };
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386
    };
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_cpu_instruction_lexeme lexeme;
    core_machine_observation before = { 0 };
    core_machine_observation after = { 0 };
    core_machine_run_result result;
    core_machine *machine = STD_NULL;
    C_INT failed = core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
        core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, PREVIEW_RESET_PHYSICAL, program,
            sizeof(program)) != TYPE_STATUS_OK;

    if (!failed) {
        machine->executor_cpu.data.eflags &= ~VCPU_EFLAGS_ZF;
        if (core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
            machine->executor_cpu.data.eip != 0xfff0u) {
            failed = 1;
        } else if (core_machine_capture_observation(machine, &before) !=
            TYPE_STATUS_OK || !core_machine_cpu_execution_preview_lexeme(
                &machine->executor_cpu_execution, &lexeme) || !lexeme.available ||
            lexeme.byte_count != 2u || lexeme.component_count != 2u ||
            core_machine_capture_observation(machine, &after) != TYPE_STATUS_OK ||
            STD_MEMCMP(&before, &after, sizeof(before)) != 0) {
            failed = 1;
        }
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT preview_test_taken_near_jcc_target(C_VOID)
{
    static const type_unsigned_8 program[] = {
        0x66u, 0x0fu, 0x84u, 0x02u, 0x00u, 0x00u, 0x00u,
        0x90u, 0x90u, 0xf4u
    };
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386
    };
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_cpu_instruction_lexeme lexeme;
    core_machine_observation before = { 0 };
    core_machine_observation after = { 0 };
    core_machine_run_result result;
    core_machine *machine = STD_NULL;
    C_INT failed = core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
        core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, PREVIEW_RESET_PHYSICAL, program,
            sizeof(program)) != TYPE_STATUS_OK;

    if (!failed) {
        machine->executor_cpu.data.eflags |= VCPU_EFLAGS_ZF;
        if (core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
            machine->executor_cpu.data.eip != 0xfff9u) {
            failed = 1;
        } else if (core_machine_capture_observation(machine, &before) !=
            TYPE_STATUS_OK || !core_machine_cpu_execution_preview_lexeme(
                &machine->executor_cpu_execution, &lexeme) || !lexeme.available ||
            lexeme.byte_count != 1u || lexeme.component_count != 1u ||
            core_machine_capture_observation(machine, &after) != TYPE_STATUS_OK ||
            STD_MEMCMP(&before, &after, sizeof(before)) != 0) {
            failed = 1;
        }
    }
    core_machine_destroy(machine);
    return failed;
}
static C_INT preview_test_cr_mov_mod_quirk(C_VOID)
{
    static const type_unsigned_8 programs[][7] = {
        {0x0fu,0x20u,0x05u,0x78u,0x56u,0x34u,0x12u},
        {0x0fu,0x22u,0x05u,0x78u,0x56u,0x34u,0x12u}
    };
    const core_machine_config config = {.cpu_profile=CORE_MACHINE_CPU_PROFILE_80386,
        .cpu_80386_cr_mov_ignores_mod=TYPE_TRUE};
    core_machine_cpu_instruction_lexeme lexeme;
    core_machine *machine=STD_NULL; type_unsigned_8 index;
    C_INT failed=core_machine_create(&config,&machine)!=TYPE_STATUS_OK||
        core_machine_freeze_execution_providers(machine)!=TYPE_STATUS_OK||
        core_machine_reset(machine)!=TYPE_STATUS_OK;
    for(index=0u;!failed&&index<2u;++index) {
        failed|=core_machine_cpu_instruction_lexeme_scan(programs[index],7u,
            CORE_MACHINE_CPU_PROFILE_80386,TYPE_TRUE,&lexeme)||
            core_machine_memory_write(machine,PREVIEW_RESET_PHYSICAL,programs[index],7u)!=TYPE_STATUS_OK||
            !core_machine_cpu_execution_preview_lexeme(&machine->executor_cpu_execution,&lexeme)||
            !lexeme.available||lexeme.byte_count!=3u||lexeme.component_count!=3u;
    }
    core_machine_destroy(machine); return failed;
}
static C_INT preview_test_default_reset_alias(C_VOID)
{
    static const type_unsigned_8 halt[] = { 0xf4u };
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386
    };
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine *machine = STD_NULL;
    C_INT failed = core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
        core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, 0x000ffff0u, halt,
            sizeof(halt)) != TYPE_STATUS_OK ||
        core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        result.executed != 1u ||
        machine->executor_cpu.data.eip != 0xfff1u;

    core_machine_destroy(machine);
    return failed;
}

C_INT main(C_VOID)
{
    if (!preview_test_layouts()) return 2;
    if (!preview_test_unavailable()) return 3;
    if (!preview_test_accumulator_xchg_profiles()) return 13;
    if (!preview_test_primary_inc_dec_profiles()) return 12;
    if (!preview_test_immediate_register_mov_profiles()) return 14;
    if (!preview_test_moffs_mov_profiles()) return 15;
    if (!preview_test_movs_profiles()) return 16;
    if (!preview_test_cmps_profiles()) return 17;
    if (!preview_test_stos_profiles()) return 18;
    if (!preview_test_lods_profiles()) return 19;
    if (!preview_test_scas_profiles()) return 20;
    if (!preview_test_accumulator_test_profiles()) return 21;
    if (!preview_test_group2_immediate_profiles()) return 22;
    if (!preview_test_near_return_profiles()) return 23;
    if (!preview_test_far_return_profiles()) return 24;
    if (!preview_test_enter_leave_profiles()) return 25;
    if (!preview_test_int_immediate_profiles()) return 26;
    if (!preview_test_int3_into_profiles()) return 27;
    if (!preview_test_iret_profiles()) return 28;
    if (!preview_test_arpl_profiles()) return 29;
    if (!preview_test_bound_profiles()) return 30;
    if (!preview_test_imul_immediate_profiles()) return 31;
    if (!preview_test_pusha_popa_profiles()) return 32;
    if (!preview_test_lar_lsl_profiles()) return 33;
    if (!preview_test_decimal_adjust_profiles()) return 35;
    if (!preview_test_xlat_profiles()) return 36;
    if (!preview_test_lahf_sahf_profiles()) return 37;
    if (!preview_test_cli_sti_profiles()) return 38;
    if (!preview_test_hlt_profiles()) return 39;
    if (!preview_test_pushf_popf_profiles()) return 40;
    if (!preview_test_gpr_push_pop_profiles()) return 41;
    if (!preview_test_sreg_push_pop_profiles()) return 42;
    if (!preview_test_direct_flags_profiles()) return 43;
    if (!preview_test_loop_jcxz_profiles()) return 44;
    if (!preview_test_lea_profiles()) return 45;
    if (!preview_test_modrm_data_move_profiles()) return 46;
    if (!preview_test_push_immediate_profiles()) return 47;
    if (!preview_test_string_io_profiles()) return 48;
    if (!preview_test_scalar_io_profiles()) return 49;
    if (!preview_test_short_jcc_profiles()) return 50;
    if (!preview_test_direct_near_control_profiles()) return 51;
    if (!preview_test_direct_far_control_profiles()) return 52;
    if (!preview_test_primary_alu_profiles()) return 53;
    if (!preview_test_shared_prefix_profiles()) return 54;
    if (!preview_test_rm_immediate_mov_profiles()) return 55;
    if (!preview_test_near_jcc_profiles()) return 56;
    if (!preview_test_setcc_profiles()) return 57;
    if (!preview_test_bit_test_profiles()) return 58;
    if (!preview_test_double_shift_profiles()) return 59;
    if (!preview_test_bit_scan_profiles()) return 60;
    if (!preview_test_movx_profiles()) return 61;
    if (!preview_test_imul2_profiles()) return 62;
    if (!preview_test_debug_mov_profiles()) return 63;
    if (!preview_test_test_mov_profiles()) return 65;
    if (!preview_test_system_group_profiles()) return 66;
    if (!preview_test_system_selector_group_profiles()) return 67;
    if (!preview_test_lss_lfs_lgs_profiles()) return 64;
    if (!preview_test_les_lds_profiles()) return 34;
    if (!preview_test_group3_profiles()) return 10;
    if (!preview_test_group45_profiles()) return 11;
    if (preview_test_cpu_fetch_nonpublication()) return 4;
    if (preview_test_limited_fetch_nonpublication()) return 5;
    if (preview_test_taken_jcc_target()) return 6;
    if (preview_test_taken_near_jcc_target()) return 7;
    if (preview_test_default_reset_alias()) return 8;
    if (preview_test_cr_mov_mod_quirk()) return 9;
    STD_PRINTF("M5:T357:S2:CPU-TIMING-PREVIEW:OK\n");
    STD_PRINTF("M5:T401:S9:GROUP3-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S10:GROUP45-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S11:PRIMARY-INC-DEC-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S12:ACCUMULATOR-XCHG-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S13:IMMEDIATE-REGISTER-MOV-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S14:MOFFS-MOV-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S15:MOVS-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S16:CMPS-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S17:STOS-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S18:LODS-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S19:SCAS-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S20:ACCUMULATOR-TEST-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S21:GROUP2-IMMEDIATE-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S22:NEAR-RETURN-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S23:FAR-RETURN-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S24:ENTER-LEAVE-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S25:INT-IMMEDIATE-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S26:INT3-INTO-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S27:IRET-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S28:ARPL-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S29:BOUND-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S30:IMUL-IMMEDIATE-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S31:PUSHA-POPA-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S32:LAR-LSL-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S33:LES-LDS-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S34:DECIMAL-ADJUST-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S35:XLAT-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S36:LAHF-SAHF-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S37:CLI-STI-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S38:HLT-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S39:PUSHF-POPF-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S40:GPR-PUSH-POP-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S41:SREG-PUSH-POP-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S42:DIRECT-FLAGS-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S43:LOOP-JCXZ-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S44:LEA-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S45:SIGN-EXTEND-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S46:XCHG-MODRM-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S47:GPR-MOV-MODRM-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S48:SREG-MOV-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S49:MODRM-DATA-MOVE-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S50:PUSH-IMMEDIATE-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S51:STRING-IO-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S52:SCALAR-IO-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S53:SHORT-JCC-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S54:DIRECT-NEAR-CONTROL-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S55:DIRECT-FAR-CONTROL-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S56:PRIMARY-ALU-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S57:SHARED-PREFIX-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S58:RM-IMMEDIATE-MOV-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S59:NEAR-JCC-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S60:SETCC-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S61:BIT-TEST-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S62:DOUBLE-SHIFT-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S63:BIT-SCAN-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S64:MOVX-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S65:IMUL2-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S66:DEBUG-MOV-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S67:CONTROL-MOV-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S68:LSS-LFS-LGS-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S69:TEST-MOV-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S70:SYSTEM-GROUP-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S71:SYSTEM-SELECTOR-GROUP-PREVIEW-PROFILES:OK\n");
    STD_PRINTF("M5:T442:S1:CPU-LEXEME-PROFILE-BOUNDARY:OK\n");
    return 0;
}
