#include "vm/profile/model40/model40_private.h"

#include "core/platform/file.h"
#include "vm/profile/byob/blob.h"
#include "vm/profile/default_profile/pc_at_profile_private.h"

static const type_unsigned_32 vm_profile_model40_contract_ids[] = {1u};

C_VOID vm_profile_model40_core_config_initialize(core_machine_config *out_config)
{
    if (out_config == STD_NULL) return;
    *out_config = (core_machine_config) {
        .memory_bytes = 1024u * 1024u,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
        .cpu_80386_cr_mov_ignores_mod = TYPE_TRUE,
        .a20_wrap_policy = CORE_MACHINE_A20_WRAP_FIRST_TO_SECOND_MIB,
        .ticks_per_instruction = 1u,
        .instruction_timing = {1u, 0u, 0u, 0u, 0u, 0u},
        .transaction_contract = {
            .external_cycle_timing = {.page_bytes = 2048u, .page_miss_ticks = 2u,
                .page_hit_ticks = 0u,
                .overlap_policy = CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_EXPLICIT_SEQUENTIAL,
                .first_eligible_address = 0x00000000u,
                .last_eligible_address = 0x0009ffffu},
            .external_access_wait_windows = {
                {CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x03b4u, 0x03bau, 1u},
                {CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x03c0u, 0x03cfu, 1u},
                {CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x03d4u, 0x03dcu, 1u},
                {CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x07c6u, 0x07c6u, 1u},
                {CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x0bc6u, 0x0bc6u, 1u},
                {CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x0fc6u, 0x0fc6u, 1u},
                {CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, 0x000a0000u,
                    0x000affffu, 1u}},
            .dma_cycle_wait_quanta = 1u,
            .dma_cycle_bus_ready_gate_enabled = TYPE_TRUE,
            .cpu_cycle_bus_ready_gate_enabled = TYPE_TRUE,
            .cpu_prefetch_reservation_enabled = TYPE_TRUE},
        .retirement_time_contract = CORE_MACHINE_RETIREMENT_TIME_DETERMINISTIC,
        .pic_topology = CORE_MACHINE_PIC_TOPOLOGY_CASCADED,
        .dma_controller_count = CORE_MACHINE_DMA_CONTROLLER_COUNT,
        .kbc_serial_delivery_ticks = 1u,
        .clock_plan = {{1u, 1u, 0u}, {1u, 1u, 0u}, {1u, 1u, 0u},
            {1u, 1u, 0u}, {1u, 1u, 0u}, {1u, 1u, 0u}},
        .auxiliary_pit_present = TYPE_TRUE,
        .auxiliary_pit_base_port = 0x0048u,
        .kbc_aux_absent = TYPE_TRUE
    };
}

type_status vm_profile_model40_child_declaration_create(
    const vm_profile_resolver_declaration *parent,
    vm_profile_resolver_declaration *out_declaration)
{
    vm_profile_resolver_declaration declaration = {0};

    if (parent == STD_NULL || out_declaration == STD_NULL ||
        STD_STRCMP(parent->identity, "pc-at-5170") != 0) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    declaration.identity = "compaq-deskpro-386-model-40";
    declaration.parent = parent;
    declaration.provided_fields = VM_PROFILE_RESOLVER_FIELD_CORE |
        VM_PROFILE_RESOLVER_FIELD_POLICY;
    declaration.owned_fields = declaration.provided_fields;
    declaration.values.core.contract_id = vm_profile_model40_contract_ids[0];
    vm_profile_model40_core_config_initialize(&declaration.values.core.configuration);
    declaration.values.firmware_policy = VM_PROFILE_RESOLVER_FIRMWARE_POLICY_BYOB;
    declaration.values.media_policy = VM_PROFILE_RESOLVER_MEDIA_POLICY_SESSION;
    declaration.values.allowed_session_options = 0u;
    *out_declaration = declaration;
    return TYPE_STATUS_OK;
}

type_status vm_profile_model40_child_resolve(vm_resolved_profile *out_profile)
{
    vm_profile_resolver_declaration root;
    vm_profile_resolver_declaration child;
    const vm_profile_resolver_contract_catalog catalog = {
        vm_profile_model40_contract_ids,
        sizeof(vm_profile_model40_contract_ids) /
            sizeof(vm_profile_model40_contract_ids[0])};

    if (out_profile == STD_NULL ||
        vm_profile_ibm_5170_root_declaration_create(&root) != TYPE_STATUS_OK ||
        vm_profile_model40_child_declaration_create(&root, &child) != TYPE_STATUS_OK) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    return vm_profile_resolver_resolve(&child, &catalog,
        &(vm_profile_resolver_session_request) {0u}, out_profile);
}

