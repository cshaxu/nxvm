#include "core/snapshot_interface.h"

#include "core/machine.h"

enum { CORE_SNAPSHOT_VERSION = 2u, CORE_SNAPSHOT_PRG_RAM_BYTES = 8192u };

#define SNAPSHOT_STEP(expression) \
    do { if (status == LIB_STATUS_OK) status = (expression); } while (0)

static lib_status snapshot_write_u32(core_snapshot_write_callback write,
    void *context, lib_u32 value)
{
    lib_u8 bytes[4] = {(lib_u8)value, (lib_u8)(value >> 8u),
        (lib_u8)(value >> 16u), (lib_u8)(value >> 24u)};
    return write(context, bytes, sizeof(bytes));
}

static lib_status snapshot_write_u16(core_snapshot_write_callback write,
    void *context, lib_u16 value)
{
    lib_u8 bytes[2] = {(lib_u8)value, (lib_u8)(value >> 8u)};
    return write(context, bytes, sizeof(bytes));
}


static lib_status snapshot_write_u64(core_snapshot_write_callback write,
    void *context, lib_u64 value)
{
    lib_u8 bytes[8]; lib_u32 index;
    for (index = 0u; index < 8u; ++index) bytes[index] = (lib_u8)(value >> (index * 8u));
    return write(context, bytes, sizeof(bytes));
}

static lib_status snapshot_read_u32(core_snapshot_read_callback read,
    void *context, lib_u32 *out_value)
{
    lib_u8 bytes[4];
    lib_status status = read(context, bytes, sizeof(bytes));
    if (status != LIB_STATUS_OK) return status;
    *out_value = (lib_u32)bytes[0] | ((lib_u32)bytes[1] << 8u) |
        ((lib_u32)bytes[2] << 16u) | ((lib_u32)bytes[3] << 24u);
    return LIB_STATUS_OK;
}

static lib_status snapshot_read_u16(core_snapshot_read_callback read,
    void *context, lib_u16 *out_value)
{
    lib_u8 bytes[2];
    lib_status status = read(context, bytes, sizeof(bytes));
    if (status == LIB_STATUS_OK)
        *out_value = (lib_u16)((lib_u16)bytes[0] | ((lib_u16)bytes[1] << 8u));
    return status;
}

static lib_status snapshot_write_u16s(core_snapshot_write_callback write,
    void *context, const lib_u16 *values, lib_u32 count)
{
    lib_status status = LIB_STATUS_OK;
    lib_u32 index;
    for (index = 0u; index < count && status == LIB_STATUS_OK; ++index)
        status = snapshot_write_u16(write, context, values[index]);
    return status;
}

static lib_status snapshot_read_u16s(core_snapshot_read_callback read,
    void *context, lib_u16 *values, lib_u32 count)
{
    lib_status status = LIB_STATUS_OK;
    lib_u32 index;
    for (index = 0u; index < count && status == LIB_STATUS_OK; ++index)
        status = snapshot_read_u16(read, context, &values[index]);
    return status;
}


static lib_status snapshot_read_u64(core_snapshot_read_callback read,
    void *context, lib_u64 *out_value)
{
    lib_u8 bytes[8]; lib_u32 index; lib_status status = read(context, bytes, sizeof(bytes));
    if (status != LIB_STATUS_OK) return status;
    *out_value = 0u;
    for (index = 0u; index < 8u; ++index) *out_value |= (lib_u64)bytes[index] << (index * 8u);
    return LIB_STATUS_OK;
}

static lib_status snapshot_write_bool(core_snapshot_write_callback write,
    void *context, lib_bool value)
{
    lib_u8 byte = value ? 1u : 0u;
    return write(context, &byte, 1u);
}

static lib_status snapshot_read_bool(core_snapshot_read_callback read,
    void *context, lib_bool *out_value)
{
    lib_u8 byte;
    lib_status status = read(context, &byte, 1u);
    if (status != LIB_STATUS_OK)
        return status;
    if (byte > 1u)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_value = byte != 0u;
    return LIB_STATUS_OK;
}

