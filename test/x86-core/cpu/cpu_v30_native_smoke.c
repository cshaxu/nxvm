#include "support/cpu_bus_fixture.h"
#include "lib/types/file.h"

_Static_assert(X86_CORE_CPU_PROFILE_DEFAULT == 0 &&
    X86_CORE_CPU_PROFILE_8086 == 1 && X86_CORE_CPU_PROFILE_8088 == 2 &&
    X86_CORE_CPU_PROFILE_80186 == 3 && X86_CORE_CPU_PROFILE_80286 == 4 &&
    X86_CORE_CPU_PROFILE_80386 == 5 && X86_CORE_CPU_PROFILE_80486SX == 6 &&
    X86_CORE_CPU_PROFILE_V30 == 7, "Existing CPU profile values must not move");

static lib_i32 v30_family(void)
{
    static const struct {
        x86_core_cpu_profile profile;
        lib_bool integer_186;
        lib_bool protected_architecture;
        lib_bool architecture_32bit;
    } cases[] = {
        {X86_CORE_CPU_PROFILE_DEFAULT, LIB_FALSE, LIB_FALSE, LIB_FALSE},
        {X86_CORE_CPU_PROFILE_8086, LIB_FALSE, LIB_FALSE, LIB_FALSE},
        {X86_CORE_CPU_PROFILE_8088, LIB_FALSE, LIB_FALSE, LIB_FALSE},
        {X86_CORE_CPU_PROFILE_80186, LIB_TRUE, LIB_FALSE, LIB_FALSE},
        {X86_CORE_CPU_PROFILE_80286, LIB_TRUE, LIB_TRUE, LIB_FALSE},
        {X86_CORE_CPU_PROFILE_80386, LIB_TRUE, LIB_TRUE, LIB_TRUE},
        {X86_CORE_CPU_PROFILE_80486SX, LIB_TRUE, LIB_TRUE, LIB_TRUE},
        {X86_CORE_CPU_PROFILE_V30, LIB_TRUE, LIB_FALSE, LIB_FALSE}
    };

    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        if (cpu_profile_has_186_integer(cases[index].profile) != cases[index].integer_186 ||
            cpu_profile_has_protected_architecture(cases[index].profile) !=
                cases[index].protected_architecture ||
            cpu_profile_has_32bit_architecture(cases[index].profile) !=
                cases[index].architecture_32bit) return 1;
    }
    return lib_text_compare(x86_core_cpu_profile_name(X86_CORE_CPU_PROFILE_V30), "V30") != 0 ||
        x86_core_cpu_profile_has_8086_semantics(X86_CORE_CPU_PROFILE_V30) ||
        cpu_profile_allows_minimum(X86_CORE_CPU_PROFILE_V30, X86_CORE_CPU_PROFILE_80286) ||
        cpu_profile_allows_minimum((x86_core_cpu_profile)8, X86_CORE_CPU_PROFILE_8086);
}

static lib_i32 v30_reset(void)
{
    cpu_bus_fixture fixture;
    x86_core_cpu_state state;
    x86_core_cpu_prepared_entry *entry = LIB_NULL;
    const x86_core_cpu_entry_plan_state plan = { .cs = 0x100u, .ip = 0x20u,
        .sp = 0x700u, .eflags = VCPU_EFLAGS_CF };

    cpu_bus_prepare(&fixture, X86_CORE_CPU_PROFILE_V30);
    fixture.cpu.data.eflags = VCPU_EFLAGS_IF | VCPU_EFLAGS_TF | VCPU_EFLAGS_DF;
    x86_core_cpu_state_reset(&fixture.execution);
    x86_core_cpu_capture_state(&fixture.execution, &state);
    if (state.cs != 0xffffu || state.cs_base != 0xffff0u || state.eip != 0u ||
        (state.eflags & (X86_CPU_NEC_PSW_MD | VCPU_EFLAGS_IF | VCPU_EFLAGS_TF |
            VCPU_EFLAGS_DF)) != X86_CPU_NEC_PSW_MD ||
        fixture.cpu.data.ss.selector != 0u || fixture.cpu.data.ds.selector != 0u ||
        fixture.cpu.data.es.selector != 0u || fixture.execution.prefetch_count != 0u ||
        fixture.execution.prefetch_capacity != 6u) return 1;
    if (x86_core_cpu_prepare_entry(&fixture.execution, &plan, &entry) != LIB_STATUS_OK)
        return 1;
    x86_core_cpu_finish_entry(entry, LIB_TRUE);
    return fixture.cpu.data.cs.selector != plan.cs || fixture.cpu.data.eip != plan.ip ||
        fixture.cpu.data.eflags != (X86_CPU_NEC_PSW_MD | VCPU_EFLAGS_CF | 2u);
}

