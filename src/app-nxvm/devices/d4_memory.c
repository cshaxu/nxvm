#include "lib/types/types_interface.h"
#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/machine_board_state.h"

#define CORE_MACHINE_D4_CONTROL_PHYSICAL 0x80c00000u
#define CORE_MACHINE_D4_CONTROL_WINDOW_BYTES 4096u
#define CORE_MACHINE_D4_SETUP_BANK_START 0x00100000u
#define CORE_MACHINE_D4_SETUP_BANK_BYTES 0x00e00000u

static core_machine_d4_memory *core_machine_d4_state(void *owner)
{
    core_machine *machine = (core_machine *)owner;

    return machine == LIB_NULL ? LIB_NULL : &machine->board->d4_memory;
}

/* In the selected D4 setup, low nibble 1 disconnects every extension-RAM
 * bank below F00000h.  This is an External-L2 board relation, cross-checked
 * against the selected DeskPro reference model; it deliberately does not
 * attempt to generalize that model's full RAM-card table. */
static lib_i32 core_machine_d4_setup_blocks_extension(const core_machine_d4_memory *memory)
{
    return memory != LIB_NULL && (memory->ram_setup & 0x0fu) == 1u;
}

static lib_status core_machine_d4_setup_read(void *opaque, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    const core_machine_d4_memory *memory = core_machine_d4_state(opaque);

    (void)physical;
    (void)observe_only;
    if (memory == LIB_NULL || destination == 0u || !core_machine_d4_setup_blocks_extension(memory)) {
        return LIB_STATUS_UNSUPPORTED;
    }
    lib_memory_set((void *)destination, 0xff, bytes);
    return LIB_STATUS_OK;
}

static lib_status core_machine_d4_setup_write(void *opaque, lib_u32 physical,
    lib_uptr source, lib_uptr bytes)
{
    const core_machine_d4_memory *memory = core_machine_d4_state(opaque);

    (void)physical;
    (void)source;
    (void)bytes;
    return core_machine_d4_setup_blocks_extension(memory) ? LIB_STATUS_OK :
        LIB_STATUS_UNSUPPORTED;
}

static lib_status core_machine_d4_setup_query(void *opaque, lib_u32 physical,
    lib_uptr bytes, core_machine_memory_access access)
{
    const core_machine_d4_memory *memory = core_machine_d4_state(opaque);

    (void)physical;
    (void)bytes;
    (void)access;
    return core_machine_d4_setup_blocks_extension(memory) ? LIB_STATUS_OK :
        LIB_STATUS_UNSUPPORTED;
}

static lib_status core_machine_d4_control_read(void *opaque, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    core_machine_d4_memory *memory = core_machine_d4_state(opaque);
    lib_u32 offset;

    (void)observe_only;
    if (memory == LIB_NULL || destination == 0u || bytes != 1u ||
        physical < CORE_MACHINE_D4_CONTROL_PHYSICAL) return LIB_STATUS_INTERNAL_ERROR;
    offset = physical - CORE_MACHINE_D4_CONTROL_PHYSICAL;
    if (offset >= CORE_MACHINE_D4_CONTROL_WINDOW_BYTES) return LIB_STATUS_UNSUPPORTED;
    if (offset == 0u) *(lib_u8 *)destination = (lib_u8)(
        memory->diagnostic_low & ~memory->parity_fault_mask);
    else if (offset == 1u) *(lib_u8 *)destination = memory->diagnostic_high;
    else if (offset == 2u) *(lib_u8 *)destination = (lib_u8)memory->ram_setup;
    else if (offset == 3u) *(lib_u8 *)destination = (lib_u8)(memory->ram_setup >> 8u);
    else *(lib_u8 *)destination = 0xffu;
    return LIB_STATUS_OK;
}