static lib_status snapshot_write_cartridge(const core_cartridge *c,
    core_snapshot_write_callback w, void *x)
{
    lib_status status;
    /* Fixed mapper registers; no pointers, size_t, or padding enter the stream. */
    status = w(x, &c->mirroring, 9u);
    SNAPSHOT_STEP(w(x, &c->mmc3_bank_select, 1u));
    SNAPSHOT_STEP(w(x, c->mmc3_bank_data, 8u));
    SNAPSHOT_STEP(w(x, &c->mmc3_irq_latch, 3u));
    SNAPSHOT_STEP(snapshot_write_bool(w, x, c->mmc3_irq_reload));
    SNAPSHOT_STEP(snapshot_write_bool(w, x, c->mmc3_irq_enabled));
    SNAPSHOT_STEP(snapshot_write_bool(w, x, c->mmc3_irq_asserted));
    SNAPSHOT_STEP(snapshot_write_bool(w, x, c->mmc3_a12_high));
    SNAPSHOT_STEP(snapshot_write_bool(w, x, c->mmc3_prg_ram_enabled));
    SNAPSHOT_STEP(snapshot_write_bool(w, x, c->mmc3_prg_ram_protected));
    SNAPSHOT_STEP(snapshot_write_bool(w, x, c->battery_backed));
    SNAPSHOT_STEP(snapshot_write_bool(w, x, c->prg_ram_dirty));
    SNAPSHOT_STEP(snapshot_write_bool(w, x, c->chr_ram));
    return status;
}

static lib_status snapshot_read_cartridge(core_cartridge *c,
    core_snapshot_read_callback r, void *x)
{
    lib_status status;
    status = r(x, &c->mirroring, 9u);
    SNAPSHOT_STEP(r(x, &c->mmc3_bank_select, 1u));
    SNAPSHOT_STEP(r(x, c->mmc3_bank_data, 8u));
    SNAPSHOT_STEP(r(x, &c->mmc3_irq_latch, 3u));
    SNAPSHOT_STEP(snapshot_read_bool(r, x, &c->mmc3_irq_reload));
    SNAPSHOT_STEP(snapshot_read_bool(r, x, &c->mmc3_irq_enabled));
    SNAPSHOT_STEP(snapshot_read_bool(r, x, &c->mmc3_irq_asserted));
    SNAPSHOT_STEP(snapshot_read_bool(r, x, &c->mmc3_a12_high));
    SNAPSHOT_STEP(snapshot_read_bool(r, x, &c->mmc3_prg_ram_enabled));
    SNAPSHOT_STEP(snapshot_read_bool(r, x, &c->mmc3_prg_ram_protected));
    SNAPSHOT_STEP(snapshot_read_bool(r, x, &c->battery_backed));
    SNAPSHOT_STEP(snapshot_read_bool(r, x, &c->prg_ram_dirty));
    SNAPSHOT_STEP(snapshot_read_bool(r, x, &c->chr_ram));
    return status;
}

static lib_status snapshot_write_apu(const core_apu *apu,
    core_snapshot_write_callback write, void *context)
{
    lib_status status = LIB_STATUS_OK;
    SNAPSHOT_STEP(write(context, apu->registers, sizeof(apu->registers)));
    SNAPSHOT_STEP(snapshot_write_u16s(write, context, apu->pulse_timer, 2u));
    SNAPSHOT_STEP(snapshot_write_u16s(write, context, apu->pulse_counter, 2u));
    SNAPSHOT_STEP(snapshot_write_u16(write, context, apu->triangle_timer));
    SNAPSHOT_STEP(snapshot_write_u16(write, context, apu->triangle_counter));
    SNAPSHOT_STEP(snapshot_write_u16(write, context, apu->noise_counter));
    SNAPSHOT_STEP(snapshot_write_u16(write, context, apu->noise_shift));
    SNAPSHOT_STEP(snapshot_write_u16(write, context, apu->dmc_counter));
    SNAPSHOT_STEP(snapshot_write_u16(write, context, apu->dmc_address));
    SNAPSHOT_STEP(snapshot_write_u16(write, context, apu->dmc_bytes_remaining));
    SNAPSHOT_STEP(snapshot_write_u16s(write, context, apu->length, 4u));
    SNAPSHOT_STEP(write(context, apu->pulse_phase, 2u));
    SNAPSHOT_STEP(write(context, &apu->triangle_phase, 1u));
    SNAPSHOT_STEP(write(context, apu->envelope_decay, 3u));
    SNAPSHOT_STEP(write(context, apu->envelope_divider, 3u));
    SNAPSHOT_STEP(write(context, apu->sweep_divider, 2u));
    SNAPSHOT_STEP(write(context, &apu->triangle_linear, 7u));
    SNAPSHOT_STEP(snapshot_write_u32(write, context, apu->frame_cycles));
    SNAPSHOT_STEP(snapshot_write_u32(write, context, apu->sample_phase));
    SNAPSHOT_STEP(snapshot_write_bool(write, context, apu->frame_irq));
    SNAPSHOT_STEP(snapshot_write_bool(write, context, apu->dmc_irq));
    SNAPSHOT_STEP(snapshot_write_bool(write, context, apu->dmc_dma_requested));
    SNAPSHOT_STEP(snapshot_write_bool(write, context, apu->envelope_start[0]));
    SNAPSHOT_STEP(snapshot_write_bool(write, context, apu->envelope_start[1]));
    SNAPSHOT_STEP(snapshot_write_bool(write, context, apu->envelope_start[2]));
    SNAPSHOT_STEP(snapshot_write_bool(write, context, apu->sweep_reload[0]));
    SNAPSHOT_STEP(snapshot_write_bool(write, context, apu->sweep_reload[1]));
    SNAPSHOT_STEP(snapshot_write_bool(write, context, apu->triangle_reload));
    SNAPSHOT_STEP(snapshot_write_bool(write, context, apu->dmc_sample_empty));
    SNAPSHOT_STEP(snapshot_write_bool(write, context, apu->pulse_even_cycle));
    return status;
}

