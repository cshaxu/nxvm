#include "type.h"

#include "vm/profile/xt/xt_5160_268.h"

/* This bit represents only the B1 CPU/Core input. It is not a claim that a
 * 5160 board device has been bound. */
#define VM_PROFILE_XT_5160_268_CORE_DEVICE 0x00000001u
#define VM_PROFILE_XT_5160_268_CONTRACT_ID 2u

static const type_unsigned_32 vm_profile_xt_5160_268_contract_ids[] = {
    VM_PROFILE_XT_5160_268_CONTRACT_ID
};

type_status vm_profile_xt_5160_268_declaration_create(
    vm_profile_resolver_declaration *out_declaration)
{
    vm_profile_resolver_declaration declaration = {0};

    if (out_declaration == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    declaration.identity = "ibm-5160-model-268";
    declaration.provided_fields = VM_PROFILE_RESOLVER_FIELD_ALL;
    declaration.owned_fields = VM_PROFILE_RESOLVER_FIELD_ALL;
    declaration.values.core.contract_id = VM_PROFILE_XT_5160_268_CONTRACT_ID;
    declaration.values.core.configuration.memory_bytes = 256u * 1024u;
    declaration.values.core.configuration.cpu_profile = CORE_MACHINE_CPU_PROFILE_8088;
    declaration.values.core.configuration.fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE;
    declaration.values.core.configuration.pic_topology =
        CORE_MACHINE_PIC_TOPOLOGY_SINGLE;
    declaration.values.core.configuration.dma_controller_count = 1u;
    declaration.values.core.configuration.keyboard_topology =
        CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI;
    declaration.values.core.configuration.xt_ppi_keyboard =
        (core_machine_xt_ppi_keyboard_config) {0x0060u, 0x0061u, 0x0062u,
            0x0063u, 1u};
    declaration.values.core.configuration.ticks_per_instruction = 1u;
    declaration.values.enabled_devices = VM_PROFILE_XT_5160_268_CORE_DEVICE;
    declaration.values.firmware_policy = VM_PROFILE_RESOLVER_FIRMWARE_POLICY_BYOB;
    declaration.values.media_policy = VM_PROFILE_RESOLVER_MEDIA_POLICY_NONE;
    declaration.values.allowed_session_options = 0u;
    *out_declaration = declaration;
    return TYPE_STATUS_OK;
}

type_status vm_profile_xt_5160_268_resolve(
    vm_profile_xt_5160_268_resolved_profile *out_profile)
{
    vm_profile_resolver_declaration declaration;
    type_status status;
    const vm_profile_resolver_contract_catalog catalog = {
        vm_profile_xt_5160_268_contract_ids,
        sizeof(vm_profile_xt_5160_268_contract_ids) /
            sizeof(vm_profile_xt_5160_268_contract_ids[0])};

    if (out_profile == STD_NULL ||
        vm_profile_xt_5160_268_declaration_create(&declaration) != TYPE_STATUS_OK) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    status = vm_profile_resolver_resolve(&declaration, &catalog,
        &(vm_profile_resolver_session_request) {0u}, &out_profile->resolved);
    if (status != TYPE_STATUS_OK) return status;
    out_profile->topology = (core_machine_plan_topology) {0};
    /* The selected IBM CGA is one VADP-owned aperture and port grammar.
     * These scheduling quanta are the existing non-physical VADP contract;
     * selected-adapter physical timing remains a later receiver. */
    out_profile->topology.display_present = TYPE_TRUE;
    out_profile->topology.display = (core_machine_display_config) {
        .text_timing = {48u, 8u, 8u},
        .cga_vram_present = TYPE_TRUE,
        .ports = {0u, 0u, 0u, 0u, 0u, 0u, 0x03d4u, 0x03dau}
    };
    out_profile->topology.dma_present = TYPE_TRUE;
    out_profile->topology.dma = (core_machine_dma_wiring) {2u, 1u, 0u};
    out_profile->topology.fdc_present = TYPE_TRUE;
    out_profile->topology.fdc_drives = (core_machine_fdc_drive_bindings) {{
        VM_PROFILE_XT_5160_268_FDD_MEDIA_ID, CORE_MACHINE_MEDIA_ID_INVALID,
        CORE_MACHINE_MEDIA_ID_INVALID, CORE_MACHINE_MEDIA_ID_INVALID}};
    out_profile->topology.fdc = (core_machine_fdc_config) {0x03f2u, 0x03f4u,
        0x03f5u, 0u, 0u, 6u, 2u, CORE_MACHINE_FDC_UNREADY_READ_GENERIC};
    return TYPE_STATUS_OK;
}
