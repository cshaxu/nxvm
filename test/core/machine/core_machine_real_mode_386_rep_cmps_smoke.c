#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/debug_interface.h"
#include "core/machine/machine.h"
#include "core/machine/machine_interface.h"
#include "core/machine/memory_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define T292_RESET_LINEAR 0xfffffff0u
#define T292_RESET_PHYSICAL 0x000ffff0u
#define T292_RESET_WINDOW 16u
#define T292_SOURCE 0x00020000u
#define T292_DESTINATION 0x00030000u

static C_INT t292_prepare(core_machine_cpu_profile profile, core_machine **out_machine)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
        .ticks_per_instruction = 1u
    };
    static const type_unsigned_8 reset_jump[] = {0xeau, 0u, 0u, 0u, 0u};
    core_machine *machine = STD_NULL;

    if (out_machine == STD_NULL || core_machine_create(&config, &machine) !=
            TYPE_STATUS_OK || test_core_machine_fixture_register_reset_mapping(
            machine, T292_RESET_LINEAR,
            T292_RESET_PHYSICAL, T292_RESET_WINDOW) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
        core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, T292_RESET_LINEAR, reset_jump,
            sizeof(reset_jump)) != TYPE_STATUS_OK) {
        core_machine_destroy(machine);
        return 0;
    }
    *out_machine = machine;
    return 1;
}

int main(C_VOID)
{
    static const type_unsigned_8 program[] = {
        0xb8u, 0u, 0u, 0x8eu, 0xd8u, 0x8eu, 0xc0u,
        0x66u, 0xbeu, 0u, 0u, 0x02u, 0u,
        0x66u, 0xbfu, 0u, 0u, 0x03u, 0u,
        0x66u, 0xb9u, 0x03u, 0u, 0u, 0u,
        0xfcu, 0xf3u, 0x67u, 0xa6u, 0xf4u
    };
    static const type_unsigned_8 source[] = {0x11u, 0x22u, 0x33u};
    static const type_unsigned_8 destination[] = {0x11u, 0xffu, 0x33u};
    static const type_unsigned_8 scas_program[] = {
        0xb8u, 0u, 0u, 0x8eu, 0xc0u,
        0x66u, 0xbfu, 0u, 0u, 0x02u, 0u,
        0x66u, 0xb9u, 0x03u, 0u, 0u, 0u,
        0xb0u, 0x22u, 0xfcu, 0xf2u, 0x67u, 0xaeu, 0xf4u
    };
    static const type_unsigned_8 scas_bytes[] = {0x11u, 0x22u, 0x33u};
    static const type_unsigned_8 segment_program[] = {
        0xb8u, 0x00u, 0x10u, 0x8eu, 0xd8u,
        0x31u, 0xc0u, 0x8eu, 0xc0u,
        0x66u, 0xbeu, 0u, 0x10u, 0u, 0u,
        0x66u, 0xbfu, 0u, 0u, 0x03u, 0u,
        0x66u, 0xb9u, 0x01u, 0u, 0u, 0u,
        0xfcu, 0x2eu, 0xf3u, 0x67u, 0xa6u, 0xf4u
    };
    static const type_unsigned_8 segment_source[] = {0x5au};
    static const type_unsigned_8 segment_default[] = {0xeeu};
    static const type_unsigned_8 segment_destination[] = {0x5au};
    const core_machine_run_budget budget = {256u, 0u};
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    core_machine *machine = STD_NULL;
    C_INT failed = !t292_prepare(CORE_MACHINE_CPU_PROFILE_80386, &machine);

    if (!failed) {
        failed |= core_machine_memory_write(machine, 0u, program, sizeof(program)) !=
                TYPE_STATUS_OK || core_machine_memory_write(machine, T292_SOURCE,
                source, sizeof(source)) != TYPE_STATUS_OK ||
            core_machine_memory_write(machine, T292_DESTINATION, destination,
                sizeof(destination)) != TYPE_STATUS_OK ||
            core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            core_machine_get_cpu_diagnostic(machine, &diagnostic) != TYPE_STATUS_OK ||
            diagnostic.first_fault.valid || machine->executor_cpu.data.ecx != 1u ||
            machine->executor_cpu.data.esi != T292_SOURCE + 2u ||
            machine->executor_cpu.data.edi != T292_DESTINATION + 2u ||
            (machine->executor_cpu.data.eflags & VCPU_EFLAGS_ZF) != 0u;
    }
    core_machine_destroy(machine);
    machine = STD_NULL;
    if (!failed) {
        failed |= !t292_prepare(CORE_MACHINE_CPU_PROFILE_80386, &machine) ||
            core_machine_memory_write(machine, 0u, scas_program,
                sizeof(scas_program)) != TYPE_STATUS_OK ||
            core_machine_memory_write(machine, T292_SOURCE, scas_bytes,
                sizeof(scas_bytes)) != TYPE_STATUS_OK ||
            core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            core_machine_get_cpu_diagnostic(machine, &diagnostic) != TYPE_STATUS_OK ||
            diagnostic.first_fault.valid || machine->executor_cpu.data.ecx != 1u ||
            machine->executor_cpu.data.edi != T292_SOURCE + 2u ||
            (machine->executor_cpu.data.eflags & VCPU_EFLAGS_ZF) == 0u;
    }
    core_machine_destroy(machine);
    machine = STD_NULL;
    if (!failed) {
        failed |= !t292_prepare(CORE_MACHINE_CPU_PROFILE_80386, &machine) ||
            core_machine_memory_write(machine, 0u, segment_program,
                sizeof(segment_program)) != TYPE_STATUS_OK ||
            /* 80386 real mode still limits every segment offset to FFFFh.
             * Keep the CS override inside that architectural bound while
             * retaining a distinct DS default address. */
            core_machine_memory_write(machine, 0x00001000u, segment_source,
                sizeof(segment_source)) != TYPE_STATUS_OK ||
            core_machine_memory_write(machine, 0x00011000u, segment_default,
                sizeof(segment_default)) != TYPE_STATUS_OK ||
            core_machine_memory_write(machine, T292_DESTINATION, segment_destination,
                sizeof(segment_destination)) != TYPE_STATUS_OK ||
            core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            core_machine_get_cpu_diagnostic(machine, &diagnostic) != TYPE_STATUS_OK ||
            diagnostic.first_fault.valid || machine->executor_cpu.data.ecx != 0u ||
            machine->executor_cpu.data.esi != 0x00001001u ||
            machine->executor_cpu.data.edi != T292_DESTINATION + 1u ||
            (machine->executor_cpu.data.eflags & VCPU_EFLAGS_ZF) == 0u;
    }
    core_machine_destroy(machine);
    if (failed) return 1;
    puts("M5:T292:S1:REP-STRING:OK");
    return 0;
}
