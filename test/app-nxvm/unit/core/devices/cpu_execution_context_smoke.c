#include "lib/types/types_interface.h"
#include <stdio.h>



#include "app-nxvm/devices/cpu.h"

#include "app-nxvm/devices/cpu_instructions.h"
#include "app-nxvm/devices/cpu_timing.h"

typedef struct cpu_reset_case {
    core_machine_cpu_profile profile;
    lib_u32 code_base;
    lib_u32 first_fetch;
} cpu_reset_case;

#include "support/cpu_bus_fixture.h"

static lib_i32 cpu_bus_cases(const cpu_reset_case *reset)
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

static lib_i32 cpu_timing_case(core_machine_cpu_profile profile)
{
    cpu_bus_fixture first;
    cpu_bus_fixture second;
    core_machine_cpu_timing_result first_result;
    core_machine_cpu_timing_result second_result;

    cpu_bus_prepare(&first, profile);
    cpu_bus_prepare(&second, profile);
    first.memory[0x100u] = second.memory[0x100u] = 0xf3u;
    first.memory[0x101u] = second.memory[0x101u] = 0xa4u;
    first.cpu.data.cx = second.cpu.data.cx = 2u;
    first.cpu.data.si = second.cpu.data.si = 0x400u;
    first.cpu.data.di = second.cpu.data.di = 0x500u;
    core_machine_cpu_execution_refresh(&first.execution);
    if (!core_machine_cpu_timing_select(&first.execution, &first_result) ||
        first_result.source_timing_unallocated || first_result.ticks == 0u ||
        first_result.repeat_phase != CORE_MACHINE_RETIREMENT_REPEAT_FIRST)
        return 1;
    core_machine_cpu_execution_refresh(&second.execution);
    if (!core_machine_cpu_timing_select(&second.execution, &second_result) ||
        second_result.repeat_phase != CORE_MACHINE_RETIREMENT_REPEAT_FIRST ||
        first_result.ticks != second_result.ticks) return 1;
    core_machine_cpu_execution_refresh(&first.execution);
    if (!core_machine_cpu_timing_select(&first.execution, &first_result) ||
        first_result.repeat_phase != CORE_MACHINE_RETIREMENT_REPEAT_CONTINUATION ||
        first.cpu.data.cx != 0u || second.cpu.data.cx != 1u) return 1;
    second_result = core_machine_cpu_capture_timing(&second.execution);
    if (second_result.repeat_phase != CORE_MACHINE_RETIREMENT_REPEAT_FIRST ||
        second_result.form_id != first_result.form_id ||
        second_result.key_id != first_result.key_id) return 1;
    core_machine_cpu_state_reset(&second.execution);
    return second.execution.source_repeat_active ||
        second_result.repeat_phase != CORE_MACHINE_RETIREMENT_REPEAT_FIRST;
}

static lib_i32 cpu_signal_case(core_machine_cpu_profile profile)
{
    cpu_bus_fixture fixture;
    core_machine_cpu_state state;

    cpu_bus_prepare(&fixture, profile);
    fixture.memory[0x100u] = 0xf4u;
    fixture.memory[0x09u] = 0x02u;
    core_machine_cpu_capture_state(&fixture.execution, &state);
    if (state.cs != 0u || state.cs_base != 0u || state.eip != 0x100u ||
        state.eflags != fixture.cpu.data.eflags || state.halted ||
        core_machine_cpu_linear_pc(&fixture.execution) != 0x100u) return 1;

    core_machine_cpu_set_nmi_mask(&fixture.execution, LIB_TRUE);
    if (!core_machine_cpu_nmi_is_masked(&fixture.execution) ||
        core_machine_cpu_request_nmi(&fixture.execution) ||
        fixture.cpu.data.flagNMI) return 1;
    core_machine_cpu_execution_refresh(&fixture.execution);
    if (!core_machine_cpu_is_halted(&fixture.execution)) return 1;
    core_machine_cpu_set_nmi_mask(&fixture.execution, LIB_FALSE);
    if (core_machine_cpu_nmi_is_masked(&fixture.execution) ||
        fixture.cpu.data.flagNMI) return 1;
    if (!core_machine_cpu_request_nmi(&fixture.execution) ||
        !core_machine_cpu_request_nmi(&fixture.execution)) return 1;

    /* Masking after admission retains the single pending edge until delivery. */
    core_machine_cpu_set_nmi_mask(&fixture.execution, LIB_TRUE);
    core_machine_cpu_execution_refresh(&fixture.execution);
    if (!core_machine_cpu_is_halted(&fixture.execution) ||
        !fixture.cpu.data.flagNMI) return 1;
    core_machine_cpu_set_nmi_mask(&fixture.execution, LIB_FALSE);
    core_machine_cpu_execution_refresh(&fixture.execution);
    core_machine_cpu_capture_state(&fixture.execution, &state);
    if (state.halted || state.eip != 0x200u || fixture.cpu.data.flagNMI ||
        fixture.cpu.data.sp != 0x6fau || fixture.acknowledgements != 0u ||
        fixture.instructions.data.except != 0u) return 1;
    fixture.cpu.data.eip = 0x345u;
    return state.eip != 0x200u;
}

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