static lib_status core_machine_d4_control_write(void *opaque, lib_u32 physical,
    lib_uptr source, lib_uptr bytes)
{
    core_machine_d4_memory *memory = core_machine_d4_state(opaque);
    lib_u32 offset;

    if (memory == LIB_NULL || source == 0u || bytes != 1u ||
        physical < CORE_MACHINE_D4_CONTROL_PHYSICAL) return LIB_STATUS_INTERNAL_ERROR;
    offset = physical - CORE_MACHINE_D4_CONTROL_PHYSICAL;
    if (offset >= CORE_MACHINE_D4_CONTROL_WINDOW_BYTES) return LIB_STATUS_UNSUPPORTED;
    if (offset == 0u) {
        memory->control = *(const lib_u8 *)source | 0xfcu;
        memory->parity_fault_mask = 0u;
    } else if (offset == 2u) {
        memory->ram_setup = (lib_u16)((memory->ram_setup & 0xff00u) |
            *(const lib_u8 *)source);
    }
    return LIB_STATUS_OK;
}

static lib_status core_machine_d4_control_query(void *opaque, lib_u32 physical,
    lib_uptr bytes, core_machine_memory_access access)
{
    (void)opaque;
    if (physical < CORE_MACHINE_D4_CONTROL_PHYSICAL ||
        physical - CORE_MACHINE_D4_CONTROL_PHYSICAL >= CORE_MACHINE_D4_CONTROL_WINDOW_BYTES ||
        bytes != 1u) return LIB_STATUS_UNSUPPORTED;
    return access == CORE_MACHINE_MEMORY_ACCESS_READ || access == CORE_MACHINE_MEMORY_ACCESS_WRITE ?
        LIB_STATUS_OK : LIB_STATUS_UNSUPPORTED;
}

static void core_machine_d4_parity_fault(void *opaque, lib_u32 physical)
{
    core_machine *machine = (core_machine *)opaque;

    if (machine == LIB_NULL) return;
    machine->board->d4_memory.parity_fault_mask |= (lib_u8)(1u << (physical & 3u));
    (void)core_machine_report_d4_iochk_fault(machine);
}

static void core_machine_d4_memory_write_observer(void *opaque, lib_u32 physical,
    lib_uptr bytes)
{
    core_machine *machine = (core_machine *)opaque;

    (void)physical;
    (void)bytes;
    if (machine != LIB_NULL && machine->board->d4_memory.parity_fault_mask != 0u) {
        (void)core_machine_clear_d4_iochk_fault(machine);
    }
}

lib_i32 core_machine_d4_memory_config_is_valid(const core_machine_d4_memory_config *config)
{
    return config != LIB_NULL && config->present == LIB_TRUE;
}

lib_status core_machine_d4_memory_configure(core_machine *machine,
    const core_machine_d4_memory_config *config)
{
    static const core_machine_memory_device_route routes[2] = {
        { CORE_MACHINE_D4_SETUP_BANK_START, CORE_MACHINE_D4_SETUP_BANK_BYTES,
            { core_machine_d4_setup_read, core_machine_d4_setup_write,
                core_machine_d4_setup_query },
            CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT },
        { CORE_MACHINE_D4_CONTROL_PHYSICAL, CORE_MACHINE_D4_CONTROL_WINDOW_BYTES,
            { core_machine_d4_control_read, core_machine_d4_control_write,
                core_machine_d4_control_query },
            CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT }
    };
    static const core_machine_memory_parity_config parity = {
        1024u * 1024u, core_machine_d4_parity_fault
    };
    lib_status status;

    if (machine == LIB_NULL || !core_machine_d4_memory_config_is_valid(config) ||
        machine->board->d4_memory.configured) return LIB_STATUS_INVALID_ARGUMENT;
    status = core_machine_install_memory_device_routes(machine, routes, 2u,
        core_machine_d4_memory_write_observer, &parity, machine);
    if (status != LIB_STATUS_OK) return status;
    machine->board->d4_memory.diagnostic_low = config->diagnostic_low;
    machine->board->d4_memory.diagnostic_high = config->diagnostic_high;
    machine->board->d4_memory.reset_ram_setup = config->ram_setup;
    machine->board->d4_memory.ram_setup = config->ram_setup;
    machine->board->d4_memory.configured = LIB_TRUE;
    core_machine_d4_memory_reset(machine);
    return LIB_STATUS_OK;
}

void core_machine_d4_memory_reset(core_machine *machine)
{
    if (machine == LIB_NULL || !machine->board->d4_memory.configured) return;
    machine->board->d4_memory.control = 0xffu;
    machine->board->d4_memory.parity_fault_mask = 0u;
    machine->board->d4_memory.ram_setup = machine->board->d4_memory.reset_ram_setup;
}
