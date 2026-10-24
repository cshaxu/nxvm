#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

#define DESCRIPTOR_GDT_ADDRESS 0x0300u
#define DESCRIPTOR_LOAD_ADDRESS 0x0240u
#define DESCRIPTOR_LDT_SELECTOR 0x0018u
#define DESCRIPTOR_TSS16_SELECTOR 0x0020u
#define DESCRIPTOR_LDT_NOT_PRESENT_SELECTOR 0x0028u
#define DESCRIPTOR_TSS16_BUSY_SELECTOR 0x0030u
#define DESCRIPTOR_TSS16_NOT_PRESENT_SELECTOR 0x0038u
#define DESCRIPTOR_TSS32_SELECTOR 0x0040u

static lib_i32 descriptor_run(cpu_instruction_fixture *state,
    const lib_u8 *code, lib_u8 bytes, t_cpu *after)
{
    const lib_u8 expected_eip = bytes != 0u && code[bytes - 1u] == 0xf4u ?
        bytes - 1u : bytes;

    lib_memory_copy(state->memory, code, bytes);
    x86_core_cpu_execution_refresh(&state->execution);
    *after = state->cpu;
    return !state->execution.stop_requested && !state->fault.valid &&
        after->data.eip == expected_eip;
}

static lib_i32 descriptor_run_fault(cpu_instruction_fixture *state,
    const lib_u8 *code, lib_u8 bytes, lib_u32 exception,
    lib_u32 exception_code, t_cpu *after)
{
    if ((state->cpu.data.cr0 & VCPU_CR0_PE) == 0u) {
        lib_u8 vector = 0u;

        while ((UINT32_C(1) << vector) != exception) ++vector;
        if (!cpu_instruction_expect_real_fault(state, code, bytes, vector))
            return 0;
        *after = state->cpu;
        return 1;
    }
    (void)exception_code;
    (void)cpu_instruction_run(state, code, bytes, after);
    return x86_core_cpu_is_shutdown(&state->execution) &&
        !state->execution.stop_requested && !state->fault.valid;
}

static void descriptor_set_tables(cpu_instruction_fixture *state,
    lib_u32 gdtr_base, lib_u16 gdtr_limit, lib_u32 idtr_base,
    lib_u16 idtr_limit)
{
    state->cpu.data.gdtr.base = gdtr_base;
    state->cpu.data.gdtr.limit = gdtr_limit;
    state->cpu.data.idtr.base = idtr_base;
    state->cpu.data.idtr.limit = idtr_limit;
}

static lib_i32 descriptor_tables_equal(const t_cpu *first, const t_cpu *second)
{
    return first->data.gdtr.base == second->data.gdtr.base &&
        first->data.gdtr.limit == second->data.gdtr.limit &&
        first->data.idtr.base == second->data.idtr.base &&
        first->data.idtr.limit == second->data.idtr.limit;
}

static void descriptor_enter_protected(cpu_instruction_fixture *state,
    lib_u8 cpl)
{
    t_cpu *cpu = &state->cpu;

    cpu->data.cr0 |= VCPU_CR0_PE;
    cpu->data.cs.selector = (lib_u16)(0x0008u | cpl);
    cpu->data.cs.dpl = cpl;
    cpu->data.cs.base = 0u;
    cpu->data.cs.limit = 0xffffu;
    cpu->data.cs.flagValid = LIB_TRUE;
    cpu->data.cs.sregtype = SREG_CODE;
    cpu->data.cs.seg.executable = LIB_TRUE;
    cpu->data.ds.base = 0u;
    cpu->data.ds.limit = 0xffffu;
    cpu->data.ds.selector = (lib_u16)(0x0010u | cpl);
    cpu->data.ds.flagValid = LIB_TRUE;
    cpu->data.ds.sregtype = SREG_DATA;
    cpu->data.ds.seg.executable = LIB_FALSE;
    cpu->data.ds.seg.data.writable = LIB_TRUE;
    cpu->data.ds.dpl = cpl;
}

static void descriptor_enter_user_protected(cpu_instruction_fixture *state)
{
    descriptor_enter_protected(state, 3u);
    state->cpu.data.cs.selector = 0x004bu;
    state->cpu.data.ds.selector = 0x0053u;
}

static lib_i32 descriptor_sreg_equal(const t_cpu_data_sreg *first,
    const t_cpu_data_sreg *second)
{
    return first->flagValid == second->flagValid &&
        first->selector == second->selector &&
        first->sregtype == second->sregtype && first->base == second->base &&
        first->limit == second->limit && first->dpl == second->dpl &&
        first->sys.type == second->sys.type;
}

static void descriptor_seed_system_sreg(t_cpu_data_sreg *sreg,
    t_cpu_data_sreg_type type, lib_u16 selector)
{
    lib_memory_set(sreg, 0, sizeof(*sreg));
    sreg->flagValid = LIB_TRUE;
    sreg->selector = selector;
    sreg->sregtype = type;
    sreg->base = 0x00000500u;
    sreg->limit = 0x000000ffu;
    sreg->dpl = 0u;
    sreg->sys.type = type == SREG_TR ? VCPU_DESC_SYS_TYPE_TSS_16_BUSY :
        VCPU_DESC_SYS_TYPE_LDT;
}

