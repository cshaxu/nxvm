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
    const vm_profile_resolver_contract_catalog catalog = {
        vm_profile_xt_5160_268_contract_ids,
        sizeof(vm_profile_xt_5160_268_contract_ids) /
            sizeof(vm_profile_xt_5160_268_contract_ids[0])};

    if (out_profile == STD_NULL ||
        vm_profile_xt_5160_268_declaration_create(&declaration) != TYPE_STATUS_OK) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    return vm_profile_resolver_resolve(&declaration, &catalog,
        &(vm_profile_resolver_session_request) {0u}, &out_profile->resolved);
}
