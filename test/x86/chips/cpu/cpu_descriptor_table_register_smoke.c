#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

/* REAL_UD_TERMINAL_CPU_OWNER: invalid DTTR stops at the CPU. */
/* DTTR instructions own descriptor interpretation; this fixture owns only CPU state. */
static void dttr_prepare(cpu_instruction_fixture *state,
    core_machine_cpu_profile profile)
{
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xff,0xff,0,0x20,0,0x9a,0,0,
        0xff,0xff,0,0,0,0x92,0,0,
        0x1f,0,0,0x50,0,0x82,0,0,
        0x67,0,0,0x60,0,0x89,0,0
    };

    cpu_instruction_prepare(state, profile);
    state->cpu.data.cr0 |= VCPU_CR0_PE;
    state->cpu.data.cs.flagValid = LIB_TRUE;
    state->cpu.data.cs.selector = 0x08u;
    state->cpu.data.cs.sregtype = SREG_CODE;
    state->cpu.data.cs.seg.executable = LIB_TRUE;
    state->cpu.data.cs.seg.exec.readable = LIB_TRUE;
    state->cpu.data.cs.limit = 0xffffu;
    state->cpu.data.ds.flagValid = LIB_TRUE;
    state->cpu.data.ds.selector = 0x10u;
    state->cpu.data.ds.sregtype = SREG_DATA;
    state->cpu.data.ds.seg.data.writable = LIB_TRUE;
    state->cpu.data.ds.limit = 0xffffu;
    state->cpu.data.ss = state->cpu.data.ds;
    state->cpu.data.ss.sregtype = SREG_STACK;
    state->cpu.data.gdtr.base = 0x0300u;
    state->cpu.data.gdtr.limit = sizeof(gdt) - 1u;
    lib_memory_copy(state->memory + state->cpu.data.gdtr.base, gdt,
        sizeof(gdt));
}

static lib_i32 dttr_run(cpu_instruction_fixture *state, const lib_u8 *code,
    lib_u8 bytes, t_cpu *after)
{
    lib_u8 steps;

    lib_memory_copy(state->memory, code, bytes);
    for (steps = 0u; steps != 16u && state->cpu.data.eip < bytes &&
        !state->execution.stop_requested; ++steps)
        core_machine_cpu_execution_refresh(&state->execution);
    *after = state->cpu;
    return state->cpu.data.eip == bytes && !state->execution.stop_requested &&
        !state->fault.valid;
}

static lib_i32 dttr_test_registers(void)
{
    static const lib_u8 lldt[] = {0xb8u,0x18u,0,0x0fu,0,0xd0u};
    static const lib_u8 sldt[] = {0x0fu,0,0xc0u};
    static const lib_u8 ltr[] = {0xb8u,0x20u,0,0x0fu,0,0xd8u};
    static const lib_u8 str[] = {0x0fu,0,0xc8u};
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    lib_u8 profile;

    for (profile = 0u; profile != 2u; ++profile) {
        cpu_instruction_fixture state;
        t_cpu after;
        lib_u8 access = 0u;

        dttr_prepare(&state, profiles[profile]);
        if (!dttr_run(&state, lldt, sizeof(lldt), &after) ||
            after.data.ldtr.selector != 0x18u || after.data.eip != sizeof(lldt))
            return 0;
        dttr_prepare(&state, profiles[profile]);
        if (!dttr_run(&state, ltr, sizeof(ltr), &after) ||
            after.data.tr.selector != 0x20u ||
            after.data.tr.sys.type != VCPU_DESC_SYS_TYPE_TSS_32_BUSY)
            return 0;
        access = state.memory[0x0300u + 4u * 8u + 5u];
        if (access != 0x8bu) return 0;
        dttr_prepare(&state, profiles[profile]);
        state.cpu.data.eax = 0xa1a10018u;
        if (!dttr_run(&state, sldt, sizeof(sldt), &after) ||
            (lib_u16)after.data.eax != 0u || after.data.eip != sizeof(sldt))
            return 0;
        dttr_prepare(&state, profiles[profile]);
        if (!dttr_run(&state, str, sizeof(str), &after) ||
            (lib_u16)after.data.eax != 0u || after.data.eip != sizeof(str))
            return 0;
    }
    return 1;
}

static lib_i32 dttr_test_memory_and_attributes(void)
{
    static const lib_u8 sldt[] = {0xb8u,0x18u,0,0x0fu,0,0xd0u,
        0x0fu,0,0x06u,0,0x40u};
    static const lib_u8 attributes[][6] = {
        {0x66u,0x0fu,0,0xd0u,0,0},
        {0x67u,0x0fu,0,0xd0u,0,0},
        {0x66u,0x67u,0x0fu,0,0xd0u,0}
    };
    static const lib_u8 attribute_bytes[] = {4u,4u,5u};
    cpu_instruction_fixture state;
    t_cpu after;
    lib_u16 selector = 0x20u;
    lib_u8 form;

    dttr_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    if (!dttr_run(&state, sldt, sizeof(sldt), &after)) return 0;
    lib_memory_copy(&selector, state.memory + 0x4000u, sizeof(selector));
    if (selector != 0x18u) return 0;
    for (form = 0u; form != 3u; ++form) {
        dttr_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        state.cpu.data.eax = 0xa1a10018u;
        if (!dttr_run(&state, attributes[form], attribute_bytes[form], &after) ||
            after.data.ldtr.selector != 0x18u ||
            after.data.eip != attribute_bytes[form]) return 0;
    }
    dttr_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    if (!dttr_run(&state, (const lib_u8[]){0xb8u,0,0,0x0fu,0,0xd0u},
            6u, &after) || after.data.ldtr.selector != 0u ||
        after.data.ldtr.flagValid) return 0;
    return 1;
}

static lib_i32 dttr_expect_ud(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes)
{
    cpu_instruction_fixture state;
    cpu_instruction_prepare(&state, profile);
    return cpu_instruction_expect_real_fault(&state, code, bytes, 6u);
}

static lib_i32 dttr_test_rejections(void)
{
    static const lib_u8 form[] = {0x0fu,0,0xc0u};
    static const lib_u8 lock[] = {0xf0u,0x0fu,0,0xc0u};
    static const lib_u8 prefix66[] = {0x66u,0x0fu,0,0xc0u};
    static const lib_u8 prefix67[] = {0x67u,0x0fu,0,0xc0u};

    return dttr_expect_ud(CORE_MACHINE_CPU_PROFILE_80186, form, sizeof(form)) &&
        dttr_expect_ud(CORE_MACHINE_CPU_PROFILE_80286, form, sizeof(form)) &&
        dttr_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, form, sizeof(form)) &&
        dttr_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, lock, sizeof(lock)) &&
        dttr_expect_ud(CORE_MACHINE_CPU_PROFILE_80286, prefix66, sizeof(prefix66)) &&
        dttr_expect_ud(CORE_MACHINE_CPU_PROFILE_80286, prefix67, sizeof(prefix67));
}

lib_i32 main(void)
{
    lib_i32 registers = dttr_test_registers();
    lib_i32 forms = dttr_test_memory_and_attributes();
    lib_i32 rejections = dttr_test_rejections();

    if (!registers || !forms || !rejections) {
        lib_c_fprintf(lib_c_stderr, "DTTR CPU failed register=%d forms=%d reject=%d\n",
            registers, forms, rejections);
        return 1;
    }
    lib_c_printf("%s\n", "DTTR-CPU:OK");
    return 0;
}
