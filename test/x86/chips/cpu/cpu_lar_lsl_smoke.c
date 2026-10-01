#include "support/cpu_descriptor_query_fixture.h"
#include <stdio.h>

static lib_i32 lar_lsl_run_case(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes, lib_u16 selector, lib_u32 eax,
    lib_u8 expected_zf, lib_u32 expected_eax)
{
    cpu_instruction_fixture fixture;
    t_cpu after;

    cpu_instruction_prepare(&fixture, profile);
    cpu_descriptor_query_enter_protected(&fixture, 0u);
    cpu_descriptor_query_install_gdt(&fixture);
    fixture.cpu.data.eax = eax;
    fixture.cpu.data.ecx = selector;
    fixture.cpu.data.eflags = VCPU_EFLAGS_CF;
    if (!cpu_descriptor_query_run(&fixture, code, bytes, &after)) return 0;
    return !!CPU_DESCRIPTOR_QUERY_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_ZF) ==
            expected_zf && after.data.eax == expected_eax &&
        CPU_DESCRIPTOR_QUERY_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_CF);
}

static lib_i32 lar_lsl_test_register_forms(void)
{
    static const lib_u8 lar[] = {0x0fu, 0x02u, 0xc1u, 0xf4u};
    static const lib_u8 lsl[] = {0x0fu, 0x03u, 0xc1u, 0xf4u};
    const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    lib_size profile;

    for (profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        if (!lar_lsl_run_case(profiles[profile], lar, sizeof(lar), 0x0010u,
                0xa1a10000u, LIB_TRUE, 0xa1a19300u) ||
            !lar_lsl_run_case(profiles[profile], lsl, sizeof(lsl), 0x0010u,
                0xa1a10000u, LIB_TRUE, 0xa1a1ffffu) ||
            !lar_lsl_run_case(profiles[profile], lar, sizeof(lar), 0x0018u,
                0xa1a10000u, LIB_FALSE, 0xa1a10000u) ||
            !lar_lsl_run_case(profiles[profile], lsl, sizeof(lsl), 0x0020u,
                0xa1a10000u, LIB_TRUE, 0xa1a1ffffu)) return 0;
    }
    return 1;
}

static lib_i32 lar_lsl_test_386_attributes_and_memory(void)
{
    static const lib_u8 lar32[] = {0x66u, 0x0fu, 0x02u, 0xc1u, 0xf4u};
    static const lib_u8 lsl32[] = {0x66u, 0x0fu, 0x03u, 0xc1u, 0xf4u};
    static const lib_u8 lar_memory[] = {
        0x0fu, 0x02u, 0x0eu, 0x00u, 0x01u, 0xf4u
    };
    static const lib_u8 lsl_memory[] = {
        0x0fu, 0x03u, 0x0eu, 0x00u, 0x01u, 0xf4u
    };
    const lib_u16 selector = 0x0010u;
    const lib_u16 page_selector = 0x0020u;
    cpu_instruction_fixture fixture;
    t_cpu after;

    if (!lar_lsl_run_case(CORE_MACHINE_CPU_PROFILE_80386, lar32,
            sizeof(lar32), selector, 0xa1a10000u, LIB_TRUE, 0x00009300u) ||
        !lar_lsl_run_case(CORE_MACHINE_CPU_PROFILE_80386, lsl32,
            sizeof(lsl32), selector, 0xa1a10000u, LIB_TRUE, 0x0000ffffu))
        return 0;
    cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
    cpu_descriptor_query_enter_protected(&fixture, 0u);
    cpu_descriptor_query_install_gdt(&fixture);
    fixture.cpu.data.ecx = page_selector;
    if (!cpu_descriptor_query_run(&fixture, lsl32, sizeof(lsl32), &after) ||
        after.data.eax != 0x0fffffffu) return 0;
    cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
    cpu_descriptor_query_enter_protected(&fixture, 0u);
    cpu_descriptor_query_install_gdt(&fixture);
    fixture.cpu.data.ecx = 0x1234u;
    lib_memory_copy(fixture.memory + 0x0100u, &selector, sizeof(selector));
    if (!cpu_descriptor_query_run(&fixture, lar_memory, sizeof(lar_memory),
            &after) || after.data.ecx != 0x00009300u ||
        !CPU_DESCRIPTOR_QUERY_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_ZF)) return 0;
    cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
    cpu_descriptor_query_enter_protected(&fixture, 0u);
    cpu_descriptor_query_install_gdt(&fixture);
    fixture.cpu.data.ecx = 0x1234u;
    lib_memory_copy(fixture.memory + 0x0100u, &selector, sizeof(selector));
    return cpu_descriptor_query_run(&fixture, lsl_memory, sizeof(lsl_memory),
        &after) && after.data.ecx == 0x0000ffffu &&
        CPU_DESCRIPTOR_QUERY_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_ZF);
}