static lib_i32 descriptor_install_selector_tables(cpu_instruction_fixture *state)
{
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0,0,0x9au,0,0,
        0xffu,0xffu,0,0,0,0x92u,0,0,
        0xffu,0,0,0x05u,0,0x82u,0,0,
        0x2bu,0,0,0x06u,0,0x81u,0,0,
        0xffu,0,0,0x07u,0,0x02u,0,0,
        0x2bu,0,0,0x08u,0,0x83u,0,0,
        0x2bu,0,0,0x09u,0,0x01u,0,0,
        0x67u,0,0,0x0au,0,0x89u,0,0,
        0xffu,0xffu,0,0,0,0xfau,0,0,
        0xffu,0xffu,0,0,0,0xf2u,0,0
    };

    descriptor_set_tables(state, DESCRIPTOR_GDT_ADDRESS,
        (lib_u16)(sizeof(gdt) - 1u), 0u, 0u);
    lib_memory_copy(state->memory + DESCRIPTOR_GDT_ADDRESS, gdt, sizeof(gdt));
    return 1;
}

static lib_i32 descriptor_test_selector_stores(x86_core_cpu_profile profile)
{
    static const lib_u8 register_code[][5] = {
        {0x66u,0x0fu,0x00u,0xc0u,0xf4u},
        {0x66u,0x0fu,0x00u,0xc8u,0xf4u}
    };
    static const lib_u8 memory_code[][6] = {
        {0x0fu,0x00u,0x06u,0x00u,0x02u,0xf4u},
        {0x0fu,0x00u,0x0eu,0x00u,0x02u,0xf4u}
    };
    const lib_u16 selectors[] = {
        DESCRIPTOR_LDT_SELECTOR, DESCRIPTOR_TSS16_SELECTOR
    };
    lib_size index;

    for (index = 0u; index < 2u; ++index) {
        cpu_instruction_fixture state;
        t_cpu after;

        cpu_instruction_prepare(&state, profile);
        descriptor_enter_user_protected(&state);
        descriptor_seed_system_sreg(&state.cpu.data.ldtr, SREG_LDTR,
            DESCRIPTOR_LDT_SELECTOR);
        descriptor_seed_system_sreg(&state.cpu.data.tr, SREG_TR,
            DESCRIPTOR_TSS16_SELECTOR);
        state.cpu.data.eax = 0xdeadbeefu;
        if (!descriptor_run(&state, register_code[index],
                sizeof(register_code[index]), &after) ||
            after.data.eax != (0xdead0000u | selectors[index])) return 0;
    }
    for (index = 0u; index < 2u; ++index) {
        cpu_instruction_fixture state;
        t_cpu after;
        lib_u16 observed = 0u;

        cpu_instruction_prepare(&state, profile);
        descriptor_enter_protected(&state, 0u);
        descriptor_seed_system_sreg(&state.cpu.data.ldtr, SREG_LDTR,
            DESCRIPTOR_LDT_SELECTOR);
        descriptor_seed_system_sreg(&state.cpu.data.tr, SREG_TR,
            DESCRIPTOR_TSS16_SELECTOR);
        if (!descriptor_run(&state, memory_code[index],
                sizeof(memory_code[index]), &after)) return 0;
        lib_memory_copy(&observed, state.memory + 0x0200u, sizeof(observed));
        if (observed != selectors[index]) return 0;
    }
    return 1;
}