static lib_i32 cpu_execution_context_reset_case(
    core_machine_cpu_execution_context *context, t_cpu *cpu,
    const cpu_reset_case *test_case)
{
    const core_machine_instruction_timing timing = { .base_ticks = 1u };

    if (context == LIB_NULL || cpu == LIB_NULL || test_case == LIB_NULL) return 1;
    core_machine_cpu_execution_context_bind_profiles(context, test_case->profile,
        X86_FPU_PROFILE_NONE, LIB_FALSE, &timing);
    core_machine_cpu_state_reset(context);
    return cpu->data.cs.selector != 0xf000u || cpu->data.eip != 0x0000fff0u ||
        cpu->data.cs.base != test_case->code_base ||
        cpu->data.cs.base + cpu->data.eip != test_case->first_fetch ||
        (test_case->profile == CORE_MACHINE_CPU_PROFILE_80386 &&
         cpu->data.edx != 0x00000300u);
}

static lib_i32 cpu_instance_case(const cpu_reset_case *test_case)
{
    core_machine_cpu_execution_context *first = LIB_NULL;
    core_machine_cpu_execution_context *second = LIB_NULL;
    const core_machine_instruction_timing timing = { .base_ticks = 1u };
    core_machine_cpu_state first_state;
    core_machine_cpu_state second_state;
    core_machine_cpu_prepared_entry *entry = LIB_NULL;
    core_machine_entry_plan_state entry_state = {
        .cs = 0x1234u, .ip = 0x5678u, .sp = 0x700u, .eax = 0x12345678u
    };
    lib_i32 failed = 0;

    if (core_machine_cpu_create(&cpu_bus_provider, LIB_NULL, &first) !=
            LIB_STATUS_OK ||
        core_machine_cpu_create(&cpu_bus_provider, LIB_NULL, &second) !=
            LIB_STATUS_OK) {
        core_machine_cpu_destroy(second);
        core_machine_cpu_destroy(first);
        return 1;
    }
    core_machine_cpu_execution_context_bind_profiles(first, test_case->profile,
        X86_FPU_PROFILE_NONE, LIB_FALSE, &timing);
    core_machine_cpu_execution_context_bind_profiles(second, test_case->profile,
        X86_FPU_PROFILE_NONE, LIB_FALSE, &timing);
    core_machine_cpu_state_initialize(first);
    core_machine_cpu_state_initialize(second);
    core_machine_cpu_state_reset(first);
    core_machine_cpu_state_reset(second);
    core_machine_cpu_capture_state(first, &first_state);
    core_machine_cpu_capture_state(second, &second_state);
    failed |= first_state.cs != 0xf000u || second_state.cs != 0xf000u ||
        first_state.cs_base != test_case->code_base ||
        second_state.cs_base != test_case->code_base ||
        core_machine_cpu_linear_pc(first) != test_case->first_fetch ||
        core_machine_cpu_linear_pc(second) != test_case->first_fetch;
    core_machine_cpu_execution_request_stop(first);
    failed |= core_machine_cpu_execution_consume_stop_request(second) ||
        !core_machine_cpu_execution_consume_stop_request(first);
    failed |= core_machine_cpu_prepare_entry(first, &entry_state, &entry) !=
        LIB_STATUS_OK;
    failed |= core_machine_cpu_linear_pc(first) != test_case->first_fetch;
    core_machine_cpu_finish_entry(entry, LIB_FALSE);
    failed |= core_machine_cpu_linear_pc(first) != test_case->first_fetch;
    entry = LIB_NULL;
    failed |= core_machine_cpu_prepare_entry(first, &entry_state, &entry) !=
        LIB_STATUS_OK;
    core_machine_cpu_finish_entry(entry, LIB_TRUE);
    failed |= core_machine_cpu_linear_pc(first) != 0x179b8u ||
        core_machine_cpu_linear_pc(second) != test_case->first_fetch;
    entry_state.eflags = 0x80000000u;
    entry = LIB_NULL;
    failed |= core_machine_cpu_prepare_entry(first, &entry_state, &entry) !=
        LIB_STATUS_INVALID_ARGUMENT || entry != LIB_NULL ||
        core_machine_cpu_linear_pc(first) != 0x179b8u;
    core_machine_cpu_destroy(first);
    core_machine_cpu_state_reset(second);
    failed |= core_machine_cpu_linear_pc(second) != test_case->first_fetch;
    core_machine_cpu_destroy(second);
    core_machine_cpu_destroy(LIB_NULL);
    return failed;
}

