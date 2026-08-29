#include "core/machine/machine.h"

#define CORE_MACHINE_D4_CONTROL_PHYSICAL 0x80c00000u
#define CORE_MACHINE_D4_CONTROL_WINDOW_BYTES 4096u
#define CORE_MACHINE_D4_COMPATIBILITY_START 0x00fe0000u
#define CORE_MACHINE_D4_COMPATIBILITY_HIGH_START 0xffee0000u
#define CORE_MACHINE_D4_REPLACEMENT_START 0x000e0000u
#define CORE_MACHINE_D4_SETUP_BANK_START 0x00100000u
#define CORE_MACHINE_D4_SETUP_BANK_BYTES 0x00e00000u

static C_INT core_machine_d4_memory_is_active(const core_machine_d4_memory *memory,
    type_unsigned_32 physical)
{
    return memory != STD_NULL && ((physical >= CORE_MACHINE_D4_COMPATIBILITY_START &&
        physical - CORE_MACHINE_D4_COMPATIBILITY_START < CORE_MACHINE_D4_MEMORY_BYTES) ||
        (physical >= CORE_MACHINE_D4_COMPATIBILITY_HIGH_START &&
            physical - CORE_MACHINE_D4_COMPATIBILITY_HIGH_START < CORE_MACHINE_D4_MEMORY_BYTES) ||
        ((memory->control & 0x01u) == 0u && physical >= CORE_MACHINE_D4_REPLACEMENT_START &&
            physical - CORE_MACHINE_D4_REPLACEMENT_START < CORE_MACHINE_D4_MEMORY_BYTES));
}

static STD_SIZE_T core_machine_d4_memory_offset(type_unsigned_32 physical)
{
    if (physical >= CORE_MACHINE_D4_COMPATIBILITY_START &&
        physical - CORE_MACHINE_D4_COMPATIBILITY_START < CORE_MACHINE_D4_MEMORY_BYTES) {
        return (STD_SIZE_T)(physical - CORE_MACHINE_D4_COMPATIBILITY_START);
    }
    if (physical >= CORE_MACHINE_D4_COMPATIBILITY_HIGH_START &&
        physical - CORE_MACHINE_D4_COMPATIBILITY_HIGH_START < CORE_MACHINE_D4_MEMORY_BYTES) {
        return (STD_SIZE_T)(physical - CORE_MACHINE_D4_COMPATIBILITY_HIGH_START);
    }
    return (STD_SIZE_T)(physical - CORE_MACHINE_D4_REPLACEMENT_START);
}

static type_status core_machine_d4_memory_read(C_VOID *opaque, type_unsigned_32 physical,
    type_virtual_address destination, type_native_unsigned bytes)
{
    core_machine_d4_memory *memory = (core_machine_d4_memory *)opaque;
    STD_SIZE_T offset;

    if (memory == STD_NULL || destination == 0u || !core_machine_d4_memory_is_active(memory, physical)) {
        return TYPE_STATUS_UNSUPPORTED;
    }
    offset = core_machine_d4_memory_offset(physical);
    if (bytes > CORE_MACHINE_D4_MEMORY_BYTES - offset) return TYPE_STATUS_FAULT;
    STD_MEMCPY((C_VOID *)destination, memory->compatibility + offset, bytes);
    return TYPE_STATUS_OK;
}

static type_status core_machine_d4_memory_write(C_VOID *opaque, type_unsigned_32 physical,
    type_virtual_address source, type_native_unsigned bytes)
{
    core_machine_d4_memory *memory = (core_machine_d4_memory *)opaque;
    STD_SIZE_T offset;

    if (memory == STD_NULL || source == 0u || !core_machine_d4_memory_is_active(memory, physical)) {
        return TYPE_STATUS_UNSUPPORTED;
    }
    if ((memory->control & 0x02u) == 0u) return TYPE_STATUS_FAULT;
    offset = core_machine_d4_memory_offset(physical);
    if (bytes > CORE_MACHINE_D4_MEMORY_BYTES - offset) return TYPE_STATUS_FAULT;
    STD_MEMCPY(memory->compatibility + offset, (const C_VOID *)source, bytes);
    return TYPE_STATUS_OK;
}

static type_status core_machine_d4_memory_query(C_VOID *opaque, type_unsigned_32 physical,
    type_native_unsigned bytes, core_machine_memory_access access)
{
    core_machine_d4_memory *memory = (core_machine_d4_memory *)opaque;
    STD_SIZE_T offset;

    if (memory == STD_NULL || !core_machine_d4_memory_is_active(memory, physical)) {
        return TYPE_STATUS_UNSUPPORTED;
    }
    offset = core_machine_d4_memory_offset(physical);
    if (bytes > CORE_MACHINE_D4_MEMORY_BYTES - offset) return TYPE_STATUS_FAULT;
    return access == CORE_MACHINE_MEMORY_ACCESS_WRITE && (memory->control & 0x02u) == 0u ?
        TYPE_STATUS_FAULT : TYPE_STATUS_OK;
}