static lib_i32 descriptor_test_selector_loads(x86_core_cpu_profile profile)
{
    static const lib_u8 lldt[] = {0x0fu,0x00u,0xd0u,0xf4u};
    static const lib_u8 ltr[] = {0x0fu,0x00u,0xd8u,0xf4u};
    static const lib_u8 memory_load_code[][7] = {
        {0x66u,0x0fu,0x00u,0x16u,0x40u,0x02u,0xf4u},
        {0x66u,0x0fu,0x00u,0x1eu,0x40u,0x02u,0xf4u}
    };
    static const lib_u8 real_code[][4] = {
        {0x0fu,0x00u,0xc0u,0u}, {0x0fu,0x00u,0xc8u,0u},
        {0x0fu,0x00u,0xd0u,0u}, {0x0fu,0x00u,0xd8u,0u}
    };
    static const lib_u16 lldt_selectors[] = {
        0x0004u, DESCRIPTOR_TSS16_SELECTOR, DESCRIPTOR_LDT_NOT_PRESENT_SELECTOR
    };
    static const lib_u32 lldt_exceptions[] = {
        VCPUINS_EXCEPT_GP, VCPUINS_EXCEPT_GP, VCPUINS_EXCEPT_NP
    };
    static const lib_u16 ltr_selectors[] = {
        0x0000u, 0x0004u, DESCRIPTOR_LDT_SELECTOR,
        DESCRIPTOR_TSS16_BUSY_SELECTOR, DESCRIPTOR_TSS16_NOT_PRESENT_SELECTOR
    };
    static const lib_u32 ltr_exceptions[] = {
        VCPUINS_EXCEPT_GP, VCPUINS_EXCEPT_GP, VCPUINS_EXCEPT_GP,
        VCPUINS_EXCEPT_GP, VCPUINS_EXCEPT_NP
    };
    lib_size index;

    for (index = 0u; index < 4u; ++index) {
        cpu_instruction_fixture state;
        t_cpu before, after;

        cpu_instruction_prepare(&state, profile);
        before = state.cpu;
        if (!descriptor_run_fault(&state, real_code[index], 3u,
                VCPUINS_EXCEPT_UD, 0u, &after) ||
            !descriptor_sreg_equal(&before.data.ldtr, &after.data.ldtr) ||
            !descriptor_sreg_equal(&before.data.tr, &after.data.tr))
            return 0;
    }
    {
        cpu_instruction_fixture state;
        t_cpu after;

        cpu_instruction_prepare(&state, profile);
        descriptor_enter_protected(&state, 0u);
        if (!descriptor_install_selector_tables(&state)) return 0;
        state.cpu.data.eax = 0xffff0000u | DESCRIPTOR_LDT_SELECTOR;
        if (!descriptor_run(&state, lldt, sizeof(lldt), &after) ||
            !after.data.ldtr.flagValid ||
            after.data.ldtr.selector != DESCRIPTOR_LDT_SELECTOR ||
            after.data.ldtr.base != 0x00000500u ||
            after.data.ldtr.limit != 0x000000ffu ||
            after.data.ldtr.sys.type != VCPU_DESC_SYS_TYPE_LDT) return 0;
    }
    {
        cpu_instruction_fixture state;
        t_cpu after;

        cpu_instruction_prepare(&state, profile);
        descriptor_enter_protected(&state, 0u);
        if (!descriptor_install_selector_tables(&state)) return 0;
        state.cpu.data.eax = 0xffff0000u;
        if (!descriptor_run(&state, lldt, sizeof(lldt), &after) ||
            after.data.ldtr.flagValid || after.data.ldtr.selector != 0u) return 0;
    }
    for (index = 0u; index < 2u; ++index) {
        cpu_instruction_fixture state;
        t_cpu after;
        const lib_u16 selector = index == 0u ? DESCRIPTOR_LDT_SELECTOR :
            DESCRIPTOR_TSS16_SELECTOR;

        cpu_instruction_prepare(&state, profile);
        descriptor_enter_protected(&state, 0u);
        if (!descriptor_install_selector_tables(&state)) return 0;
        lib_memory_copy(state.memory + DESCRIPTOR_LOAD_ADDRESS, &selector,
            sizeof(selector));
        if (!descriptor_run(&state, memory_load_code[index],
                sizeof(memory_load_code[index]), &after)) return 0;
        if (index == 0u) {
            if (!after.data.ldtr.flagValid || after.data.ldtr.selector != selector)
                return 0;
        } else if (!after.data.tr.flagValid || after.data.tr.selector != selector)
            return 0;
    }
    for (index = 0u; index < 3u; ++index) {
        cpu_instruction_fixture state;
        t_cpu before, after;

        cpu_instruction_prepare(&state, profile);
        descriptor_enter_protected(&state, 0u);
        if (!descriptor_install_selector_tables(&state)) return 0;
        descriptor_seed_system_sreg(&state.cpu.data.ldtr, SREG_LDTR,
            DESCRIPTOR_LDT_SELECTOR);
        state.cpu.data.eax = 0xbeef0000u | lldt_selectors[index];
        before = state.cpu;
        if (!descriptor_run_fault(&state, lldt, sizeof(lldt),
                lldt_exceptions[index], lldt_selectors[index], &after) ||
            !descriptor_sreg_equal(&before.data.ldtr, &after.data.ldtr)) return 0;
    }
    for (index = 0u; index < 5u; ++index) {
        cpu_instruction_fixture state;
        t_cpu before, after;
        lib_u8 before_access = 0u;
        lib_u8 after_access = 0u;

        cpu_instruction_prepare(&state, profile);
        descriptor_enter_protected(&state, 0u);
        if (!descriptor_install_selector_tables(&state)) return 0;
        descriptor_seed_system_sreg(&state.cpu.data.tr, SREG_TR,
            DESCRIPTOR_TSS16_SELECTOR);
        state.cpu.data.eax = 0xbeef0000u | ltr_selectors[index];
        if (ltr_selectors[index] != 0u &&
            (ltr_selectors[index] & 0x0004u) == 0u) {
            before_access = state.memory[DESCRIPTOR_GDT_ADDRESS +
                ltr_selectors[index] + 5u];
        }
        before = state.cpu;
        if (!descriptor_run_fault(&state, ltr, sizeof(ltr),
                ltr_exceptions[index], ltr_selectors[index], &after)) return 0;
        if (ltr_selectors[index] != 0u &&
            (ltr_selectors[index] & 0x0004u) == 0u) {
            after_access = state.memory[DESCRIPTOR_GDT_ADDRESS +
                ltr_selectors[index] + 5u];
            if (before_access != after_access) return 0;
        }
        if (!descriptor_sreg_equal(&before.data.tr, &after.data.tr)) return 0;
    }
    return 1;
}