static lib_status snapshot_read_apu(core_apu *apu,
    core_snapshot_read_callback read, void *context)
{
    lib_status status = LIB_STATUS_OK;
    SNAPSHOT_STEP(read(context, apu->registers, sizeof(apu->registers)));
    SNAPSHOT_STEP(snapshot_read_u16s(read, context, apu->pulse_timer, 2u));
    SNAPSHOT_STEP(snapshot_read_u16s(read, context, apu->pulse_counter, 2u));
    SNAPSHOT_STEP(snapshot_read_u16(read, context, &apu->triangle_timer));
    SNAPSHOT_STEP(snapshot_read_u16(read, context, &apu->triangle_counter));
    SNAPSHOT_STEP(snapshot_read_u16(read, context, &apu->noise_counter));
    SNAPSHOT_STEP(snapshot_read_u16(read, context, &apu->noise_shift));
    SNAPSHOT_STEP(snapshot_read_u16(read, context, &apu->dmc_counter));
    SNAPSHOT_STEP(snapshot_read_u16(read, context, &apu->dmc_address));
    SNAPSHOT_STEP(snapshot_read_u16(read, context, &apu->dmc_bytes_remaining));
    SNAPSHOT_STEP(snapshot_read_u16s(read, context, apu->length, 4u));
    SNAPSHOT_STEP(read(context, apu->pulse_phase, 2u));
    SNAPSHOT_STEP(read(context, &apu->triangle_phase, 1u));
    SNAPSHOT_STEP(read(context, apu->envelope_decay, 3u));
    SNAPSHOT_STEP(read(context, apu->envelope_divider, 3u));
    SNAPSHOT_STEP(read(context, apu->sweep_divider, 2u));
    SNAPSHOT_STEP(read(context, &apu->triangle_linear, 7u));
    SNAPSHOT_STEP(snapshot_read_u32(read, context, &apu->frame_cycles));
    SNAPSHOT_STEP(snapshot_read_u32(read, context, &apu->sample_phase));
    SNAPSHOT_STEP(snapshot_read_bool(read, context, &apu->frame_irq));
    SNAPSHOT_STEP(snapshot_read_bool(read, context, &apu->dmc_irq));
    SNAPSHOT_STEP(snapshot_read_bool(read, context, &apu->dmc_dma_requested));
    SNAPSHOT_STEP(snapshot_read_bool(read, context, &apu->envelope_start[0]));
    SNAPSHOT_STEP(snapshot_read_bool(read, context, &apu->envelope_start[1]));
    SNAPSHOT_STEP(snapshot_read_bool(read, context, &apu->envelope_start[2]));
    SNAPSHOT_STEP(snapshot_read_bool(read, context, &apu->sweep_reload[0]));
    SNAPSHOT_STEP(snapshot_read_bool(read, context, &apu->sweep_reload[1]));
    SNAPSHOT_STEP(snapshot_read_bool(read, context, &apu->triangle_reload));
    SNAPSHOT_STEP(snapshot_read_bool(read, context, &apu->dmc_sample_empty));
    SNAPSHOT_STEP(snapshot_read_bool(read, context, &apu->pulse_even_cycle));
    if (status == LIB_STATUS_OK) {
        lib_memory_set(apu->samples, 0, sizeof(apu->samples));
        apu->sample_read = 0u;
        apu->sample_write = 0u;
        apu->sample_count = 0u;
        apu->dropped_samples = 0u;
    }
    return status;
}

