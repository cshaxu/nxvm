#include "lib/types/types_interface.h"
#include "lib/types/file.h"



#include "x86-core/cpu/cpu.h"

#include "x86-core/cpu/cpu_instructions.h"

#include "support/cpu_bus_fixture.h"

static lib_i32 cpu_debug_case(x86_core_cpu_profile profile)
{
    cpu_bus_fixture fixture;
    x86_core_cpu_debug_cpu_snapshot snapshot;
    x86_core_cpu_debug_instruction_observation instruction;
    x86_core_cpu_debug_register_patch patch = {0};
    lib_u32 value;

    cpu_bus_prepare(&fixture, profile);
    patch.mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EAX) |
        X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_DS);
    patch.values[X86_CORE_CPU_DEBUG_EAX] = 0x12345678u;
    patch.values[X86_CORE_CPU_DEBUG_DS] = 0x1234u;
    if (x86_core_cpu_debug_patch_registers(&fixture.execution, &patch) !=
            LIB_STATUS_OK ||
        x86_core_cpu_debug_read_register(&fixture.execution,
            X86_CORE_CPU_DEBUG_EAX, &value) != LIB_STATUS_OK ||
        value != 0x12345678u ||
        x86_core_cpu_debug_capture_snapshot(&fixture.execution,
            X86_CORE_CPU_SNAPSHOT_CURRENT, &snapshot) !=
            LIB_STATUS_OK || snapshot.ds.selector != 0x1234u ||
        snapshot.ds.base != 0x12340u || snapshot.eax != value ||
        snapshot.eip != 0x100u ||
        x86_core_cpu_debug_capture_instruction(&fixture.execution, &instruction) !=
            LIB_STATUS_OK || instruction.eax != value || instruction.ds != 0x1234u)
        return 1;
    patch.mask = 0u;
    if (x86_core_cpu_debug_patch_registers(&fixture.execution, &patch) !=
            LIB_STATUS_INVALID_ARGUMENT || fixture.cpu.data.eax != value ||
        x86_core_cpu_debug_read_register(&fixture.execution,
            X86_CORE_CPU_DEBUG_REGISTER_COUNT, &value) != LIB_STATUS_INVALID_ARGUMENT)
        return 1;
    {
        x86_core_cpu_debug_cpu_snapshot entry, current;

        fixture.memory[0x100u] = 0xb8u;
        fixture.memory[0x101u] = 0x34u;
        fixture.memory[0x102u] = 0x12u;
        x86_core_cpu_execution_refresh(&fixture.execution);
        if (x86_core_cpu_debug_capture_snapshot(&fixture.execution,
                X86_CORE_CPU_SNAPSHOT_INSTRUCTION_ENTRY, &entry) != LIB_STATUS_OK ||
            x86_core_cpu_debug_capture_snapshot(&fixture.execution,
                X86_CORE_CPU_SNAPSHOT_CURRENT, &current) != LIB_STATUS_OK ||
            entry.eax != 0x12345678u || entry.eip != 0x100u ||
            current.eax != 0x12341234u || current.eip != 0x103u ||
            entry.ds.selector != snapshot.ds.selector ||
            current.ds.base != snapshot.ds.base ||
            snapshot.eax != 0x12345678u || snapshot.eip != 0x100u ||
            x86_core_cpu_debug_capture_snapshot(&fixture.execution,
                (x86_core_cpu_snapshot_point)2, &snapshot) != LIB_STATUS_INVALID_ARGUMENT ||
            snapshot.eax != 0x12345678u) return 1;
    }
    return 0;
}

lib_i32 main(void)
{
    static const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_8086,
        X86_CORE_CPU_PROFILE_8088,
        X86_CORE_CPU_PROFILE_80186,
        X86_CORE_CPU_PROFILE_80286,
        X86_CORE_CPU_PROFILE_80386,
        X86_CORE_CPU_PROFILE_80486SX
    };
    lib_i32 result = 0;

    for (lib_size index = 0u;
         index < sizeof(profiles) / sizeof(profiles[0]); ++index) {
        result |= cpu_debug_case(profiles[index]);
    }

    if (result != 0) return 1;

    lib_c_printf("%s\n", "CPU-CONTEXT:OK");
    return 0;
}
