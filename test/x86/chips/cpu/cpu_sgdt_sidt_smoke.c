#include "support/cpu_instruction_fixture.h"
#include <stdio.h>

/* T337_REAL_UD_TERMINAL_CPU_OWNER: invalid SGDT/SIDT stops at the CPU. */
static lib_i32 sgdt_sidt_run(cpu_instruction_fixture *state,
    const lib_u8 *code, lib_u8 bytes, t_cpu *after)
{
    lib_memory_copy(state->memory, code, bytes);
    core_machine_cpu_execution_refresh(&state->execution);
    *after = state->cpu;
    return !state->execution.stop_requested && !state->fault.valid &&
        after->data.eip == bytes;
}

static void sgdt_sidt_expected(lib_u8 image[6], lib_u16 limit,
    lib_u32 base, lib_bool operand32, core_machine_cpu_profile profile)
{
    image[0] = (lib_u8)limit;
    image[1] = (lib_u8)(limit >> 8u);
    image[2] = (lib_u8)base;
    image[3] = (lib_u8)(base >> 8u);
    image[4] = (lib_u8)(base >> 16u);
    image[5] = profile == CORE_MACHINE_CPU_PROFILE_80286 ? 0xffu :
        operand32 ? (lib_u8)(base >> 24u) : 0u;
}

static lib_i32 sgdt_sidt_case(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes, lib_bool idt, lib_bool operand32)
{
    cpu_instruction_fixture state;
    t_cpu before, after;
    lib_u8 image[6] = {0}, expected[6];
    const lib_u16 limit = idt ? 0x1357u : 0x2468u;
    const lib_u32 base = idt ? 0x89abcdefu : 0x12345678u;

    cpu_instruction_prepare(&state, profile);
    state.cpu.data.gdtr.limit = 0x2468u;
    state.cpu.data.gdtr.base = 0x12345678u;
    state.cpu.data.idtr.limit = 0x1357u;
    state.cpu.data.idtr.base = 0x89abcdefu;
    state.cpu.data.eax = 0x11223344u;
    state.cpu.data.ecx = 0x55667788u;
    state.cpu.data.eflags = VCPU_EFLAGS_IF | VCPU_EFLAGS_CF;
    lib_memory_set(state.memory + 0x0200u, 0xa5, sizeof(image));
    before = state.cpu;
    if (!sgdt_sidt_run(&state, code, bytes, &after)) return 0;
    lib_memory_copy(image, state.memory + 0x0200u, sizeof(image));
    sgdt_sidt_expected(expected, limit, base, operand32, profile);
    return lib_memory_compare(image, expected, sizeof(image)) == 0 &&
        after.data.eip == bytes && after.data.eax == before.data.eax &&
        after.data.ecx == before.data.ecx &&
        after.data.eflags == before.data.eflags;
}

static lib_i32 sgdt_sidt_test_values(void)
{
    static const lib_u8 sgdt[] = {0x0fu,0x01u,0x06u,0x00u,0x02u};
    static const lib_u8 sidt[] = {0x0fu,0x01u,0x0eu,0x00u,0x02u};
    static const lib_u8 sgdt32[] = {0x66u,0x0fu,0x01u,0x06u,0x00u,0x02u};
    static const lib_u8 sidt32[] = {0x66u,0x0fu,0x01u,0x0eu,0x00u,0x02u};
    static const lib_u8 sgdt_address32[] = {
        0x67u,0x0fu,0x01u,0x05u,0x00u,0x02u,0x00u,0x00u
    };
    static const lib_u8 sidt_address32[] = {
        0x67u,0x0fu,0x01u,0x0du,0x00u,0x02u,0x00u,0x00u
    };
    static const lib_u8 sgdt_both32[] = {
        0x66u,0x67u,0x0fu,0x01u,0x05u,0x00u,0x02u,0x00u,0x00u
    };
    static const lib_u8 sidt_both32[] = {
        0x66u,0x67u,0x0fu,0x01u,0x0du,0x00u,0x02u,0x00u,0x00u
    };

    return sgdt_sidt_case(CORE_MACHINE_CPU_PROFILE_80286, sgdt,
            sizeof(sgdt), LIB_FALSE, LIB_FALSE) &&
        sgdt_sidt_case(CORE_MACHINE_CPU_PROFILE_80286, sidt,
            sizeof(sidt), LIB_TRUE, LIB_FALSE) &&
        sgdt_sidt_case(CORE_MACHINE_CPU_PROFILE_80386, sgdt,
            sizeof(sgdt), LIB_FALSE, LIB_FALSE) &&
        sgdt_sidt_case(CORE_MACHINE_CPU_PROFILE_80386, sidt,
            sizeof(sidt), LIB_TRUE, LIB_FALSE) &&
        sgdt_sidt_case(CORE_MACHINE_CPU_PROFILE_80386, sgdt32,
            sizeof(sgdt32), LIB_FALSE, LIB_TRUE) &&
        sgdt_sidt_case(CORE_MACHINE_CPU_PROFILE_80386, sidt32,
            sizeof(sidt32), LIB_TRUE, LIB_TRUE) &&
        sgdt_sidt_case(CORE_MACHINE_CPU_PROFILE_80386, sgdt_address32,
            sizeof(sgdt_address32), LIB_FALSE, LIB_FALSE) &&
        sgdt_sidt_case(CORE_MACHINE_CPU_PROFILE_80386, sidt_address32,
            sizeof(sidt_address32), LIB_TRUE, LIB_FALSE) &&
        sgdt_sidt_case(CORE_MACHINE_CPU_PROFILE_80386, sgdt_both32,
            sizeof(sgdt_both32), LIB_FALSE, LIB_TRUE) &&
        sgdt_sidt_case(CORE_MACHINE_CPU_PROFILE_80386, sidt_both32,
            sizeof(sidt_both32), LIB_TRUE, LIB_TRUE);
}