/* In the selected D4 setup, low nibble 1 disconnects every extension-RAM
 * bank below F00000h.  This is an External-L2 board relation, cross-checked
 * against the selected DeskPro reference model; it deliberately does not
 * attempt to generalize that model's full RAM-card table. */
static C_INT core_machine_d4_setup_blocks_extension(const core_machine_d4_memory *memory)
{
    return memory != STD_NULL && (memory->ram_setup & 0x0fu) == 1u;
}

static type_status core_machine_d4_setup_read(C_VOID *opaque, type_unsigned_32 physical,
    type_virtual_address destination, type_native_unsigned bytes)
{
    core_machine_d4_memory *memory = (core_machine_d4_memory *)opaque;

    (C_VOID)physical;
    if (memory == STD_NULL || destination == 0u || !core_machine_d4_setup_blocks_extension(memory)) {
        return TYPE_STATUS_UNSUPPORTED;
    }
    STD_MEMSET((C_VOID *)destination, 0xff, bytes);
    return TYPE_STATUS_OK;
}

static type_status core_machine_d4_setup_write(C_VOID *opaque, type_unsigned_32 physical,
    type_virtual_address source, type_native_unsigned bytes)
{
    core_machine_d4_memory *memory = (core_machine_d4_memory *)opaque;

    (C_VOID)physical;
    (C_VOID)source;
    (C_VOID)bytes;
    return core_machine_d4_setup_blocks_extension(memory) ? TYPE_STATUS_OK :
        TYPE_STATUS_UNSUPPORTED;
}

static type_status core_machine_d4_setup_query(C_VOID *opaque, type_unsigned_32 physical,
    type_native_unsigned bytes, core_machine_memory_access access)
{
    core_machine_d4_memory *memory = (core_machine_d4_memory *)opaque;

    (C_VOID)physical;
    (C_VOID)bytes;
    (C_VOID)access;
    return core_machine_d4_setup_blocks_extension(memory) ? TYPE_STATUS_OK :
        TYPE_STATUS_UNSUPPORTED;
}

static type_status core_machine_d4_control_read(C_VOID *opaque, type_unsigned_32 physical,
    type_virtual_address destination, type_native_unsigned bytes)
{
    core_machine_d4_memory *memory = (core_machine_d4_memory *)opaque;
    type_unsigned_32 offset;

    if (memory == STD_NULL || destination == 0u || bytes != 1u ||
        physical < CORE_MACHINE_D4_CONTROL_PHYSICAL) return TYPE_STATUS_FAULT;
    offset = physical - CORE_MACHINE_D4_CONTROL_PHYSICAL;
    if (offset >= CORE_MACHINE_D4_CONTROL_WINDOW_BYTES) return TYPE_STATUS_UNSUPPORTED;
    if (offset == 0u) *(type_unsigned_8 *)destination = (type_unsigned_8)(0xbfu & ~memory->parity_fault_mask);
    else if (offset == 1u) *(type_unsigned_8 *)destination = memory->diagnostic_high;
    else if (offset == 2u) *(type_unsigned_8 *)destination = (type_unsigned_8)memory->ram_setup;
    else if (offset == 3u) *(type_unsigned_8 *)destination = (type_unsigned_8)(memory->ram_setup >> 8u);
    else *(type_unsigned_8 *)destination = 0xffu;
    return TYPE_STATUS_OK;
}

static type_status core_machine_d4_control_write(C_VOID *opaque, type_unsigned_32 physical,
    type_virtual_address source, type_native_unsigned bytes)
{
    core_machine_d4_memory *memory = (core_machine_d4_memory *)opaque;
    type_unsigned_32 offset;

    if (memory == STD_NULL || source == 0u || bytes != 1u ||
        physical < CORE_MACHINE_D4_CONTROL_PHYSICAL) return TYPE_STATUS_FAULT;
    offset = physical - CORE_MACHINE_D4_CONTROL_PHYSICAL;
    if (offset >= CORE_MACHINE_D4_CONTROL_WINDOW_BYTES) return TYPE_STATUS_UNSUPPORTED;
    if (offset == 0u) {
        memory->control = *(const type_unsigned_8 *)source | 0xfcu;
        memory->parity_fault_mask = 0u;
    } else if (offset == 2u) {
        memory->ram_setup = (type_unsigned_16)((memory->ram_setup & 0xff00u) |
            *(const type_unsigned_8 *)source);
    }
    return TYPE_STATUS_OK;
}

