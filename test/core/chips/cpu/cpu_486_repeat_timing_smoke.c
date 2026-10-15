#include "support/cpu_port_instruction_fixture.h"
#include "core/chips/cpu/cpu_timing.h"
#include "lib/types/file.h"

static lib_bool repeat_expect(cpu_instruction_fixture *fixture,
    core_machine_retirement_repeat_phase phase, lib_bool active,
    lib_u64 nominal, lib_u64 decode)
{
    const core_machine_cpu_execution_context before = fixture->execution;
    const t_cpuins instructions = fixture->instructions;
    const t_cpu cpu = fixture->cpu;
    lib_u64 ticks = LIB_UINT64_MAX, penalty = LIB_UINT64_MAX;

    if (fixture->fault.valid || before.timing_result.repeat_phase != phase ||
        before.source_repeat_active != active ||
        (before.timing_result.retirement_origin == CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_COMPATIBILITY &&
         !before.timing_result.source_timing_unallocated) ||
        !core_machine_80486_nominal_instruction_cost(&fixture->execution, &ticks) || ticks != nominal ||
        !core_machine_80486_decode_penalty(&fixture->execution, &penalty) || penalty != decode ||
        lib_memory_compare(&before, &fixture->execution, sizeof(before)) ||
        lib_memory_compare(&instructions, &fixture->instructions, sizeof(instructions)) ||
        lib_memory_compare(&cpu, &fixture->cpu, sizeof(cpu))) {
        lib_c_fprintf(lib_c_stderr, "Repeat phase=%u/%u active=%u/%u clocks=%llu/%llu decode=%llu/%llu\n",
            (lib_u32)before.timing_result.repeat_phase, (lib_u32)phase,
            (lib_u32)before.source_repeat_active, (lib_u32)active, ticks, nominal, penalty, decode);
        return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool repeat_memory(void)
{
    static const struct {
        lib_u8 opcode, prefix, first, iteration;
    } cases[] = {
        {0xa4u, 0xf3u, 15u, 3u}, {0xa5u, 0xf3u, 15u, 3u},
        {0xa6u, 0xf3u, 14u, 7u}, {0xa7u, 0xf3u, 14u, 7u},
        {0xa6u, 0xf2u, 14u, 7u}, {0xa7u, 0xf2u, 14u, 7u},
        {0xaau, 0xf3u, 11u, 4u}, {0xabu, 0xf3u, 11u, 4u},
        {0xacu, 0xf3u, 11u, 4u}, {0xadu, 0xf3u, 11u, 4u},
        {0xaeu, 0xf3u, 12u, 5u}, {0xafu, 0xf3u, 12u, 5u},
        {0xaeu, 0xf2u, 12u, 5u}, {0xafu, 0xf2u, 12u, 5u}
    };

    for (lib_u8 code32 = 0u; code32 < 2u; ++code32)
    for (lib_u8 sizes = 0u; sizes < 4u; ++sizes)
    for (lib_u8 backward = 0u; backward < 2u; ++backward)
    for (lib_u8 count = 0u; count < 4u; ++count)
    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        cpu_instruction_fixture fixture;
        lib_u8 code[6], bytes = 0u;
        const lib_bool operand_prefix = (sizes & 1u) != 0u;
        const lib_bool address_prefix = (sizes & 2u) != 0u;
        const lib_u64 prefixes = 1u + operand_prefix + address_prefix;
        t_cpu after;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX);
        fixture.cpu.data.cs.seg.exec.defsize = code32;
        fixture.cpu.data.esi = 0x2040u;
        fixture.cpu.data.edi = 0x3040u;
        fixture.cpu.data.ecx = count;
        if (backward) fixture.cpu.data.eflags |= VCPU_EFLAGS_DF;
        if (cases[index].prefix == 0xf2u)
            lib_memory_set(fixture.memory + 0x3000u, 1u, 128u);
        code[bytes++] = 0x3eu;
        if (operand_prefix) code[bytes++] = 0x66u;
        if (address_prefix) code[bytes++] = 0x67u;
        code[bytes++] = cases[index].prefix;
        code[bytes++] = cases[index].opcode;
        code[bytes] = 0x90u;
        if (cpu_instruction_run(&fixture, code, bytes + 1u, &after) != LIB_STATUS_OK)
            return LIB_FALSE;
        for (lib_u8 step = 0u; step < (count ? count : 1u); ++step) {
            const lib_bool active = count > step + 1u;
            const core_machine_retirement_repeat_phase phase = !count ?
                CORE_MACHINE_RETIREMENT_REPEAT_ZERO_COUNT : step ?
                CORE_MACHINE_RETIREMENT_REPEAT_CONTINUATION : CORE_MACHINE_RETIREMENT_REPEAT_FIRST;
            const lib_u64 first = count == 1u && cases[index].opcode <= 0xa5u ? 13u : cases[index].first;
            const lib_u64 expected = !count ? 5u : step ? cases[index].iteration : first;

            if (fixture.cpu.data.ecx != (count ? count - step - 1u : 0u) ||
                fixture.cpu.data.eip != (active ? 0u : bytes) ||
                !repeat_expect(&fixture, phase, active, expected, step ? 0u : prefixes)) {
                lib_c_fprintf(lib_c_stderr, "Memory repeat opcode=%x prefix=%x code32=%u sizes=%u df=%u count=%u step=%u\n",
                    cases[index].opcode, cases[index].prefix, code32, sizes, backward, count, step);
                return LIB_FALSE;
            }
            if (active) core_machine_cpu_execution_refresh(&fixture.execution);
        }
        core_machine_cpu_execution_refresh(&fixture.execution);
        if (!repeat_expect(&fixture, CORE_MACHINE_RETIREMENT_REPEAT_NONE, LIB_FALSE, 1u, 0u))
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool repeat_ports(void)
{
    static const lib_u8 input_setup[] = {16u, 10u, 30u, 29u};
    static const lib_u8 output_setup[] = {17u, 11u, 31u, 30u};

    for (lib_u8 mode = 0u; mode < 4u; ++mode)
    for (lib_u8 opcode = 0x6cu; opcode <= 0x6fu; ++opcode)
    for (lib_u8 sizes = 0u; sizes < 4u; ++sizes)
    for (lib_u8 count = 0u; count < 4u; ++count) {
        cpu_port_instruction_fixture port;
        cpu_instruction_fixture *fixture = &port.instruction;
        lib_u8 code[5], bytes = 0u;
        const lib_u64 setup = opcode < 0x6eu ? input_setup[mode] : output_setup[mode];
        const lib_u64 iteration = opcode < 0x6eu ? 8u : 5u;
        t_cpu after;

        cpu_port_prepare(&port, CORE_MACHINE_CPU_PROFILE_80486SX);
        fixture->cpu.data.esp = 0x8000u;
        fixture->cpu.data.edx = 0x20u;
        fixture->cpu.data.esi = 0x2000u;
        fixture->cpu.data.edi = 0x3000u;
        fixture->cpu.data.ecx = count;
        if (mode) {
            fixture->cpu.data.cr0 |= VCPU_CR0_PE;
            fixture->cpu.data.cs.dpl = mode == 1u ? 0u : 3u;
            fixture->cpu.data.ss.dpl = fixture->cpu.data.cs.dpl;
            fixture->cpu.data.ds.selector = fixture->cpu.data.es.selector =
                (lib_u16)(0x10u | fixture->cpu.data.cs.dpl);
            fixture->cpu.data.ds.dpl = fixture->cpu.data.es.dpl = fixture->cpu.data.cs.dpl;
            if (mode == 3u) fixture->cpu.data.eflags |= VCPU_EFLAGS_VM | VCPU_EFLAGS_IOPL;
            fixture->cpu.data.tr = (t_cpu_data_sreg){.flagValid = LIB_TRUE,
                .selector = 0x28u, .base = 0x600u, .limit = 0xffu,
                .sregtype = SREG_TR, .sys.type = VCPU_DESC_SYS_TYPE_TSS_32_BUSY};
            lib_memory_copy(fixture->memory + 0x666u, &(lib_u16){0x80u}, sizeof(lib_u16));
        }
        if (sizes & 1u) code[bytes++] = 0x66u;
        if (sizes & 2u) code[bytes++] = 0x67u;
        const lib_u64 prefixes = bytes;
        code[bytes++] = 0xf3u;
        code[bytes++] = opcode;
        code[bytes] = 0x90u;
        if (cpu_instruction_run(fixture, code, bytes + 1u, &after) != LIB_STATUS_OK) return LIB_FALSE;
        for (lib_u8 step = 0u; step < (count ? count : 1u); ++step) {
            const lib_bool active = count > step + 1u;
            const core_machine_retirement_repeat_phase phase = !count ?
                CORE_MACHINE_RETIREMENT_REPEAT_ZERO_COUNT : step ?
                CORE_MACHINE_RETIREMENT_REPEAT_CONTINUATION : CORE_MACHINE_RETIREMENT_REPEAT_FIRST;

            if (port.transfer_count != (count ? step + 1u : 0u) ||
                port.complete_count != port.transfer_count ||
                !repeat_expect(fixture, phase, active, !count ? setup :
                    iteration + (step ? 0u : setup), step ? 0u : prefixes)) {
                lib_c_fprintf(lib_c_stderr, "Port repeat mode=%u opcode=%x sizes=%u count=%u step=%u\n",
                    mode, opcode, sizes, count, step);
                return LIB_FALSE;
            }
            if (active) core_machine_cpu_execution_refresh(&fixture->execution);
        }
    }
    return LIB_TRUE;
}

static lib_bool repeat_boundaries(void)
{
    for (lib_u8 action = 0u; action < 6u; ++action) {
        cpu_instruction_fixture fixture;
        const lib_u8 code[] = {0x3eu, 0xf3u, 0xa4u};
        core_machine_cpu_execution_context *context = &fixture.execution;
        t_cpu after;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX);
        fixture.cpu.data.ecx = 3u;
        fixture.cpu.data.esi = 0x2000u;
        fixture.cpu.data.edi = 0x3000u;
        if (cpu_instruction_run(&fixture, code, sizeof(code), &after) != LIB_STATUS_OK ||
            !repeat_expect(&fixture, CORE_MACHINE_RETIREMENT_REPEAT_FIRST, LIB_TRUE, 15u, 1u))
            return LIB_FALSE;
        if (action == 0u) {
            core_machine_cpu_execution_request_debug_pause(context);
            if (!core_machine_cpu_execution_consume_debug_pause_request(context)) return LIB_FALSE;
            const core_machine_debug_register_patch invalid = {0};
            if (core_machine_cpu_debug_patch_registers(context, &invalid) != LIB_STATUS_INVALID_ARGUMENT)
                return LIB_FALSE;
        } else if (action == 1u) {
            const core_machine_debug_register_patch patch = {
                .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX)};
            if (core_machine_cpu_debug_patch_registers(context, &patch) != LIB_STATUS_OK) return LIB_FALSE;
        } else if (action == 2u || action == 3u) {
            const core_machine_entry_plan_state state = {.ip = 0u, .sp = 0x6000u,
                .ecx = 2u, .esi = 0x2001u, .edi = 0x3001u};
            core_machine_cpu_prepared_entry *entry = LIB_NULL;
            if (core_machine_cpu_prepare_entry(context, &state, &entry) != LIB_STATUS_OK) return LIB_FALSE;
            core_machine_cpu_finish_entry(entry, action == 3u);
        } else if (action == 4u) {
            const lib_u8 changed = 0x26u;
            if (core_machine_cpu_write_linear(context, 0u, &changed, 1u)) return LIB_FALSE;
        } else {
            if (core_machine_cpu_set_stop_clock(context, LIB_TRUE) != LIB_STATUS_OK) return LIB_FALSE;
            core_machine_cpu_execution_refresh(context);
            if (!context->source_repeat_active || fixture.cpu.data.ecx != 2u ||
                core_machine_cpu_set_clock_running(context, LIB_FALSE) != LIB_STATUS_OK)
                return LIB_FALSE;
            core_machine_cpu_execution_refresh(context);
            if (!context->source_repeat_active ||
                core_machine_cpu_set_clock_running(context, LIB_TRUE) != LIB_STATUS_OK ||
                core_machine_cpu_set_stop_clock(context, LIB_FALSE) != LIB_STATUS_OK) return LIB_FALSE;
        }
        core_machine_cpu_execution_refresh(context);
        const lib_bool first = action == 1u || action == 3u || action == 4u;
        /* Changing the bytes restarts setup, but does not erase the preceding
         * MOVS's SI/DI writes: the restarted address also has E.2's AGI. */
        const lib_u64 decode = action == 4u ? 2u : first ? 1u : 0u;
        if (!repeat_expect(&fixture, first ? CORE_MACHINE_RETIREMENT_REPEAT_FIRST :
            CORE_MACHINE_RETIREMENT_REPEAT_CONTINUATION, LIB_TRUE, first ? 15u : 3u, decode)) {
            lib_c_fprintf(lib_c_stderr, "Repeat boundary action=%u\n", action);
            return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_bool repeat_interrupt_and_fault(void)
{
    cpu_instruction_fixture fixture;
    const lib_u8 code[] = {0xf3u, 0xa4u};
    const lib_u16 vector[] = {0x100u, 0u};
    t_cpu after;

    cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX);
    fixture.cpu.data.esp = 0x6000u;
    fixture.cpu.data.ecx = 3u;
    fixture.cpu.data.esi = 0x2000u;
    fixture.cpu.data.edi = 0x3000u;
    fixture.memory[0x100u] = 0xcfu;
    lib_memory_copy(fixture.memory + 8u, vector, sizeof(vector));
    if (cpu_instruction_run(&fixture, code, sizeof(code), &after) != LIB_STATUS_OK ||
        !fixture.execution.source_repeat_active ||
        core_machine_cpu_set_nmi(&fixture.execution, LIB_TRUE) != LIB_STATUS_OK) return LIB_FALSE;
    core_machine_cpu_execution_refresh(&fixture.execution);
    /* Async entry reuses the decoder. Inspect the already-published retirement
     * and entry cost here, not a fresh nominal query on the handler's record. */
    if (fixture.cpu.data.eip != 0x100u ||
        fixture.execution.source_repeat_active ||
        fixture.execution.timing_result.repeat_phase != CORE_MACHINE_RETIREMENT_REPEAT_CONTINUATION ||
        fixture.execution.timing_result.ticks == 0u ||
        fixture.execution.timing_result.event_ticks != 29u) {
        lib_c_fprintf(lib_c_stderr, "Repeat NMI ip=%x active=%u phase=%u ticks=%llu event=%llu\n",
            fixture.cpu.data.eip, (lib_u32)fixture.execution.source_repeat_active,
            (lib_u32)fixture.execution.timing_result.repeat_phase,
            fixture.execution.timing_result.ticks, fixture.execution.timing_result.event_ticks);
        return LIB_FALSE;
    }
    core_machine_cpu_execution_refresh(&fixture.execution);
    if (fixture.cpu.data.eip != 0u || fixture.execution.source_repeat_active) {
        lib_c_fprintf(lib_c_stderr, "Repeat IRET ip=%x active=%u\n",
            fixture.cpu.data.eip, (lib_u32)fixture.execution.source_repeat_active);
        return LIB_FALSE;
    }
    core_machine_cpu_execution_refresh(&fixture.execution);
    if (!repeat_expect(&fixture, CORE_MACHINE_RETIREMENT_REPEAT_FIRST, LIB_FALSE, 13u, 0u))
        return LIB_FALSE;

    cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX);
    const lib_u8 faulting[] = {0x67u, 0xf3u, 0xa4u};
    fixture.cpu.data.ecx = 3u;
    fixture.cpu.data.ds.limit = LIB_UINT32_MAX;
    fixture.cpu.data.esi = sizeof(fixture.memory) - 1u;
    fixture.cpu.data.edi = 0x3000u;
    if (cpu_instruction_run(&fixture, faulting, sizeof(faulting), &after) != LIB_STATUS_OK ||
        !fixture.execution.source_repeat_active) return LIB_FALSE;
    core_machine_cpu_execution_refresh(&fixture.execution);
    if (!fixture.fault.valid || fixture.execution.source_repeat_active ||
        fixture.execution.timing_result.repeat_phase != CORE_MACHINE_RETIREMENT_REPEAT_NONE) {
        lib_c_fprintf(lib_c_stderr, "Repeat provider failure valid=%u active=%u phase=%u\n",
            (lib_u32)fixture.fault.valid, (lib_u32)fixture.execution.source_repeat_active,
            (lib_u32)fixture.execution.timing_result.repeat_phase);
        return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool repeat_termination_and_reset(void)
{
    for (lib_u8 opcode = 0xa6u; opcode <= 0xafu; ++opcode) {
        if (opcode != 0xa6u && opcode != 0xa7u && opcode != 0xaeu && opcode != 0xafu)
            continue;
        for (lib_u8 unequal = 0u; unequal < 2u; ++unequal) {
            cpu_instruction_fixture fixture;
            const lib_u8 code[] = {unequal ? 0xf3u : 0xf2u, opcode, 0x90u};
            t_cpu after;

            cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX);
            fixture.cpu.data.esi = 0x2000u;
            fixture.cpu.data.edi = 0x3000u;
            fixture.cpu.data.ecx = 3u;
            fixture.memory[0x3000u] = unequal;
            if (cpu_instruction_run(&fixture, code, sizeof(code), &after) != LIB_STATUS_OK ||
                fixture.cpu.data.ecx != 2u || fixture.cpu.data.eip != 2u ||
                !repeat_expect(&fixture, CORE_MACHINE_RETIREMENT_REPEAT_FIRST, LIB_FALSE,
                    opcode < 0xaeu ? 14u : 12u, 0u)) return LIB_FALSE;
            core_machine_cpu_execution_refresh(&fixture.execution);
            if (!repeat_expect(&fixture, CORE_MACHINE_RETIREMENT_REPEAT_NONE,
                LIB_FALSE, 1u, 0u)) return LIB_FALSE;
        }
    }
    {
        cpu_instruction_fixture fixture;
        const lib_u8 primitive[] = {0xa4u};
        const lib_u8 repeated[] = {0xf3u, 0xa4u};
        t_cpu after;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX);
        fixture.cpu.data.esi = 0x2000u;
        fixture.cpu.data.edi = 0x3000u;
        fixture.cpu.data.ecx = 3u;
        if (cpu_instruction_run(&fixture, primitive, sizeof(primitive), &after) != LIB_STATUS_OK ||
            fixture.cpu.data.ecx != 3u || !repeat_expect(&fixture,
                CORE_MACHINE_RETIREMENT_REPEAT_PRIMITIVE, LIB_FALSE, 7u, 0u)) return LIB_FALSE;
        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX);
        fixture.cpu.data.esi = 0x2000u;
        fixture.cpu.data.edi = 0x3000u;
        fixture.cpu.data.ecx = 3u;
        if (cpu_instruction_run(&fixture, repeated, sizeof(repeated), &after) != LIB_STATUS_OK ||
            !fixture.execution.source_repeat_active) return LIB_FALSE;
        core_machine_cpu_state_reset(&fixture.execution);
        if (fixture.execution.source_repeat_active) return LIB_FALSE;
    }
    return LIB_TRUE;
}

int main(void)
{
    if (!repeat_memory() || !repeat_ports() || !repeat_boundaries() ||
        !repeat_interrupt_and_fault() || !repeat_termination_and_reset()) return 1;
    lib_c_printf("80486:REPEAT-TIMING:OK\n");
    return 0;
}