static lib_i32 descriptor_test_store_layout(x86_core_cpu_profile profile)
{
    static const lib_u8 code[][7] = {
        {0x0fu,0x01u,0x06u,0x00u,0x02u,0xf4u,0u},
        {0x0fu,0x01u,0x0eu,0x00u,0x02u,0xf4u,0u},
        {0x66u,0x0fu,0x01u,0x06u,0x00u,0x02u,0xf4u},
        {0x66u,0x0fu,0x01u,0x0eu,0x00u,0x02u,0xf4u}
    };
    static const lib_u8 expected[][6] = {
        {0x34u,0x12u,0x12u,0xdeu,0xbcu,0x7au},
        {0x78u,0x56u,0x78u,0x56u,0x34u,0x12u},
        {0x34u,0x12u,0x12u,0xdeu,0xbcu,0x7au},
        {0x78u,0x56u,0x78u,0x56u,0x34u,0x12u}
    };
    lib_size index;

    for (index = 0u; index < 4u; ++index) {
        cpu_instruction_fixture state;
        t_cpu after;
        lib_u8 observed[6] = {0};

        cpu_instruction_prepare(&state, profile);
        descriptor_set_tables(&state, 0x7abcde12u, 0x1234u, 0x12345678u,
            0x5678u);
        if (!descriptor_run(&state, code[index],
                (lib_u8)(sizeof(code[index]) - (index < 2u)), &after)) return 0;
        lib_memory_copy(observed, state.memory + 0x0200u, sizeof(observed));
        if (lib_memory_compare(observed, expected[index], sizeof(observed)) != 0) {
            lib_c_fprintf(lib_c_stderr,
                "M5::DESCRIPTOR store index=%u got=%02x%02x%02x%02x%02x%02x want=%02x%02x%02x%02x%02x%02x\\n",
                (unsigned)index, observed[0], observed[1], observed[2],
                observed[3], observed[4], observed[5], expected[index][0],
                expected[index][1], expected[index][2], expected[index][3],
                expected[index][4], expected[index][5]);
            return 0;
        }
    }
    return 1;
}

static lib_i32 descriptor_test_protected_stores(x86_core_cpu_profile profile)
{
    static const lib_u8 code[][7] = {
        {0x66u,0x0fu,0x01u,0x06u,0x00u,0x02u,0xf4u},
        {0x66u,0x0fu,0x01u,0x0eu,0x00u,0x02u,0xf4u},
        {0x0fu,0x01u,0x06u,0x00u,0x02u,0xf4u,0u},
        {0x0fu,0x01u,0x0eu,0x00u,0x02u,0xf4u,0u}
    };
    static const lib_u8 expected[][6] = {
        {0x34u,0x12u,0x12u,0xdeu,0xbcu,0x7au},
        {0x78u,0x56u,0x78u,0x56u,0x34u,0x12u},
        {0x34u,0x12u,0x12u,0xdeu,0xbcu,0x7au},
        {0x78u,0x56u,0x78u,0x56u,0x34u,0x12u}
    };
    lib_size index;

    for (lib_u8 mode = 0u; mode < 5u; ++mode)
    for (index = 0u; index < sizeof(code) / sizeof(code[0]); ++index) {
        cpu_instruction_fixture state;
        t_cpu before, after;
        lib_u8 observed[8] = {0};
        const lib_u8 bytes = (lib_u8)(sizeof(code[index]) - (index >= 2u));

        cpu_instruction_prepare(&state, profile);
        descriptor_set_tables(&state, 0x7abcde12u, 0x1234u, 0x12345678u,
            0x5678u);
        descriptor_enter_protected(&state, mode < 4u ? mode : 3u);
        state.cpu.data.ss = state.cpu.data.ds;
        state.cpu.data.ss.sregtype = SREG_STACK;
        if (mode == 4u) {
            state.cpu.data.eflags |= VCPU_EFLAGS_VM;
            state.cpu.data.cs.selector = 0u;
            state.cpu.data.ds.selector = 0u;
            state.cpu.data.ss.selector = 0u;
        }
        lib_memory_set(state.memory + 0x01ffu, 0xa5, sizeof(observed));
        before = state.cpu;
        before.data.eip = bytes - 1u;
        if (!descriptor_run(&state, code[index], bytes, &after))
            return 0;
        lib_memory_copy(observed, state.memory + 0x01ffu, sizeof(observed));
        if (observed[0] != 0xa5u || observed[7] != 0xa5u ||
            lib_memory_compare(observed + 1u, expected[index],
                sizeof(expected[index])) != 0 ||
            lib_memory_compare(&before, &after, sizeof(before)) != 0) {
            lib_c_fprintf(lib_c_stderr,
                "descriptor store CPU=%u mode=%u form=%u ip=%u\n",
                profile, mode, (lib_u32)index, after.data.eip);
            return 0;
        }
    }
    return 1;
}