static lib_i32 v30_native_program(void)
{
    static const lib_u8 reset_jump[] = {0xeau, 0x00u, 0x01u, 0x00u, 0x00u};
    static const lib_u8 program[] = {
        0xbcu,0x00u,0x07u,               /* MOV SP,0700 */
        0xbau,0x34u,0x12u,               /* MOV DX,1234 */
        0xbbu,0x00u,0x05u,               /* MOV BX,0500 */
        0xb8u,0x34u,0x12u,               /* MOV AX,1234 */
        0x89u,0x07u,                     /* MOV [BX],AX */
        0x50u,0x31u,0xc0u,0x59u,         /* PUSH AX; XOR AX,AX; POP CX */
        0x83u,0xc1u,0x01u,               /* ADD CX,1 */
        0x89u,0x0eu,0x04u,0x05u,         /* MOV [0504],CX */
        0xb0u,0x5au,0xeeu,0xedu,         /* MOV AL,5A; OUT DX,AL; IN AX,DX */
        0xa3u,0x02u,0x05u,               /* MOV [0502],AX */
        0xb9u,0x03u,0x00u,               /* MOV CX,3 */
        0xfeu,0x07u,0xe2u,0xfcu,         /* INC byte [BX]; LOOP back */
        0xf4u                            /* HLT */
    };
    cpu_bus_fixture fixture;

    cpu_bus_prepare(&fixture, X86_CORE_CPU_PROFILE_V30);
    lib_memory_copy(&fixture.memory[0x7f0u], reset_jump, sizeof(reset_jump));
    lib_memory_copy(&fixture.memory[0x100u], program, sizeof(program));
    x86_core_cpu_state_reset(&fixture.execution);
    for (lib_u32 step = 0u; step < 40u && !fixture.cpu.data.flagHalt; ++step) {
        x86_core_cpu_execution_refresh(&fixture.execution);
        if (fixture.execution.stop_requested || fixture.faults ||
            fixture.delivered_exceptions || !x86_core_cpu_capture_timing(
                &fixture.execution).source_timing_unallocated) return 1;
    }
    return !fixture.cpu.data.flagHalt || fixture.failed || fixture.port_active ||
        fixture.completions != 2u || fixture.cpu.data.cs.selector != 0u ||
        fixture.cpu.data.eip != 0x100u + sizeof(program) ||
        fixture.cpu.data.sp != 0x700u || fixture.cpu.data.cx != 0u ||
        fixture.cpu.data.ax != 0x5au || fixture.memory[0x500u] != 0x37u ||
        fixture.memory[0x501u] != 0x12u || fixture.memory[0x502u] != 0x5au ||
        fixture.memory[0x503u] != 0u || fixture.memory[0x504u] != 0x35u ||
        fixture.memory[0x505u] != 0x12u ||
        (fixture.cpu.data.eflags & X86_CPU_NEC_PSW_MD) == 0u;
}

static lib_i32 v30_flags(void)
{
    static const lib_u8 program[] = {
        0x68u,0x01u,0x00u,0x9du,          /* PUSH 1; POPF: MD remains native */
        0x9cu,0x58u,0xf4u                 /* PUSHF; POP AX; HLT */
    };
    cpu_bus_fixture fixture;
    x86_core_cpu_debug_register_patch patch = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EFLAGS),
        .values = {[X86_CORE_CPU_DEBUG_EFLAGS] = 0u}
    };
    lib_u32 ignored = 0u;

    cpu_bus_prepare(&fixture, X86_CORE_CPU_PROFILE_V30);
    if (x86_core_cpu_debug_patch_registers(&fixture.execution, &patch) != LIB_STATUS_OK ||
        fixture.cpu.data.eflags != (X86_CPU_NEC_PSW_MD | 2u)) return 1;
    patch.mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CR0);
    patch.values[X86_CORE_CPU_DEBUG_CR0] = VCPU_CR0_PE;
    if (x86_core_cpu_debug_patch_registers(&fixture.execution, &patch) != LIB_STATUS_UNSUPPORTED ||
        fixture.cpu.data.cr0 != 0u || x86_core_cpu_debug_read_register(&fixture.execution,
            X86_CORE_CPU_DEBUG_FS, &ignored) != LIB_STATUS_UNSUPPORTED) return 1;
    lib_memory_copy(&fixture.memory[0x100u], program, sizeof(program));
    for (lib_u32 step = 0u; step < 6u && !fixture.cpu.data.flagHalt; ++step)
        x86_core_cpu_execution_refresh(&fixture.execution);
    return !fixture.cpu.data.flagHalt || fixture.execution.stop_requested ||
        fixture.cpu.data.ax != 0xf003u || fixture.cpu.data.sp != 0x700u ||
        fixture.faults || fixture.delivered_exceptions;
}