C_INT vm_profile_model40_external_rom_is_valid(
    const vm_profile_model40_external_rom *rom)
{
    return rom != STD_NULL && rom->even_bytes != STD_NULL &&
        rom->odd_bytes != STD_NULL &&
        rom->chip_byte_count == VM_PROFILE_MODEL40_ROM_CHIP_BYTES;
}


C_INT vm_profile_model40_byob_manifest_is_valid(
    const vm_profile_model40_byob_manifest *manifest)
{
    return manifest != STD_NULL && manifest->even_path != STD_NULL && manifest->odd_path != STD_NULL &&
        manifest->even_path[0] != '\0' && manifest->odd_path[0] != '\0' &&
        manifest->provenance != STD_NULL && manifest->provenance[0] != '\0' &&
        manifest->even_sha256 != STD_NULL && manifest->odd_sha256 != STD_NULL &&
        STD_STRLEN(manifest->even_sha256) == 64u && STD_STRLEN(manifest->odd_sha256) == 64u;
}

type_status vm_profile_model40_byob_manifest_load(
    const vm_profile_model40_byob_manifest *manifest,
    type_unsigned_8 *even_bytes, type_unsigned_8 *odd_bytes,
    vm_profile_model40_external_rom *out_rom)
{
    type_status status;
    if (!vm_profile_model40_byob_manifest_is_valid(manifest) || even_bytes == STD_NULL ||
        odd_bytes == STD_NULL || out_rom == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    status = vm_profile_byob_blob_load(&(vm_profile_byob_blob) {
        manifest->even_path, manifest->even_sha256,
        VM_PROFILE_MODEL40_ROM_CHIP_BYTES}, even_bytes);
    if (status != TYPE_STATUS_OK) return status;
    status = vm_profile_byob_blob_load(&(vm_profile_byob_blob) {
        manifest->odd_path, manifest->odd_sha256,
        VM_PROFILE_MODEL40_ROM_CHIP_BYTES}, odd_bytes);
    if (status != TYPE_STATUS_OK) { STD_MEMSET(even_bytes, 0, VM_PROFILE_MODEL40_ROM_CHIP_BYTES); return status; }
    *out_rom = (vm_profile_model40_external_rom) { even_bytes, odd_bytes, VM_PROFILE_MODEL40_ROM_CHIP_BYTES };
    return TYPE_STATUS_OK;
}
static C_VOID vm_profile_model40_rom_materialize(
    const vm_profile_model40_external_rom *rom,
    type_unsigned_8 *window)
{
    STD_SIZE_T index;

    for (index = 0u; index < VM_PROFILE_MODEL40_ROM_WINDOW_BYTES; ++index) {
        STD_SIZE_T logical = index % VM_PROFILE_MODEL40_ROM_LOGICAL_BYTES;

        window[index] = (logical & 1u) == 0u ? rom->even_bytes[logical >> 1u] :
            rom->odd_bytes[logical >> 1u];
    }
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
    status = core_machine_firmware_register_immutable_rom_alias(firmware,
        VM_PROFILE_MODEL40_ROM_LOW_PHYSICAL_START,
        VM_PROFILE_MODEL40_ROM_COMPATIBILITY_ALIAS_START, sizeof(window));
    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_firmware_register_immutable_rom_alias(firmware,
        VM_PROFILE_MODEL40_ROM_LOW_PHYSICAL_START,
        VM_PROFILE_MODEL40_ROM_HIGH_ALIAS_START, sizeof(window));
    if (status != TYPE_STATUS_OK) return status;
    return core_machine_firmware_register_immutable_rom_alias(firmware,
        VM_PROFILE_MODEL40_ROM_LOW_PHYSICAL_START,
        VM_PROFILE_MODEL40_ROM_HIGH_RESET_ALIAS_START, sizeof(window));
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
        (physical >= VM_PROFILE_MODEL40_D4_COMPATIBILITY_HIGH_START &&
            physical - VM_PROFILE_MODEL40_D4_COMPATIBILITY_HIGH_START <
                VM_PROFILE_MODEL40_D4_COMPATIBILITY_BYTES) ||
        ((memory->control & 0x01u) == 0u &&
            physical >= VM_PROFILE_MODEL40_D4_REPLACEMENT_START &&
            physical - VM_PROFILE_MODEL40_D4_REPLACEMENT_START <
                VM_PROFILE_MODEL40_D4_REPLACEMENT_BYTES));
}