static lib_i32 lar_lsl_test_bp_ss_source(void)
{
    static const lib_u8 code[] = {
        0x0fu, 0x03u, 0x4eu, 0x00u, 0xf4u
    };
    const lib_u16 selector = 0x0010u;
    cpu_instruction_fixture fixture;
    t_cpu after;

    cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
    cpu_descriptor_query_enter_protected(&fixture, 0u);
    cpu_descriptor_query_install_gdt(&fixture);
    /* This is the former real-board bootstrap's loaded SS cache, made
     * explicit here so the instruction's default-SS addressing is CPU-local. */
    fixture.cpu.data.ss.base = 0x3000u;
    fixture.cpu.data.ss.limit = 0xffffu;
    fixture.cpu.data.ss.selector = 0x0010u;
    fixture.cpu.data.ss.flagValid = LIB_TRUE;
    fixture.cpu.data.ss.sregtype = SREG_DATA;
    fixture.cpu.data.ss.seg.data.writable = LIB_TRUE;
    fixture.cpu.data.ecx = 0x1234u;
    fixture.cpu.data.ebp = 0x0100u;
    lib_memory_copy(fixture.memory + 0x3100u, &selector, sizeof(selector));
    return cpu_descriptor_query_run(&fixture, code, sizeof(code), &after) &&
        after.data.ecx == 0x0000ffffu && after.data.ebp == 0x00000100u &&
        CPU_DESCRIPTOR_QUERY_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_ZF);
}

static lib_i32 lar_lsl_test_sib_source(void)
{
    static const lib_u8 code[] = {
        0x66u, 0xbcu, 0x00u, 0x01u, 0x00u, 0x00u,
        0x67u, 0x0fu, 0x02u, 0x0cu, 0x24u, 0xf4u
    };
    const lib_u16 selector = 0x0010u;
    cpu_instruction_fixture fixture;

    if (!cpu_descriptor_query_boot_protected(&fixture,
            CORE_MACHINE_CPU_PROFILE_80386)) return 0;
    fixture.cpu.data.eip = 0u;
    fixture.cpu.data.flagHalt = LIB_FALSE;
    fixture.cpu.data.ecx = 0x1234u;
    lib_memory_copy(fixture.memory + 0x4100u, &selector, sizeof(selector));
    lib_memory_copy(fixture.memory + CPU_DESCRIPTOR_QUERY_CODE_ADDRESS, code,
        sizeof(code));
    if (!cpu_descriptor_query_execute(&fixture, 16u) ||
        !fixture.cpu.data.flagHalt || fixture.cpu.data.ecx != 0x00009300u ||
        fixture.cpu.data.esp != 0x00000100u ||
        !CPU_DESCRIPTOR_QUERY_BIT_IS_SET(fixture.cpu.data.eflags, VCPU_EFLAGS_ZF)) {
        return 0;
    }
    return 1;
}

static lib_i32 lar_lsl_test_sib_and_overrides(void)
{
    static const lib_u8 override_codes[][7] = {
        {0x26u,0x0fu,0x02u,0x0eu,0x20u,0x01u,0xf4u},
        {0x64u,0x0fu,0x03u,0x0eu,0x20u,0x01u,0xf4u},
        {0x65u,0x0fu,0x02u,0x0eu,0x20u,0x01u,0xf4u}
    };
    static const lib_u32 bases[] = {0x4000u, 0x5000u, 0x6000u};
    static const lib_u32 expected[] = {
        0x00009300u, 0x0000ffffu, 0x00009300u
    };
    const lib_u16 selector = 0x0010u;
    cpu_instruction_fixture fixture;
    t_cpu after;
    lib_size index;

    for (index = 0u; index < sizeof(override_codes) / sizeof(override_codes[0]);
        ++index) {
        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
        cpu_descriptor_query_enter_protected(&fixture, 0u);
        cpu_descriptor_query_install_gdt(&fixture);
        fixture.cpu.data.es.base = bases[index];
        fixture.cpu.data.es.limit = 0xffffu;
        fixture.cpu.data.es.selector = 0x0010u;
        fixture.cpu.data.es.flagValid = LIB_TRUE;
        fixture.cpu.data.es.sregtype = SREG_DATA;
        fixture.cpu.data.es.seg.data.writable = LIB_TRUE;
        fixture.cpu.data.fs.base = bases[index];
        fixture.cpu.data.fs.limit = 0xffffu;
        fixture.cpu.data.fs.selector = 0x0010u;
        fixture.cpu.data.gs.base = bases[index];
        fixture.cpu.data.gs.limit = 0xffffu;
        fixture.cpu.data.gs.selector = 0x0010u;
        fixture.cpu.data.fs.flagValid = LIB_TRUE;
        fixture.cpu.data.gs.flagValid = LIB_TRUE;
        fixture.cpu.data.ecx = 0x1234u;
        lib_memory_copy(fixture.memory + bases[index] + 0x120u, &selector,
            sizeof(selector));
        if (!cpu_descriptor_query_run(&fixture, override_codes[index],
                sizeof(override_codes[index]), &after) ||
            after.data.ecx != expected[index] ||
            !CPU_DESCRIPTOR_QUERY_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_ZF)) return 0;
    }
    return 1;
}