static lib_i32 sgdt_sidt_expect_ud(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes)
{
    cpu_instruction_fixture state;
    t_cpu before, after;

    cpu_instruction_prepare(&state, profile);
    state.cpu.data.idtr.limit = 0x17u;
    before = state.cpu;
    (void)cpu_instruction_run(&state, code, bytes, &after);
    return state.execution.stop_requested && state.fault.valid &&
        (state.fault.exception_mask & VCPUINS_EXCEPT_UD) &&
        lib_memory_compare(&before, &after, sizeof(before)) == 0;
}

static lib_i32 sgdt_sidt_test_rejections(void)
{
    static const lib_u8 low[] = {0x0fu,0x01u,0x06u,0x00u,0x02u};
    static const lib_u8 register_form[] = {0x0fu,0x01u,0xc0u};
    static const lib_u8 reserved_form[] = {0x0fu,0x01u,0x2eu,0x00u,0x02u};
    static const lib_u8 lock_form[] = {0xf0u,0x0fu,0x01u,0x06u,0x00u,0x02u};
    lib_u8 table;

    for (table = 0u; table != 2u; ++table) {
        lib_u8 code[sizeof(low)], direct[sizeof(register_form)],
            lock[sizeof(lock_form)];
        lib_memory_copy(code, low, sizeof(code));
        lib_memory_copy(direct, register_form, sizeof(direct));
        lib_memory_copy(lock, lock_form, sizeof(lock));
        code[2] |= table << 3u;
        direct[2] |= table << 3u;
        lock[3] |= table << 3u;
        if (!sgdt_sidt_expect_ud(CORE_MACHINE_CPU_PROFILE_80186, code,
                sizeof(code)) || !sgdt_sidt_expect_ud(
                CORE_MACHINE_CPU_PROFILE_80386, direct, sizeof(direct)) ||
            !sgdt_sidt_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, lock,
                sizeof(lock))) return 0;
    }
    return sgdt_sidt_expect_ud(CORE_MACHINE_CPU_PROFILE_80386,
        reserved_form, sizeof(reserved_form));
}

static lib_i32 sgdt_sidt_test_segments_and_vm86(void)
{
    static const lib_u8 forms[][6] = {
        {0x0fu,0x01u,0x06u,0x00u,0x02u,0u},
        {0x0fu,0x01u,0x46u,0x10u,0u,0u},
        {0x26u,0x0fu,0x01u,0x0eu,0x00u,0x03u}
    };
    static const lib_u8 bytes[] = {5u,4u,6u};
    static const lib_u32 addresses[] = {0x0200u,0x4030u,0x8300u};
    lib_u8 form;

    for (form = 0u; form != 3u; ++form) {
        cpu_instruction_fixture state;
        t_cpu after;
        lib_u8 image[6], expected[6];

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        if (form == 1u) state.cpu.data.ss.base = 0x4000u;
        if (form == 2u) state.cpu.data.es.base = 0x8000u;
        state.cpu.data.ebp = 0x20u;
        state.cpu.data.gdtr.limit = 0x2468u;
        state.cpu.data.gdtr.base = 0x12345678u;
        state.cpu.data.idtr.limit = 0x1357u;
        state.cpu.data.idtr.base = 0x89abcdefu;
        lib_memory_set(state.memory + addresses[form], 0xa5, sizeof(image));
        if (!sgdt_sidt_run(&state, forms[form], bytes[form], &after)) return 0;
        lib_memory_copy(image, state.memory + addresses[form], sizeof(image));
        sgdt_sidt_expected(expected, form == 2u ? 0x1357u : 0x2468u,
            form == 2u ? 0x89abcdefu : 0x12345678u, LIB_FALSE,
            CORE_MACHINE_CPU_PROFILE_80386);
        if (lib_memory_compare(image, expected, sizeof(image)) != 0) return 0;
    }
    {
        static const lib_u8 sgdt[] = {0x0fu,0x01u,0x06u,0x00u,0x02u};
        cpu_instruction_fixture state;
        t_cpu after;
        lib_u8 image[6], expected[6];

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        state.cpu.data.cr0 |= VCPU_CR0_PE;
        state.cpu.data.eflags |= VCPU_EFLAGS_VM | VCPU_EFLAGS_IOPL;
        state.cpu.data.cs.dpl = 3u;
        state.cpu.data.ds.dpl = 3u;
        state.cpu.data.ss.dpl = 3u;
        state.cpu.data.gdtr.limit = 0x9876u;
        state.cpu.data.gdtr.base = 0x12345678u;
        if (!sgdt_sidt_run(&state, sgdt, sizeof(sgdt), &after)) { fprintf(stderr, "vm run\n"); return 0; }
        lib_memory_copy(image, state.memory + 0x0200u, sizeof(image));
        sgdt_sidt_expected(expected, 0x9876u, 0x12345678u, LIB_FALSE,
            CORE_MACHINE_CPU_PROFILE_80386);
        if (lib_memory_compare(image, expected, sizeof(image)) != 0) { fprintf(stderr, "vm image\n"); return 0; }
    }
    return 1;
}

lib_i32 main(void)
{
    lib_i32 values = sgdt_sidt_test_values();
    lib_i32 rejections = sgdt_sidt_test_rejections();
    lib_i32 routes = sgdt_sidt_test_segments_and_vm86();

    if (!values || !rejections || !routes) {
        fprintf(stderr, "M5:T539:S42:SGDT-SIDT CPU failed values=%d reject=%d routes=%d\n",
            values, rejections, routes);
        return 1;
    }
    puts("M5:T539:S42:SGDT-SIDT-CPU:OK");
    return 0;
}