static lib_status snapshot_write_ppu(const core_ppu *ppu,
    core_snapshot_write_callback write, void *context)
{
    lib_status status = LIB_STATUS_OK;
    SNAPSHOT_STEP(write(context, ppu->ciram, sizeof(ppu->ciram)));
    SNAPSHOT_STEP(write(context, ppu->palette, sizeof(ppu->palette)));
    SNAPSHOT_STEP(write(context, &ppu->control, 5u));
    SNAPSHOT_STEP(write(context, ppu->oam, sizeof(ppu->oam)));
    SNAPSHOT_STEP(write(context, &ppu->oam_address, 1u));
    SNAPSHOT_STEP(write(context, ppu->selected_sprites, 8u));
    SNAPSHOT_STEP(write(context, &ppu->selected_sprite_count, 1u));
    SNAPSHOT_STEP(write(context, ppu->secondary_oam, 32u));
    SNAPSHOT_STEP(write(context, &ppu->secondary_oam_count, 1u));
    SNAPSHOT_STEP(write(context, ppu->sprite_pattern_low, 8u));
    SNAPSHOT_STEP(write(context, ppu->sprite_pattern_high, 8u));
    SNAPSHOT_STEP(write(context, ppu->next_sprites, 8u));
    SNAPSHOT_STEP(write(context, &ppu->next_sprite_count, 2u));
    SNAPSHOT_STEP(write(context, ppu->next_secondary_oam, 32u));
    SNAPSHOT_STEP(write(context, &ppu->next_secondary_oam_count, 1u));
    SNAPSHOT_STEP(snapshot_write_u16(write, context, ppu->address));
    SNAPSHOT_STEP(snapshot_write_u16(write, context, ppu->temporary_address));
    SNAPSHOT_STEP(write(context, &ppu->background_tile, 4u));
    SNAPSHOT_STEP(write(context, ppu->background_prefetch_pattern_low, 2u));
    SNAPSHOT_STEP(write(context, ppu->background_prefetch_pattern_high, 2u));
    SNAPSHOT_STEP(write(context, ppu->background_prefetch_attribute, 2u));
    SNAPSHOT_STEP(snapshot_write_u16(write, context, ppu->background_pattern_shift_low));
    SNAPSHOT_STEP(snapshot_write_u16(write, context, ppu->background_pattern_shift_high));
    SNAPSHOT_STEP(snapshot_write_u16(write, context, ppu->background_attribute_shift_low));
    SNAPSHOT_STEP(snapshot_write_u16(write, context, ppu->background_attribute_shift_high));
    SNAPSHOT_STEP(snapshot_write_bool(write, context, ppu->address_high));
    SNAPSHOT_STEP(write(context, &ppu->fine_x, 1u));
    SNAPSHOT_STEP(snapshot_write_u16(write, context, ppu->dot));
    SNAPSHOT_STEP(snapshot_write_u16(write, context, ppu->scanline));
    SNAPSHOT_STEP(snapshot_write_bool(write, context, ppu->odd_frame));
    SNAPSHOT_STEP(snapshot_write_u16s(write, context, ppu->completed,
        CORE_PPU_WIDTH * CORE_PPU_HEIGHT));
    SNAPSHOT_STEP(snapshot_write_u16s(write, context, ppu->pixel_delay, 4u));
    SNAPSHOT_STEP(snapshot_write_bool(write, context, ppu->frame_ready));
    SNAPSHOT_STEP(snapshot_write_u32(write, context, ppu->frame_revision));
    SNAPSHOT_STEP(snapshot_write_bool(write, context, ppu->nmi_line));
    SNAPSHOT_STEP(snapshot_write_bool(write, context, ppu->suppress_vblank));
    return status;
}

