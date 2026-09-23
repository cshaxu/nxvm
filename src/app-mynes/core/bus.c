#include "core/bus.h"

static void core_bus_trace(core_machine *machine, lib_u16 address, lib_u8 value,
    core_bus_transfer_kind kind)
{
    if (machine->trace_count >= CORE_MACHINE_TRACE_CAPACITY) return;
    machine->trace[machine->trace_count++] = (core_bus_transfer) {
        .address = address,
        .value = value,
        .kind = kind
    };
}

void core_bus_trace_reset(core_machine *machine)
{
    if (machine != LIB_NULL) machine->trace_count = 0u;
}

static void core_bus_slot(core_machine *machine)
{
    lib_u32 dot;

    ++machine->slots;
    for (dot = 0u; dot < 3u; ++dot)
        core_ppu_tick(&machine->ppu, machine->cartridge);
    core_apu_tick(&machine->apu);
    (void)core_machine_set_ppu_nmi_line(machine, machine->ppu.nmi_line);
    core_machine_refresh_cartridge_irq(machine);
    core_machine_refresh_apu_irq(machine);
}

lib_status core_bus_read(core_machine *machine, lib_u16 address, lib_u8 *out_value)
{
    if (machine == LIB_NULL || out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    /* PPUSTATUS at the dot immediately before vblank is sampled by the CPU
     * before this bus slot's PPU edge.  Preserve that sole same-slot ordering
     * so the register can arm its documented one-dot vblank suppression. */
    if (address >= 0x2000u && address < 0x4000u && (address & 7u) == 2u &&
        machine->ppu.scanline == 241u && machine->ppu.dot == 0u) {
        *out_value = core_ppu_cpu_read(&machine->ppu, machine->cartridge, 2u);
        core_bus_slot(machine);
        machine->data_latch = *out_value;
        core_bus_trace(machine, address, *out_value, CORE_BUS_TRANSFER_READ);
        return LIB_STATUS_OK;
    }
    core_bus_slot(machine);
    if (address < 0x2000u) *out_value = machine->ram[address & 0x07ffu];
    else if (address < 0x4000u)
        *out_value = core_ppu_cpu_read(&machine->ppu, machine->cartridge,
            (lib_u8)(address & 7u));
    else if (address == 0x4015u) *out_value = core_apu_cpu_read(&machine->apu);
    else if (address == 0x4016u)
        *out_value = (lib_u8)((machine->data_latch & 0xe0u) |
            core_controller_read(&machine->controller));
    else if (address == 0x4017u) *out_value = machine->data_latch & 0xe0u;
    else if (address >= 0x4000u && address <= 0x4014u)
        *out_value = machine->data_latch;
    else if (address >= 0x6000u && address < 0x8000u &&
        machine->cartridge != LIB_NULL && machine->cartridge->mapper == 4u)
        *out_value = core_cartridge_cpu_read(machine->cartridge, address);
    else if (address >= 0x4020u && address < 0x8000u) *out_value = machine->data_latch;
    else if (address >= 0x8000u) *out_value = core_cartridge_cpu_read(machine->cartridge, address);
    else return LIB_STATUS_UNSUPPORTED;
    machine->data_latch = *out_value;
    core_bus_trace(machine, address, *out_value, CORE_BUS_TRANSFER_READ);
    return LIB_STATUS_OK;
}

lib_status core_bus_write(core_machine *machine, lib_u16 address, lib_u8 value)
{
    if (machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    core_bus_slot(machine);
    machine->data_latch = value;
    core_bus_trace(machine, address, value, CORE_BUS_TRANSFER_WRITE);
    if (address < 0x2000u) {
        machine->ram[address & 0x07ffu] = value;
        return LIB_STATUS_OK;
    }
    if (address < 0x4000u) {
        core_ppu_cpu_write(&machine->ppu, machine->cartridge, (lib_u8)(address & 7u), value);
        (void)core_machine_set_ppu_nmi_line(machine, machine->ppu.nmi_line);
        return LIB_STATUS_OK;
    }
    if (address == 0x4016u) {
        core_controller_write_strobe(&machine->controller, value);
        return LIB_STATUS_OK;
    }
    if (address >= 0x4000u && address <= 0x4013u) {
        core_apu_cpu_write(&machine->apu, (lib_u8)(address - 0x4000u), value);
        core_machine_refresh_apu_irq(machine);
        return LIB_STATUS_OK;
    }
    if (address == 0x4015u || address == 0x4017u) {
        core_apu_cpu_write(&machine->apu, (lib_u8)(address - 0x4000u), value);
        core_machine_refresh_apu_irq(machine);
        return LIB_STATUS_OK;
    }
    if (address == 0x4014u) {
        machine->dma_page = value;
        machine->dma_pending = LIB_TRUE;
        return LIB_STATUS_OK;
    }
    if (address >= 0x8000u) {
        if (!core_cartridge_cpu_write(machine->cartridge, address, value))
            return LIB_STATUS_INVALID_STATE;
        core_machine_refresh_cartridge_irq(machine);
        return LIB_STATUS_OK;
    }
    if (address >= 0x6000u && address < 0x8000u && machine->cartridge != LIB_NULL &&
        machine->cartridge->mapper == 4u) {
        (void)core_cartridge_cpu_write(machine->cartridge, address, value);
        return LIB_STATUS_OK;
    }
    if (address >= 0x4020u) return LIB_STATUS_OK;
    return LIB_STATUS_UNSUPPORTED;
}