static lib_i32 cpu_prefetch_case(core_machine_cpu_profile profile)
{
    cpu_bus_fixture fixture;
    core_machine_cpu_execution_context *cpu = &fixture.execution;
    lib_bool byte_queue = profile == CORE_MACHINE_CPU_PROFILE_8088;
    lib_u8 count = byte_queue ? 4u : 15u;
    lib_u32 next = byte_queue ? 0x11u : 0x17u;
    lib_i32 failed = 0;

    cpu_bus_prepare(&fixture, profile);
    cpu->prefetch_linear = 0x10u;
    cpu->prefetch_count = count;
    cpu->prefetch_valid = LIB_TRUE;
    cpu->prefetch_expected_linear = next;
    cpu->prefetch_expected_valid = LIB_TRUE;
    core_machine_cpu_execution_reserve_prefetch(cpu);
    failed |= !cpu->prefetch_reservation_valid ||
        cpu->prefetch_reservation_linear != (byte_queue ? 0x14u : next) ||
        cpu->prefetch_reservation_count != (byte_queue ? 1u : count);
    core_machine_cpu_execution_advance_prefetch_reservation(cpu);
    failed |= cpu->prefetch_reservation_valid || !cpu->prefetch_valid ||
        cpu->prefetch_linear != (byte_queue ? next : 0x10u) ||
        cpu->prefetch_count != count;
    if (byte_queue) cpu->prefetch_expected_linear = 0x12u;
    core_machine_cpu_execution_reserve_prefetch(cpu);
    failed |= !cpu->prefetch_reservation_valid;
    core_machine_cpu_execution_invalidate_prefetch(cpu);
    failed |= cpu->prefetch_reservation_valid || cpu->prefetch_valid ||
        cpu->prefetch_expected_valid;
    cpu->prefetch_linear = 0x10u;
    cpu->prefetch_count = count;
    cpu->prefetch_valid = LIB_TRUE;
    cpu->prefetch_expected_linear = next;
    cpu->prefetch_expected_valid = LIB_TRUE;
    core_machine_cpu_execution_reserve_prefetch(cpu);
    failed |= !cpu->prefetch_reservation_valid;
    core_machine_cpu_state_reset(cpu);
    failed |= cpu->prefetch_reservation_valid || cpu->prefetch_valid ||
        cpu->prefetch_expected_valid;
    return failed;
}

static lib_i32 cpu_80186_lgdt_gate(void)
{
    /* Migrated from the 286 board test: 80186 cannot load an IDTR, so the
       deliberately unavailable vector belongs to this chip-owned fixture. */
    static const lib_u8 program[] = { 0x0fu, 0x01u, 0x16u, 0x00u, 0x01u };
    static const lib_u8 gdtr[] = { 0x17u, 0u, 0u, 0x03u, 0u, 0u };
    static const lib_u8 gdt[] = {
        0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
        0xffu, 0xffu, 0u, 0x20u, 0u, 0x9au, 0u, 0u,
        0xffu, 0xffu, 0u, 0x30u, 0u, 0x92u, 0u, 0u
    };
    cpu_bus_fixture fixture;

    cpu_bus_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80186);
    fixture.cpu.data.eip = 0u;
    fixture.cpu.data.idtr.limit = 0x17u;
    lib_memory_copy(fixture.memory, program, sizeof(program));
    lib_memory_copy(fixture.memory + 0x0100u, gdtr, sizeof(gdtr));
    lib_memory_copy(fixture.memory + 0x0300u, gdt, sizeof(gdt));
    core_machine_cpu_execution_refresh(&fixture.execution);
    return fixture.instructions.data.except != VCPUINS_EXCEPT_UD ||
        fixture.faults != 1u || !fixture.fault.valid ||
        fixture.fault.exception_mask != VCPUINS_EXCEPT_UD ||
        fixture.fault.point.eip != 0u || fixture.cpu.data.eip != 0u;
}

