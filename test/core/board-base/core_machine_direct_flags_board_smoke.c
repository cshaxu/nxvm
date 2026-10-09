#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "core/x86/debug_interface.h"
#include "cpu_board_irq_fixture.h"
#include "cpu_board_flags_protected_fixture.h"

static lib_u32 direct_flags_expected(lib_u8 opcode, lib_u32 flags)
{
    switch (opcode) {
    case 0xf5u: return flags ^ CORE_MACHINE_DEBUG_EFLAGS_CF;
    case 0xf8u: return flags & ~CORE_MACHINE_DEBUG_EFLAGS_CF;
    case 0xf9u: return flags | CORE_MACHINE_DEBUG_EFLAGS_CF;
    case 0xfcu: return flags & ~CORE_MACHINE_DEBUG_EFLAGS_DF;
    case 0xfdu: return flags | CORE_MACHINE_DEBUG_EFLAGS_DF;
    default: return flags;
    }
}

static lib_i32 direct_flags_gprs_same(
    const core_machine_debug_cpu_snapshot *before,
    const core_machine_debug_cpu_snapshot *after, lib_bool irq)
{
    return after->eax == before->eax && after->ecx == before->ecx &&
        after->edx == before->edx && after->ebx == before->ebx &&
        (irq || after->esp == before->esp) &&
        after->ebp == before->ebp && after->esi == before->esi &&
        after->edi == before->edi;
}

static lib_i32 direct_flags_test_protected(void)
{
    static const lib_u8 opcodes[] = {0xf5u,0xf8u,0xf9u,0xfcu,0xfdu};
    const lib_u32 flags = CORE_MACHINE_DEBUG_EFLAGS_IF | CORE_MACHINE_DEBUG_EFLAGS_CF |
        CORE_MACHINE_DEBUG_EFLAGS_DF | CORE_MACHINE_DEBUG_EFLAGS_PF | CORE_MACHINE_DEBUG_EFLAGS_AF |
        CORE_MACHINE_DEBUG_EFLAGS_ZF | CORE_MACHINE_DEBUG_EFLAGS_SF | CORE_MACHINE_DEBUG_EFLAGS_OF |
        0x00003000u;

    for (lib_size opcode = 0u; opcode < sizeof(opcodes); ++opcode) {
        test_cpu_board_flags_protected_result result = {0};

        if (!test_cpu_board_flags_protected_run(&opcodes[opcode], 1u,
            0xaabbccddU, flags, &result) ||
            result.diagnostic.first_fault.valid ||
            result.after.eip != 1u ||
            !direct_flags_gprs_same(&result.before, &result.after, LIB_FALSE) ||
            result.after.eflags != direct_flags_expected(opcodes[opcode],
                result.before.eflags) ||
            lib_memory_compare(&result.before.cs, &result.after.cs,
                sizeof(result.before.cs)) != 0 ||
            lib_memory_compare(&result.before.ds, &result.after.ds,
                sizeof(result.before.ds)) != 0 ||
            lib_memory_compare(&result.before.es, &result.after.es,
                sizeof(result.before.es)) != 0 ||
            lib_memory_compare(&result.before.ss, &result.after.ss,
                sizeof(result.before.ss)) != 0) return 0;
    }
    return 1;
}

static lib_i32 direct_flags_test_irq(void)
{
    static const lib_u8 opcodes[] = {0xf5u,0xf8u,0xf9u,0xfcu,0xfdu};
    const lib_u32 flags = CORE_MACHINE_DEBUG_EFLAGS_IF | CORE_MACHINE_DEBUG_EFLAGS_CF |
        CORE_MACHINE_DEBUG_EFLAGS_DF | CORE_MACHINE_DEBUG_EFLAGS_PF | CORE_MACHINE_DEBUG_EFLAGS_AF |
        CORE_MACHINE_DEBUG_EFLAGS_ZF | CORE_MACHINE_DEBUG_EFLAGS_SF | CORE_MACHINE_DEBUG_EFLAGS_OF;

    for (lib_size opcode = 0u; opcode < sizeof(opcodes); ++opcode) {
        const test_cpu_board_irq_case entry = {
            .code = {opcodes[opcode],0x90u},
            .eax = 0xaabbccddU, .flags = flags, .budget = 2u
        };
        test_cpu_board_irq_result result = {0};

        if (!test_cpu_board_irq_run(&entry, &result) ||
            result.after.eip != 0x101u || result.frame_ip != 1u ||
            !direct_flags_gprs_same(&result.before, &result.after, LIB_TRUE) ||
            result.after.eflags != (direct_flags_expected(opcodes[opcode],
                result.before.eflags) & ~CORE_MACHINE_DEBUG_EFLAGS_IF) ||
            !(result.isr & VPIC_ISR_IRQ(0u)) ||
            (result.irr & VPIC_IRR_IRQ(0u)) != 0u) return 0;
    }
    return 1;
}

int main(void)
{
    if (!direct_flags_test_protected() || !direct_flags_test_irq()) {
        lib_c_fprintf(lib_c_stderr, "%s", "BOARD-DIRECT-FLAGS:FAIL\n");
        return 1;
    }
    lib_c_printf("%s\n", "BOARD-DIRECT-FLAGS:OK");
    return 0;
}
