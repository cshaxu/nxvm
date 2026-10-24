#include "support/cpu_instruction_fixture.h"
#include "x86-core/cpu/cpu_timing.h"
#include "lib/types/file.h"

typedef struct bus_fixture {
    cpu_instruction_fixture instruction;
    lib_u64 write_wait;
    lib_u64 read_wait;
    lib_u64 port_wait;
    lib_u32 failed_write;
    lib_u32 reads;
    lib_u32 port_transfers;
    lib_u32 port_completions;
    lib_bool fail_port;
} bus_fixture;

static lib_status bus_read(void *opaque, lib_u32 address, void *destination,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch, lib_u8 cache_control)
{
    bus_fixture *fixture = opaque;

    ++fixture->reads;
    return cpu_instruction_read(&fixture->instruction, address, destination,
        bytes, provenance, observe_only, reset_fetch, cache_control);
}

static lib_status bus_write(void *opaque, lib_u32 address, const void *source,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance, lib_u8 cache_control)
{
    bus_fixture *fixture = opaque;

    if (fixture->failed_write != 0u && address <= fixture->failed_write &&
        (lib_u64)address + bytes > fixture->failed_write) return LIB_STATUS_IO_ERROR;
    return cpu_instruction_write(&fixture->instruction, address, source, bytes,
        provenance, cache_control);
}

static lib_status bus_port(void *opaque, lib_u16 port, lib_u8 bytes,
    lib_bool write, lib_u32 *value)
{
    bus_fixture *fixture = opaque;

    (void)port;
    (void)bytes;
    if (fixture->fail_port) return LIB_STATUS_IO_ERROR;
    ++fixture->port_transfers;
    if (!write) *value = 0xa5u;
    return LIB_STATUS_OK;
}

static void bus_port_complete(void *opaque, lib_u16 port, lib_u8 bytes, lib_bool write)
{
    bus_fixture *fixture = opaque;

    (void)port;
    (void)bytes;
    (void)write;
    ++fixture->port_completions;
}

static lib_status bus_cache(void *opaque, lib_u32 address,
    x86_core_cpu_cache_query query, x86_core_cpu_cache_memory *memory)
{
    (void)opaque;
    *memory = (x86_core_cpu_cache_memory){address, query == X86_CORE_CPU_CACHE_FILL_PROBE};
    return LIB_STATUS_OK;
}

static lib_u64 bus_wait(void *opaque, x86_core_cpu_external_cycle_phase phase,
    x86_core_cpu_external_cycle_space space, lib_u32 address, lib_u8 bytes,
    lib_u8 write, x86_core_cpu_memory_access_provenance provenance)
{
    bus_fixture *fixture = opaque;

    (void)address;
    (void)bytes;
    if (phase != X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT) return 0u;
    if (space == X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_PORT) return fixture->port_wait;
    return write ? fixture->write_wait :
        provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA ? fixture->read_wait : 0u;
}

static const x86_core_cpu_bus_provider bus_provider = {
    .read_memory = bus_read, .write_memory = bus_write,
    .transfer_port = bus_port, .complete_port = bus_port_complete,
    .query_cache_memory = bus_cache,
    .interrupt_pending = cpu_instruction_interrupt_pending,
    .special_cycle = cpu_instruction_special_cycle,
    .cancel_special_cycle = cpu_instruction_cancel_special_cycle
};

static lib_bool bus_step(bus_fixture *fixture, lib_u32 address)
{
    fixture->instruction.cpu.data.eip = address;
    x86_core_cpu_execution_invalidate_prefetch(&fixture->instruction.execution);
    x86_core_cpu_execution_refresh(&fixture->instruction.execution);
    return !fixture->instruction.fault.valid && !fixture->instruction.delivered_exception.valid &&
        !fixture->instruction.execution.stop_requested &&
        !fixture->instruction.execution.timing_bus_active;
}

