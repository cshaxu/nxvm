#include "support/cpu_descriptor_query_fixture.h"
#include "lib/types/file.h"

#define VERR_VERW_CPU_VERR_MODRM 0xe0u
#define VERR_VERW_CPU_VERW_MODRM 0xe8u

static void verr_verw_prepare(cpu_instruction_fixture *fixture,
    x86_core_cpu_profile profile)
{
    cpu_instruction_prepare(fixture, profile);
    cpu_descriptor_query_enter_protected(fixture, 0u);
    cpu_descriptor_query_install_gdt(fixture);
    /* Selector 20h names execute-only code in the original S58 matrix. */
    fixture->memory[CPU_DESCRIPTOR_QUERY_GDT_ADDRESS + 0x20u + 5u] = 0x98u;
}

static lib_i32 verr_verw_run_case(x86_core_cpu_profile profile,
    lib_u8 modrm, lib_u16 selector, lib_u8 expected_zf)
{
    const lib_u8 code[] = {0x0fu, 0x00u, modrm, 0xf4u};
    cpu_instruction_fixture fixture;
    t_cpu before;
    t_cpu after;

    verr_verw_prepare(&fixture, profile);
    fixture.cpu.data.eax = 0xa1a10000u | selector;
    fixture.cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_SF | VCPU_EFLAGS_OF | VCPU_EFLAGS_IF;
    before = fixture.cpu;
    if (!cpu_descriptor_query_run(&fixture, code, sizeof(code), &after)) return 0;
    return after.data.eax == before.data.eax && after.data.ecx == before.data.ecx &&
        !!CPU_DESCRIPTOR_QUERY_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_ZF) ==
            expected_zf && (after.data.eflags & ~VCPU_EFLAGS_ZF) ==
            (before.data.eflags & ~VCPU_EFLAGS_ZF);
}

static lib_i32 verr_verw_test_outcomes(void)
{
    static const lib_u16 selectors[] = {
        0x0010u, 0x0008u, 0x0020u, 0x0018u, 0x0000u, 0x0013u
    };
    static const lib_u8 expected_verr[] = {1u, 1u, 0u, 1u, 0u, 0u};
    static const lib_u8 expected_verw[] = {1u, 0u, 0u, 1u, 0u, 0u};
    const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_80286, X86_CORE_CPU_PROFILE_80386,
        X86_CORE_CPU_PROFILE_80486SX
    };
    lib_size profile;
    lib_size selector;

    for (profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        for (selector = 0u; selector < sizeof(selectors) / sizeof(selectors[0]);
            ++selector) {
            if (!verr_verw_run_case(profiles[profile], VERR_VERW_CPU_VERR_MODRM,
                    selectors[selector], expected_verr[selector]) ||
                !verr_verw_run_case(profiles[profile], VERR_VERW_CPU_VERW_MODRM,
                    selectors[selector], expected_verw[selector])) return 0;
        }
    }
    return 1;
}

static lib_i32 verr_verw_test_memory_and_prefix_forms(x86_core_cpu_profile profile)
{
    static const lib_u8 codes[][6] = {
        {0x0fu,0x00u,0x26u,0x00u,0x01u,0xf4u},
        {0x0fu,0x00u,0x2eu,0x00u,0x01u,0xf4u},
        {0x66u,0x0fu,0x00u,VERR_VERW_CPU_VERR_MODRM,0xf4u,0u},
        {0x67u,0x0fu,0x00u,VERR_VERW_CPU_VERW_MODRM,0xf4u,0u}
    };
    static const lib_u8 sizes[] = {6u, 6u, 5u, 5u};
    const lib_u16 selector = 0x0010u;
    lib_size index;

    for (index = 0u; index < sizeof(codes) / sizeof(codes[0]); ++index) {
        cpu_instruction_fixture fixture;
        t_cpu after;

        verr_verw_prepare(&fixture, profile);
        fixture.cpu.data.eax = 0xa1a10010u;
        lib_memory_copy(fixture.memory + 0x0100u, &selector, sizeof(selector));
        if (!cpu_descriptor_query_run(&fixture, codes[index], sizes[index], &after) ||
            !CPU_DESCRIPTOR_QUERY_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_ZF)) return 0;
    }
    return 1;
}