static lib_i32 descriptor_test_load_layout(x86_core_cpu_profile profile)
{
    static const lib_u8 code[][7] = {
        {0x0fu,0x01u,0x16u,0x40u,0x02u,0xf4u,0u},
        {0x0fu,0x01u,0x1eu,0x40u,0x02u,0xf4u,0u},
        {0x66u,0x0fu,0x01u,0x16u,0x40u,0x02u,0xf4u},
        {0x66u,0x0fu,0x01u,0x1eu,0x40u,0x02u,0xf4u}
    };
    static const lib_u8 source[] = {0xbcu,0x9au,0x78u,0x56u,0x34u,0x12u};
    lib_size index;

    for (index = 0u; index < 4u; ++index) {
        cpu_instruction_fixture state;
        t_cpu after;

        cpu_instruction_prepare(&state, profile);
        lib_memory_copy(state.memory + DESCRIPTOR_LOAD_ADDRESS, source,
            sizeof(source));
        if (!descriptor_run(&state, code[index],
                (lib_u8)(sizeof(code[index]) - (index < 2u)), &after)) return 0;
        if (index == 0u || index == 2u) {
            if (after.data.gdtr.limit != 0x9abcu || after.data.gdtr.base !=
                (index == 0u ? 0x00345678u : 0x12345678u)) return 0;
        } else if (after.data.idtr.limit != 0x9abcu || after.data.idtr.base !=
            (index == 1u ? 0x00345678u : 0x12345678u)) return 0;
    }
    for (index = 0u; index < 2u; ++index) {
        cpu_instruction_fixture state;
        t_cpu after;

        cpu_instruction_prepare(&state, profile);
        descriptor_enter_protected(&state, 0u);
        lib_memory_copy(state.memory + DESCRIPTOR_LOAD_ADDRESS, source,
            sizeof(source));
        if (!descriptor_run(&state, code[index],
                (lib_u8)(sizeof(code[index]) - 1u), &after)) return 0;
        if (index == 0u) {
            if (after.data.gdtr.limit != 0x9abcu ||
                after.data.gdtr.base != 0x00345678u) return 0;
        } else if (after.data.idtr.limit != 0x9abcu ||
            after.data.idtr.base != 0x00345678u) return 0;
    }
    return 1;
}

static lib_i32 descriptor_test_register_and_privilege_faults(x86_core_cpu_profile profile)
{
    static const lib_u8 register_code[][4] = {
        {0x0fu,0x01u,0xc0u,0u}, {0x0fu,0x01u,0xc8u,0u},
        {0x0fu,0x01u,0xd0u,0u}, {0x0fu,0x01u,0xd8u,0u}
    };
    static const lib_u8 load_code[][6] = {
        {0x0fu,0x01u,0x16u,0x40u,0x02u,0xf4u},
        {0x0fu,0x01u,0x1eu,0x40u,0x02u,0xf4u}
    };
    static const lib_u8 source[] = {0xbcu,0x9au,0x78u,0x56u,0x34u,0x12u};
    lib_size index;

    for (index = 0u; index < 4u; ++index) {
        cpu_instruction_fixture state;
        t_cpu before, after;

        cpu_instruction_prepare(&state, profile);
        /* Real #UD delivery needs a real-mode IVT; retain the unrelated GDTR
         * value so the rejected form still proves it is not modified. */
        descriptor_set_tables(&state, 0x11112222u, 0x3333u, 0u, 0x03ffu);
        before = state.cpu;
        after = before;
        if (!descriptor_run_fault(&state, register_code[index], 3u,
                VCPUINS_EXCEPT_UD, 0u, &after) ||
            !descriptor_tables_equal(&before, &after)) return 0;
    }
    for (index = 0u; index < 2u; ++index) {
        cpu_instruction_fixture state;
        t_cpu before, after;

        cpu_instruction_prepare(&state, profile);
        descriptor_set_tables(&state, 0x11112222u, 0x3333u, 0u, 0u);
        descriptor_enter_protected(&state, 3u);
        lib_memory_copy(state.memory + DESCRIPTOR_LOAD_ADDRESS, source,
            sizeof(source));
        before = state.cpu;
        after = before;
        if (!descriptor_run_fault(&state, load_code[index],
                sizeof(load_code[index]), VCPUINS_EXCEPT_GP, 0u, &after) ||
            !descriptor_tables_equal(&before, &after)) return 0;
    }
    return 1;
}

static lib_i32 descriptor_test_memory_faults_preserve_tables(x86_core_cpu_profile profile)
{
    static const lib_u8 store_code[][6] = {
        {0x0fu,0x01u,0x06u,0x00u,0x02u,0xf4u},
        {0x0fu,0x01u,0x0eu,0x00u,0x02u,0xf4u}
    };
    static const lib_u8 load_code[][6] = {
        {0x0fu,0x01u,0x16u,0x00u,0x02u,0xf4u},
        {0x0fu,0x01u,0x1eu,0x00u,0x02u,0xf4u}
    };
    lib_size index;

    for (index = 0u; index < 2u; ++index) {
        cpu_instruction_fixture state;
        t_cpu before, after;

        cpu_instruction_prepare(&state, profile);
        descriptor_set_tables(&state, 0x11112222u, 0x3333u, 0u, 0u);
        descriptor_enter_protected(&state, 0u);
        state.cpu.data.ds.limit = 0x01ffu;
        before = state.cpu;
        if (!descriptor_run_fault(&state, store_code[index],
                sizeof(store_code[index]), VCPUINS_EXCEPT_GP, 0u, &after) ||
            !descriptor_tables_equal(&before, &after)) return 0;
    }
    for (index = 0u; index < 2u; ++index) {
        cpu_instruction_fixture state;
        t_cpu before, after;

        cpu_instruction_prepare(&state, profile);
        descriptor_set_tables(&state, 0x11112222u, 0x3333u, 0u, 0u);
        descriptor_enter_protected(&state, 0u);
        state.cpu.data.ds.limit = 0x01ffu;
        before = state.cpu;
        if (!descriptor_run_fault(&state, load_code[index],
                sizeof(load_code[index]), VCPUINS_EXCEPT_GP, 0u, &after) ||
            !descriptor_tables_equal(&before, &after)) return 0;
    }
    return 1;
}