static lib_bool bus_cost(bus_fixture *fixture, lib_u64 baseline, lib_u64 expected)
{
    const x86_core_cpu_execution_context execution = fixture->instruction.execution;
    const t_cpuins instructions = fixture->instruction.instructions;
    const t_cpu cpu = fixture->instruction.cpu;
    const lib_u32 reads = fixture->reads, writes = fixture->instruction.writes;
    lib_u64 ticks = 999u;
    lib_bool modeled = LIB_FALSE;

    if (!x86_core_cpu_80486_bus_instruction_cost(&fixture->instruction.execution,
            baseline, &ticks, &modeled) || ticks != expected ||
        modeled != (instructions.data.timing_bus_service_ticks != 0u ||
            instructions.data.timing_bus_invalidation_ticks != 0u) ||
        fixture->reads != reads || fixture->instruction.writes != writes ||
        lib_memory_compare(&execution, &fixture->instruction.execution, sizeof(execution)) ||
        lib_memory_compare(&instructions, &fixture->instruction.instructions, sizeof(instructions)) ||
        lib_memory_compare(&cpu, &fixture->instruction.cpu, sizeof(cpu))) {
        lib_c_printf("Bus cost: base=%llu ticks=%llu/%llu service=%llu stall=%llu wait=%llu credit=%llu count=%u\n",
            baseline, ticks, expected, instructions.data.timing_bus_service_ticks,
            instructions.data.timing_bus_stall_ticks, instructions.data.timing_bus_wait_ticks,
            execution.bus_timing.service_credit, (lib_u32)execution.bus_timing.count);
        return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool bus_prepare(bus_fixture *fixture, lib_bool warm_data)
{
    static const struct {lib_u8 offset; lib_u8 code[5]; lib_u8 bytes;} code[] = {
        {0u, {0xe9u,13u,0u},3u}, {16u,{0xe9u,13u,0u},3u}, {32u,{0xe9u,13u,0u},3u},
        {3u,{0xa3u,0u,0x20u},3u}, {6u,{0x66u,0xa3u,15u,0x20u},4u},
        {10u,{0xa1u,0u,0x30u},3u}, {13u,{0xecu},1u}, {14u,{0xeeu},1u},
        {19u,{0xf3u,0x6eu},2u}, {21u,{0x87u,6u,0u,0x20u},4u},
        {25u,{0xa1u,0u,0x40u},3u}, {35u,{0xa1u,0u,0x20u},3u},
        {38u,{0xf3u,0xe6u,0x6eu},3u}
    };
    lib_memory_set(fixture, 0u, sizeof(*fixture));
    cpu_instruction_prepare_with_bus(&fixture->instruction, X86_CORE_CPU_PROFILE_80486SX,
        &bus_provider, fixture);
    fixture->instruction.cpu.data.cr0 &= ~(VCPU_CR0_CD | VCPU_CR0_NW);
    lib_memory_set(fixture->instruction.memory, 0x90u, 64u);
    for (lib_size index = 0u; index < sizeof(code) / sizeof(code[0]); ++index)
        lib_memory_copy(fixture->instruction.memory + code[index].offset,
            code[index].code, code[index].bytes);
    x86_core_cpu_execution_context_bind_external_cycle_provider(&fixture->instruction.execution,
        bus_wait, fixture);
    /* Real jumps fill the three code lines before any buffered write. */
    for (lib_u8 address = 0u; address < 48u; address += 16u) {
        if (!bus_step(fixture, address) || !bus_cost(fixture, 1u, 5u)) return LIB_FALSE;
        x86_core_cpu_execution_advance_bus(&fixture->instruction.execution, 5u, LIB_TRUE, 0u);
    }
    if (warm_data) {
        if (!bus_step(fixture, 35u) || !bus_cost(fixture, 1u, 10u)) return LIB_FALSE;
        x86_core_cpu_execution_advance_bus(&fixture->instruction.execution, 10u, LIB_TRUE, 0u);
    }
    fixture->instruction.cpu.data.eax = 0x12345678u;
    fixture->instruction.cpu.data.dx = 0x80u;
    fixture->instruction.cpu.data.esi = 0x2000u;
    return fixture->instruction.execution.bus_timing.service_credit == 0u;
}

static lib_bool bus_write_fifo(void)
{
    static const lib_u64 waits[] = {0u,9u,30u};

    for (lib_u8 warm = 0u; warm < 2u; ++warm)
    for (lib_size index = 0u; index < sizeof(waits) / sizeof(waits[0]); ++index) {
        bus_fixture fixture;
        lib_u64 previous = 0u;
        const lib_u64 duration = 2u + waits[index];

        if (!bus_prepare(&fixture, warm)) return LIB_FALSE;
        fixture.write_wait = waits[index];
        for (lib_u64 count = 1u; count <= 10u; ++count) {
            const lib_u64 capacity_bound = count > 4u ? duration * (count - 4u) + 1u : 0u;
            const lib_u64 elapsed = count > capacity_bound ? count : capacity_bound;
            const lib_u64 step = elapsed - previous;

            if (!bus_step(&fixture, 3u) || !bus_cost(&fixture, 1u, step) ||
                fixture.instruction.instructions.data.timing_bus_stall_ticks != step - 1u ||
                fixture.instruction.writes != count ||
                fixture.instruction.execution.bus_timing.count != count - elapsed / duration)
                return LIB_FALSE;
            x86_core_cpu_execution_advance_bus(&fixture.instruction.execution, step, LIB_TRUE, 0u);
            if (fixture.instruction.execution.bus_timing.service_credit != 0u) return LIB_FALSE;
            previous = elapsed;
        }
        const cpu_bus_timing pending = fixture.instruction.execution.bus_timing;
        x86_core_cpu_execution_advance_bus(&fixture.instruction.execution, 1000u, LIB_FALSE, 0u);
        if (lib_memory_compare(&pending, &fixture.instruction.execution.bus_timing, sizeof(pending)))
            return LIB_FALSE;
        x86_core_cpu_execution_advance_bus(&fixture.instruction.execution,
            10u * duration - previous, LIB_TRUE, 0u);
        if (fixture.instruction.execution.bus_timing.count != 0u || fixture.instruction.writes != 10u)
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool bus_read_barriers(void)
{
    for (lib_u8 action = 0u; action < 6u; ++action) {
        bus_fixture fixture;

        if (!bus_prepare(&fixture, action != 1u)) return LIB_FALSE;
        fixture.write_wait = 9u;
        for (lib_u8 count = 1u; count <= 6u; ++count) {
            const lib_u64 cost = count <= 4u ? 1u : count == 5u ? 8u : 11u;

            if (!bus_step(&fixture, 3u) || !bus_cost(&fixture, 1u, cost)) return LIB_FALSE;
            x86_core_cpu_execution_advance_bus(&fixture.instruction.execution, cost, LIB_TRUE, 0u);
        }
        if (action == 2u)
            x86_core_cpu_signal_cache_write(&fixture.instruction.execution, 0x5000u, 1u);
        const lib_u8 address = action == 3u ? 13u : action == 4u ? 14u : action == 5u ? 21u : 10u;
        const lib_u64 expected = action == 0u ? 15u : action == 3u || action == 4u ? 45u :
            action == 5u ? 56u : 48u;
        if (!bus_step(&fixture, address) || !bus_cost(&fixture, 1u, expected) ||
            fixture.instruction.execution.bus_timing.count != (action == 0u ? 3u : 0u))
            return LIB_FALSE;
        x86_core_cpu_execution_advance_bus(&fixture.instruction.execution, expected, LIB_TRUE, 0u);
        if (action == 0u) {
            if (!bus_step(&fixture, 25u) || !bus_cost(&fixture, 1u, 38u) ||
                fixture.instruction.execution.bus_timing.count != 0u) return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_bool bus_controls_and_failure(void)
{
    for (lib_u8 mode = 0u; mode < 5u; ++mode) {
        bus_fixture fixture;
        lib_u64 ticks = 999u;
        lib_bool modeled = LIB_TRUE;

        if (!bus_prepare(&fixture, LIB_TRUE)) return LIB_FALSE;
        fixture.write_wait = mode == 4u ? LIB_UINT64_MAX : 9u;
        if (mode == 0u || mode == 1u) {
            fixture.instruction.cpu.data.cr0 |= VCPU_CR0_CD;
            if (mode == 1u) fixture.instruction.cpu.data.cr0 |= VCPU_CR0_NW;
        }
        if (mode == 3u) fixture.failed_write = 0x2010u;
        if (bus_step(&fixture, mode == 3u ? 6u : 3u) != (mode != 3u)) return LIB_FALSE;
        if (mode == 3u || mode == 4u) {
            if (x86_core_cpu_80486_bus_instruction_cost(&fixture.instruction.execution,
                    1u, &ticks, &modeled) || ticks != 999u || !modeled ||
                fixture.instruction.writes != 1u ||
                fixture.instruction.execution.bus_timing.count != (mode == 3u ? 1u : 0u))
                return LIB_FALSE;
        } else if (!bus_cost(&fixture, 1u, mode == 0u ? 11u : 1u) ||
            fixture.instruction.execution.bus_timing.count != (mode == 2u ? 1u : 0u))
            return LIB_FALSE;
        x86_core_cpu_state_reset(&fixture.instruction.execution);
        if (fixture.instruction.execution.bus_timing.count != 0u ||
            fixture.instruction.execution.bus_timing.service_credit != 0u ||
            fixture.instruction.execution.bus_timing.invalid || fixture.instruction.execution.timing_bus_active)
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool bus_ports_and_order(void)
{
    bus_fixture fixture;

    if (!bus_prepare(&fixture, LIB_TRUE)) return LIB_FALSE;
    fixture.port_wait = 9u;
    fixture.instruction.cpu.data.ecx = 6u;
    for (lib_u8 count = 1u; count <= 6u; ++count) {
        const lib_u64 cost = count <= 4u ? 1u : count == 5u ? 8u : 11u;

        if (!bus_step(&fixture, 19u) || !bus_cost(&fixture, 1u, cost) ||
            fixture.port_transfers != count || fixture.port_completions != count ||
            fixture.instruction.cpu.data.ecx != 6u - count ||
            fixture.instruction.cpu.data.esi != 0x2000u + count)
            return LIB_FALSE;
        x86_core_cpu_execution_advance_bus(&fixture.instruction.execution, cost, LIB_TRUE, 0u);
    }
    if (fixture.instruction.execution.bus_timing.count != 4u) return LIB_FALSE;

    if (!bus_prepare(&fixture, LIB_TRUE)) return LIB_FALSE;
    fixture.write_wait = 9u;
    fixture.port_wait = 3u;
    if (!bus_step(&fixture, 3u) || !bus_cost(&fixture, 1u, 1u)) return LIB_FALSE;
    x86_core_cpu_execution_advance_bus(&fixture.instruction.execution, 1u, LIB_TRUE, 0u);
    /* An ignored REP and an immediate port numbered 6Eh are not OUTS. */
    if (!bus_step(&fixture, 38u) || !bus_cost(&fixture, 1u, 15u) ||
        fixture.instruction.execution.bus_timing.count != 0u ||
        fixture.port_transfers != 1u || fixture.port_completions != 1u)
        return LIB_FALSE;

    for (lib_u8 kind = 0u; kind < 3u; ++kind) {
        lib_u64 ticks = 999u;
        lib_bool modeled = LIB_TRUE;

        if (!bus_prepare(&fixture, LIB_TRUE)) return LIB_FALSE;
        fixture.write_wait = 9u;
        if (!bus_step(&fixture, 3u)) return LIB_FALSE;
        x86_core_cpu_execution_advance_bus(&fixture.instruction.execution, 1u, LIB_TRUE, 0u);
        fixture.fail_port = LIB_TRUE;
        fixture.instruction.cpu.data.ecx = 2u;
        if (bus_step(&fixture, kind == 0u ? 19u : kind == 1u ? 13u : 14u) ||
            fixture.instruction.execution.bus_timing.count != (kind == 0u ? 1u : 0u) ||
            fixture.port_transfers != 0u || fixture.port_completions != 0u ||
            fixture.instruction.cpu.data.ecx != 2u ||
            x86_core_cpu_80486_bus_instruction_cost(&fixture.instruction.execution,
                1u, &ticks, &modeled) || ticks != 999u || !modeled)
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool bus_mixed_slots_and_split(void)
{
    static const lib_u64 waits[] = {3u,9u,5u,20u,0u};
    static const lib_u64 drains[] = {10u,7u,22u,2u};
    bus_fixture fixture;

    if (!bus_prepare(&fixture, LIB_TRUE)) return LIB_FALSE;
    for (lib_size index = 0u; index < sizeof(waits) / sizeof(waits[0]); ++index) {
        const lib_u64 cost = index == 4u ? 2u : 1u;

        fixture.write_wait = waits[index];
        if (!bus_step(&fixture, 3u) || !bus_cost(&fixture, 1u, cost)) return LIB_FALSE;
        x86_core_cpu_execution_advance_bus(&fixture.instruction.execution, cost, LIB_TRUE, 0u);
    }
    for (lib_size index = 0u; index < sizeof(drains) / sizeof(drains[0]); ++index) {
        const cpu_bus_timing *bus = &fixture.instruction.execution.bus_timing;

        if (bus->count != 4u - index || bus->writes[bus->head].ticks != drains[index])
            return LIB_FALSE;
        x86_core_cpu_execution_advance_bus(&fixture.instruction.execution, drains[index], LIB_TRUE, 0u);
    }
    if (fixture.instruction.execution.bus_timing.count != 0u || fixture.instruction.writes != 5u)
        return LIB_FALSE;

    if (!bus_prepare(&fixture, LIB_TRUE)) return LIB_FALSE;
    fixture.write_wait = 9u;
    if (!bus_step(&fixture, 6u) || !bus_cost(&fixture, 1u, 2u) ||
        fixture.instruction.execution.bus_timing.count != 2u || fixture.instruction.writes != 2u ||
        lib_memory_compare(fixture.instruction.memory + 0x200fu,
            (const lib_u8[]){0x78u,0x56u,0x34u,0x12u}, 4u)) return LIB_FALSE;
    return LIB_TRUE;
}

static lib_bool bus_invalidation_contention(void)
{
    for (lib_u8 resource = 0u; resource < 4u; ++resource) {
        bus_fixture fixture;
        lib_u64 initial = 999u;

        if (!bus_prepare(&fixture, LIB_TRUE) || !bus_step(&fixture, 48u)) return LIB_FALSE;
        x86_core_cpu_execution_advance_bus(&fixture.instruction.execution, 5u, LIB_TRUE, 0u);
        /* The following NOP uses the real prefetched bytes, not the cache. */
        x86_core_cpu_execution_refresh(&fixture.instruction.execution);
        if (fixture.instruction.instructions.data.timing_cache_used ||
            fixture.instruction.instructions.data.timing_bus_service_ticks != 0u)
            return LIB_FALSE;
        if (resource != 0u && !bus_step(&fixture,
                resource == 1u ? 48u : resource == 2u ? 3u : 10u)) return LIB_FALSE;
        if (!x86_core_cpu_80486_bus_instruction_cost(&fixture.instruction.execution,
                1u, &initial, LIB_NULL)) return LIB_FALSE;
        const cpu_bus_timing pending = fixture.instruction.execution.bus_timing;

        x86_core_cpu_execution_advance_bus(&fixture.instruction.execution, 0u, LIB_FALSE, 99u);
        if (fixture.instruction.instructions.data.timing_bus_invalidation_ticks != 0u)
            return LIB_FALSE;
        x86_core_cpu_execution_advance_bus(&fixture.instruction.execution, 3u, LIB_FALSE, 99u);
        if (!bus_cost(&fixture, 1u, initial + (resource == 0u ? 0u : 3u)) ||
            lib_memory_compare(&pending, &fixture.instruction.execution.bus_timing, sizeof(pending)))
            return LIB_FALSE;
        x86_core_cpu_execution_advance_bus(&fixture.instruction.execution, 7u, LIB_FALSE, 0u);
        if (fixture.instruction.instructions.data.timing_bus_invalidation_ticks !=
                (resource == 0u ? 0u : 3u)) return LIB_FALSE;

        for (lib_u8 guard = 0u; guard < 4u; ++guard) {
            fixture.instruction.execution.preview_mode = guard == 0u;
            fixture.instruction.execution.clock_running = guard != 1u;
            fixture.instruction.execution.stop_granted = guard == 2u;
            if (guard == 3u) fixture.instruction.cpu.data.cr0 |= VCPU_CR0_NW;
            x86_core_cpu_execution_advance_bus(&fixture.instruction.execution, 100u, LIB_FALSE, 100u);
            if (fixture.instruction.instructions.data.timing_bus_invalidation_ticks !=
                    (resource == 0u ? 0u : 3u)) return LIB_FALSE;
        }
        fixture.instruction.execution.preview_mode = LIB_FALSE;
        fixture.instruction.execution.clock_running = LIB_TRUE;
        fixture.instruction.execution.stop_granted = LIB_FALSE;
        fixture.instruction.cpu.data.cr0 &= ~VCPU_CR0_NW;
        if (resource != 0u) {
            x86_core_cpu_execution_advance_bus(&fixture.instruction.execution,
                LIB_UINT64_MAX, LIB_FALSE, LIB_UINT64_MAX);
            lib_bool modeled = LIB_FALSE;
            lib_u64 rejected = 999u;

            if (!fixture.instruction.execution.bus_timing.invalid ||
                x86_core_cpu_80486_bus_instruction_cost(&fixture.instruction.execution,
                    1u, &rejected, &modeled) || rejected != 999u || modeled)
                return LIB_FALSE;
        }
        x86_core_cpu_execution_reset(&fixture.instruction.execution);
        if (fixture.instruction.execution.bus_timing.invalid ||
            fixture.instruction.instructions.data.timing_bus_invalidation_ticks != 0u ||
            fixture.instruction.instructions.data.timing_cache_used) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool bus_debug_and_clock(void)
{
    bus_fixture fixture;
    lib_u8 value;
    x86_core_cpu_instruction_lexeme lexeme;

    if (!bus_prepare(&fixture, LIB_TRUE)) return LIB_FALSE;
    fixture.write_wait = 9u;
    if (!bus_step(&fixture, 3u) || !bus_cost(&fixture, 1u, 1u)) return LIB_FALSE;
    lib_u64 rejected = 999u;
    lib_bool modeled = LIB_TRUE;
    if (x86_core_cpu_80486_bus_instruction_cost(LIB_NULL, 1u, &rejected, &modeled) ||
        x86_core_cpu_80486_bus_instruction_cost(&fixture.instruction.execution, 0u, &rejected, &modeled) ||
        x86_core_cpu_80486_bus_instruction_cost(&fixture.instruction.execution, 1u, LIB_NULL, &modeled) ||
        rejected != 999u || !modeled) return LIB_FALSE;
    fixture.instruction.execution.instruction_wait_stall = LIB_TRUE;
    if (x86_core_cpu_80486_bus_instruction_cost(&fixture.instruction.execution,
            1u, &rejected, &modeled) || rejected != 999u || !modeled) return LIB_FALSE;
    fixture.instruction.execution.instruction_wait_stall = LIB_FALSE;
    const cpu_bus_timing pending = fixture.instruction.execution.bus_timing;
    const lib_u64 wait = fixture.instruction.instructions.data.timing_external_wait_ticks;
    fixture.read_wait = 7u;
    if (x86_core_cpu_read_linear(&fixture.instruction.execution, 0x5000u, &value, 1u) ||
        !x86_core_cpu_execution_preview_lexeme(&fixture.instruction.execution, &lexeme) ||
        lib_memory_compare(&pending, &fixture.instruction.execution.bus_timing, sizeof(pending)) ||
        fixture.instruction.instructions.data.timing_external_wait_ticks != wait)
        return LIB_FALSE;
    fixture.instruction.execution.clock_running = LIB_FALSE;
    x86_core_cpu_execution_advance_bus(&fixture.instruction.execution, 100u, LIB_TRUE, 0u);
    if (lib_memory_compare(&pending, &fixture.instruction.execution.bus_timing, sizeof(pending)))
        return LIB_FALSE;
    fixture.instruction.execution.clock_running = LIB_TRUE;
    x86_core_cpu_execution_advance_bus(&fixture.instruction.execution, 1u, LIB_TRUE, 0u);
    if (fixture.instruction.execution.bus_timing.service_credit != 0u ||
        fixture.instruction.execution.bus_timing.writes[pending.head].ticks != pending.writes[pending.head].ticks)
        return LIB_FALSE;
    x86_core_cpu_execution_advance_bus(&fixture.instruction.execution, 10u, LIB_TRUE, 0u);
    if (fixture.instruction.execution.bus_timing.count != 0u || fixture.instruction.writes != 1u)
        return LIB_FALSE;
    const x86_core_cpu_instruction_timing timing = {.base_ticks = 1u};
    x86_core_cpu_execution_context_bind_profiles(&fixture.instruction.execution,
        X86_CORE_CPU_PROFILE_80386, X86_FPU_PROFILE_NONE, LIB_FALSE,
        X86_CORE_CPU_PROTECTED_DATA_RELOAD_STRICT, &timing);
    return !x86_core_cpu_80486_bus_instruction_cost(&fixture.instruction.execution,
        1u, &rejected, &modeled) && rejected == 999u && modeled;
}

int main(void)
{
    if (!bus_write_fifo()) { lib_c_printf("Bus FIFO failed\n"); return 1; }
    if (!bus_read_barriers()) { lib_c_printf("Bus barriers failed\n"); return 1; }
    if (!bus_controls_and_failure()) { lib_c_printf("Bus control/failure failed\n"); return 1; }
    if (!bus_ports_and_order()) { lib_c_printf("Bus port/order failed\n"); return 1; }
    if (!bus_mixed_slots_and_split()) { lib_c_printf("Bus mixed/split failed\n"); return 1; }
    if (!bus_invalidation_contention()) { lib_c_printf("Bus invalidation failed\n"); return 1; }
    if (!bus_debug_and_clock()) { lib_c_printf("Bus debugger/clock failed\n"); return 1; }
    lib_c_printf("80486:BUS-TIMING:OK\n");
    return 0;
}
