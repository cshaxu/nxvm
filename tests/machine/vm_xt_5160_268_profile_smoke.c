#include "type.h"

#include "core/product/session/session_provider.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/provider.h"
#include "vm/product/session_catalog.h"
#include "vm/profile/xt/xt_5160_268.h"

static C_INT vm_xt_5160_268_declaration_is_fixed(C_VOID)
{
    vm_profile_resolver_declaration declaration;
    vm_profile_xt_5160_268_resolved_profile profile;

    if (vm_profile_xt_5160_268_declaration_create(&declaration) != TYPE_STATUS_OK ||
        vm_profile_xt_5160_268_resolve(&profile) != TYPE_STATUS_OK ||
        STD_STRCMP(profile.resolved.identity, "ibm-5160-model-268") != 0 ||
        profile.resolved.owned_fields != VM_PROFILE_RESOLVER_FIELD_ALL ||
        profile.resolved.values.core.configuration.memory_bytes != 256u * 1024u ||
        profile.resolved.values.core.configuration.cpu_profile !=
            CORE_MACHINE_CPU_PROFILE_8088 ||
        profile.resolved.values.core.configuration.fpu_profile !=
            CORE_MACHINE_FPU_PROFILE_NONE ||
        profile.resolved.values.core.configuration.pic_topology !=
            CORE_MACHINE_PIC_TOPOLOGY_SINGLE ||
        profile.resolved.values.core.configuration.dma_controller_count != 1u ||
        profile.resolved.values.allowed_session_options != 0u ||
        profile.resolved.values.port_leaf_count != 0u ||
        profile.resolved.values.memory_window_count != 0u ||
        profile.resolved.values.irq_route_count != 0u ||
        profile.resolved.values.drq_route_count != 0u) return 1;
    declaration.values.core.configuration.memory_bytes = 512u * 1024u;
    return profile.resolved.values.core.configuration.memory_bytes != 256u * 1024u;
}

static C_INT vm_xt_5160_268_is_not_a_runnable_at_alias(C_VOID)
{
    vm_session_config config = {
        .profile_kind = VM_SESSION_PROFILE_IBM_5160_MODEL_268,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    vm_session *session = STD_NULL;

    if (STD_STRCMP(vm_session_profile_name(config.profile_kind),
            "ibm-5160-model-268") != 0 ||
        vm_session_create(&config, &session) != TYPE_STATUS_UNSUPPORTED ||
        session != STD_NULL) return 1;
    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_8086;
    if (vm_session_create(&config, &session) != TYPE_STATUS_INVALID_ARGUMENT) return 1;
    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_DEFAULT;
    config.memory_bytes = 512u * 1024u;
    return vm_session_create(&config, &session) != TYPE_STATUS_INVALID_ARGUMENT;
}

static C_INT vm_xt_5160_268_request_is_fixed(C_VOID)
{
    vm_product_session_request request = {
        .profile = "ibm-5160-model-268", .display = "console", .boot = "rom"
    };
    const core_product_session_open_options options = {
        0u, STD_NULL, &request, sizeof(request)
    };
    core_product_session_provider provider;
    C_VOID *session = STD_NULL;

    vm_session_provider_initialize(&provider);
    if (provider.open(provider.context, 0u, &options, &session) !=
        TYPE_STATUS_UNSUPPORTED || session != STD_NULL) return 1;
    STD_STRCPY(request.cpu, "8086");
    return provider.open(provider.context, 0u, &options, &session) !=
        TYPE_STATUS_INVALID_STATE;
}

int main(void)
{
    if (vm_xt_5160_268_declaration_is_fixed() ||
        vm_xt_5160_268_is_not_a_runnable_at_alias() ||
        vm_xt_5160_268_request_is_fixed()) return 1;
    STD_PRINTF("M5:T484:S3:XT-FIXED-PROFILE:OK\n");
    return 0;
}