static lib_i32 cpu_paging_prepare(cpu_bus_fixture *fixture,
    core_machine_cpu_profile profile, const lib_u8 *code, lib_size bytes)
{
    cpu_bus_prepare(fixture, profile);
    core_machine_cpu_state_reset(&fixture->execution);
    if (bytes > sizeof(fixture->memory) ||
        core_machine_cpu_execution_load_segment(&fixture->execution,
            &fixture->cpu.data.cs, 0u) ||
        core_machine_cpu_execution_load_segment(&fixture->execution,
            &fixture->cpu.data.ds, 0u) ||
        core_machine_cpu_execution_load_segment(&fixture->execution,
            &fixture->cpu.data.es, 0u) ||
        core_machine_cpu_execution_load_segment(&fixture->execution,
            &fixture->cpu.data.ss, 0u)) return 0;
    fixture->cpu.data.eip = 0u;
    fixture->cpu.data.idtr.limit = 0x17u;
    lib_memory_copy(fixture->memory, code, bytes);
    return 1;
}

/* These producer-rejection cases use an artificial unavailable IDT. Keep
 * them inside the CPU owner, including their original 80186 variants. */
static lib_i32 cpu_paging_control_gate(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_size bytes)
{
    cpu_bus_fixture fixture;

    if (!cpu_paging_prepare(&fixture, profile, code, bytes)) return 1;
    for (lib_u32 step = 0u; step < 128u && fixture.faults == 0u; ++step)
        core_machine_cpu_execution_refresh(&fixture.execution);
    return fixture.instructions.data.except != VCPUINS_EXCEPT_UD ||
        fixture.faults != 1u || !fixture.fault.valid ||
        fixture.fault.exception_mask != VCPUINS_EXCEPT_UD;
}

static lib_i32 cpu_paging_control_forms(void)
{
    static const lib_u8 write_reserved_cr1[] = {
        0x66u, 0xb8u, 0x34u, 0x12u, 0x00u, 0x00u,
        0x0fu, 0x22u, 0xc8u
    };
    static const lib_u8 pg_without_pe[] = {
        0x66u, 0xb8u, 0x00u, 0x00u, 0x00u, 0x80u,
        0x0fu, 0x22u, 0xc0u
    };
    static const lib_u8 unaligned_cr3[] = {
        0x66u, 0xb8u, 0x34u, 0x12u, 0x00u, 0x00u,
        0x0fu, 0x22u, 0xd8u
    };
    static const lib_u8 read_cr0[] = { 0x0fu, 0x20u, 0xc0u };
    lib_i32 failed = 0;

    failed |= cpu_paging_control_gate(CORE_MACHINE_CPU_PROFILE_80386,
        write_reserved_cr1, sizeof(write_reserved_cr1));
    failed |= cpu_paging_control_gate(CORE_MACHINE_CPU_PROFILE_80386,
        pg_without_pe, sizeof(pg_without_pe));
    failed |= cpu_paging_control_gate(CORE_MACHINE_CPU_PROFILE_80386,
        unaligned_cr3, sizeof(unaligned_cr3));
    failed |= cpu_paging_control_gate(CORE_MACHINE_CPU_PROFILE_80286,
        read_cr0, sizeof(read_cr0));
    failed |= cpu_paging_control_gate(CORE_MACHINE_CPU_PROFILE_80186,
        read_cr0, sizeof(read_cr0));
    return failed;
}

