#include "vm/profile/model40/model40_private.h"

#include "core/platform/file.h"
#include "vm/profile/byob/blob.h"
#include "vm/profile/default_profile/pc_at_profile_private.h"

static const type_unsigned_32 vm_profile_model40_contract_ids[] = {1u};

C_VOID vm_profile_model40_core_config_initialize(core_machine_config *out_config)
{
    if (out_config == STD_NULL) return;
    *out_config = (core_machine_config) {
        .memory_bytes = 2u * 1024u * 1024u,
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
    STD_NULL,
    STD_NULL
};

const core_machine_firmware_provider *vm_profile_model40_firmware_provider(C_VOID)
{
    return &vm_profile_model40_provider;
}