static lib_i32 lar_lsl_test_visibility(void)
{
    static const lib_u8 lsl[] = {0x0fu, 0x03u, 0xc1u, 0xf4u};
    cpu_instruction_fixture fixture;
    t_cpu after;

    cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
    cpu_descriptor_query_enter_protected(&fixture, 3u);
    cpu_descriptor_query_install_gdt(&fixture);
    fixture.cpu.data.ecx = 0x0013u;
    fixture.cpu.data.eax = 0xa1a10000u;
    if (!cpu_descriptor_query_run(&fixture, lsl, sizeof(lsl), &after)) return 0;
    return !CPU_DESCRIPTOR_QUERY_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_ZF) &&
        after.data.eax == 0xa1a10000u;
}

static lib_i32 lar_lsl_test_rejection(void)
{
    static const lib_u8 lar[] = {0x0fu, 0x02u, 0xc1u};
    static const lib_u8 lsl[] = {0x0fu, 0x03u, 0xc1u};
    static const lib_u8 lock_lar[] = {0xf0u, 0x0fu, 0x02u, 0xc1u};
    cpu_instruction_fixture fixture;

    cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80186);
    if (!cpu_descriptor_query_fault(&fixture, lar, sizeof(lar),
            VCPUINS_EXCEPT_UD)) return 0;
    cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
    if (!cpu_descriptor_query_fault(&fixture, lock_lar, sizeof(lock_lar),
            VCPUINS_EXCEPT_UD)) return 0;
    cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
    fixture.cpu.data.cr0 |= VCPU_CR0_PE;
    fixture.cpu.data.eflags |= VCPU_EFLAGS_VM;
    return cpu_descriptor_query_fault(&fixture, lsl, sizeof(lsl),
        VCPUINS_EXCEPT_UD);
}

static lib_i32 lar_lsl_test_source_limit(void)
{
    static const lib_u8 codes[][5] = {
        {0x0fu,0x02u,0x0eu,0x10u,0x00u},
        {0x0fu,0x03u,0x0eu,0x10u,0x00u}
    };
    const lib_u16 selector = 0x0010u;
    lib_size index;

    for (index = 0u; index < sizeof(codes) / sizeof(codes[0]); ++index) {
        cpu_instruction_fixture fixture;
        t_cpu before;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
        cpu_descriptor_query_enter_protected(&fixture, 0u);
        cpu_descriptor_query_install_gdt(&fixture);
        fixture.cpu.data.ds.limit = 0x000fu;
        fixture.cpu.data.eax = 0xa1a10000u;
        fixture.cpu.data.ecx = 0x1234u;
        before = fixture.cpu;
        lib_memory_copy(fixture.memory + 0x0010u, &selector, sizeof(selector));
        if (!cpu_descriptor_query_fault(&fixture, codes[index],
                sizeof(codes[index]), VCPUINS_EXCEPT_DF) ||
            fixture.cpu.data.eax != before.data.eax ||
            fixture.cpu.data.ecx != before.data.ecx ||
            fixture.cpu.data.eip != before.data.eip ||
            fixture.cpu.data.eflags != before.data.eflags) return 0;
    }
    return 1;
}

static lib_i32 lar_lsl_test_ldt_selector(void)
{
    static const lib_u8 ldt_descriptor[] = {
        0xffu,0xffu,0x00u,0x70u,0x00u,0x92u,0x00u,0x00u
    };
    static const lib_u8 ldt_table_descriptor[] = {
        0x0fu,0x00u,0x00u,0x05u,0x00u,0x82u,0x00u,0x00u
    };
    static const lib_u8 codes[][13] = {
        {0xb8u,0x30u,0x00u,0x0fu,0x00u,0xd0u,
         0xb9u,0x0cu,0x00u,0x0fu,0x02u,0xc1u,0xf4u},
        {0xb8u,0x30u,0x00u,0x0fu,0x00u,0xd0u,
         0xb9u,0x0cu,0x00u,0x0fu,0x03u,0xc1u,0xf4u}
    };
    const lib_u32 expected_eax[] = {0x00009200u, 0x0000ffffu};
    lib_size index;

    for (index = 0u; index < sizeof(codes) / sizeof(codes[0]); ++index) {
        cpu_instruction_fixture fixture;

        if (!cpu_descriptor_query_boot_protected(&fixture,
                CORE_MACHINE_CPU_PROFILE_80386)) return 0;
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
            fixture.cpu.data.eax != expected_eax[index] ||
            !CPU_DESCRIPTOR_QUERY_BIT_IS_SET(fixture.cpu.data.eflags,
                VCPU_EFLAGS_ZF)) return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!lar_lsl_test_register_forms() ||
        !lar_lsl_test_386_attributes_and_memory() ||
        !lar_lsl_test_bp_ss_source() ||
        !lar_lsl_test_sib_source() ||
        !lar_lsl_test_sib_and_overrides() ||
        !lar_lsl_test_visibility() || !lar_lsl_test_rejection() ||
        !lar_lsl_test_source_limit() || !lar_lsl_test_ldt_selector()) return 1;
    puts("M5:T539:S44:LAR-LSL:OK");
    return 0;
}