static lib_i32 cpu_paging_invlpg_case(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_size bytes)
{
    cpu_bus_fixture fixture;
    t_cpu before;

    if (!cpu_paging_prepare(&fixture, profile, code, bytes)) return 1;
    fixture.cpu.data.eax = 0x11223344u;
    fixture.cpu.data.ecx = 0x55667788u;
    fixture.cpu.data.edx = 0x99aabbccu;
    fixture.cpu.data.ebx = 0xddeeff00u;
    fixture.cpu.data.esi = 0x13579bdfu;
    fixture.cpu.data.edi = 0x2468ace0u;
    fixture.cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_IF;
    before = fixture.cpu;
    core_machine_cpu_execution_refresh(&fixture.execution);
    return fixture.instructions.data.except != VCPUINS_EXCEPT_UD ||
        fixture.faults != 1u || !fixture.fault.valid ||
        fixture.fault.exception_mask != VCPUINS_EXCEPT_UD ||
        fixture.fault.exception_code != 0u || fixture.fault.point.cs != 0u ||
        fixture.fault.point.linear_pc != 0u ||
        lib_memory_compare(&fixture.cpu, &before, sizeof(before)) != 0;
}

static lib_i32 cpu_paging_invlpg_rejection(void)
{
    static const lib_u8 invlpg[] = {
        0x0fu, 0x01u, 0x3eu, 0x00u, 0x20u
    };
    static const lib_u8 operand_size[] = {
        0x66u, 0x0fu, 0x01u, 0x3eu, 0x00u, 0x20u
    };
    static const lib_u8 address_size[] = {
        0x67u, 0x0fu, 0x01u, 0x3eu, 0x00u, 0x20u
    };
    static const lib_u8 combined_size[] = {
        0x66u, 0x67u, 0x0fu, 0x01u, 0x3eu, 0x00u, 0x20u
    };
    static const lib_u8 locked[] = {
        0xf0u, 0x0fu, 0x01u, 0x3eu, 0x00u, 0x20u
    };
    if (cpu_paging_invlpg_case(CORE_MACHINE_CPU_PROFILE_80186, invlpg,
            sizeof(invlpg))) return 1;
    if (cpu_paging_invlpg_case(CORE_MACHINE_CPU_PROFILE_80286, invlpg,
            sizeof(invlpg))) return 2;
    if (cpu_paging_invlpg_case(CORE_MACHINE_CPU_PROFILE_80386,
            invlpg, sizeof(invlpg))) return 3;
    if (cpu_paging_invlpg_case(CORE_MACHINE_CPU_PROFILE_80386,
            operand_size, sizeof(operand_size))) return 4;
    if (cpu_paging_invlpg_case(CORE_MACHINE_CPU_PROFILE_80386,
            address_size, sizeof(address_size))) return 5;
    if (cpu_paging_invlpg_case(CORE_MACHINE_CPU_PROFILE_80386,
            combined_size, sizeof(combined_size))) return 6;
    if (cpu_paging_invlpg_case(CORE_MACHINE_CPU_PROFILE_80386,
            locked, sizeof(locked))) return 7;
    return 0;
}

static lib_i32 cpu_paging_cr0_mutable_controls(void)
{
    static const lib_u8 write_mutable[] = {
        0x66u, 0xb8u, 0x1eu, 0x00u, 0x00u, 0x00u,
        0x0fu, 0x22u, 0xc0u,
        0x0fu, 0x20u, 0xc1u,
        0xf4u
    };
    const lib_u32 mutable = VCPU_CR0_MP | VCPU_CR0_EM | VCPU_CR0_TS |
        VCPU_CR0_ET;
    cpu_bus_fixture fixture;
    t_cpu before;
    t_cpu cpu;
    lib_i32 failed = !cpu_paging_prepare(&fixture,
        CORE_MACHINE_CPU_PROFILE_80386, write_mutable, sizeof(write_mutable));

    if (!failed) {
        before = fixture.cpu;
        for (lib_u32 step = 0u; step < 4u && fixture.faults == 0u; ++step)
            core_machine_cpu_execution_refresh(&fixture.execution);
        cpu = fixture.cpu;

        failed |= fixture.faults != 0u || !cpu.data.flagHalt || cpu.data.cr0 != mutable ||
            cpu.data.eax != mutable || cpu.data.ecx != mutable ||
            cpu.data.eip != sizeof(write_mutable) || cpu.data.eflags != 0x02u ||
            cpu.data.ebx != 0u || cpu.data.edx != 0x00000300u || cpu.data.esp != 0u ||
            cpu.data.ebp != 0u || cpu.data.esi != 0u || cpu.data.edi != 0u ||
            lib_memory_compare(&cpu.data.es, &before.data.es, sizeof(cpu.data.es)) != 0 ||
            lib_memory_compare(&cpu.data.cs, &before.data.cs, sizeof(cpu.data.cs)) != 0 ||
            lib_memory_compare(&cpu.data.ss, &before.data.ss, sizeof(cpu.data.ss)) != 0 ||
            lib_memory_compare(&cpu.data.ds, &before.data.ds, sizeof(cpu.data.ds)) != 0 ||
            lib_memory_compare(&cpu.data.fs, &before.data.fs, sizeof(cpu.data.fs)) != 0 ||
            lib_memory_compare(&cpu.data.gs, &before.data.gs, sizeof(cpu.data.gs)) != 0;
    }
    core_machine_cpu_execution_finalize(&fixture.execution);
    return failed;
}

