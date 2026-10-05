#include "support/cpu_bus_fixture.h"
#include "lib/types/file.h"

typedef struct cpu_bus_reset_case {
    core_machine_cpu_profile profile;
    lib_u32 code_base;
    lib_u32 first_fetch;
} cpu_bus_reset_case;

static lib_i32 cpu_bus_cases(const cpu_bus_reset_case *reset)
{
    cpu_bus_fixture fixture;
    core_machine_cpu_instruction_lexeme lexeme;
    lib_u8 value = 0x5au;
    lib_u32 expected_address = reset->profile <= CORE_MACHINE_CPU_PROFILE_80186 ?
        0x00023456u : reset->profile == CORE_MACHINE_CPU_PROFILE_80286 ?
        0x00223456u : 0x01223456u;

    cpu_bus_prepare(&fixture, reset->profile);
    if (core_machine_cpu_write_linear(&fixture.execution, 0x01223456u, &value, 1u) ||
        fixture.last_address != expected_address) return 1;
    value = 0u;
    if (core_machine_cpu_read_linear(&fixture.execution, 0x01223456u, &value, 1u) ||
        fixture.last_address != expected_address || value != 0x5au) return 1;
    fixture.memory[0x100u] = 0x90u;
    if (!core_machine_cpu_execution_preview_lexeme(&fixture.execution, &lexeme) ||
        fixture.observations == 0u || fixture.cpu.data.eip != 0x100u ||
        fixture.completions != 0u || fixture.instruction_count != 0u) return 1;
    fixture.cpu.data.cs.base = reset->code_base;
    fixture.cpu.data.cs.selector = 0xf000u;
    fixture.cpu.data.eip = 0xfff0u;
    fixture.memory[reset->first_fetch % sizeof(fixture.memory)] = 0x90u;
    if (!core_machine_cpu_execution_preview_lexeme(&fixture.execution, &lexeme) ||
        (fixture.reset_fetches != 0u) !=
            (reset->profile >= CORE_MACHINE_CPU_PROFILE_80286)) return 1;

    /* A synchronous write observer sees the consumed operand's PC, not the
     * previous instruction or its entry PC. The copy outlives this callback. */
    cpu_bus_prepare(&fixture, reset->profile);
    fixture.memory[0x100u] = 0xa3u;
    fixture.memory[0x101u] = 0x00u;
    fixture.memory[0x102u] = 0x04u;
    core_machine_cpu_execution_refresh(&fixture.execution);
    if (fixture.writes == 0u || fixture.write_cpu.cs != 0u ||
        fixture.write_cpu.cs_base != 0u || fixture.write_cpu.eip != 0x103u ||
        fixture.memory[0x400u] != 0x10u || fixture.memory[0x401u] != 0x32u ||
        fixture.instructions.data.except != 0u) return 1;
    fixture.cpu.data.eip = 0x200u;
    if (fixture.write_cpu.eip != 0x103u) return 1;

    for (lib_u8 bytes = 1u; bytes <= 4u; bytes *= 2u) {
        if (bytes == 4u && reset->profile != CORE_MACHINE_CPU_PROFILE_80386) continue;
        for (lib_u8 write = 0u; write < 2u; ++write) {
            lib_u32 offset = 0x100u;
            lib_u32 mask = bytes == 1u ? 0xffu : bytes == 2u ? 0xffffu : LIB_UINT32_MAX;

            cpu_bus_prepare(&fixture, reset->profile);
            if (bytes == 4u) fixture.memory[offset++] = 0x66u;
            fixture.memory[offset] = write ? (bytes == 1u ? 0xeeu : 0xefu) :
                (bytes == 1u ? 0xecu : 0xedu);
            core_machine_cpu_execution_refresh(&fixture.execution);
            if (fixture.failed || fixture.port_active || fixture.completions != 1u ||
                fixture.instructions.data.except != 0u ||
                (write && fixture.port_value != (0x76543210u & mask))) return 1;
            if (fixture.instruction_count != 1u ||
                fixture.instruction.point.eip != 0x100u ||
                fixture.instruction.eax != 0x76543210u ||
                fixture.instruction.dx != 0x1234u) return 1;
            {
                core_machine_cpu_instruction_observation retired;

                core_machine_cpu_execution_copy_observation(&fixture.execution, &retired);
                if (retired.point.eip != offset + 1u ||
                    retired.old_eip != 0x100u ||
                    retired.operand_size_32 != (bytes == 4u) ||
                    retired.point.byte_count != CORE_MACHINE_CPU_DIAGNOSTIC_BYTES ||
                    retired.point.bytes[offset - 0x100u] != fixture.memory[offset] ||
                    retired.eax != fixture.cpu.data.eax) return 1;
            }

            /* A rejected transfer must not publish an operand or completion. */
            cpu_bus_prepare(&fixture, reset->profile);
            if (bytes == 4u) fixture.memory[0x100u] = 0x66u;
            fixture.memory[offset] = write ? (bytes == 1u ? 0xeeu : 0xefu) :
                (bytes == 1u ? 0xecu : 0xedu);
            fixture.fail_port = LIB_TRUE;
            core_machine_cpu_execution_refresh(&fixture.execution);
            if (fixture.failed || fixture.port_active || fixture.completions != 0u ||
                fixture.instructions.data.except != VCPUINS_EXCEPT_CE ||
                fixture.cpu.data.eax != 0x76543210u ||
                fixture.port_value != 0x12345678u) return 1;
            if (fixture.faults != 1u || !fixture.fault.valid ||
                fixture.fault.exception_mask != VCPUINS_EXCEPT_CE ||
                fixture.fault.exception_code != 0x1234u ||
                fixture.fault.point.cs != 0u || fixture.fault.point.eip != 0x100u ||
                fixture.fault.eax != 0x76543210u) return 1;
            fixture.cpu.data.eax = 0u;
            if (fixture.fault.eax != 0x76543210u) return 1;
        }
    }
    cpu_bus_prepare(&fixture, reset->profile);
    fixture.memory[0x100u] = 0x90u;
    fixture.memory[0xc1u] = 0x02u;
    fixture.cpu.data.eflags |= VCPU_EFLAGS_IF;
    fixture.interrupt = LIB_TRUE;
    core_machine_cpu_execution_refresh(&fixture.execution);
    if (fixture.acknowledgements != 1u || fixture.cpu.data.eip != 0x200u ||
        fixture.cpu.data.sp != 0x6fau || fixture.instructions.data.except != 0u) return 1;
    cpu_bus_prepare(&fixture, reset->profile);
    fixture.memory[0x100u] = 0x90u;
    fixture.cpu.data.eflags |= VCPU_EFLAGS_IF;
    fixture.interrupt = LIB_TRUE;
    fixture.fail_acknowledge = LIB_TRUE;
    core_machine_cpu_execution_refresh(&fixture.execution);
    if (fixture.acknowledgements != 0u || !fixture.interrupt ||
        fixture.cpu.data.eip != 0x101u || fixture.cpu.data.sp != 0x700u ||
        fixture.instructions.data.except != 0u) return 1;
    fixture.fail_transfer = LIB_TRUE;
    return !core_machine_cpu_read_linear(&fixture.execution, 0x100u, &value, 1u);
}

lib_i32 main(void)
{
    static const cpu_bus_reset_case cases[] = {
        {CORE_MACHINE_CPU_PROFILE_8086, 0x000f0000u, 0x000ffff0u},
        {CORE_MACHINE_CPU_PROFILE_8088, 0x000f0000u, 0x000ffff0u},
        {CORE_MACHINE_CPU_PROFILE_80186, 0x000f0000u, 0x000ffff0u},
        {CORE_MACHINE_CPU_PROFILE_80286, 0x00ff0000u, 0x00fffff0u},
        {CORE_MACHINE_CPU_PROFILE_80386, 0xffff0000u, 0xfffffff0u}
    };

    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index)
        if (cpu_bus_cases(&cases[index])) return 1;
    lib_c_printf("%s\n", "M5:T539:S87:CPU-EXECUTION-BUS:OK");
    return 0;
}
