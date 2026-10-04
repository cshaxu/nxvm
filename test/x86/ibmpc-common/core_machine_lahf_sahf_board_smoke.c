#include "lib/types/types_interface.h"
#include <stdio.h>
#include "cpu_board_flags_protected_fixture.h"
#include "cpu_board_irq_fixture.h"

#define LAHF_SAHF_MASK (CORE_MACHINE_DEBUG_EFLAGS_CF | CORE_MACHINE_DEBUG_EFLAGS_PF | CORE_MACHINE_DEBUG_EFLAGS_AF | \
    CORE_MACHINE_DEBUG_EFLAGS_ZF | CORE_MACHINE_DEBUG_EFLAGS_SF)

static lib_i32 lahf_sahf_nonparticipants_same(
    const core_machine_debug_cpu_snapshot *before,
    const core_machine_debug_cpu_snapshot *after, lib_bool irq)
{
    return before->ecx == after->ecx && before->edx == after->edx &&
        before->ebx == after->ebx &&
        (irq || before->esp == after->esp) &&
        before->ebp == after->ebp && before->esi == after->esi &&
        before->edi == after->edi;
}

static lib_i32 lahf_sahf_test_protected(void)
{
    static const lib_u8 opcodes[] = {0x9fu,0x9eu};
    const lib_u32 flags = CORE_MACHINE_DEBUG_EFLAGS_IF | CORE_MACHINE_DEBUG_EFLAGS_DF |
        CORE_MACHINE_DEBUG_EFLAGS_OF | 0x00003000u | LAHF_SAHF_MASK;

    for (lib_size opcode = 0u; opcode < sizeof(opcodes); ++opcode) {
        test_cpu_board_flags_protected_result result = {0};
        lib_u32 eax = opcodes[opcode] == 0x9eu ?
            0xaabb00ddU : 0xaabbccddU;

        if (!test_cpu_board_flags_protected_run(&opcodes[opcode], 1u,
            eax, flags, &result) || result.diagnostic.first_fault.valid ||
            result.after.eip != 1u ||
            !lahf_sahf_nonparticipants_same(&result.before, &result.after,
                LIB_FALSE) ||
            (opcodes[opcode] == 0x9eu ?
                result.after.eax != result.before.eax ||
                (result.after.eflags & LAHF_SAHF_MASK) != 0u ||
                (result.after.eflags & ~LAHF_SAHF_MASK) !=
                    (result.before.eflags & ~LAHF_SAHF_MASK) :
                result.after.eax != ((result.before.eax & 0xffff00ffu) |
                    ((LAHF_SAHF_MASK | 0x02u) << 8u)) ||
                result.after.eflags != result.before.eflags) ||
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

static lib_i32 lahf_sahf_test_irq(void)
{
    static const lib_u8 opcodes[] = {0x9fu,0x9eu};

    for (lib_size opcode = 0u; opcode < sizeof(opcodes); ++opcode) {
        const lib_bool sahf = opcodes[opcode] == 0x9eu;
        const test_cpu_board_irq_case entry = {
            .code = {opcodes[opcode],0x90u},
            .eax = sahf ? 0xaabbffddU : 0xaabbccddU,
            .flags = CORE_MACHINE_DEBUG_EFLAGS_IF, .budget = 2u
        };
        test_cpu_board_irq_result result = {0};

        if (!test_cpu_board_irq_run(&entry, &result) ||
            result.after.eip != 0x101u || result.frame_ip != 1u ||
            !lahf_sahf_nonparticipants_same(&result.before, &result.after,
                LIB_TRUE) ||
            result.after.eax != (sahf ? 0xaabbffddU : 0xaabb02ddU) ||
            result.after.eflags != (sahf ? LAHF_SAHF_MASK : 0u) ||
            !(result.isr & VPIC_ISR_IRQ(0u)) ||
            (result.irr & VPIC_IRR_IRQ(0u)) != 0u) return 0;
    }
    return 1;
}

int main(void)
{
    if (!lahf_sahf_test_protected() || !lahf_sahf_test_irq()) {
        fputs("M5:T539:S36:BOARD-LAHF-SAHF:FAIL\n", stderr);
        return 1;
    }
    puts("M5:T539:S36:BOARD-LAHF-SAHF:OK");
    return 0;
}