/* Private pending-event and whole-cache invariants belong to the CPU owner;
 * the board interrupt corpus separately checks PIC, frames and transactions. */
static void cpu_interrupt_prepare(cpu_bus_fixture *fixture, lib_u8 vector,
    lib_u8 gate_type)
{
    const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0xfau,0x40u,0,
        0xffu,0xffu,0,0,0,0x92u,0xcfu,0
    };
    const lib_u8 gate[] = {0u,1u,0x0bu,0u,0u,gate_type,0u,0u};
    t_cpu *cpu = &fixture->cpu;

    cpu_bus_prepare(fixture, CORE_MACHINE_CPU_PROFILE_80386);
    lib_memory_copy(fixture->memory + 0x300u, gdt, sizeof(gdt));
    lib_memory_copy(fixture->memory + 0x400u + vector * 8u, gate, sizeof(gate));
    cpu->data.cr0 = VCPU_CR0_PE;
    cpu->data.gdtr.flagValid = LIB_TRUE;
    cpu->data.gdtr.sregtype = SREG_GDTR;
    cpu->data.gdtr.base = 0x300u;
    cpu->data.gdtr.limit = sizeof(gdt) - 1u;
    cpu->data.idtr.flagValid = LIB_TRUE;
    cpu->data.idtr.sregtype = SREG_IDTR;
    cpu->data.idtr.base = 0x400u;
    cpu->data.idtr.limit = 0x187u;
    cpu->data.cs.flagValid = LIB_TRUE;
    cpu->data.cs.selector = 0x0bu;
    cpu->data.cs.sregtype = SREG_CODE;
    cpu->data.cs.base = 0x2000u;
    cpu->data.cs.limit = 0xffffu;
    cpu->data.cs.dpl = 3u;
    cpu->data.cs.seg.accessed = LIB_FALSE;
    cpu->data.cs.seg.executable = LIB_TRUE;
    cpu->data.cs.seg.exec.defsize = LIB_TRUE;
    cpu->data.cs.seg.exec.conform = LIB_FALSE;
    cpu->data.cs.seg.exec.readable = LIB_TRUE;
    cpu->data.ss.flagValid = LIB_TRUE;
    cpu->data.ss.selector = 0x10u;
    cpu->data.ss.sregtype = SREG_STACK;
    cpu->data.ss.base = 0u;
    cpu->data.ss.limit = 0xffffffffu;
    cpu->data.ss.dpl = 0u;
    cpu->data.ss.seg.accessed = LIB_FALSE;
    cpu->data.ss.seg.executable = LIB_FALSE;
    cpu->data.ss.seg.data.big = LIB_TRUE;
    cpu->data.ss.seg.data.expdown = LIB_FALSE;
    cpu->data.ss.seg.data.writable = LIB_TRUE;
    cpu->data.eip = 0u;
    cpu->data.esp = 0x8000u;
    cpu->data.eflags = 0x302u;
    cpu->data.flagHalt = LIB_FALSE;
}

/* Preserve the full private-cache comparisons from the board UD corpus;
 * a copied public snapshot deliberately does not expose cache bookkeeping. */