static lib_i32 descriptor_test_c7_segment_override_real_mode(void)
{
    static const lib_u8 code[] = {
        0x26u,0xc7u,0x47u,0x02u,0xffu,0xffu,0xf4u
    };
    cpu_instruction_fixture state;
    t_cpu after;
    lib_u16 observed = 0u;

    cpu_instruction_prepare(&state, X86_CORE_CPU_PROFILE_80386);
    state.cpu.data.ebx = 0u;
    if (!descriptor_run(&state, code, sizeof(code), &after)) return 0;
    lib_memory_copy(&observed, state.memory + 2u, sizeof(observed));
    return observed == 0xffffu;
}

static lib_i32 descriptor_test_paged_physical_gdt(void)
{
    static const lib_u8 code[] = {0x8eu,0xd8u,0xf4u};
    static const lib_u8 gdt[] = {
        0u,0u,0u,0u,0u,0u,0u,0u,
        0xffu,0xffu,0u,0u,0u,0x9au,0u,0u,
        0xffu,0xffu,0u,0u,0u,0x92u,0u,0u
    };
    cpu_instruction_fixture state;
    t_cpu after;

    cpu_instruction_prepare(&state, X86_CORE_CPU_PROFILE_80386);
    descriptor_enter_protected(&state, 0u);
    descriptor_set_tables(&state, 0x00008000u, (lib_u16)(sizeof(gdt) - 1u),
        0u, 0u);
    lib_memory_copy(state.memory, code, sizeof(code));
    lib_memory_copy(state.memory + 0x8000u, gdt, sizeof(gdt));
    lib_memory_copy(state.memory + 0x1000u, (const lib_u32[]){0x00002003u},
        sizeof(lib_u32));
    lib_memory_copy(state.memory + 0x2000u, (const lib_u32[]){0x00000003u},
        sizeof(lib_u32));
    state.cpu.data.eax = 0x00000010u;
    state.cpu.data.cr3 = 0x00001000u;
    state.cpu.data.cr0 |= VCPU_CR0_PG;
    return descriptor_run(&state, code, sizeof(code), &after) &&
        after.data.ds.selector == 0x0010u &&
        state.memory[0x8000u + 16u + 5u] == 0x93u &&
        state.memory[0x2000u + 8u * sizeof(lib_u32)] == 0u;
}

static lib_bool descriptor_test_selector_fault_vectors(x86_core_cpu_profile profile)
{
    static const struct {
        lib_u16 selector, error;
        lib_u8 extension, vector;
    } cases[] = {
        {0x23u,0x20u,2u,13u}, {0x2bu,0x28u,2u,11u},
        {0x1fu,0x1cu,2u,13u}, {0x1bu,0x18u,3u,13u},
        {0x33u,0x30u,3u,13u}, {0x3bu,0x38u,3u,11u},
        {0x47u,0x44u,3u,13u}, {3u,0u,3u,13u},
        {0x5bu,0x58u,2u,13u}, {0x5bu,0x58u,3u,13u}
    };
    const lib_u8 gate[] = {0u,0x20u,8u,0u,0u,0x8eu,0u,0u};

    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        for (lib_u8 form = 0u; form < 4u; ++form) {
            cpu_instruction_fixture state;
            t_cpu before, after;
            lib_u8 code[6] = {0};
            lib_u8 bytes = 0u;
            lib_u8 descriptors[0x58];
            lib_u32 frame[4] = {0};

            cpu_instruction_prepare(&state, profile);
            descriptor_enter_protected(&state, 0u);
            if (!descriptor_install_selector_tables(&state)) return LIB_FALSE;
            descriptor_seed_system_sreg(&state.cpu.data.ldtr, SREG_LDTR,
                DESCRIPTOR_LDT_SELECTOR);
            descriptor_seed_system_sreg(&state.cpu.data.tr, SREG_TR,
                DESCRIPTOR_TSS16_SELECTOR);
            state.cpu.data.ss = state.cpu.data.ds;
            state.cpu.data.ss.sregtype = SREG_STACK;
            state.cpu.data.esp = 0x8000u;
            state.cpu.data.idtr = (t_cpu_data_sreg){.flagValid = LIB_TRUE,
                .sregtype = SREG_IDTR, .base = 0x1000u, .limit = 0x7fu};
            lib_memory_copy(state.memory + 0x1000u + cases[index].vector * 8u,
                gate, sizeof(gate));
            state.cpu.data.eax = 0xa5a50000u | cases[index].selector;
            state.cpu.data.eflags = 2u | VCPU_EFLAGS_IF | VCPU_EFLAGS_DF |
                VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
            state.cpu.data.eip = 0x3000u;
            lib_memory_copy(state.memory + 0x5000u, &cases[index].selector, 2u);
            if (form & 1u) code[bytes++] = 0x66u;
            code[bytes++] = 0x0fu;
            code[bytes++] = 0u;
            code[bytes++] = (lib_u8)((cases[index].extension << 3u) |
                (form < 2u ? 0xc0u : 6u));
            if (form >= 2u) {
                code[bytes++] = 0u;
                code[bytes++] = 0x50u;
            }
            lib_memory_copy(state.memory + 0x3000u, code, bytes);
            lib_memory_copy(descriptors, state.memory + DESCRIPTOR_GDT_ADDRESS,
                sizeof(descriptors));
            before = state.cpu;
            x86_core_cpu_execution_refresh(&state.execution);
            after = state.cpu;
            lib_memory_copy(frame, state.memory + after.data.ss.base + after.data.sp,
                sizeof(frame));
            /* Exception entry legitimately sets the handler's Code Accessed bit. */
            descriptors[13u] |= 1u;
            if (state.execution.stop_requested || state.fault.valid ||
                x86_core_cpu_is_shutdown(&state.execution) ||
                !state.delivered_exception.valid ||
                state.delivered_exception.exception_mask != (UINT32_C(1) << cases[index].vector) ||
                state.delivered_exception.exception_code != cases[index].error ||
                state.delivered_exception.point.eip != before.data.eip ||
                state.delivered_exception.point.cs != before.data.cs.selector ||
                after.data.eip != 0x2000u || after.data.cs.selector != 8u ||
                after.data.eax != before.data.eax || after.data.esp != 0x7ff0u ||
                after.data.ecx != before.data.ecx || after.data.edx != before.data.edx ||
                after.data.ebx != before.data.ebx || after.data.ebp != before.data.ebp ||
                after.data.esi != before.data.esi || after.data.edi != before.data.edi ||
                after.data.eflags != (before.data.eflags & ~VCPU_EFLAGS_IF) ||
                !descriptor_tables_equal(&before, &after) ||
                !descriptor_sreg_equal(&before.data.ldtr, &after.data.ldtr) ||
                !descriptor_sreg_equal(&before.data.tr, &after.data.tr) ||
                frame[0] != cases[index].error || frame[1] != 0x3000u || frame[2] != 8u ||
                (frame[3] & ~VCPU_EFLAGS_RF) != before.data.eflags ||
                lib_memory_compare(descriptors, state.memory + DESCRIPTOR_GDT_ADDRESS,
                    sizeof(descriptors)) != 0) {
                lib_c_fprintf(lib_c_stderr,
                    "descriptor vector CPU=%u case=%u form=%u mask=%08x error=%u ip=%u\n",
                    profile, (lib_u32)index, form, state.delivered_exception.exception_mask,
                    state.delivered_exception.exception_code, after.data.eip);
                return LIB_FALSE;
            }
        }
    }
    return LIB_TRUE;
}