static lib_i32 verr_verw_test_protected_memory_forms(x86_core_cpu_profile profile)
{
    static const lib_u8 verr_code[] = {
        0x0fu,0x00u,0x26u,0x10u,0x00u,0xf4u
    };
    static const lib_u8 verw_code[] = {
        0x0fu,0x00u,0x2eu,0x10u,0x00u,0xf4u
    };
    const lib_u16 selector = 0x0010u;
    const lib_u8 *codes[] = {verr_code, verw_code};
    lib_size index;

    for (index = 0u; index < sizeof(codes) / sizeof(codes[0]); ++index) {
        cpu_instruction_fixture fixture;

        if (!cpu_descriptor_query_boot_protected(&fixture,
                profile)) return 0;
        fixture.cpu.data.eip = 0u;
        fixture.cpu.data.flagHalt = LIB_FALSE;
        fixture.cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_IF;
        lib_memory_copy(fixture.memory + 0x3010u, &selector, sizeof(selector));
        lib_memory_copy(fixture.memory + CPU_DESCRIPTOR_QUERY_CODE_ADDRESS,
            codes[index], 6u);
        if (!cpu_descriptor_query_execute(&fixture, 16u) ||
            !fixture.cpu.data.flagHalt ||
            !CPU_DESCRIPTOR_QUERY_BIT_IS_SET(fixture.cpu.data.eflags,
                VCPU_EFLAGS_ZF)) return 0;
    }
    return 1;
}

static lib_i32 verr_verw_test_rejection(x86_core_cpu_profile profile)
{
    static const lib_u8 verr[] = {0x0fu, 0x00u, VERR_VERW_CPU_VERR_MODRM};
    static const lib_u8 lock_verw[] = {
        0xf0u, 0x0fu, 0x00u, VERR_VERW_CPU_VERW_MODRM
    };
    cpu_instruction_fixture fixture;

    cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80186);
    if (!cpu_descriptor_query_fault(&fixture, verr, sizeof(verr),
            VCPUINS_EXCEPT_UD)) return 0;
    cpu_instruction_prepare(&fixture, profile);
    if (!cpu_descriptor_query_fault(&fixture, lock_verw, sizeof(lock_verw),
            VCPUINS_EXCEPT_UD)) return 0;
    cpu_instruction_prepare(&fixture, profile);
    fixture.cpu.data.cr0 |= VCPU_CR0_PE;
    fixture.cpu.data.eflags |= VCPU_EFLAGS_VM;
    return cpu_descriptor_query_fault(&fixture, verr, sizeof(verr),
        VCPUINS_EXCEPT_UD);
}

static lib_i32 verr_verw_test_source_limit(x86_core_cpu_profile profile)
{
    static const lib_u8 codes[][5] = {
        {0x0fu,0x00u,0x26u,0x10u,0x00u},
        {0x0fu,0x00u,0x2eu,0x10u,0x00u}
    };
    const lib_u16 selector = 0x0010u;
    lib_size index;

    for (index = 0u; index < sizeof(codes) / sizeof(codes[0]); ++index) {
        cpu_instruction_fixture fixture;
        t_cpu before;

        verr_verw_prepare(&fixture, profile);
        fixture.cpu.data.ds.limit = 0x000fu;
        fixture.cpu.data.eax = 0xa1a10010u;
        fixture.cpu.data.eflags = VCPU_EFLAGS_CF;
        before = fixture.cpu;
        lib_memory_copy(fixture.memory + 0x0010u, &selector, sizeof(selector));
        if (!cpu_descriptor_query_fault(&fixture, codes[index],
                sizeof(codes[index]), VCPUINS_EXCEPT_DF) ||
            fixture.cpu.data.eax != before.data.eax ||
            fixture.cpu.data.eip != before.data.eip ||
            fixture.cpu.data.eflags != before.data.eflags) return 0;
    }
    return 1;
}