static lib_status snapshot_read_ppu(core_ppu *ppu,
    core_snapshot_read_callback read, void *context)
{
    lib_status status = LIB_STATUS_OK;
    SNAPSHOT_STEP(read(context, ppu->ciram, sizeof(ppu->ciram)));
    SNAPSHOT_STEP(read(context, ppu->palette, sizeof(ppu->palette)));
    SNAPSHOT_STEP(read(context, &ppu->control, 5u));
    SNAPSHOT_STEP(read(context, ppu->oam, sizeof(ppu->oam)));
    SNAPSHOT_STEP(read(context, &ppu->oam_address, 1u));
    SNAPSHOT_STEP(read(context, ppu->selected_sprites, 8u));
    SNAPSHOT_STEP(read(context, &ppu->selected_sprite_count, 1u));
    SNAPSHOT_STEP(read(context, ppu->secondary_oam, 32u));
    SNAPSHOT_STEP(read(context, &ppu->secondary_oam_count, 1u));
    SNAPSHOT_STEP(read(context, ppu->sprite_pattern_low, 8u));
    SNAPSHOT_STEP(read(context, ppu->sprite_pattern_high, 8u));
    SNAPSHOT_STEP(read(context, ppu->next_sprites, 8u));
    SNAPSHOT_STEP(read(context, &ppu->next_sprite_count, 2u));
    SNAPSHOT_STEP(read(context, ppu->next_secondary_oam, 32u));
    SNAPSHOT_STEP(read(context, &ppu->next_secondary_oam_count, 1u));
    SNAPSHOT_STEP(snapshot_read_u16(read, context, &ppu->address));
    SNAPSHOT_STEP(snapshot_read_u16(read, context, &ppu->temporary_address));
    SNAPSHOT_STEP(read(context, &ppu->background_tile, 4u));
    SNAPSHOT_STEP(read(context, ppu->background_prefetch_pattern_low, 2u));
    SNAPSHOT_STEP(read(context, ppu->background_prefetch_pattern_high, 2u));
    SNAPSHOT_STEP(read(context, ppu->background_prefetch_attribute, 2u));
    SNAPSHOT_STEP(snapshot_read_u16(read, context, &ppu->background_pattern_shift_low));
    SNAPSHOT_STEP(snapshot_read_u16(read, context, &ppu->background_pattern_shift_high));
    SNAPSHOT_STEP(snapshot_read_u16(read, context, &ppu->background_attribute_shift_low));
    SNAPSHOT_STEP(snapshot_read_u16(read, context, &ppu->background_attribute_shift_high));
    SNAPSHOT_STEP(snapshot_read_bool(read, context, &ppu->address_high));
    SNAPSHOT_STEP(read(context, &ppu->fine_x, 1u));
    SNAPSHOT_STEP(snapshot_read_u16(read, context, &ppu->dot));
    SNAPSHOT_STEP(snapshot_read_u16(read, context, &ppu->scanline));
    SNAPSHOT_STEP(snapshot_read_bool(read, context, &ppu->odd_frame));
    SNAPSHOT_STEP(snapshot_read_u16s(read, context, ppu->completed,
        CORE_PPU_WIDTH * CORE_PPU_HEIGHT));
    SNAPSHOT_STEP(snapshot_read_u16s(read, context, ppu->pixel_delay, 4u));
    SNAPSHOT_STEP(snapshot_read_bool(read, context, &ppu->frame_ready));
    SNAPSHOT_STEP(snapshot_read_u32(read, context, &ppu->frame_revision));
    SNAPSHOT_STEP(snapshot_read_bool(read, context, &ppu->nmi_line));
    SNAPSHOT_STEP(snapshot_read_bool(read, context, &ppu->suppress_vblank));
    if (status == LIB_STATUS_OK && (ppu->dot > 340u || ppu->scanline > 261u ||
        ppu->selected_sprite_count > 8u || ppu->next_sprite_count > 8u ||
        ppu->secondary_oam_count > 32u || ppu->next_secondary_oam_count > 32u))
        status = LIB_STATUS_INVALID_ARGUMENT;
    return status;
}

