#include "lib/types/file.h"
#include "x86/core/debug_interface.h"
#include "x86/chips/cpu/cpu_interface.h"
#include "x86/core/device_support_interface.h"
#include "ibmpc/board-common/machine_board_interface.h"
static lib_u16 legacy_alu_real_flags_known_mask(
    core_machine_cpu_profile profile)
{
    return profile < CORE_MACHINE_CPU_PROFILE_80286 ? 0x0fd5u : 0x7fd5u;
}

static lib_i32 legacy_alu_divide_error_delivery(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186
    };
    static const lib_u8 code[] = {0xf6u, 0xf1u};
    static const lib_u8 handler[] = {0xf4u};
    const lib_u16 code_offset = 0x0200u, handler_offset = 0x0100u;
    lib_u8 index;

    for (index = 0u; index < sizeof(profiles) / sizeof(profiles[0]);
        ++index) {
        const core_machine_config config = {
            .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
            .cpu_profile = profiles[index],
            .fpu_profile = X86_FPU_PROFILE_NONE
        };
        core_machine *machine = LIB_NULL;
        core_machine_debug_register_patch patch = {0};
        core_machine_debug_cpu_snapshot before = {0}, after = {0};
        core_machine_cpu_diagnostic diagnostic = {0};
        core_machine_run_result result = {0};
        lib_u16 frame[3] = {0};
        lib_u16 known = legacy_alu_real_flags_known_mask(profiles[index]);
        lib_i32 failed = core_machine_create(&config, &machine, LIB_NULL) !=
                LIB_STATUS_OK ||
            core_machine_freeze_execution_providers(machine) !=
                LIB_STATUS_OK ||
            core_machine_reset(machine) != LIB_STATUS_OK;

        if (!failed) {
            patch.mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ECX) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDX) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP);
            patch.values[CORE_MACHINE_DEBUG_EAX] = 5u;
            patch.values[CORE_MACHINE_DEBUG_EDX] = 0xaabbccddu;
            patch.values[CORE_MACHINE_DEBUG_ESP] = 0x00008000u;
            patch.values[CORE_MACHINE_DEBUG_EFLAGS] = CORE_MACHINE_DEBUG_EFLAGS_CF |
                CORE_MACHINE_DEBUG_EFLAGS_IF | CORE_MACHINE_DEBUG_EFLAGS_DF;
            patch.values[CORE_MACHINE_DEBUG_EIP] = code_offset;
            failed = core_machine_debug_patch_registers(machine, &patch) !=
                    LIB_STATUS_OK ||
                core_machine_memory_write(machine, code_offset, code,
                    sizeof(code)) != LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0u, &handler_offset,
                    sizeof(handler_offset)) != LIB_STATUS_OK ||
                core_machine_memory_write(machine, handler_offset, handler,
                    sizeof(handler)) != LIB_STATUS_OK ||
                core_machine_debug_capture_cpu_snapshot(machine,
                    CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) !=
                    LIB_STATUS_OK ||
                core_machine_run(machine, (core_machine_run_budget){1u, 0u},
                    &result) != LIB_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_BUDGET ||
                core_machine_get_cpu_diagnostic(machine, &diagnostic) !=
                    LIB_STATUS_OK ||
                core_machine_debug_capture_cpu_snapshot(machine,
                    CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) !=
                    LIB_STATUS_OK ||
                diagnostic.first_fault.valid ||
                !diagnostic.last_delivered_exception.valid ||
                !CORE_MACHINE_BIT_IS_SET(
                    diagnostic.last_delivered_exception.exception_mask,
                    VCPUINS_EXCEPT_DE) ||
                after.eip != handler_offset || after.eax != before.eax ||
                after.ecx != before.ecx || after.edx != before.edx ||
                after.eflags !=
                    (before.eflags & ~(CORE_MACHINE_DEBUG_EFLAGS_IF | CORE_MACHINE_DEBUG_EFLAGS_TF)) ||
                after.esp != ((before.esp & 0xffff0000u) |
                    (lib_u16)(before.esp - 6u)) ||
                core_machine_memory_read(machine,
                    after.ss.base + (lib_u16)after.esp, frame,
                    sizeof(frame)) != LIB_STATUS_OK ||
                frame[0] != code_offset + (profiles[index] ==
                    CORE_MACHINE_CPU_PROFILE_8086 ? sizeof(code) : 0u) ||
                frame[1] != before.cs.selector ||
                (frame[2] & known) !=
                    ((((lib_u16)(before.eflags & ~0xfffc802au)) |
                        0x02u) & known);
        }
        core_machine_destroy(machine);
        if (failed) return 0;
    }
    return 1;
}

int main(void)
{
    if (!legacy_alu_divide_error_delivery()) return 1;
    lib_c_printf("LEGACY-ALU-DIVIDE-BOARD:OK\n");
    return 0;
}