static lib_i32 v30_preview(void)
{
    static const struct {
        lib_u8 code[5];
        lib_u8 size;
    } cases[] = {
        {{0xb8u,0x34u,0x12u},3u},
        {{0x89u,0x86u,0xfeu,0xffu},4u},
        {{0x68u,0x34u,0x12u},3u},
        {{0xc8u,0x04u,0x00u,0x02u},4u}
    };
    x86_core_cpu_instruction_lexeme lexeme;

    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        if (!x86_core_cpu_instruction_lexeme_scan(cases[index].code, cases[index].size,
                X86_CORE_CPU_PROFILE_V30, LIB_FALSE, &lexeme) ||
            !lexeme.available || lexeme.byte_count != cases[index].size ||
            x86_core_cpu_instruction_lexeme_scan(cases[index].code, cases[index].size,
                X86_CORE_CPU_PROFILE_V30, LIB_TRUE, &lexeme)) return 1;
        for (lib_u8 size = 0u; size < cases[index].size; ++size) {
            if (x86_core_cpu_instruction_lexeme_scan(cases[index].code, size,
                    X86_CORE_CPU_PROFILE_V30, LIB_FALSE, &lexeme)) return 1;
        }
    }
    return x86_core_cpu_instruction_lexeme_scan(cases[0].code, cases[0].size,
        (x86_core_cpu_profile)8, LIB_FALSE, &lexeme);
}

static lib_i32 v30_not_admitted(void)
{
    static const lib_u8 cases[][3] = {
        {0x63u,0xc0u,0u},                  /* Not Intel ARPL */
        {0x66u,0x90u,0u},                  /* Not Intel size override */
        {0x67u,0x90u,0u},
        {0x0fu,0x20u,0xc0u}                /* Not Intel 0F MOV CR */
    };
    cpu_bus_fixture fixture;
    x86_core_cpu_instruction_lexeme lexeme;
    const x86_core_cpu_instruction_timing timing = {.base_ticks = 999u};

    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        cpu_bus_prepare(&fixture, X86_CORE_CPU_PROFILE_V30);
        lib_memory_copy(&fixture.memory[0x100u], cases[index], sizeof(cases[index]));
        x86_core_cpu_execution_refresh(&fixture.execution);
        if (!fixture.execution.stop_requested || fixture.delivered_exceptions ||
            fixture.faults != 1u || fixture.cpu.data.eip != 0x100u ||
            fixture.cpu.data.cr0 != 0u || x86_core_cpu_instruction_lexeme_scan(cases[index],
                sizeof(cases[index]), X86_CORE_CPU_PROFILE_V30, LIB_FALSE, &lexeme)) return 1;
    }
    cpu_bus_prepare(&fixture, X86_CORE_CPU_PROFILE_V30);
    fixture.cpu.data.eflags &= ~X86_CPU_NEC_PSW_MD;
    fixture.memory[0x100u] = 0x90u;
    x86_core_cpu_execution_refresh(&fixture.execution);
    if (!fixture.execution.stop_requested || fixture.cpu.data.eip != 0x100u) return 1;
    x86_core_cpu_state_reset(&fixture.execution);
    return fixture.execution.stop_requested ||
        (fixture.cpu.data.eflags & X86_CPU_NEC_PSW_MD) == 0u ||
        x86_core_cpu_timing_maximum_ticks(X86_CORE_CPU_PROFILE_V30, &timing) != 0u;
}

lib_i32 main(void)
{
    if (v30_family()) { lib_c_printf("V30:FAMILY:FAIL\n"); return 1; }
    if (v30_reset()) { lib_c_printf("V30:RESET:FAIL\n"); return 1; }
    if (v30_native_program()) { lib_c_printf("V30:NATIVE:FAIL\n"); return 1; }
    if (v30_flags()) { lib_c_printf("V30:FLAGS:FAIL\n"); return 1; }
    if (v30_preview()) { lib_c_printf("V30:PREVIEW:FAIL\n"); return 1; }
    if (v30_not_admitted()) { lib_c_printf("V30:ADMISSION:FAIL\n"); return 1; }
    lib_c_printf("V30:NATIVE-FAMILY:OK\n");
    return 0;
}