static lib_bool descriptor_test_privileged_load_vectors(x86_core_cpu_profile profile)
{
    const lib_u8 gate[] = {0u,0x20u,8u,0u,0u,0x8eu,0u,0u};
    const lib_u8 source[] = {0x18u,0u,0x78u,0x56u,0x34u,0x12u};

    for (lib_u8 mode = 1u; mode <= 4u; ++mode)
    for (lib_u8 operation = 0u; operation < 4u; ++operation)
    for (lib_u8 form = 0u; form < 4u; ++form) {
        cpu_instruction_fixture state;
        t_cpu before, after;
        lib_u8 code[9], bytes = 0u;
        lib_u32 frame[10] = {0};
        const lib_bool vm86 = mode == 4u;
        const lib_u8 cpl = vm86 ? 3u : mode;
        const lib_u8 vector = vm86 && operation < 2u ? 6u : 13u;
        const lib_u8 error_words = vector == 13u ? 1u : 0u;
        const lib_u8 frame_words = (lib_u8)((vm86 ? 9u : 5u) + error_words);
        const lib_u32 stack = 0x9000u;
        const lib_u16 stack_selector = 0x10u;

        cpu_instruction_prepare(&state, profile);
        descriptor_enter_protected(&state, cpl);
        if (!descriptor_install_selector_tables(&state)) return LIB_FALSE;
        state.memory[DESCRIPTOR_GDT_ADDRESS + 9u * 8u + 5u] =
            (lib_u8)(0x9au | (cpl << 5u));
        state.memory[DESCRIPTOR_GDT_ADDRESS + 10u * 8u + 5u] =
            (lib_u8)(0x92u | (cpl << 5u));
        state.cpu.data.cs.selector = (lib_u16)(0x48u | cpl);
        state.cpu.data.ds.selector = (lib_u16)(0x50u | cpl);
        state.cpu.data.ss = state.cpu.data.ds;
        state.cpu.data.ss.sregtype = SREG_STACK;
        descriptor_seed_system_sreg(&state.cpu.data.ldtr, SREG_LDTR, 0x18u);
        descriptor_seed_system_sreg(&state.cpu.data.tr, SREG_TR, 0x40u);
        state.cpu.data.tr.base = 0x0a00u;
        state.cpu.data.tr.limit = 0x67u;
        state.cpu.data.tr.sys.type = VCPU_DESC_SYS_TYPE_TSS_32_BUSY;
        lib_memory_copy(state.memory + 0x0a04u, &stack, sizeof(stack));
        lib_memory_copy(state.memory + 0x0a08u, &stack_selector, sizeof(stack_selector));
        state.cpu.data.idtr = (t_cpu_data_sreg){.flagValid = LIB_TRUE,
            .sregtype = SREG_IDTR, .base = 0x1000u, .limit = 0x7fu};
        lib_memory_copy(state.memory + 0x1000u + vector * 8u, gate, sizeof(gate));
        state.cpu.data.eip = 0x3000u;
        state.cpu.data.esp = 0x8000u;
        state.cpu.data.eax = 0xa5a50018u;
        state.cpu.data.eflags = 2u | VCPU_EFLAGS_IF | VCPU_EFLAGS_OF |
            VCPU_EFLAGS_CF | VCPU_EFLAGS_DF;
        if (vm86) {
            state.cpu.data.eflags |= VCPU_EFLAGS_VM;
            state.cpu.data.cs.selector = 0u;
            state.cpu.data.ds.selector = 0u;
            state.cpu.data.ss.selector = 0u;
        }
        if (form & 1u) code[bytes++] = 0x66u;
        if (form & 2u) code[bytes++] = 0x67u;
        code[bytes++] = 0x0fu;
        code[bytes++] = operation < 2u ? 0u : 1u;
        code[bytes++] = (lib_u8)(((2u + (operation & 1u)) << 3u) |
            (form & 2u ? 5u : 6u));
        code[bytes++] = 0u;
        code[bytes++] = 0x50u;
        if (form & 2u) {
            code[bytes++] = 0u;
            code[bytes++] = 0u;
        }
        lib_memory_copy(state.memory + 0x3000u, code, bytes);
        lib_memory_copy(state.memory + 0x5000u, source, sizeof(source));
        before = state.cpu;
        x86_core_cpu_execution_refresh(&state.execution);
        after = state.cpu;
        if (state.execution.stop_requested || state.fault.valid ||
            x86_core_cpu_is_shutdown(&state.execution) ||
            !state.delivered_exception.valid ||
            state.delivered_exception.exception_mask != (UINT32_C(1) << vector) ||
            state.delivered_exception.exception_code != 0u ||
            state.delivered_exception.point.eip != before.data.eip ||
            state.delivered_exception.point.cs != before.data.cs.selector ||
            after.data.eip != 0x2000u || after.data.cs.selector != 8u ||
            after.data.ss.selector != stack_selector || after.data.ss.base != 0u ||
            after.data.esp != stack - frame_words * 4u ||
            after.data.eax != before.data.eax || after.data.ecx != before.data.ecx ||
            after.data.edx != before.data.edx || after.data.ebx != before.data.ebx ||
            after.data.ebp != before.data.ebp || after.data.esi != before.data.esi ||
            after.data.edi != before.data.edi ||
            after.data.eflags != (before.data.eflags & ~(VCPU_EFLAGS_VM | VCPU_EFLAGS_IF)) ||
            !descriptor_tables_equal(&before, &after) ||
            !descriptor_sreg_equal(&before.data.ldtr, &after.data.ldtr) ||
            !descriptor_sreg_equal(&before.data.tr, &after.data.tr) ||
            lib_memory_compare(source, state.memory + 0x5000u, sizeof(source)) != 0) {
            lib_c_fprintf(lib_c_stderr, "descriptor privilege CPU=%u mode=%u op=%u form=%u\n",
                profile, mode, operation, form);
            return LIB_FALSE;
        }
        lib_memory_copy(frame, state.memory + after.data.esp, frame_words * 4u);
        if ((error_words && frame[0] != 0u) ||
            frame[error_words] != before.data.eip ||
            frame[error_words + 1u] != before.data.cs.selector ||
            frame[error_words + 2u] != (before.data.eflags | VCPU_EFLAGS_RF) ||
            frame[error_words + 3u] != before.data.esp ||
            frame[error_words + 4u] != before.data.ss.selector ||
            (vm86 && (frame[error_words + 5u] != before.data.es.selector ||
                frame[error_words + 6u] != before.data.ds.selector ||
                frame[error_words + 7u] != before.data.fs.selector ||
                frame[error_words + 8u] != before.data.gs.selector))) return LIB_FALSE;
    }
    return LIB_TRUE;
}