static lib_status snapshot_write_machine(const core_machine *machine,
    core_snapshot_write_callback write, void *context)
{
    lib_status status = LIB_STATUS_OK;
    lib_u32 index;
    SNAPSHOT_STEP(snapshot_write_apu(&machine->apu, write, context));
    SNAPSHOT_STEP(snapshot_write_ppu(&machine->ppu, write, context));
    SNAPSHOT_STEP(write(context, &machine->controller.live, 5u));
    SNAPSHOT_STEP(snapshot_write_bool(write, context, machine->controller.strobe));
    SNAPSHOT_STEP(write(context, machine->ram, sizeof(machine->ram)));
    SNAPSHOT_STEP(write(context, &machine->a, 5u));
    SNAPSHOT_STEP(snapshot_write_u16(write, context, machine->pc));
    SNAPSHOT_STEP(write(context, &machine->data_latch, 2u));
    SNAPSHOT_STEP(snapshot_write_bool(write, context, machine->irq_asserted));
    SNAPSHOT_STEP(snapshot_write_bool(write, context, machine->external_irq_asserted));
    SNAPSHOT_STEP(snapshot_write_bool(write, context, machine->cartridge_irq_asserted));
    SNAPSHOT_STEP(snapshot_write_bool(write, context, machine->apu_irq_asserted));
    SNAPSHOT_STEP(snapshot_write_bool(write, context, machine->external_nmi_asserted));
    SNAPSHOT_STEP(snapshot_write_bool(write, context, machine->ppu_nmi_asserted));
    SNAPSHOT_STEP(snapshot_write_bool(write, context, machine->irq_poll_i));
    SNAPSHOT_STEP(snapshot_write_bool(write, context, machine->nmi_asserted));
    SNAPSHOT_STEP(snapshot_write_bool(write, context, machine->nmi_pending));
    SNAPSHOT_STEP(snapshot_write_bool(write, context, machine->nmi_defer_once));
    SNAPSHOT_STEP(write(context, &machine->interrupt_phase, 2u));
    SNAPSHOT_STEP(snapshot_write_bool(write, context, machine->dma_pending));
    SNAPSHOT_STEP(snapshot_write_bool(write, context, machine->dma_active));
    SNAPSHOT_STEP(snapshot_write_bool(write, context, machine->dma_alignment_pending));
    SNAPSHOT_STEP(write(context, &machine->dma_phase, 1u));
    SNAPSHOT_STEP(snapshot_write_u16(write, context, machine->dma_index));
    SNAPSHOT_STEP(write(context, &machine->dma_latch, 1u));
    SNAPSHOT_STEP(snapshot_write_u16(write, context, machine->dmc_dma_address));
    SNAPSHOT_STEP(write(context, &machine->dmc_dma_phase, 2u));
    SNAPSHOT_STEP(snapshot_write_bool(write, context, machine->dmc_dma_active));
    SNAPSHOT_STEP(snapshot_write_u64(write, context, machine->cycles));
    SNAPSHOT_STEP(snapshot_write_u64(write, context, machine->slots));
    SNAPSHOT_STEP(snapshot_write_u64(write, context, machine->instructions));
    SNAPSHOT_STEP(write(context, &machine->trap.a, 5u));
    SNAPSHOT_STEP(snapshot_write_u16(write, context, machine->trap.pc));
    SNAPSHOT_STEP(snapshot_write_u64(write, context, machine->trap.cycles));
    SNAPSHOT_STEP(snapshot_write_u64(write, context, machine->trap.instructions));
    SNAPSHOT_STEP(snapshot_write_bool(write, context, machine->trap.cartridge_present));
    SNAPSHOT_STEP(snapshot_write_bool(write, context, machine->trap.trap_valid));
    SNAPSHOT_STEP(snapshot_write_u32(write, context, (lib_u32)machine->trap.trap_reason));
    SNAPSHOT_STEP(snapshot_write_u16(write, context, machine->trap.trap_pc));
    SNAPSHOT_STEP(snapshot_write_u16(write, context, machine->trap.trap_address));
    SNAPSHOT_STEP(write(context, &machine->trap.trap_opcode, 1u));
    SNAPSHOT_STEP(snapshot_write_u32(write, context, machine->trace_count));
    for (index = 0u; index < CORE_MACHINE_TRACE_CAPACITY && status == LIB_STATUS_OK; ++index) {
        SNAPSHOT_STEP(snapshot_write_u16(write, context, machine->trace[index].address));
        SNAPSHOT_STEP(write(context, &machine->trace[index].value, 1u));
        SNAPSHOT_STEP(snapshot_write_u32(write, context, (lib_u32)machine->trace[index].kind));
    }
    SNAPSHOT_STEP(snapshot_write_u16s(write, context, machine->breakpoints, 16u));
    SNAPSHOT_STEP(snapshot_write_u16(write, context, machine->breakpoint_count));
    SNAPSHOT_STEP(snapshot_write_u16(write, context, machine->breakpoint_bypass_pc));
    SNAPSHOT_STEP(snapshot_write_bool(write, context, machine->breakpoint_bypass_valid));
    return status;
}