static lib_i32 cpu_ud_cache_preservation(void)
{
    static const lib_u8 forms[][3] = {
        {0xf1u, 0u, 0u}, {0x0fu, 0x01u, 0xf8u},
        {0x0fu, 0x25u, 0xc0u}, {0x62u, 0xc0u, 0u},
        {0xf0u, 0x90u, 0u}
    };

    for (lib_size index = 0u; index < sizeof(forms) / sizeof(forms[0]); ++index) {
        for (lib_u8 reject = 0u; reject != 2u; ++reject) {
            cpu_bus_fixture fixture;
            t_cpu before;

            cpu_interrupt_prepare(&fixture, 6u, reject ? 0x80u : 0x8eu);
            /* The original UD cases use ring-zero 16-bit protected code. */
            fixture.memory[0x30du] = 0x9au;
            fixture.memory[0x30eu] = 0u;
            fixture.memory[0x432u] = 0x08u;
            fixture.cpu.data.cs.selector = 0x08u;
            fixture.cpu.data.cs.dpl = 0u;
            fixture.cpu.data.cs.seg.exec.defsize = LIB_FALSE;
            fixture.cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_IF |
                VCPU_EFLAGS_DF;
            lib_memory_copy(fixture.memory, forms[index], sizeof(forms[index]));
            before = fixture.cpu;
            core_machine_cpu_execution_refresh(&fixture.execution);
            if (lib_memory_compare(&fixture.cpu.data.es, &before.data.es,
                    sizeof(before.data.es)) ||
                lib_memory_compare(&fixture.cpu.data.ss, &before.data.ss,
                    sizeof(before.data.ss)) ||
                lib_memory_compare(&fixture.cpu.data.ds, &before.data.ds,
                    sizeof(before.data.ds)) ||
                lib_memory_compare(&fixture.cpu.data.fs, &before.data.fs,
                    sizeof(before.data.fs)) ||
                lib_memory_compare(&fixture.cpu.data.gs, &before.data.gs,
                    sizeof(before.data.gs))) return 1;
            if (reject) {
                if (!fixture.faults || fixture.fault.exception_mask != VCPUINS_EXCEPT_UD ||
                    fixture.cpu.data.eip != before.data.eip ||
                    fixture.cpu.data.esp != before.data.esp ||
                    fixture.cpu.data.eflags != before.data.eflags) return 1;
            } else if (fixture.faults || fixture.cpu.data.eip != 0x100u ||
                fixture.cpu.data.esp != before.data.esp - 12u) return 1;
        }
    }
    return 0;
}

static lib_i32 cpu_interrupt_pending_and_rollback(void)
{
    for (lib_u8 reject = 0u; reject != 2u; ++reject) {
        cpu_bus_fixture fixture;
        t_cpu before;

        cpu_interrupt_prepare(&fixture, 2u, reject ? 0x80u : 0x8eu);
        fixture.memory[0] = 0x90u;
        fixture.cpu.data.eflags = 0x202u;
        if (!core_machine_cpu_request_nmi(&fixture.execution)) return 1;
        before = fixture.cpu;
        core_machine_cpu_execution_refresh(&fixture.execution);
        if (fixture.cpu.data.flagNMI != reject) return 1;
        if (reject) {
            if (!fixture.faults || fixture.cpu.data.esp != before.data.esp ||
                fixture.cpu.data.eflags != before.data.eflags ||
                lib_memory_compare(&fixture.cpu.data.cs, &before.data.cs,
                    sizeof(before.data.cs)) ||
                lib_memory_compare(&fixture.cpu.data.ss, &before.data.ss,
                    sizeof(before.data.ss))) return 1;
        } else if (fixture.faults || fixture.cpu.data.esp != 0x7ff4u ||
            fixture.cpu.data.eip != 0x100u) return 1;
    }
    for (lib_u8 failure = 0u; failure != 4u; ++failure) {
        cpu_bus_fixture fixture;
        t_cpu before;
        const lib_u8 code[] = {0x0fu, 0x01u, 0xf0u};
        const lib_u8 gate = failure == 0u ? 0x80u :
            failure == 1u ? 0x0eu : 0x8eu;

        cpu_interrupt_prepare(&fixture, 0x0du, gate);
        lib_memory_copy(fixture.memory, code, sizeof(code));
        if (failure == 2u) fixture.memory[0x30du] = 0x7au;
        if (failure == 3u) fixture.cpu.data.ss.limit = 0x7ffeu;
        before = fixture.cpu;
        core_machine_cpu_execution_refresh(&fixture.execution);
        if (!fixture.faults || !(fixture.fault.exception_mask & VCPUINS_EXCEPT_DF) ||
            fixture.fault.exception_code != 0u ||
            fixture.cpu.data.eip != before.data.eip ||
            fixture.cpu.data.esp != before.data.esp ||
            fixture.cpu.data.eflags != before.data.eflags ||
            lib_memory_compare(&fixture.cpu.data.cs, &before.data.cs,
                sizeof(before.data.cs)) ||
            lib_memory_compare(&fixture.cpu.data.ss, &before.data.ss,
                sizeof(before.data.ss))) return 1;
    }
    return 0;
}