lib_i32 main(void)
{
    static const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_80386, X86_CORE_CPU_PROFILE_80486SX
    };

    if (!descriptor_test_c7_segment_override_real_mode() ||
        !descriptor_test_paged_physical_gdt()) return 1;
    for (lib_size index = 0u; index < sizeof(profiles) / sizeof(profiles[0]); ++index) {
        const lib_i32 stores = descriptor_test_store_layout(profiles[index]);
        const lib_i32 protected_stores = descriptor_test_protected_stores(profiles[index]);
        const lib_i32 loads = descriptor_test_load_layout(profiles[index]);
        const lib_i32 faults = descriptor_test_register_and_privilege_faults(profiles[index]);
        const lib_i32 memory_faults = descriptor_test_memory_faults_preserve_tables(profiles[index]);
        const lib_i32 selector_stores = descriptor_test_selector_stores(profiles[index]);
        const lib_i32 selector_loads = descriptor_test_selector_loads(profiles[index]);

        if (!stores || !protected_stores || !loads || !faults || !memory_faults ||
            !selector_stores || !selector_loads ||
            !descriptor_test_selector_fault_vectors(profiles[index]) ||
            !descriptor_test_privileged_load_vectors(profiles[index])) {
            lib_c_fprintf(lib_c_stderr,
                "descriptor CPU profile=%u stores=%d protected-stores=%d loads=%d faults=%d memory-faults=%d selector-stores=%d selector-loads=%d\n",
                profiles[index], stores, protected_stores, loads, faults, memory_faults,
                selector_stores, selector_loads);
            return 1;
        }
    }
    lib_c_printf("%s\n", "DESCRIPTOR-SYSTEM-CPU:OK");
    return 0;
}