static lib_status snapshot_read_machine(core_machine *machine,
    core_snapshot_read_callback read, void *context)
{
    lib_status status = LIB_STATUS_OK;
    lib_u32 index, kind, reason;
    SNAPSHOT_STEP(snapshot_read_apu(&machine->apu, read, context));
    SNAPSHOT_STEP(snapshot_read_ppu(&machine->ppu, read, context));
    SNAPSHOT_STEP(read(context, &machine->controller.live, 5u));
    SNAPSHOT_STEP(snapshot_read_bool(read, context, &machine->controller.strobe));
    SNAPSHOT_STEP(read(context, machine->ram, sizeof(machine->ram)));
    SNAPSHOT_STEP(read(context, &machine->a, 5u));
    SNAPSHOT_STEP(snapshot_read_u16(read, context, &machine->pc));
    SNAPSHOT_STEP(read(context, &machine->data_latch, 2u));
    SNAPSHOT_STEP(snapshot_read_bool(read, context, &machine->irq_asserted));
    SNAPSHOT_STEP(snapshot_read_bool(read, context, &machine->external_irq_asserted));
    SNAPSHOT_STEP(snapshot_read_bool(read, context, &machine->cartridge_irq_asserted));
    SNAPSHOT_STEP(snapshot_read_bool(read, context, &machine->apu_irq_asserted));
    SNAPSHOT_STEP(snapshot_read_bool(read, context, &machine->external_nmi_asserted));
    SNAPSHOT_STEP(snapshot_read_bool(read, context, &machine->ppu_nmi_asserted));
    SNAPSHOT_STEP(snapshot_read_bool(read, context, &machine->irq_poll_i));
    SNAPSHOT_STEP(snapshot_read_bool(read, context, &machine->nmi_asserted));
    SNAPSHOT_STEP(snapshot_read_bool(read, context, &machine->nmi_pending));
    SNAPSHOT_STEP(snapshot_read_bool(read, context, &machine->nmi_defer_once));
    SNAPSHOT_STEP(read(context, &machine->interrupt_phase, 2u));
    SNAPSHOT_STEP(snapshot_read_bool(read, context, &machine->dma_pending));
    SNAPSHOT_STEP(snapshot_read_bool(read, context, &machine->dma_active));
    SNAPSHOT_STEP(snapshot_read_bool(read, context, &machine->dma_alignment_pending));
    SNAPSHOT_STEP(read(context, &machine->dma_phase, 1u));
    SNAPSHOT_STEP(snapshot_read_u16(read, context, &machine->dma_index));
    SNAPSHOT_STEP(read(context, &machine->dma_latch, 1u));
    SNAPSHOT_STEP(snapshot_read_u16(read, context, &machine->dmc_dma_address));
    SNAPSHOT_STEP(read(context, &machine->dmc_dma_phase, 2u));
    SNAPSHOT_STEP(snapshot_read_bool(read, context, &machine->dmc_dma_active));
    SNAPSHOT_STEP(snapshot_read_u64(read, context, &machine->cycles));
    SNAPSHOT_STEP(snapshot_read_u64(read, context, &machine->slots));
    SNAPSHOT_STEP(snapshot_read_u64(read, context, &machine->instructions));
    SNAPSHOT_STEP(read(context, &machine->trap.a, 5u));
    SNAPSHOT_STEP(snapshot_read_u16(read, context, &machine->trap.pc));
    SNAPSHOT_STEP(snapshot_read_u64(read, context, &machine->trap.cycles));
    SNAPSHOT_STEP(snapshot_read_u64(read, context, &machine->trap.instructions));
    SNAPSHOT_STEP(snapshot_read_bool(read, context, &machine->trap.cartridge_present));
    SNAPSHOT_STEP(snapshot_read_bool(read, context, &machine->trap.trap_valid));
    SNAPSHOT_STEP(snapshot_read_u32(read, context, &reason));
    SNAPSHOT_STEP(snapshot_read_u16(read, context, &machine->trap.trap_pc));
    SNAPSHOT_STEP(snapshot_read_u16(read, context, &machine->trap.trap_address));
    SNAPSHOT_STEP(read(context, &machine->trap.trap_opcode, 1u));
    if (status == LIB_STATUS_OK && reason > (lib_u32)CORE_MACHINE_STOP_BUS_FAILURE)
        status = LIB_STATUS_INVALID_ARGUMENT;
    if (status == LIB_STATUS_OK)
        machine->trap.trap_reason = (core_machine_stop_reason)reason;
    SNAPSHOT_STEP(snapshot_read_u32(read, context, &machine->trace_count));
    for (index = 0u; index < CORE_MACHINE_TRACE_CAPACITY && status == LIB_STATUS_OK; ++index) {
        SNAPSHOT_STEP(snapshot_read_u16(read, context, &machine->trace[index].address));
        SNAPSHOT_STEP(read(context, &machine->trace[index].value, 1u));
        SNAPSHOT_STEP(snapshot_read_u32(read, context, &kind));
        if (status == LIB_STATUS_OK && kind > (lib_u32)CORE_BUS_TRANSFER_WRITE)
            status = LIB_STATUS_INVALID_ARGUMENT;
        if (status == LIB_STATUS_OK)
            machine->trace[index].kind = (core_bus_transfer_kind)kind;
    }
    SNAPSHOT_STEP(snapshot_read_u16s(read, context, machine->breakpoints, 16u));
    SNAPSHOT_STEP(snapshot_read_u16(read, context, &machine->breakpoint_count));
    SNAPSHOT_STEP(snapshot_read_u16(read, context, &machine->breakpoint_bypass_pc));
    SNAPSHOT_STEP(snapshot_read_bool(read, context, &machine->breakpoint_bypass_valid));
    if (status == LIB_STATUS_OK && (machine->controller.index > 8u ||
        machine->trace_count > CORE_MACHINE_TRACE_CAPACITY || machine->breakpoint_count > 16u))
        status = LIB_STATUS_INVALID_ARGUMENT;
    return status;
}