static STD_SIZE_T vm_profile_model40_d4_memory_offset(type_unsigned_32 physical)
{
    if (physical >= VM_PROFILE_MODEL40_D4_COMPATIBILITY_START &&
        physical - VM_PROFILE_MODEL40_D4_COMPATIBILITY_START <
            VM_PROFILE_MODEL40_D4_COMPATIBILITY_BYTES) {
        return (STD_SIZE_T)(physical - VM_PROFILE_MODEL40_D4_COMPATIBILITY_START);
    }
    if (physical >= VM_PROFILE_MODEL40_D4_COMPATIBILITY_HIGH_START &&
        physical - VM_PROFILE_MODEL40_D4_COMPATIBILITY_HIGH_START <
            VM_PROFILE_MODEL40_D4_COMPATIBILITY_BYTES) {
        return (STD_SIZE_T)(physical - VM_PROFILE_MODEL40_D4_COMPATIBILITY_HIGH_START);
    }
    return (STD_SIZE_T)(physical - VM_PROFILE_MODEL40_D4_REPLACEMENT_START);
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

static C_VOID vm_profile_model40_d4_parity_fault(C_VOID *opaque,
    type_unsigned_32 physical)
{
    vm_profile_model40_d4_memory *memory = (vm_profile_model40_d4_memory *)opaque;
    if (memory == STD_NULL || memory->machine == STD_NULL) return;
    memory->parity_fault_mask |= (type_unsigned_8)(1u << (physical & 3u));
    (C_VOID)core_machine_report_d4_iochk_fault(memory->machine);
}

static C_VOID vm_profile_model40_d4_memory_write_observer(C_VOID *opaque,
    type_unsigned_32 physical, type_native_unsigned bytes)
{
    vm_profile_model40_d4_memory *memory = (vm_profile_model40_d4_memory *)opaque;
    (C_VOID)physical;
    (C_VOID)bytes;
    if (memory != STD_NULL && memory->machine != STD_NULL &&
        memory->parity_fault_mask != 0u) {
        (C_VOID)core_machine_clear_d4_iochk_fault(memory->machine);
    }
}
static type_status vm_profile_model40_d4_control_read(C_VOID *opaque,
    type_unsigned_32 physical, type_virtual_address destination,
    type_native_unsigned bytes)
{
    vm_profile_model40_d4_memory *memory = (vm_profile_model40_d4_memory *)opaque;
    type_unsigned_32 offset;

    if (memory == STD_NULL || physical < VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL ||
        destination == 0u || bytes != 1u) return TYPE_STATUS_FAULT;
    offset = physical - VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL;
    if (offset >= VM_PROFILE_MODEL40_D4_CONTROL_WINDOW_BYTES) return TYPE_STATUS_UNSUPPORTED;
    if (offset >= VM_PROFILE_MODEL40_D4_CONTROL_REGISTER_BYTES) {
        *(type_unsigned_8 *)destination = 0xffu;
    } else if (offset == 0u) {
        *(type_unsigned_8 *)destination = (type_unsigned_8)(0xbfu & ~memory->parity_fault_mask);
    } else if (offset == 1u) {
        *(type_unsigned_8 *)destination = memory->diagnostic_high;
    } else if (offset == 2u) {
        *(type_unsigned_8 *)destination = (type_unsigned_8)memory->ram_setup;
    } else {
        *(type_unsigned_8 *)destination = (type_unsigned_8)(memory->ram_setup >> 8u);
    }
    return TYPE_STATUS_OK;
}

static type_status vm_profile_model40_d4_control_write(C_VOID *opaque,
    type_unsigned_32 physical, type_virtual_address source,
    type_native_unsigned bytes)
{
    vm_profile_model40_d4_memory *memory = (vm_profile_model40_d4_memory *)opaque;
    type_unsigned_32 offset;

    if (memory == STD_NULL || physical < VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL ||
        source == 0u || bytes != 1u) return TYPE_STATUS_FAULT;
    offset = physical - VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL;
    if (offset >= VM_PROFILE_MODEL40_D4_CONTROL_WINDOW_BYTES) return TYPE_STATUS_UNSUPPORTED;
    if (offset == 0u) {
        memory->control = *(const type_unsigned_8 *)source | 0xfcu;
        memory->parity_fault_mask = 0u;
    } else if (offset == 2u) {
        memory->ram_setup = (type_unsigned_16)((memory->ram_setup & 0xff00u) |
            *(const type_unsigned_8 *)source);
    }
    return TYPE_STATUS_OK;
}

static type_status vm_profile_model40_d4_control_query(C_VOID *opaque,
    type_unsigned_32 physical, type_native_unsigned bytes,
    core_machine_memory_access access)
{
    (C_VOID)opaque;
    if (physical < VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL ||
        physical - VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL >=
            VM_PROFILE_MODEL40_D4_CONTROL_WINDOW_BYTES || bytes != 1u) {
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
    if (memory != STD_NULL) {
        memory->control = 0xffu;
        /* 1 MiB private model: Tier-2 86Box jumper/settings visibility. */
        memory->diagnostic_high = 0xfdu;
        memory->ram_setup = 0xfc44u;
        memory->parity_fault_mask = 0u;
    }
}

type_status vm_profile_model40_d4_memory_load_compatibility(
    vm_profile_model40_d4_memory *memory,
    const vm_profile_model40_external_rom *rom)
{
    if (memory == STD_NULL || !vm_profile_model40_external_rom_is_valid(rom)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    vm_profile_model40_rom_materialize(rom, memory->compatibility);
    return TYPE_STATUS_OK;
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
    memory->machine = machine;
    status = core_machine_register_memory_device(machine,
        VM_PROFILE_MODEL40_D4_REPLACEMENT_START,
        VM_PROFILE_MODEL40_D4_REPLACEMENT_BYTES, &memory_callbacks, memory);
    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_register_memory_device(machine,
        VM_PROFILE_MODEL40_D4_COMPATIBILITY_START,
        VM_PROFILE_MODEL40_D4_COMPATIBILITY_BYTES, &memory_callbacks, memory);
    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_register_memory_device(machine,
        VM_PROFILE_MODEL40_D4_COMPATIBILITY_HIGH_START,
        VM_PROFILE_MODEL40_D4_COMPATIBILITY_BYTES, &memory_callbacks, memory);
    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_register_memory_device(machine,
        VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL, VM_PROFILE_MODEL40_D4_CONTROL_WINDOW_BYTES,
        &control_callbacks, memory);
    if (status != TYPE_STATUS_OK) return status;
    return core_machine_register_memory_write_observer(machine,
        vm_profile_model40_d4_memory_write_observer, memory);
}

type_status vm_profile_model40_d4_memory_materialize_plan(
    vm_profile_model40_d4_memory *memory, core_machine_plan *plan)
{
    static const core_machine_memory_device_callbacks memory_callbacks = {
        vm_profile_model40_d4_memory_read, vm_profile_model40_d4_memory_write,
        vm_profile_model40_d4_memory_query };
    static const core_machine_memory_device_callbacks control_callbacks = {
        vm_profile_model40_d4_control_read, vm_profile_model40_d4_control_write,
        vm_profile_model40_d4_control_query };
    type_status status;

    if (memory == STD_NULL || plan == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    status = core_machine_plan_register_memory_device(plan,
        VM_PROFILE_MODEL40_D4_REPLACEMENT_START, VM_PROFILE_MODEL40_D4_REPLACEMENT_BYTES,
        &memory_callbacks, memory);
    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_plan_register_memory_device(plan,
        VM_PROFILE_MODEL40_D4_COMPATIBILITY_START, VM_PROFILE_MODEL40_D4_COMPATIBILITY_BYTES,
        &memory_callbacks, memory);
    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_plan_register_memory_device(plan,
        VM_PROFILE_MODEL40_D4_COMPATIBILITY_HIGH_START,
        VM_PROFILE_MODEL40_D4_COMPATIBILITY_BYTES, &memory_callbacks, memory);
    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_plan_register_memory_device(plan,
        VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL,
        VM_PROFILE_MODEL40_D4_CONTROL_WINDOW_BYTES, &control_callbacks, memory);
    if (status != TYPE_STATUS_OK) return status;
    return core_machine_plan_enable_d4_memory_parity(plan, &memory->parity_fault_mask);
}

type_status vm_profile_model40_d4_memory_enable_parity(core_machine *machine,
    vm_profile_model40_d4_memory *memory)
{
    if (machine == STD_NULL || memory == STD_NULL || memory->machine != machine) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    return core_machine_enable_memory_parity(machine, 1024u * 1024u,
        vm_profile_model40_d4_parity_fault, memory);
}
