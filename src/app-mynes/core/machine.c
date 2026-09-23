#include "core/bus.h"
#include "core/cpu.h"
#include "core/machine.h"

lib_status core_machine_create(core_machine **out_machine,
    const lib_u8 *rom, lib_size rom_size, const core_machine_options *options)
{
    core_machine *machine;
    lib_status status;

    if (out_machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_machine = LIB_NULL;
    if (options == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    machine = lib_allocate_zero(1u, sizeof(*machine));
    if (machine == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    machine->initial_ram_byte = options->initial_ram_byte;
    status = core_cartridge_create(&machine->cartridge, rom, rom_size);
    if (status != LIB_STATUS_OK) {
        core_machine_destroy(machine);
        return status;
    }
    status = core_machine_reset(machine, CORE_RESET_POWER);
    if (status != LIB_STATUS_OK) {
        core_machine_destroy(machine);
        return status;
    }
    *out_machine = machine;
    return LIB_STATUS_OK;
}

void core_machine_destroy(core_machine *machine)
{
    if (machine == LIB_NULL) return;
    core_cartridge_destroy(machine->cartridge);
    lib_release(machine);
}

lib_status core_machine_reset(core_machine *machine, core_reset_kind kind)
{
    lib_status status;
    lib_u8 low;
    lib_u8 high;

    if (machine == LIB_NULL || machine->cartridge == LIB_NULL) return LIB_STATUS_INVALID_STATE;
    core_bus_trace_reset(machine);
    if (kind == CORE_RESET_POWER) {
        lib_memory_set(machine->ram, machine->initial_ram_byte, sizeof(machine->ram));
        machine->a = 0u;
        machine->x = 0u;
        machine->y = 0u;
        /* The reset bus sequence consumes three read-only stack cycles.  Seed
         * S at zero so those decrements produce the documented $fd state. */
        machine->s = 0u;
        machine->p = 0x24u;
        machine->data_latch = 0u;
        machine->pc = 0u;
        machine->cycles = 0u;
        machine->instructions = 0u;
    } else if (kind == CORE_RESET_WARM) {
        machine->p = (lib_u8)((machine->p | 0x24u) & (lib_u8)~0x10u);
    } else return LIB_STATUS_INVALID_ARGUMENT;
    core_ppu_reset(&machine->ppu, kind);
    core_apu_reset(&machine->apu);
    core_controller_reset(&machine->controller);
    machine->irq_asserted = LIB_FALSE;
    machine->external_irq_asserted = LIB_FALSE;
    machine->cartridge_irq_asserted = LIB_FALSE;
    machine->apu_irq_asserted = LIB_FALSE;
    machine->external_nmi_asserted = LIB_FALSE;
    machine->ppu_nmi_asserted = LIB_FALSE;
    machine->irq_poll_i = LIB_TRUE;
    machine->nmi_asserted = LIB_FALSE;
    machine->nmi_pending = LIB_FALSE;
    machine->nmi_defer_once = LIB_FALSE;
    machine->interrupt_phase = 0u;
    machine->dma_pending = LIB_FALSE;
    machine->dma_active = LIB_FALSE;
    machine->dma_alignment_pending = LIB_FALSE;
    machine->dma_phase = 0u;
    machine->dma_index = 0u;
    machine->dma_latch = 0u;
    machine->dmc_dma_address = 0u;
    machine->dmc_dma_phase = 0u;
    machine->dmc_dma_latch = 0u;
    machine->dmc_dma_active = LIB_FALSE;
    machine->breakpoint_bypass_valid = LIB_FALSE;
    machine->trap = (core_observation) { 0 };
    /* MOS-H reset timing: two discarded program reads, followed by three
     * read-only stack cycles, then the reset-vector low/high reads. */
    status = core_bus_read(machine, machine->pc, &low);
    if (status == LIB_STATUS_OK) status = core_bus_read(machine, machine->pc, &low);
    if (status == LIB_STATUS_OK) status = core_bus_read(machine,
        (lib_u16)(0x0100u | machine->s), &low);
    if (status == LIB_STATUS_OK) --machine->s;
    if (status == LIB_STATUS_OK) status = core_bus_read(machine,
        (lib_u16)(0x0100u | machine->s), &low);
    if (status == LIB_STATUS_OK) --machine->s;
    if (status == LIB_STATUS_OK) status = core_bus_read(machine,
        (lib_u16)(0x0100u | machine->s), &low);
    if (status == LIB_STATUS_OK) --machine->s;
    if (status == LIB_STATUS_OK) status = core_bus_read(machine, 0xfffcu, &low);
    if (status == LIB_STATUS_OK) status = core_bus_read(machine, 0xfffdu, &high);
    if (status != LIB_STATUS_OK) return status;
    machine->pc = (lib_u16)low | ((lib_u16)high << 8u);
    if (kind == CORE_RESET_POWER)
        machine->cycles = 7u;
    else
        machine->cycles += 7u;
    return LIB_STATUS_OK;
}

static void core_machine_set_nmi_line(core_machine *machine, lib_bool asserted)
{
    if (asserted && !machine->nmi_asserted) {
        machine->nmi_pending = LIB_TRUE;
        if (machine->interrupt_phase == 2u) machine->nmi_defer_once = LIB_TRUE;
    }
    if (!asserted && machine->nmi_asserted && machine->interrupt_phase == 2u) {
        machine->nmi_pending = LIB_FALSE;
        machine->nmi_defer_once = LIB_FALSE;
    }
    machine->nmi_asserted = asserted;
}

lib_status core_machine_set_interrupt_lines(core_machine *machine,
    lib_bool irq_asserted, lib_bool nmi_asserted)
{
    if (machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    machine->external_irq_asserted = irq_asserted;
    machine->irq_asserted = irq_asserted || machine->cartridge_irq_asserted ||
        machine->apu_irq_asserted;
    machine->external_nmi_asserted = nmi_asserted;
    core_machine_set_nmi_line(machine, nmi_asserted || machine->ppu_nmi_asserted);
    return LIB_STATUS_OK;
}

void core_machine_refresh_cartridge_irq(core_machine *machine)
{
    if (machine == LIB_NULL) return;
    machine->cartridge_irq_asserted = core_cartridge_irq_asserted(machine->cartridge);
    machine->irq_asserted = machine->external_irq_asserted || machine->cartridge_irq_asserted ||
        machine->apu_irq_asserted;
}

void core_machine_refresh_apu_irq(core_machine *machine)
{
    if (machine == LIB_NULL) return;
    machine->apu_irq_asserted = core_apu_irq_asserted(&machine->apu);
    machine->irq_asserted = machine->external_irq_asserted || machine->cartridge_irq_asserted ||
        machine->apu_irq_asserted;
}

lib_status core_machine_set_ppu_nmi_line(core_machine *machine, lib_bool asserted)
{
    if (machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    machine->ppu_nmi_asserted = asserted;
    core_machine_set_nmi_line(machine, asserted || machine->external_nmi_asserted);
    return LIB_STATUS_OK;
}

static lib_status core_machine_service_dma_budget(core_machine *machine,
    lib_u32 budget, lib_bool *out_serviced, lib_u32 *out_cycles)
{
    lib_u64 start_slots;
    lib_status status;

    if (machine == LIB_NULL || out_serviced == LIB_NULL || out_cycles == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_serviced = LIB_FALSE;
    *out_cycles = 0u;
    if (!machine->dma_pending && !machine->dma_active) return LIB_STATUS_OK;
    start_slots = machine->slots;
    if (!machine->dma_active) {
        machine->dma_active = LIB_TRUE;
        machine->dma_pending = LIB_FALSE;
        machine->dma_alignment_pending = (machine->slots & 1u) != 0u;
        machine->dma_phase = 0u;
        machine->dma_index = 0u;
    }
    status = LIB_STATUS_OK;
    while (status == LIB_STATUS_OK && budget != 0u && machine->dma_active) {
        if (machine->dma_phase == 0u) {
            status = core_bus_read(machine, machine->pc, &machine->dma_latch);
            machine->dma_phase = machine->dma_alignment_pending ? 1u : 2u;
        } else if (machine->dma_phase == 1u) {
            status = core_bus_read(machine, machine->pc, &machine->dma_latch);
            machine->dma_phase = 2u;
        } else if (machine->dma_phase == 2u) {
            status = core_bus_read(machine, (lib_u16)(((lib_u16)machine->dma_page << 8u) |
                machine->dma_index), &machine->dma_latch);
            machine->dma_phase = 3u;
        } else {
            status = core_bus_write(machine, 0x2004u, machine->dma_latch);
            ++machine->dma_index;
            if (machine->dma_index == 256u) machine->dma_active = LIB_FALSE;
            else machine->dma_phase = 2u;
        }
        --budget;
    }
    if (status != LIB_STATUS_OK) return status;
    *out_serviced = LIB_TRUE;
    *out_cycles = (lib_u32)(machine->slots - start_slots);
    machine->cycles += *out_cycles;
    return LIB_STATUS_OK;
}

static lib_status core_machine_service_dmc_budget(core_machine *machine,
    lib_u32 budget, lib_bool *out_serviced, lib_u32 *out_cycles)
{
    lib_u64 start_slots;
    lib_status status = LIB_STATUS_OK;

    if (machine == LIB_NULL || out_serviced == LIB_NULL || out_cycles == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_serviced = LIB_FALSE;
    *out_cycles = 0u;
    if (!machine->dmc_dma_active && !core_apu_take_dmc_request(&machine->apu,
            &machine->dmc_dma_address)) return LIB_STATUS_OK;
    start_slots = machine->slots;
    if (!machine->dmc_dma_active) {
        machine->dmc_dma_active = LIB_TRUE;
        machine->dmc_dma_phase = 0u;
    }
    while (status == LIB_STATUS_OK && budget != 0u && machine->dmc_dma_active) {
        if (machine->dmc_dma_phase < 3u)
            status = core_bus_read(machine, machine->pc, &machine->dmc_dma_latch);
        else status = core_bus_read(machine, machine->dmc_dma_address, &machine->dmc_dma_latch);
        if (status != LIB_STATUS_OK) break;
        ++machine->dmc_dma_phase;
        --budget;
        if (machine->dmc_dma_phase == 4u) {
            core_apu_complete_dmc_read(&machine->apu, machine->dmc_dma_latch);
            machine->dmc_dma_active = LIB_FALSE;
            core_machine_refresh_apu_irq(machine);
        }
    }
    if (status != LIB_STATUS_OK) return status;
    *out_serviced = LIB_TRUE;
    *out_cycles = (lib_u32)(machine->slots - start_slots);
    machine->cycles += *out_cycles;
    return LIB_STATUS_OK;
}

lib_status core_machine_service_dma(core_machine *machine,
    lib_bool *out_serviced, lib_u32 *out_cycles)
{
    return core_machine_service_dma_budget(machine, LIB_UINT32_MAX, out_serviced, out_cycles);
}

static lib_status core_machine_execute(core_machine *machine,
    lib_u32 instruction_limit, lib_u32 cycle_limit, lib_bool honor_breakpoints,
    core_run_result *out_result)
{
    core_run_result result = { 0 };
    lib_u32 instruction_cycles;
    lib_status status;

    if (machine == LIB_NULL || out_result == LIB_NULL || instruction_limit == 0u ||
        cycle_limit == 0u) return LIB_STATUS_INVALID_ARGUMENT;
    while (result.instructions < instruction_limit && result.cycles < cycle_limit &&
        !machine->trap.trap_valid) {
        lib_u32 breakpoint_index;
        lib_bool serviced;
        lib_bool breakpoint_hit = LIB_FALSE;

        for (breakpoint_index = 0u; breakpoint_index < machine->breakpoint_count;
            ++breakpoint_index) {
            if (machine->breakpoints[breakpoint_index] == machine->pc) {
                breakpoint_hit = LIB_TRUE;
                break;
            }
        }
        if (honor_breakpoints && breakpoint_hit && (!machine->breakpoint_bypass_valid ||
            machine->breakpoint_bypass_pc != machine->pc)) {
            machine->breakpoint_bypass_pc = machine->pc;
            machine->breakpoint_bypass_valid = LIB_TRUE;
            result.reason = CORE_MACHINE_STOP_BREAKPOINT;
            *out_result = result;
            return LIB_STATUS_OK;
        }
        if (honor_breakpoints && machine->breakpoint_bypass_valid &&
            machine->breakpoint_bypass_pc == machine->pc)
            machine->breakpoint_bypass_valid = LIB_FALSE;
        status = core_machine_service_dmc_budget(machine, cycle_limit - result.cycles,
            &serviced, &instruction_cycles);
        if (status != LIB_STATUS_OK) return status;
        if (serviced) {
            result.cycles += instruction_cycles;
            continue;
        }
        status = core_machine_service_dma_budget(machine, cycle_limit - result.cycles,
            &serviced, &instruction_cycles);
        if (status != LIB_STATUS_OK) return status;
        if (serviced) {
            result.cycles += instruction_cycles;
            continue;
        }
        status = core_machine_service_interrupt(machine, &serviced, &instruction_cycles);
        if (status != LIB_STATUS_OK) return status;
        if (serviced) {
            result.cycles += instruction_cycles;
            continue;
        }
        status = core_machine_step(machine, &instruction_cycles);
        if (status != LIB_STATUS_OK) return status;
        result.cycles += instruction_cycles;
        if (!machine->trap.trap_valid) ++result.instructions;
    }
    result.reason = machine->trap.trap_valid ? machine->trap.trap_reason : CORE_MACHINE_STOP_BUDGET;
    result.trap_valid = machine->trap.trap_valid;
    *out_result = result;
    return LIB_STATUS_OK;
}

lib_status core_machine_run(core_machine *machine, lib_u32 instruction_limit,
    lib_u32 cycle_limit, core_run_result *out_result)
{
    return core_machine_execute(machine, instruction_limit, cycle_limit, LIB_TRUE,
        out_result);
}

lib_status core_machine_debug_step(core_machine *machine, lib_u32 instruction_limit,
    lib_u32 cycle_limit, core_run_result *out_result)
{
    return core_machine_execute(machine, instruction_limit, cycle_limit, LIB_FALSE,
        out_result);
}

lib_status core_machine_observe(const core_machine *machine, core_observation *out)
{
    if (machine == LIB_NULL || out == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out = (core_observation) {
        .a = machine->a, .x = machine->x, .y = machine->y, .s = machine->s,
        .p = machine->p, .pc = machine->pc, .cycles = machine->cycles,
        .instructions = machine->instructions, .cartridge_present = machine->cartridge != LIB_NULL,
        .trap_valid = machine->trap.trap_valid, .trap_reason = machine->trap.trap_reason,
        .trap_pc = machine->trap.trap_pc, .trap_address = machine->trap.trap_address,
        .trap_opcode = machine->trap.trap_opcode
    };
    return LIB_STATUS_OK;
}

lib_status core_machine_trace_copy(const core_machine *machine,
    core_bus_transfer *out_transfers, lib_u32 capacity, lib_u32 *out_count)
{
    if (machine == LIB_NULL || out_count == LIB_NULL ||
        (out_transfers == LIB_NULL && machine->trace_count != 0u) ||
        capacity < machine->trace_count) return LIB_STATUS_INVALID_ARGUMENT;
    if (machine->trace_count != 0u)
        lib_memory_copy(out_transfers, machine->trace,
            (lib_size)machine->trace_count * sizeof(*out_transfers));
    *out_count = machine->trace_count;
    return LIB_STATUS_OK;
}

lib_status core_machine_peek(const core_machine *machine, lib_u16 address,
    lib_u32 count, lib_u8 *out_bytes)
{
    lib_u32 index;

    if (machine == LIB_NULL || out_bytes == LIB_NULL || count == 0u || count > 256u ||
        (lib_u32)address + count > 65536u) return LIB_STATUS_INVALID_ARGUMENT;
    for (index = 0u; index < count; ++index) {
        lib_u16 candidate = (lib_u16)(address + index);
        if (candidate < 0x2000u || candidate >= 0x8000u) continue;
        else return LIB_STATUS_UNSUPPORTED;
    }
    for (index = 0u; index < count; ++index) {
        lib_u16 candidate = (lib_u16)(address + index);
        if (candidate < 0x2000u) out_bytes[index] = machine->ram[candidate & 0x07ffu];
        else out_bytes[index] = core_cartridge_cpu_read(machine->cartridge, candidate);
    }
    return LIB_STATUS_OK;
}

lib_status core_machine_poke(core_machine *machine, lib_u16 address,
    const lib_u8 *bytes, lib_u32 count)
{
    lib_u32 index;

    if (machine == LIB_NULL || bytes == LIB_NULL || count == 0u || count > 64u ||
        (lib_u32)address + count > 0x2000u) return LIB_STATUS_INVALID_ARGUMENT;
    for (index = 0u; index < count; ++index)
        machine->ram[((lib_u32)address + index) & 0x07ffu] = bytes[index];
    return LIB_STATUS_OK;
}

lib_status core_machine_breakpoint_set(core_machine *machine, lib_u16 address,
    lib_bool enabled)
{
    lib_u32 index;

    if (machine == LIB_NULL || (enabled != LIB_FALSE && enabled != LIB_TRUE))
        return LIB_STATUS_INVALID_ARGUMENT;
    for (index = 0u; index < machine->breakpoint_count; ++index) {
        if (machine->breakpoints[index] != address) continue;
        if (enabled) return LIB_STATUS_OK;
        for (; index + 1u < machine->breakpoint_count; ++index)
            machine->breakpoints[index] = machine->breakpoints[index + 1u];
        --machine->breakpoint_count;
        return LIB_STATUS_OK;
    }
    if (!enabled) return LIB_STATUS_OK;
    if (machine->breakpoint_count >= 16u) return LIB_STATUS_LIMIT_EXCEEDED;
    for (index = machine->breakpoint_count; index > 0u &&
        machine->breakpoints[index - 1u] > address; --index)
        machine->breakpoints[index] = machine->breakpoints[index - 1u];
    machine->breakpoints[index] = address;
    ++machine->breakpoint_count;
    return LIB_STATUS_OK;
}

lib_status core_machine_breakpoint_list(const core_machine *machine,
    lib_u16 *out_addresses, lib_u32 capacity, lib_u32 *out_count)
{
    if (machine == LIB_NULL || out_count == LIB_NULL ||
        (out_addresses == LIB_NULL && machine->breakpoint_count != 0u) ||
        capacity < machine->breakpoint_count) return LIB_STATUS_INVALID_ARGUMENT;
    if (machine->breakpoint_count != 0u)
        lib_memory_copy(out_addresses, machine->breakpoints,
            (lib_size)machine->breakpoint_count * sizeof(*out_addresses));
    *out_count = machine->breakpoint_count;
    return LIB_STATUS_OK;
}
