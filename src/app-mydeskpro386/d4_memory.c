#include "lib/types/types_interface.h"
#include "app-mydeskpro386/d4_memory.h"

#define MYDESKPRO386_D4_CONTROL_PHYSICAL 0x80c00000u
#define MYDESKPRO386_D4_CONTROL_WINDOW_BYTES 4096u
#define MYDESKPRO386_D4_SETUP_BANK_START 0x00100000u
#define MYDESKPRO386_D4_SETUP_BANK_BYTES 0x00e00000u
#define MYDESKPRO386_D4_RELOCATED_START 0x000e0000u
#define MYDESKPRO386_D4_RELOCATED_BYTES 0x00020000u

static mydeskpro386_d4_memory *mydeskpro386_d4_state(void *owner)
{
    return (mydeskpro386_d4_memory *)owner;
}

static lib_status mydeskpro386_d4_apply_control(mydeskpro386_d4_memory *memory,
    lib_u8 control)
{
    lib_bool selected;
    lib_bool writable;
    lib_status status;

    if (memory == LIB_NULL || memory->core == LIB_NULL) return LIB_STATUS_INVALID_STATE;
    selected = (control & 0x01u) == 0u ? LIB_TRUE : LIB_FALSE;
    writable = (control & 0x02u) != 0u ? LIB_TRUE : LIB_FALSE;
    status = x86_core_machine_set_memory_alias_decode(memory->core,
        MYDESKPRO386_D4_RELOCATED_START, MYDESKPRO386_D4_RELOCATED_BYTES,
        selected, writable);
    if (status == LIB_STATUS_OK) memory->control = control | 0xfcu;
    return status;
}

/* In the selected D4 setup, low nibble 1 disconnects every extension-RAM
 * bank below F00000h.  This is an External-L2 board relation, cross-checked
 * against the selected DeskPro reference model; it deliberately does not
 * attempt to generalize that model's full RAM-card table. */
static lib_bool mydeskpro386_d4_setup_blocks_extension(const mydeskpro386_d4_memory *memory)
{
    return memory != LIB_NULL && (memory->ram_setup & 0x0fu) == 1u;
}

static lib_status mydeskpro386_d4_setup_read(void *opaque, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    const mydeskpro386_d4_memory *memory = mydeskpro386_d4_state(opaque);

    (void)physical;
    (void)observe_only;
    if (memory == LIB_NULL || destination == 0u || !mydeskpro386_d4_setup_blocks_extension(memory)) {
        return LIB_STATUS_UNSUPPORTED;
    }
    lib_memory_set((void *)destination, 0xff, bytes);
    return LIB_STATUS_OK;
}

static lib_status mydeskpro386_d4_setup_write(void *opaque, lib_u32 physical,
    lib_uptr source, lib_uptr bytes)
{
    const mydeskpro386_d4_memory *memory = mydeskpro386_d4_state(opaque);

    (void)physical;
    (void)source;
    (void)bytes;
    return mydeskpro386_d4_setup_blocks_extension(memory) ? LIB_STATUS_OK :
        LIB_STATUS_UNSUPPORTED;
}

static lib_status mydeskpro386_d4_setup_query(void *opaque, lib_u32 physical,
    lib_uptr bytes, x86_core_machine_memory_access access)
{
    const mydeskpro386_d4_memory *memory = mydeskpro386_d4_state(opaque);

    (void)physical;
    (void)bytes;
    (void)access;
    return mydeskpro386_d4_setup_blocks_extension(memory) ? LIB_STATUS_OK :
        LIB_STATUS_UNSUPPORTED;
}

static lib_status mydeskpro386_d4_control_read(void *opaque, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    mydeskpro386_d4_memory *memory = mydeskpro386_d4_state(opaque);
    lib_u32 offset;
    lib_u16 value;

    (void)observe_only;
    if (memory == LIB_NULL || destination == 0u || (bytes != 1u && bytes != 2u) ||
        physical < MYDESKPRO386_D4_CONTROL_PHYSICAL) return LIB_STATUS_INTERNAL_ERROR;
    offset = physical - MYDESKPRO386_D4_CONTROL_PHYSICAL;
    if (offset > MYDESKPRO386_D4_CONTROL_WINDOW_BYTES - bytes) return LIB_STATUS_UNSUPPORTED;
    if (bytes == 1u) {
        if (offset == 0u) value = memory->diagnostic_low & ~memory->parity_fault_mask;
        else if (offset == 1u) value = memory->diagnostic_high;
        else if (offset == 2u) value = (lib_u8)memory->ram_setup;
        else if (offset == 3u) value = (lib_u8)(memory->ram_setup >> 8u);
        else value = 0xffu;
    } else if (offset == 0u) {
        value = (lib_u16)(memory->diagnostic_low & ~memory->parity_fault_mask) |
            (lib_u16)((lib_u16)memory->diagnostic_high << 8u);
    } else if (offset == 2u) value = memory->ram_setup;
    else value = 0xffffu;
    lib_memory_copy((void *)destination, &value, bytes);
    return LIB_STATUS_OK;
}