static lib_i32 verr_verw_test_ldt_selector(x86_core_cpu_profile profile)
{
    static const lib_u8 ldt_descriptor[] = {
        0xffu,0xffu,0x00u,0x70u,0x00u,0x92u,0x00u,0x00u
    };
    static const lib_u8 ldt_table_descriptor[] = {
        0x0fu,0x00u,0x00u,0x05u,0x00u,0x82u,0x00u,0x00u
    };
    static const lib_u8 codes[][13] = {
        {0xb8u,0x30u,0x00u,0x0fu,0x00u,0xd0u,
         0xb9u,0x0cu,0x00u,0x0fu,0x00u,0xe1u,0xf4u},
        {0xb8u,0x30u,0x00u,0x0fu,0x00u,0xd0u,
         0xb9u,0x0cu,0x00u,0x0fu,0x00u,0xe9u,0xf4u}
    };
    lib_size index;

    for (index = 0u; index < sizeof(codes) / sizeof(codes[0]); ++index) {
        cpu_instruction_fixture fixture;

        if (!cpu_descriptor_query_boot_protected(&fixture,
                profile)) return 0;
        fixture.cpu.data.gdtr.limit = 0x0037u;
        fixture.cpu.data.eip = 0u;
        fixture.cpu.data.flagHalt = LIB_FALSE;
        lib_memory_copy(fixture.memory + CPU_DESCRIPTOR_QUERY_GDT_ADDRESS +
            0x30u, ldt_table_descriptor, sizeof(ldt_table_descriptor));
        lib_memory_copy(fixture.memory + 0x0508u, ldt_descriptor,
            sizeof(ldt_descriptor));
        lib_memory_copy(fixture.memory + CPU_DESCRIPTOR_QUERY_CODE_ADDRESS,
            codes[index], sizeof(codes[index]));
        if (!cpu_descriptor_query_execute(&fixture, 32u) ||
            !fixture.cpu.data.flagHalt || !fixture.cpu.data.ldtr.flagValid ||
            fixture.cpu.data.ldtr.selector != 0x0030u ||
            !CPU_DESCRIPTOR_QUERY_BIT_IS_SET(fixture.cpu.data.eflags,
                VCPU_EFLAGS_ZF)) return 0;
    }
    return 1;
}

static lib_i32 verr_verw_test_type_presence_privilege(x86_core_cpu_profile profile)
{
    static const lib_u8 code[][7] = {
        {0x0fu,0x00u,VERR_VERW_CPU_VERR_MODRM,0xf4u},
        {0x0fu,0x00u,VERR_VERW_CPU_VERW_MODRM,0xf4u},
        {0x66u,0x0fu,0x00u,0x26u,0x00u,0x01u,0xf4u},
        {0x66u,0x0fu,0x00u,0x2eu,0x00u,0x01u,0xf4u}
    };
    static const lib_u8 bytes[] = {4u,4u,7u,7u};

    for (lib_u32 index = 0u; index < 16384u; ++index) {
        const lib_u8 type = (lib_u8)(index & 31u);
        const lib_u8 present = (lib_u8)((index >> 5u) & 1u);
        const lib_u8 dpl = (lib_u8)((index >> 6u) & 3u);
        const lib_u8 cpl = (lib_u8)((index >> 8u) & 3u);
        const lib_u8 rpl = (lib_u8)((index >> 10u) & 3u);
        const lib_u8 form = (lib_u8)(index >> 12u);
        const lib_bool data = type >= 16u && type < 24u;
        const lib_bool readable = data || (type >= 24u && (type & 2u) != 0u);
        const lib_bool writable = data && (type & 2u) != 0u;
        const lib_bool conforming = (type & 0x1cu) == 0x1cu;
        const lib_bool expected_zf = ((form & 1u) != 0u ? writable : readable) &&
            (conforming || (cpl <= dpl && rpl <= dpl));
        const lib_u16 selector = (lib_u16)(0x18u | rpl);
        const lib_u8 descriptor[] = {0xffu,0xffu,0u,0u,0u,
            (lib_u8)((present << 7u) | (dpl << 5u) | type),0u,0u};
        const lib_u32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
            VCPU_EFLAGS_AF | VCPU_EFLAGS_SF | VCPU_EFLAGS_OF | VCPU_EFLAGS_IF;
        cpu_instruction_fixture fixture;
        t_cpu after;

        verr_verw_prepare(&fixture, profile);
        cpu_descriptor_query_enter_protected(&fixture, cpl);
        lib_memory_copy(fixture.memory + CPU_DESCRIPTOR_QUERY_GDT_ADDRESS +
            0x18u, descriptor, sizeof(descriptor));
        lib_memory_copy(fixture.memory + 0x100u, &selector, sizeof(selector));
        fixture.cpu.data.eax = 0xa1a10000u | selector;
        fixture.cpu.data.ecx = 0xb2b20000u;
        fixture.cpu.data.eflags = flags;
        if (!cpu_descriptor_query_run(&fixture, code[form], bytes[form], &after) ||
            x86_core_cpu_is_shutdown(&fixture.execution) ||
            after.data.eip != (lib_u32)(bytes[form] - 1u) ||
            after.data.eax != (0xa1a10000u | selector) ||
            after.data.ecx != 0xb2b20000u ||
            after.data.eflags != (flags | (expected_zf ? VCPU_EFLAGS_ZF : 0u)) ||
            fixture.writes != 0u || lib_memory_compare(fixture.memory +
                CPU_DESCRIPTOR_QUERY_GDT_ADDRESS + 0x18u, descriptor,
                sizeof(descriptor)) != 0) {
            lib_c_fprintf(lib_c_stderr, "VERR-VERW:TYPE profile=%u case=%u\n",
                profile, index);
            return 0;
        }
    }
    return 1;
}

