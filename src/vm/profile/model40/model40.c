#include "vm/profile/model40/model40.h"

C_INT vm_profile_model40_external_rom_is_valid(
    const vm_profile_model40_external_rom *rom)
{
    return rom != STD_NULL && rom->even_bytes != STD_NULL &&
        rom->odd_bytes != STD_NULL &&
        rom->chip_byte_count == VM_PROFILE_MODEL40_ROM_CHIP_BYTES;
}

static C_VOID vm_profile_model40_rom_materialize(
    const vm_profile_model40_external_rom *rom,
    type_unsigned_8 *window)
{
    STD_SIZE_T index;

    for (index = 0u; index < VM_PROFILE_MODEL40_ROM_LOGICAL_BYTES; ++index) {
        window[index] = (index & 1u) == 0u ? rom->even_bytes[index >> 1u] :
            rom->odd_bytes[index >> 1u];
    }
    STD_MEMCPY(window + VM_PROFILE_MODEL40_ROM_LOGICAL_BYTES, window,
        VM_PROFILE_MODEL40_ROM_LOGICAL_BYTES);
}

static type_status vm_profile_model40_firmware_configure(C_VOID *opaque,
    core_machine_firmware_context *firmware)
{
    const vm_profile_model40_external_rom *rom =
        (const vm_profile_model40_external_rom *)opaque;
    type_unsigned_8 window[VM_PROFILE_MODEL40_ROM_WINDOW_BYTES];
    type_status status;

    if (!vm_profile_model40_external_rom_is_valid(rom)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    vm_profile_model40_rom_materialize(rom, window);
    status = core_machine_firmware_register_immutable_rom(firmware,
        VM_PROFILE_MODEL40_ROM_LOW_PHYSICAL_START, window, sizeof(window));
    if (status != TYPE_STATUS_OK) return status;
    return core_machine_firmware_register_immutable_rom(firmware,
        VM_PROFILE_MODEL40_ROM_HIGH_PHYSICAL_START, window, sizeof(window));
}

static type_status vm_profile_model40_firmware_reset(C_VOID *opaque,
    core_machine_firmware_context *firmware)
{
    (C_VOID)firmware;
    return vm_profile_model40_external_rom_is_valid(
        (const vm_profile_model40_external_rom *)opaque) ? TYPE_STATUS_OK :
        TYPE_STATUS_INVALID_ARGUMENT;
}

static const core_machine_firmware_provider vm_profile_model40_provider = {
    vm_profile_model40_firmware_configure,
    vm_profile_model40_firmware_reset,
    STD_NULL
};

const core_machine_firmware_provider *vm_profile_model40_firmware_provider(C_VOID)
{
    return &vm_profile_model40_provider;
}

static C_INT vm_profile_model40_d4_memory_is_active(
    const vm_profile_model40_d4_memory *memory, type_unsigned_32 physical)
{
    return memory != STD_NULL && ((physical >= VM_PROFILE_MODEL40_D4_COMPATIBILITY_START &&
        physical - VM_PROFILE_MODEL40_D4_COMPATIBILITY_START <
            VM_PROFILE_MODEL40_D4_COMPATIBILITY_BYTES) ||
        ((memory->control & 0x01u) == 0u &&
            physical >= VM_PROFILE_MODEL40_D4_REPLACEMENT_START &&
            physical - VM_PROFILE_MODEL40_D4_REPLACEMENT_START <
                VM_PROFILE_MODEL40_D4_REPLACEMENT_BYTES));
}

static STD_SIZE_T vm_profile_model40_d4_memory_offset(type_unsigned_32 physical)
{
    return physical >= VM_PROFILE_MODEL40_D4_COMPATIBILITY_START ?
        (STD_SIZE_T)(physical - VM_PROFILE_MODEL40_D4_COMPATIBILITY_START) :
        (STD_SIZE_T)(physical - VM_PROFILE_MODEL40_D4_REPLACEMENT_START);
}

static type_status vm_profile_model40_d4_memory_read(C_VOID *opaque,
    type_unsigned_32 physical, type_virtual_address destination,
    type_native_unsigned bytes)
{
    vm_profile_model40_d4_memory *memory = (vm_profile_model40_d4_memory *)opaque;
    STD_SIZE_T offset;

    if (memory == STD_NULL || destination == 0u || !vm_profile_model40_d4_memory_is_active(
            memory, physical)) return TYPE_STATUS_UNSUPPORTED;
    offset = vm_profile_model40_d4_memory_offset(physical);
    if (bytes > VM_PROFILE_MODEL40_D4_COMPATIBILITY_BYTES - offset) {
        return TYPE_STATUS_FAULT;
    }
    STD_MEMCPY((C_VOID *)destination, memory->compatibility + offset, bytes);
    return TYPE_STATUS_OK;
}

static type_status vm_profile_model40_d4_memory_write(C_VOID *opaque,
    type_unsigned_32 physical, type_virtual_address source,
    type_native_unsigned bytes)
{
    vm_profile_model40_d4_memory *memory = (vm_profile_model40_d4_memory *)opaque;
    STD_SIZE_T offset;

    if (memory == STD_NULL || source == 0u || !vm_profile_model40_d4_memory_is_active(
            memory, physical)) return TYPE_STATUS_UNSUPPORTED;
    if ((memory->control & 0x02u) == 0u) return TYPE_STATUS_FAULT;
    offset = vm_profile_model40_d4_memory_offset(physical);
    if (bytes > VM_PROFILE_MODEL40_D4_COMPATIBILITY_BYTES - offset) {
        return TYPE_STATUS_FAULT;
    }
    STD_MEMCPY(memory->compatibility + offset, (C_VOID *)source, bytes);
    return TYPE_STATUS_OK;
}

static type_status vm_profile_model40_d4_memory_query(C_VOID *opaque,
    type_unsigned_32 physical, type_native_unsigned bytes,
    core_machine_memory_access access)
{
    vm_profile_model40_d4_memory *memory = (vm_profile_model40_d4_memory *)opaque;
    STD_SIZE_T offset;

    if (memory == STD_NULL || !vm_profile_model40_d4_memory_is_active(memory, physical)) {
        return TYPE_STATUS_UNSUPPORTED;
    }
    offset = vm_profile_model40_d4_memory_offset(physical);
    if (bytes > VM_PROFILE_MODEL40_D4_COMPATIBILITY_BYTES - offset) {
        return TYPE_STATUS_FAULT;
    }
    if (access == CORE_MACHINE_MEMORY_ACCESS_WRITE && (memory->control & 0x02u) == 0u) {
        return TYPE_STATUS_FAULT;
    }
    return TYPE_STATUS_OK;
}

static type_status vm_profile_model40_d4_control_read(C_VOID *opaque,
    type_unsigned_32 physical, type_virtual_address destination,
    type_native_unsigned bytes)
{
    (C_VOID)opaque;
    if (physical != VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL || destination == 0u ||
        bytes != 1u) return TYPE_STATUS_FAULT;
    /* No D4 parity fault source is modelled here.  The selected 1 MiB board
     * therefore reports all four parity bytes good, its 1 MiB jumper active,
     * and no 512/640 KiB or 2 MiB option selection. */
    *(type_unsigned_8 *)destination = 0xbfu;
    return TYPE_STATUS_OK;
}

static type_status vm_profile_model40_d4_control_write(C_VOID *opaque,
    type_unsigned_32 physical, type_virtual_address source,
    type_native_unsigned bytes)
{
    vm_profile_model40_d4_memory *memory = (vm_profile_model40_d4_memory *)opaque;

    if (memory == STD_NULL || physical != VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL ||
        source == 0u || bytes != 1u) return TYPE_STATUS_FAULT;
    memory->control = *(const type_unsigned_8 *)source | 0xfcu;
    return TYPE_STATUS_OK;
}

static type_status vm_profile_model40_d4_control_query(C_VOID *opaque,
    type_unsigned_32 physical, type_native_unsigned bytes,
    core_machine_memory_access access)
{
    (C_VOID)opaque;
    if (physical != VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL || bytes != 1u) {
        return TYPE_STATUS_UNSUPPORTED;
    }
    return access == CORE_MACHINE_MEMORY_ACCESS_READ ||
        access == CORE_MACHINE_MEMORY_ACCESS_WRITE ? TYPE_STATUS_OK :
        TYPE_STATUS_UNSUPPORTED;
}
C_VOID vm_profile_model40_d4_memory_initialize(vm_profile_model40_d4_memory *memory)
{
    if (memory == STD_NULL) return;
    STD_MEMSET(memory, 0, sizeof(*memory));
    vm_profile_model40_d4_memory_reset(memory);
}

C_VOID vm_profile_model40_d4_memory_reset(vm_profile_model40_d4_memory *memory)
{
    if (memory != STD_NULL) memory->control = 0xffu;
}

type_status vm_profile_model40_d4_memory_register(core_machine *machine,
    vm_profile_model40_d4_memory *memory)
{
    static const core_machine_memory_device_callbacks memory_callbacks = {
        vm_profile_model40_d4_memory_read, vm_profile_model40_d4_memory_write,
        vm_profile_model40_d4_memory_query };
    static const core_machine_memory_device_callbacks control_callbacks = {
        vm_profile_model40_d4_control_read, vm_profile_model40_d4_control_write,
        vm_profile_model40_d4_control_query };
    type_status status;

    if (machine == STD_NULL || memory == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    status = core_machine_register_memory_device(machine,
        VM_PROFILE_MODEL40_D4_REPLACEMENT_START,
        VM_PROFILE_MODEL40_D4_REPLACEMENT_BYTES, &memory_callbacks, memory);
    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_register_memory_device(machine,
        VM_PROFILE_MODEL40_D4_COMPATIBILITY_START,
        VM_PROFILE_MODEL40_D4_COMPATIBILITY_BYTES, &memory_callbacks, memory);
    if (status != TYPE_STATUS_OK) return status;
    return core_machine_register_memory_device(machine,
        VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL, 1u, &control_callbacks, memory);
}