static type_status core_machine_d4_control_query(C_VOID *opaque, type_unsigned_32 physical,
    type_native_unsigned bytes, core_machine_memory_access access)
{
    (C_VOID)opaque;
    if (physical < CORE_MACHINE_D4_CONTROL_PHYSICAL ||
        physical - CORE_MACHINE_D4_CONTROL_PHYSICAL >= CORE_MACHINE_D4_CONTROL_WINDOW_BYTES ||
        bytes != 1u) return TYPE_STATUS_UNSUPPORTED;
    return access == CORE_MACHINE_MEMORY_ACCESS_READ || access == CORE_MACHINE_MEMORY_ACCESS_WRITE ?
        TYPE_STATUS_OK : TYPE_STATUS_UNSUPPORTED;
}

static C_VOID core_machine_d4_parity_fault(C_VOID *opaque, type_unsigned_32 physical)
{
    core_machine *machine = (core_machine *)opaque;

    if (machine == STD_NULL) return;
    machine->d4_memory.parity_fault_mask |= (type_unsigned_8)(1u << (physical & 3u));
    (C_VOID)core_machine_report_d4_iochk_fault(machine);
}

static C_VOID core_machine_d4_memory_write_observer(C_VOID *opaque, type_unsigned_32 physical,
    type_native_unsigned bytes)
{
    core_machine *machine = (core_machine *)opaque;

    (C_VOID)physical;
    (C_VOID)bytes;
    if (machine != STD_NULL && machine->d4_memory.parity_fault_mask != 0u) {
        (C_VOID)core_machine_clear_d4_iochk_fault(machine);
    }
}

C_INT core_machine_d4_memory_config_is_valid(const core_machine_d4_memory_config *config)
{
    return config != STD_NULL && config->present == TYPE_TRUE && config->even_rom != STD_NULL &&
        config->odd_rom != STD_NULL && config->rom_chip_bytes == CORE_MACHINE_D4_MEMORY_ROM_CHIP_BYTES;
}

type_status core_machine_d4_memory_configure(core_machine *machine,
    const core_machine_d4_memory_config *config)
{
    static const core_machine_memory_device_callbacks memory_callbacks = {
        core_machine_d4_memory_read, core_machine_d4_memory_write, core_machine_d4_memory_query };
    static const core_machine_memory_device_callbacks control_callbacks = {
        core_machine_d4_control_read, core_machine_d4_control_write, core_machine_d4_control_query };
    static const core_machine_memory_device_callbacks setup_callbacks = {
        core_machine_d4_setup_read, core_machine_d4_setup_write, core_machine_d4_setup_query };
    STD_SIZE_T index;
    type_status status;

    if (machine == STD_NULL || !core_machine_d4_memory_config_is_valid(config) ||
        machine->d4_memory.configured) return TYPE_STATUS_INVALID_ARGUMENT;
    for (index = 0u; index < CORE_MACHINE_D4_MEMORY_BYTES; ++index) {
        STD_SIZE_T logical = index % (config->rom_chip_bytes * 2u);
        machine->d4_memory.compatibility[index] = (logical & 1u) == 0u ?
            config->even_rom[logical >> 1u] : config->odd_rom[logical >> 1u];
    }
    machine->d4_memory.diagnostic_high = config->diagnostic_high;
    machine->d4_memory.reset_ram_setup = config->ram_setup;
    machine->d4_memory.ram_setup = config->ram_setup;
    machine->d4_memory.configured = TYPE_TRUE;
    core_machine_d4_memory_reset(machine);
    status = core_machine_register_memory_replacement_device(machine, CORE_MACHINE_D4_REPLACEMENT_START,
        CORE_MACHINE_D4_MEMORY_BYTES, &memory_callbacks, &machine->d4_memory);
    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_register_memory_replacement_device(machine, CORE_MACHINE_D4_COMPATIBILITY_START,
        CORE_MACHINE_D4_MEMORY_BYTES, &memory_callbacks, &machine->d4_memory);
    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_register_memory_replacement_device(machine, CORE_MACHINE_D4_COMPATIBILITY_HIGH_START,
        CORE_MACHINE_D4_MEMORY_BYTES, &memory_callbacks, &machine->d4_memory);
    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_register_memory_replacement_device(machine, CORE_MACHINE_D4_SETUP_BANK_START,
        CORE_MACHINE_D4_SETUP_BANK_BYTES, &setup_callbacks, &machine->d4_memory);
    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_register_memory_replacement_device(machine, CORE_MACHINE_D4_CONTROL_PHYSICAL,
        CORE_MACHINE_D4_CONTROL_WINDOW_BYTES, &control_callbacks, &machine->d4_memory);
    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_enable_memory_parity(machine, 1024u * 1024u,
        core_machine_d4_parity_fault, machine);
    if (status != TYPE_STATUS_OK) return status;
    return core_machine_register_memory_write_observer(machine,
        core_machine_d4_memory_write_observer, machine);
}

C_VOID core_machine_d4_memory_reset(core_machine *machine)
{
    if (machine == STD_NULL || !machine->d4_memory.configured) return;
    machine->d4_memory.control = 0xffu;
    machine->d4_memory.parity_fault_mask = 0u;
    machine->d4_memory.ram_setup = machine->d4_memory.reset_ram_setup;
}