static lib_i32 verr_verw_test_invalid_ldtr_is_negative(void)
{
    static const lib_u8 code[][4] = {
        {0x0fu,0x00u,VERR_VERW_CPU_VERR_MODRM,0xf4u},
        {0x0fu,0x00u,VERR_VERW_CPU_VERW_MODRM,0xf4u}
    };
    static const lib_u8 descriptor[] = {0xffu,0xffu,0,0,0,0x93u,0,0};
    static const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_80286,
        X86_CORE_CPU_PROFILE_80386, X86_CORE_CPU_PROFILE_80486SX
    };
    lib_size profile_index;
    lib_size index;

    for (profile_index = 0u; profile_index < sizeof(profiles) /
            sizeof(profiles[0]); ++profile_index) {
        for (index = 0u; index < sizeof(code) / sizeof(code[0]); ++index) {
            cpu_instruction_fixture fixture;
            t_cpu after;

            verr_verw_prepare(&fixture, profiles[profile_index]);
            fixture.cpu.data.ldtr.flagValid = LIB_FALSE;
            fixture.cpu.data.ldtr.selector = 0x0030u;
            fixture.cpu.data.ldtr.base = 0x0700u;
            fixture.cpu.data.ldtr.limit = 0xffffu;
            fixture.cpu.data.eax = 0xa1a1000cu;
            lib_memory_copy(fixture.memory + 0x0708u, descriptor,
                sizeof(descriptor));
            if (!cpu_descriptor_query_run(&fixture, code[index],
                    sizeof(code[index]), &after) ||
                x86_core_cpu_is_shutdown(&fixture.execution) ||
                CPU_DESCRIPTOR_QUERY_BIT_IS_SET(after.data.eflags,
                    VCPU_EFLAGS_ZF)) return 0;
        }
    }
    return 1;
}

static lib_i32 verr_verw_test_descriptor_bytes_unchanged(x86_core_cpu_profile profile)
{
    static const lib_u8 code[][4] = {
        {0x0fu,0x00u,VERR_VERW_CPU_VERR_MODRM,0xf4u},
        {0x0fu,0x00u,VERR_VERW_CPU_VERW_MODRM,0xf4u}
    };
    static const lib_u8 descriptor[] = {0xffu,0xffu,0,0,0,0x93u,0,0};
    lib_size index;

    for (index = 0u; index < sizeof(code) / sizeof(code[0]); ++index) {
        cpu_instruction_fixture fixture;
        t_cpu after;

        verr_verw_prepare(&fixture, profile);
        fixture.cpu.data.eax = 0xa1a10010u;
        lib_memory_copy(fixture.memory + CPU_DESCRIPTOR_QUERY_GDT_ADDRESS + 0x10u,
            descriptor, sizeof(descriptor));
        if (!cpu_descriptor_query_run(&fixture, code[index], sizeof(code[index]),
                &after) || !CPU_DESCRIPTOR_QUERY_BIT_IS_SET(after.data.eflags,
                VCPU_EFLAGS_ZF) || lib_memory_compare(fixture.memory +
                CPU_DESCRIPTOR_QUERY_GDT_ADDRESS + 0x10u, descriptor,
                sizeof(descriptor)) != 0) return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    static const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_80386, X86_CORE_CPU_PROFILE_80486SX
    };

    if (!verr_verw_test_outcomes() ||
        !verr_verw_test_invalid_ldtr_is_negative()) return 1;
    for (lib_size index = 0u; index < sizeof(profiles) / sizeof(profiles[0]); ++index) {
        if (!verr_verw_test_type_presence_privilege(profiles[index]) ||
            !verr_verw_test_memory_and_prefix_forms(profiles[index]) ||
            !verr_verw_test_protected_memory_forms(profiles[index]) ||
            !verr_verw_test_rejection(profiles[index]) ||
            !verr_verw_test_source_limit(profiles[index]) ||
            !verr_verw_test_ldt_selector(profiles[index]) ||
            !verr_verw_test_descriptor_bytes_unchanged(profiles[index])) {
            lib_c_fprintf(lib_c_stderr, "VERR-VERW:FAIL profile=%u\n",
                profiles[index]);
            return 1;
        }
    }
    lib_c_printf("%s\n", "VERR-VERW:OK");
    return 0;
}
