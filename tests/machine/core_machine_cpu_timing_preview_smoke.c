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
    static const type_unsigned_8 program[] = { 0x0fu, 0x20u, 0x05u,
        0x78u, 0x56u, 0x34u, 0x12u };
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .cpu_80386_cr_mov_ignores_mod = TYPE_TRUE
    };
    core_machine_cpu_instruction_lexeme lexeme;
    core_machine *machine = STD_NULL;
    C_INT failed = core_machine_cpu_instruction_lexeme_scan(program,
        sizeof(program), CORE_MACHINE_CPU_PROFILE_80386, TYPE_TRUE, &lexeme) ||
        core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
        core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, PREVIEW_RESET_PHYSICAL, program,
            sizeof(program)) != TYPE_STATUS_OK;

    if (!failed) failed |= !core_machine_cpu_execution_preview_lexeme(
        &machine->executor_cpu_execution, &lexeme) || !lexeme.available ||
        lexeme.byte_count != 3u || lexeme.component_count != 3u ||
        core_machine_cpu_instruction_lexeme_scan((const type_unsigned_8[]){
            0x0fu, 0x21u, 0x05u, 0x78u, 0x56u, 0x34u, 0x12u }, 7u,
            CORE_MACHINE_CPU_PROFILE_80386, TYPE_TRUE, &lexeme);
    core_machine_destroy(machine);
    return failed;
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
    return 0;
}
