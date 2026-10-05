#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

static lib_u32 setcc_flags(lib_u8 condition, lib_i32 truth)
{
    static const lib_u32 true_flags[16] = {
        VCPU_EFLAGS_OF, 0u, VCPU_EFLAGS_CF, 0u,
        VCPU_EFLAGS_ZF, 0u, VCPU_EFLAGS_CF, 0u,
        VCPU_EFLAGS_SF, 0u, VCPU_EFLAGS_PF, 0u,
        VCPU_EFLAGS_SF, 0u, VCPU_EFLAGS_ZF, 0u
    };
    static const lib_u32 false_flags[16] = {
        0u, VCPU_EFLAGS_OF, 0u, VCPU_EFLAGS_CF,
        0u, VCPU_EFLAGS_ZF, 0u, VCPU_EFLAGS_CF,
        0u, VCPU_EFLAGS_SF, 0u, VCPU_EFLAGS_PF,
        0u, VCPU_EFLAGS_SF, 0u, VCPU_EFLAGS_ZF
    };

    return truth ? true_flags[condition] : false_flags[condition];
}

static lib_i32 setcc_test_register_conditions(void)
{
    lib_u8 condition;
    lib_i32 truth;

    for (condition = 0u; condition != 16u; ++condition)
    for (truth = 0; truth != 2; ++truth) {
        const lib_u8 code[] = {0x0fu, (lib_u8)(0x90u + condition), 0xc0u};
        const lib_u32 flags = setcc_flags(condition, truth);
        cpu_instruction_fixture state;
        t_cpu after;

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        state.cpu.data.eax = 0x123456a5u;
        state.cpu.data.eflags = flags;
        if (cpu_instruction_run(&state, code, sizeof(code), &after) !=
                LIB_STATUS_OK || state.fault.valid ||
            after.data.eax != (0x12345600u | (lib_u32)truth) ||
            after.data.eflags != flags || after.data.eip != sizeof(code)) return 0;
    }
    return 1;
}

static lib_i32 setcc_test_memory_conditions(void)
{
    const lib_u16 destination = 0x1200u;
    lib_u8 condition;
    lib_i32 truth;

    for (condition = 0u; condition != 16u; ++condition)
    for (truth = 0; truth != 2; ++truth) {
        const lib_u8 code[] = {0x0fu, (lib_u8)(0x90u + condition), 0x06u,
            (lib_u8)destination, (lib_u8)(destination >> 8u)};
        const lib_u32 flags = setcc_flags(condition, truth);
        const lib_u8 initial = 0xa5u;
        lib_u8 value = 0u;
        cpu_instruction_fixture state;
        t_cpu after;

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        state.cpu.data.eax = 0x87654321u;
        state.cpu.data.eflags = flags;
        if (cpu_instruction_write(&state, destination, &initial, sizeof(initial),
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
            cpu_instruction_run(&state, code, sizeof(code), &after) !=
                LIB_STATUS_OK || state.fault.valid ||
            cpu_instruction_read(&state, destination, &value, sizeof(value),
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) !=
                LIB_STATUS_OK || value != (lib_u8)truth ||
            after.data.eax != 0x87654321u || after.data.eflags != flags ||
            after.data.eip != sizeof(code)) return 0;
    }
    return 1;
}

static lib_i32 setcc_test_prefix_forms(void)
{
    static const lib_u8 operand_prefix[] = {0x66u, 0x0fu, 0x94u, 0xc0u};
    static const lib_u8 address_prefix[] = {0x67u, 0x0fu, 0x94u, 0x06u};
    const lib_u32 flags = VCPU_EFLAGS_ZF | VCPU_EFLAGS_CF;
    const lib_u32 address = 0x00002345u;
    const lib_u8 initial = 0xa5u;
    lib_u8 value = 0u;
    cpu_instruction_fixture state;
    t_cpu after;

    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    state.cpu.data.eax = 0x112233a5u;
    state.cpu.data.eflags = flags;
    if (cpu_instruction_run(&state, operand_prefix, sizeof(operand_prefix),
            &after) != LIB_STATUS_OK || state.fault.valid ||
        after.data.eax != 0x11223301u || after.data.eflags != flags ||
        after.data.eip != sizeof(operand_prefix)) return 0;

    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    state.cpu.data.esi = address;
    state.cpu.data.eflags = flags;
    if (cpu_instruction_write(&state, address, &initial, sizeof(initial),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
        cpu_instruction_run(&state, address_prefix, sizeof(address_prefix),
            &after) != LIB_STATUS_OK || state.fault.valid ||
        cpu_instruction_read(&state, address, &value, sizeof(value),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) !=
            LIB_STATUS_OK || value != 1u || after.data.esi != address ||
        after.data.eflags != flags ||
        after.data.eip != sizeof(address_prefix)) return 0;
    return 1;
}

/* T337_REAL_UD_TERMINAL_CPU_OWNER: SETcc before 386 is CPU-owned #UD. */
static lib_i32 setcc_test_pre_fault_nonpublication(void)
{
    static const lib_u8 ud_code[] = {0x0fu, 0x94u, 0xc0u};
    const lib_u32 flags = VCPU_EFLAGS_ZF | VCPU_EFLAGS_OF;
    cpu_instruction_fixture state;
    t_cpu after;

    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80286);
    state.cpu.data.eax = 0x556677a5u;
    state.cpu.data.eflags = flags;
    state.cpu.data.idtr.limit = 0x17u;
    if (cpu_instruction_run(&state, ud_code, sizeof(ud_code), &after) !=
            LIB_STATUS_INTERNAL_ERROR || !state.fault.valid ||
        !X86_CPU_BIT_IS_SET(state.fault.exception_mask, VCPUINS_EXCEPT_UD) ||
        after.data.eax != 0x556677a5u || after.data.eflags != flags ||
        after.data.eip != 0u) return 0;
    return 1;
}

lib_i32 main(void)
{
    if (!setcc_test_register_conditions() ||
        !setcc_test_memory_conditions() || !setcc_test_prefix_forms() ||
        !setcc_test_pre_fault_nonpublication()) return 1;
    lib_c_printf("M5:T539:S30:CPU-SETCC:OK\n");
    return 0;
}
