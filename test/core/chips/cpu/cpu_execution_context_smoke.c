#include "lib/types/types_interface.h"
#include "lib/types/file.h"



#include "core/chips/cpu/cpu.h"

#include "core/chips/cpu/cpu_instructions.h"

#include "support/cpu_bus_fixture.h"

static lib_i32 cpu_debug_case(core_machine_cpu_profile profile)
{
    cpu_bus_fixture fixture;
    core_machine_debug_cpu_snapshot snapshot;
    core_machine_debug_instruction_observation instruction;
    core_machine_debug_register_patch patch = {0};
    lib_u32 value;

    cpu_bus_prepare(&fixture, profile);
    patch.mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
        CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS);
    patch.values[CORE_MACHINE_DEBUG_EAX] = 0x12345678u;
    patch.values[CORE_MACHINE_DEBUG_DS] = 0x1234u;
    if (core_machine_cpu_debug_patch_registers(&fixture.execution, &patch) !=
            LIB_STATUS_OK ||
        core_machine_cpu_debug_read_register(&fixture.execution,
            CORE_MACHINE_DEBUG_EAX, &value) != LIB_STATUS_OK ||
        value != 0x12345678u ||
        core_machine_cpu_debug_capture_snapshot(&fixture.execution,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &snapshot) !=
            LIB_STATUS_OK || snapshot.ds.selector != 0x1234u ||
        snapshot.ds.base != 0x12340u || snapshot.eax != value ||
        snapshot.eip != 0x100u ||
        core_machine_cpu_debug_capture_instruction(&fixture.execution, &instruction) !=
            LIB_STATUS_OK || instruction.eax != value || instruction.ds != 0x1234u)
        return 1;
    patch.mask = 0u;
    if (core_machine_cpu_debug_patch_registers(&fixture.execution, &patch) !=
            LIB_STATUS_INVALID_ARGUMENT || fixture.cpu.data.eax != value ||
        core_machine_cpu_debug_read_register(&fixture.execution,
            CORE_MACHINE_DEBUG_REGISTER_COUNT, &value) != LIB_STATUS_INVALID_ARGUMENT)
        return 1;
    {
        core_machine_debug_cpu_snapshot entry, current;

        fixture.memory[0x100u] = 0xb8u;
        fixture.memory[0x101u] = 0x34u;
        fixture.memory[0x102u] = 0x12u;
        core_machine_cpu_execution_refresh(&fixture.execution);
        if (core_machine_cpu_debug_capture_snapshot(&fixture.execution,
                CORE_MACHINE_CPU_SNAPSHOT_INSTRUCTION_ENTRY, &entry) != LIB_STATUS_OK ||
            core_machine_cpu_debug_capture_snapshot(&fixture.execution,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &current) != LIB_STATUS_OK ||
            entry.eax != 0x12345678u || entry.eip != 0x100u ||
            current.eax != 0x12341234u || current.eip != 0x103u ||
            entry.ds.selector != snapshot.ds.selector ||
            current.ds.base != snapshot.ds.base ||
            snapshot.eax != 0x12345678u || snapshot.eip != 0x100u ||
            core_machine_cpu_debug_capture_snapshot(&fixture.execution,
                (core_machine_cpu_snapshot_point)2, &snapshot) != LIB_STATUS_INVALID_ARGUMENT ||
            snapshot.eax != 0x12345678u) return 1;
    }
    return 0;
}

lib_i32 main(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_8088,
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
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
