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
    core_machine_cpu_execution_refresh(&state->execution);
    *after = state->cpu;
    return !state->execution.stop_requested && !state->fault.valid &&
        after->data.eip == expected_eip;
}

static lib_i32 descriptor_run_fault(cpu_instruction_fixture *state,
    const lib_u8 *code, lib_u8 bytes, lib_u32 exception,
    lib_u32 exception_code, t_cpu *after)
{
    (void)cpu_instruction_run(state, code, bytes, after);
    if (state->execution.cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386 &&
        (state->cpu.data.cr0 & VCPU_CR0_PE) != 0u &&
        (exception == VCPUINS_EXCEPT_TS || exception == VCPUINS_EXCEPT_NP ||
            exception == VCPUINS_EXCEPT_SS || exception == VCPUINS_EXCEPT_GP)) {
        exception = VCPUINS_EXCEPT_DF;
        exception_code = 0u;
    }
    return state->execution.stop_requested && state->fault.valid &&
        (state->fault.exception_mask & exception) != 0u &&
        state->fault.exception_code == exception_code;
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

static lib_i32 descriptor_test_selector_stores(void)
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

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
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

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
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

static lib_i32 descriptor_test_selector_loads(void)
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

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        state.cpu.data.idtr.limit = 0x17u;
        before = state.cpu;
        if (!descriptor_run_fault(&state, real_code[index], 3u,
                VCPUINS_EXCEPT_UD, 0u, &after) ||
            !descriptor_sreg_equal(&before.data.ldtr, &after.data.ldtr) ||
            !descriptor_sreg_equal(&before.data.tr, &after.data.tr)) return 0;
    }
    {
        cpu_instruction_fixture state;
        t_cpu after;

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
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

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
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

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
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

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
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

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
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

static lib_i32 descriptor_test_store_layout(void)
{
    static const lib_u8 code[][7] = {
        {0x0fu,0x01u,0x06u,0x00u,0x02u,0xf4u,0u},
        {0x0fu,0x01u,0x0eu,0x00u,0x02u,0xf4u,0u},
        {0x66u,0x0fu,0x01u,0x06u,0x00u,0x02u,0xf4u},
        {0x66u,0x0fu,0x01u,0x0eu,0x00u,0x02u,0xf4u}
    };
    static const lib_u8 expected[][6] = {
        {0x34u,0x12u,0x12u,0xdeu,0xbcu,0u},
        {0x78u,0x56u,0x78u,0x56u,0x34u,0u},
        {0x34u,0x12u,0x12u,0xdeu,0xbcu,0x7au},
        {0x78u,0x56u,0x78u,0x56u,0x34u,0x12u}
    };
    lib_size index;

    for (index = 0u; index < 4u; ++index) {
        cpu_instruction_fixture state;
        t_cpu after;
        lib_u8 observed[6] = {0};

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        descriptor_set_tables(&state, 0x7abcde12u, 0x1234u, 0x12345678u,
            0x5678u);
        if (!descriptor_run(&state, code[index],
                (lib_u8)(sizeof(code[index]) - (index < 2u)), &after)) return 0;
        lib_memory_copy(observed, state.memory + 0x0200u, sizeof(observed));
        if (lib_memory_compare(observed, expected[index], sizeof(observed)) != 0)
            return 0;
    }
    return 1;
}

static lib_i32 descriptor_test_protected_stores(void)
{
    static const lib_u8 code[][7] = {
        {0x66u,0x0fu,0x01u,0x06u,0x00u,0x02u,0xf4u},
        {0x66u,0x0fu,0x01u,0x0eu,0x00u,0x02u,0xf4u}
    };
    static const lib_u8 expected[][6] = {
        {0x34u,0x12u,0x12u,0xdeu,0xbcu,0x7au},
        {0x78u,0x56u,0x78u,0x56u,0x34u,0x12u}
    };
    lib_size index;

    for (index = 0u; index < 2u; ++index) {
        cpu_instruction_fixture state;
        t_cpu after;
        lib_u8 observed[6] = {0};

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        descriptor_set_tables(&state, 0x7abcde12u, 0x1234u, 0x12345678u,
            0x5678u);
        descriptor_enter_protected(&state, 0u);
        if (!descriptor_run(&state, code[index], sizeof(code[index]), &after))
            return 0;
        lib_memory_copy(observed, state.memory + 0x0200u, sizeof(observed));
        if (lib_memory_compare(observed, expected[index], sizeof(observed)) != 0)
            return 0;
    }
    return 1;
}

static lib_i32 descriptor_test_load_layout(void)
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

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
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

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
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

static lib_i32 descriptor_test_register_and_privilege_faults(void)
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

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        descriptor_set_tables(&state, 0x11112222u, 0x3333u, 0x44445555u,
            0x6666u);
        before = state.cpu;
        if (!descriptor_run_fault(&state, register_code[index], 3u,
                VCPUINS_EXCEPT_UD, 0u, &after) ||
            !descriptor_tables_equal(&before, &after)) return 0;
    }
    for (index = 0u; index < 2u; ++index) {
        cpu_instruction_fixture state;
        t_cpu before, after;

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        descriptor_set_tables(&state, 0x11112222u, 0x3333u, 0u, 0u);
        descriptor_enter_protected(&state, 3u);
        lib_memory_copy(state.memory + DESCRIPTOR_LOAD_ADDRESS, source,
            sizeof(source));
        before = state.cpu;
        if (!descriptor_run_fault(&state, load_code[index],
                sizeof(load_code[index]), VCPUINS_EXCEPT_GP, 0u, &after) ||
            !descriptor_tables_equal(&before, &after)) return 0;
    }
    return 1;
}

static lib_i32 descriptor_test_memory_faults_preserve_tables(void)
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

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
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

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
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

    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    state.cpu.data.ebx = 0u;
    if (!descriptor_run(&state, code, sizeof(code), &after)) return 0;
    lib_memory_copy(&observed, state.memory + 2u, sizeof(observed));
    return observed == 0xffffu;
}

lib_i32 main(void)
{
    const lib_i32 stores = descriptor_test_store_layout();
    const lib_i32 protected_stores = descriptor_test_protected_stores();
    const lib_i32 loads = descriptor_test_load_layout();
    const lib_i32 faults = descriptor_test_register_and_privilege_faults();
    const lib_i32 memory_faults = descriptor_test_memory_faults_preserve_tables();
    const lib_i32 selector_stores = descriptor_test_selector_stores();
    const lib_i32 selector_loads = descriptor_test_selector_loads();
    const lib_i32 c7 = descriptor_test_c7_segment_override_real_mode();

    if (!stores || !protected_stores || !loads || !faults || !memory_faults ||
        !selector_stores || !selector_loads || !c7) {
        lib_c_fprintf(lib_c_stderr, "M5:T539:S43:descriptor cpu failed stores=%d protected-stores=%d loads=%d faults=%d memory-faults=%d selector-stores=%d selector-loads=%d c7=%d\n",
            stores, protected_stores, loads, faults, memory_faults,
            selector_stores, selector_loads, c7);
        return 1;
    }
    lib_c_printf("%s\n", "M5:T539:S43:DESCRIPTOR-SYSTEM-CPU:OK");
    return 0;
}