lib_status core_snapshot_write(const core_machine *machine,
    core_snapshot_write_callback write, void *context)
{
    static const lib_u8 magic[4] = {'M','N','S','1'};
    const core_cartridge *cartridge;
    lib_status status;
    if (machine == LIB_NULL || machine->cartridge == LIB_NULL || write == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    cartridge = machine->cartridge;
    status = write(context, magic, sizeof(magic));
    if (status == LIB_STATUS_OK) status = snapshot_write_u32(write, context, CORE_SNAPSHOT_VERSION);
    if (status == LIB_STATUS_OK) status = snapshot_write_u64(write, context, cartridge->content_identity);
    if (status == LIB_STATUS_OK) status = snapshot_write_u32(write, context, cartridge->mapper);
    if (status == LIB_STATUS_OK) status = snapshot_write_u32(write, context,
        cartridge->prg_ram != LIB_NULL ? CORE_SNAPSHOT_PRG_RAM_BYTES : 0u);
    if (status == LIB_STATUS_OK) status = snapshot_write_u32(write, context, (lib_u32)cartridge->chr_ram ? (lib_u32)cartridge->chr_bytes : 0u);
    if (status == LIB_STATUS_OK) status = snapshot_write_machine(machine, write, context);
    if (status == LIB_STATUS_OK) status = snapshot_write_cartridge(cartridge, write, context);
    if (status == LIB_STATUS_OK && cartridge->prg_ram != LIB_NULL)
        status = write(context, cartridge->prg_ram, CORE_SNAPSHOT_PRG_RAM_BYTES);
    if (status == LIB_STATUS_OK && cartridge->chr_ram)
        status = write(context, cartridge->chr, cartridge->chr_bytes);
    return status;
}

lib_status core_snapshot_read(core_machine *machine,
    core_snapshot_read_callback read, void *context)
{
    lib_u8 magic[4], *staged_prg = LIB_NULL, *staged_chr = LIB_NULL;
    lib_u32 version, mapper, prg_ram_bytes, chr_ram_bytes;
    lib_u64 identity; core_machine candidate; core_cartridge cartridge; lib_status status;
    if (machine == LIB_NULL || machine->cartridge == LIB_NULL || read == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    status = read(context, magic, sizeof(magic));
    if (status != LIB_STATUS_OK ||
        lib_memory_compare(magic, "MNS1", sizeof(magic)) != 0)
        return status == LIB_STATUS_OK ? LIB_STATUS_INVALID_ARGUMENT : status;
    if ((status = snapshot_read_u32(read, context, &version)) != LIB_STATUS_OK || version != CORE_SNAPSHOT_VERSION ||
        (status = snapshot_read_u64(read, context, &identity)) != LIB_STATUS_OK ||
        identity != machine->cartridge->content_identity ||
        (status = snapshot_read_u32(read, context, &mapper)) != LIB_STATUS_OK || mapper != machine->cartridge->mapper ||
        (status = snapshot_read_u32(read, context, &prg_ram_bytes)) != LIB_STATUS_OK ||
        prg_ram_bytes != (machine->cartridge->prg_ram == LIB_NULL ? 0u : CORE_SNAPSHOT_PRG_RAM_BYTES) ||
        (status = snapshot_read_u32(read, context, &chr_ram_bytes)) != LIB_STATUS_OK ||
        chr_ram_bytes != (machine->cartridge->chr_ram ? (lib_u32)machine->cartridge->chr_bytes : 0u))
        return status == LIB_STATUS_OK ? LIB_STATUS_INVALID_ARGUMENT : status;
    candidate = *machine; cartridge = *machine->cartridge;
    if ((status = snapshot_read_machine(&candidate, read, context)) != LIB_STATUS_OK ||
        (status = snapshot_read_cartridge(&cartridge, read, context)) != LIB_STATUS_OK)
        return status == LIB_STATUS_OK ? LIB_STATUS_INVALID_ARGUMENT : status;
    if (prg_ram_bytes != 0u) {
        staged_prg = lib_allocate(prg_ram_bytes);
        if (staged_prg == LIB_NULL) return LIB_STATUS_NO_MEMORY;
        status = read(context, staged_prg, prg_ram_bytes);
    }
    if (status == LIB_STATUS_OK && chr_ram_bytes != 0u) {
        staged_chr = lib_allocate(chr_ram_bytes);
        if (staged_chr == LIB_NULL) status = LIB_STATUS_NO_MEMORY;
        else status = read(context, staged_chr, chr_ram_bytes);
    }
    if (status != LIB_STATUS_OK) {
        lib_release(staged_prg); lib_release(staged_chr);
        return status;
    }
    candidate.cartridge = machine->cartridge;
    *machine = candidate;
    {
        lib_u8 *prg = machine->cartridge->prg;
        lib_u8 *chr = machine->cartridge->chr;
        lib_u8 *prg_ram = machine->cartridge->prg_ram;
        lib_size prg_bytes = machine->cartridge->prg_bytes;
        lib_size chr_bytes = machine->cartridge->chr_bytes;
        *machine->cartridge = cartridge;
        machine->cartridge->prg = prg; machine->cartridge->chr = chr;
        machine->cartridge->prg_ram = prg_ram;
        machine->cartridge->prg_bytes = prg_bytes;
        machine->cartridge->chr_bytes = chr_bytes;
    }
    if (staged_prg != LIB_NULL)
        lib_memory_copy(machine->cartridge->prg_ram, staged_prg, prg_ram_bytes);
    if (staged_chr != LIB_NULL)
        lib_memory_copy(machine->cartridge->chr, staged_chr, chr_ram_bytes);
    lib_release(staged_prg); lib_release(staged_chr);
    return LIB_STATUS_OK;
}