lib_i32 main(void)
{
    static const cpu_reset_case reset_cases[] = {
        {CORE_MACHINE_CPU_PROFILE_8086, 0x000f0000u, 0x000ffff0u},
        {CORE_MACHINE_CPU_PROFILE_8088, 0x000f0000u, 0x000ffff0u},
        {CORE_MACHINE_CPU_PROFILE_80186, 0x000f0000u, 0x000ffff0u},
        {CORE_MACHINE_CPU_PROFILE_80286, 0x00ff0000u, 0x00fffff0u},
        {CORE_MACHINE_CPU_PROFILE_80386, 0xffff0000u, 0xfffffff0u}
    };
    t_cpu first_cpu = {0};
    t_cpu second_cpu = {0};
    t_cpuins first_instructions = {0};
    t_cpuins second_instructions = {0};
    core_machine_cpu_execution_context first = {0};
    core_machine_cpu_execution_context second = {0};
    lib_i32 result = 0;

    core_machine_cpu_execution_context_initialize(
        &first, &first_cpu, &first_instructions, LIB_NULL, LIB_NULL);
    core_machine_cpu_execution_context_initialize(
        &second, &second_cpu, &second_instructions, LIB_NULL, LIB_NULL);
    core_machine_cpu_state_initialize(&first);
    core_machine_cpu_state_initialize(&second);
    core_machine_cpu_state_reset(&first);
    core_machine_cpu_state_reset(&second);

    result |= first_cpu.data.cs.selector != 0xf000u;
    result |= first_cpu.data.eip != 0x0000fff0u;
    result |= second_cpu.data.cs.selector != 0xf000u;
    result |= second_cpu.data.eip != 0x0000fff0u;

    first_cpu.data.eip = 0x12345678u;
    first_instructions.data.except = VCPUINS_EXCEPT_UD;
    core_machine_cpu_execution_request_stop(&first);
    core_machine_cpu_execution_request_reset(&first);

    result |= second_cpu.data.eip != 0x0000fff0u;
    result |= second_instructions.data.except != 0u;
    result |= core_machine_cpu_execution_consume_stop_request(&second);
    result |= core_machine_cpu_execution_consume_reset_request(&second);
    result |= !core_machine_cpu_execution_consume_stop_request(&first);
    result |= !core_machine_cpu_execution_consume_reset_request(&first);

    for (lib_size index = 0u;
         index < sizeof(reset_cases) / sizeof(reset_cases[0]); ++index) {
        result |= cpu_execution_context_reset_case(&first, &first_cpu,
            &reset_cases[index]);
        result |= cpu_bus_cases(&reset_cases[index]);
        result |= cpu_debug_case(reset_cases[index].profile);
        result |= cpu_signal_case(reset_cases[index].profile);
        result |= cpu_timing_case(reset_cases[index].profile);
        result |= cpu_instance_case(&reset_cases[index]);
        result |= cpu_prefetch_case(reset_cases[index].profile);
    }

    result |= cpu_interrupt_pending_and_rollback();
    result |= cpu_ud_cache_preservation();
    result |= cpu_80186_lgdt_gate();
    result |= cpu_paging_control_forms();
    result |= cpu_paging_cr0_mutable_controls();
    result |= cpu_paging_invlpg_rejection();
    core_machine_cpu_execution_finalize(&second);
    core_machine_cpu_execution_finalize(&first);
    if (result != 0) return 1;

    puts("M5:T66:S1:CPU-CONTEXT:OK");
    return 0;
}