static lib_status mydeskpro386_d4_control_write(void *opaque, lib_u32 physical,
    lib_uptr source, lib_uptr bytes)
{
    mydeskpro386_d4_memory *memory = mydeskpro386_d4_state(opaque);
    lib_u32 offset;
    lib_u16 value = 0u;

    if (memory == LIB_NULL || source == 0u || (bytes != 1u && bytes != 2u) ||
        physical < MYDESKPRO386_D4_CONTROL_PHYSICAL) return LIB_STATUS_INTERNAL_ERROR;
    offset = physical - MYDESKPRO386_D4_CONTROL_PHYSICAL;
    if (offset > MYDESKPRO386_D4_CONTROL_WINDOW_BYTES - bytes) return LIB_STATUS_UNSUPPORTED;
    lib_memory_copy(&value, (const void *)source, bytes);
    if (offset == 0u) {
        lib_status status = mydeskpro386_d4_apply_control(memory,
            (lib_u8)value);

        if (status != LIB_STATUS_OK) return status;
        memory->parity_fault_mask = 0u;
    } else if (offset == 2u) {
        if (bytes == 1u) memory->ram_setup = (lib_u16)(
            (memory->ram_setup & 0xff00u) | value);
        else memory->ram_setup = value;
    }
    return LIB_STATUS_OK;
}

static lib_status mydeskpro386_d4_control_query(void *opaque, lib_u32 physical,
    lib_uptr bytes, x86_core_machine_memory_access access)
{
    (void)opaque;
    if (physical < MYDESKPRO386_D4_CONTROL_PHYSICAL ||
        (bytes != 1u && bytes != 2u) ||
        physical - MYDESKPRO386_D4_CONTROL_PHYSICAL >
            MYDESKPRO386_D4_CONTROL_WINDOW_BYTES - bytes) return LIB_STATUS_UNSUPPORTED;
    return access == X86_CORE_MACHINE_MEMORY_ACCESS_READ || access == X86_CORE_MACHINE_MEMORY_ACCESS_WRITE ?
        LIB_STATUS_OK : LIB_STATUS_UNSUPPORTED;
}

static void mydeskpro386_d4_parity_fault(void *opaque, lib_u32 physical)
{
    mydeskpro386_d4_memory *memory = mydeskpro386_d4_state(opaque);

    if (memory == LIB_NULL) return;
    memory->parity_fault_mask |= (lib_u8)(1u << (physical & 3u));
    if (memory->iochk_output != LIB_NULL)
        memory->iochk_output(memory->iochk_context, LIB_TRUE);
}

static void mydeskpro386_d4_memory_write_observer(void *opaque, lib_u32 physical,
    lib_uptr bytes)
{
    mydeskpro386_d4_memory *memory = mydeskpro386_d4_state(opaque);

    (void)physical;
    (void)bytes;
    if (memory != LIB_NULL && memory->parity_fault_mask != 0u &&
        memory->iochk_output != LIB_NULL) {
        memory->iochk_output(memory->iochk_context, LIB_FALSE);
    }
}

lib_bool mydeskpro386_d4_memory_config_is_valid(const mydeskpro386_d4_memory_config *config)
{
    return config != LIB_NULL && config->present == LIB_TRUE;
}

lib_status mydeskpro386_d4_memory_configure(x86_core_machine *core,
    mydeskpro386_d4_memory *memory, const mydeskpro386_d4_memory_config *config,
    mydeskpro386_d4_iochk_output output, void *context)
{
    static const x86_core_machine_memory_device_route routes[2] = {
        {.physical_start = MYDESKPRO386_D4_SETUP_BANK_START,
            .bytes = MYDESKPRO386_D4_SETUP_BANK_BYTES,
            .callbacks = {mydeskpro386_d4_setup_read, mydeskpro386_d4_setup_write,
                mydeskpro386_d4_setup_query},
            .mode = X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT,
            .failure = LIB_NULL},
        {.physical_start = MYDESKPRO386_D4_CONTROL_PHYSICAL,
            .bytes = MYDESKPRO386_D4_CONTROL_WINDOW_BYTES,
            .callbacks = {mydeskpro386_d4_control_read, mydeskpro386_d4_control_write,
                mydeskpro386_d4_control_query},
            .mode = X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT,
            .failure = LIB_NULL}
    };
    static const x86_core_machine_memory_parity_config parity = {
        1024u * 1024u, mydeskpro386_d4_parity_fault
    };
    lib_status status;

    if (core == LIB_NULL || memory == LIB_NULL ||
        !mydeskpro386_d4_memory_config_is_valid(config) ||
        memory->configured) return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_core_machine_install_memory_device_routes(core, routes, 2u,
        mydeskpro386_d4_memory_write_observer, &parity, memory);
    if (status != LIB_STATUS_OK) return status;
    status = x86_core_machine_set_memory_alias_decode(core,
        MYDESKPRO386_D4_RELOCATED_START, MYDESKPRO386_D4_RELOCATED_BYTES,
        LIB_FALSE, LIB_TRUE);
    if (status != LIB_STATUS_OK) {
        lib_status rollback = x86_core_machine_remove_memory_device_routes(core, memory);

        return rollback == LIB_STATUS_OK ? status : rollback;
    }
    memory->control = 0xffu;
    memory->parity_fault_mask = 0u;
    memory->diagnostic_low = config->diagnostic_low;
    memory->diagnostic_high = config->diagnostic_high;
    memory->reset_ram_setup = config->ram_setup;
    memory->ram_setup = config->ram_setup;
    memory->iochk_output = output;
    memory->iochk_context = context;
    memory->core = core;
    memory->configured = LIB_TRUE;
    return LIB_STATUS_OK;
}

void mydeskpro386_d4_memory_reset(mydeskpro386_d4_memory *memory)
{
    if (memory == LIB_NULL || !memory->configured) return;
    memory->control = 0xffu;
    (void)mydeskpro386_d4_apply_control(memory, 0xffu);
    memory->parity_fault_mask = 0u;
    memory->ram_setup = memory->reset_ram_setup;
}
