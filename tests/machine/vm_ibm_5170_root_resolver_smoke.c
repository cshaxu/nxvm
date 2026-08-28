#include "type.h"

#include "core/machine/machine_interface.h"
#include "vm/profile/default_profile/pc_at_profile_private.h"
#include "vm/profile/model40/model40_private.h"

static const type_unsigned_32 vm_ibm_5170_root_contract_ids[] = {1u};

static C_INT vm_ibm_5170_root_port_leaves_match(
    const vm_resolved_profile *resolved)
{
    const vm_profile_default_pc_at_descriptor *descriptor =
        vm_profile_ibm_5170_model_339_descriptor_get();
    STD_SIZE_T role;
    STD_SIZE_T result_index = 0u;

    if (resolved == STD_NULL) return 0;
    for (role = 0u; role <= VM_PROFILE_DEFAULT_PC_AT_DEVICE_BOARD; ++role) {
        STD_SIZE_T ordinal;

        for (ordinal = 0u;; ++ordinal) {
            const vm_profile_default_pc_at_port_leaf *leaf =
                vm_profile_default_pc_at_port_leaf_at(descriptor,
                    (vm_profile_default_pc_at_device_role)role, ordinal);
            const vm_profile_resolver_port_leaf *resolved_leaf;

            if (leaf == STD_NULL) break;
            if (result_index == resolved->values.port_leaf_count) return 0;
            resolved_leaf = &resolved->values.port_leaves[result_index++];
            if (resolved_leaf->device != (1u << (type_unsigned_32)role) ||
                resolved_leaf->port != leaf->port || resolved_leaf->read != leaf->read ||
                resolved_leaf->write != leaf->write) return 0;
        }
    }
    return result_index == resolved->values.port_leaf_count;
}

static C_INT vm_ibm_5170_root_snapshot_matches(
    const vm_profile_default_pc_at_resolved_profile *root)
{
    const vm_profile_default_pc_at_descriptor *source =
        vm_profile_ibm_5170_model_339_descriptor_get();
    const vm_profile_default_pc_at_descriptor *snapshot;

    if (root == STD_NULL || source == STD_NULL) return 0;
    snapshot = &root->descriptor;
    return STD_STRCMP(snapshot->identity, "pc-at-5170") == 0 &&
        snapshot->compatibility_revision == source->compatibility_revision &&
        snapshot->cpu_profile == source->cpu_profile &&
        snapshot->fpu_profile == source->fpu_profile &&
        snapshot->default_memory_bytes == source->default_memory_bytes &&
        snapshot->unpopulated_extended_memory == source->unpopulated_extended_memory &&
        snapshot->fdc_bounce_segment == source->fdc_bounce_segment &&
        snapshot->planar_parity_present == source->planar_parity_present &&
        snapshot->ega_present == source->ega_present &&
        snapshot->cga_vram_present == source->cga_vram_present &&
        snapshot->firmware_slot == source->firmware_slot &&
        snapshot->diskette_drive_a_field_upgrade == source->diskette_drive_a_field_upgrade &&
        STD_MEMCMP(&snapshot->rom, &source->rom, sizeof(snapshot->rom)) == 0 &&
        STD_MEMCMP(&snapshot->cmos, &source->cmos, sizeof(snapshot->cmos)) == 0 &&
        snapshot->port_leaf_count == source->port_leaf_count &&
        STD_MEMCMP(snapshot->port_leaves, source->port_leaves,
            source->port_leaf_count * sizeof(*source->port_leaves)) == 0 &&
        snapshot->route_count == source->route_count &&
        STD_MEMCMP(snapshot->routes, source->routes,
            source->route_count * sizeof(*source->routes)) == 0 &&
        STD_MEMCMP(&snapshot->hdc, &source->hdc,
            sizeof(snapshot->hdc)) == 0 &&
        snapshot->firmware_service_count == source->firmware_service_count &&
        STD_MEMCMP(snapshot->firmware_services, source->firmware_services,
            source->firmware_service_count * sizeof(*source->firmware_services)) == 0;
}

static C_INT vm_ibm_5170_root_is_copied_and_complete(C_VOID)
{
    vm_profile_resolver_declaration declaration;
    vm_profile_default_pc_at_resolved_profile root;
    vm_resolved_profile *resolved = &root.resolved;
    core_machine_plan *plan = STD_NULL;

    const type_status declaration_status =
        vm_profile_ibm_5170_root_declaration_create(&declaration);
    const type_status resolve_status = vm_profile_ibm_5170_root_resolve(&root);

    if (declaration_status != TYPE_STATUS_OK || resolve_status != TYPE_STATUS_OK ||
        STD_STRCMP(resolved->identity, "pc-at-5170") != 0 ||
        resolved->owned_fields != VM_PROFILE_RESOLVER_FIELD_ALL ||
        !vm_ibm_5170_root_snapshot_matches(&root) ||
        !vm_profile_default_pc_at_descriptor_is_valid(&root.descriptor) ||
        resolved->values.core.configuration.cpu_profile !=
            CORE_MACHINE_CPU_PROFILE_80286 ||
        resolved->values.core.configuration.memory_bytes != 512u * 1024u ||
        resolved->values.core.configuration.pic_topology !=
            CORE_MACHINE_PIC_TOPOLOGY_CASCADED ||
        resolved->values.core.configuration.dma_controller_count !=
            CORE_MACHINE_DMA_CONTROLLER_COUNT ||
        resolved->values.core.configuration.time_axis.kind !=
            CORE_MACHINE_TIME_AXIS_MACRO_PROPORTIONAL ||
        resolved->values.core.configuration.time_axis.ticks_per_second != 8000000u ||
        resolved->values.core.controller_timing_rules.pit_clock !=
            CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK ||
        resolved->values.core.controller_timing_rules.dma_service !=
            CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_DMA_SERVICE_PHASES ||
        root.topology.absent_memory_count != 1u ||
        !root.topology.planar_parity_present ||
        !root.topology.display_present || root.topology.display.ega_present ||
        !root.topology.display.cga_vram_present ||
        root.topology.display.ports.crtc_first != 0x03d4u ||
        root.topology.display.ports.crtc_last != 0x03dau ||
        !root.topology.dma_present ||
        root.topology.dma.fdc_channel != 2u ||
        root.topology.dma.controller_count != CORE_MACHINE_DMA_CONTROLLER_COUNT ||
        root.topology.dma.cascade_channel != CORE_MACHINE_DMA_CASCADE_CHANNEL ||
        !root.topology.rtc_cmos_present ||
        root.topology.rtc_cmos.index_port != 0x0070u ||
        root.topology.rtc_cmos.data_port != 0x0071u ||
        root.topology.rtc_cmos.irq != 8u ||
        root.topology.rtc_cmos.timing.provenance != CORE_MACHINE_RTC_TIMING_L3_SOURCE ||
        !vm_ibm_5170_root_port_leaves_match(resolved) ||
        resolved->values.memory_window_count != 1u ||
        resolved->values.memory_windows[0].first != 0x000b8000u ||
        resolved->values.memory_windows[0].last != 0x000bffffu ||
        resolved->values.irq_route_count != 5u ||
        resolved->values.drq_route_count != 1u ||
        resolved->values.firmware_policy != VM_PROFILE_RESOLVER_FIRMWARE_POLICY_BUILTIN ||
        resolved->values.media_policy != VM_PROFILE_RESOLVER_MEDIA_POLICY_SESSION ||
        resolved->values.allowed_session_options != 0u) return 1;
    declaration.values.core.configuration.memory_bytes = 1024u * 1024u;
    const type_status create_status = core_machine_plan_create(
        &resolved->values.core.configuration, &plan);
    const type_status timing_status = create_status == TYPE_STATUS_OK ?
        core_machine_plan_set_controller_timing_rules(plan,
            &resolved->values.core.controller_timing_rules) : TYPE_STATUS_INVALID_STATE;
    const type_status topology_status = timing_status == TYPE_STATUS_OK ?
        core_machine_plan_set_topology(plan, &root.topology) : TYPE_STATUS_INVALID_STATE;
    if (resolved->values.core.configuration.memory_bytes != 512u * 1024u ||
        create_status != TYPE_STATUS_OK || timing_status != TYPE_STATUS_OK ||
        topology_status != TYPE_STATUS_OK) {
        core_machine_plan_destroy(plan);
        return 1;
    }
    core_machine_plan_destroy(plan);
    return 0;
}

static C_INT vm_ibm_5170_root_rejects_invalid_declaration(C_VOID)
{
    vm_profile_resolver_declaration declaration;
    vm_resolved_profile resolved;
    const vm_profile_resolver_contract_catalog catalog = {
        vm_ibm_5170_root_contract_ids,
        sizeof(vm_ibm_5170_root_contract_ids) / sizeof(vm_ibm_5170_root_contract_ids[0])};
    const vm_profile_resolver_session_request request = {0u};

    if (vm_profile_ibm_5170_root_declaration_create(&declaration) != TYPE_STATUS_OK) {
        return 1;
    }
    declaration.values.port_leaves[1].port = declaration.values.port_leaves[0].port;
    if (vm_profile_resolver_resolve(&declaration, &catalog, &request, &resolved) ==
        TYPE_STATUS_OK) return 1;
    if (vm_profile_ibm_5170_root_declaration_create(&declaration) != TYPE_STATUS_OK) {
        return 1;
    }
    declaration.values.irq_routes[1].line = declaration.values.irq_routes[0].line;
    if (vm_profile_resolver_resolve(&declaration, &catalog, &request, &resolved) ==
        TYPE_STATUS_OK) return 1;
    if (vm_profile_ibm_5170_root_declaration_create(&declaration) != TYPE_STATUS_OK) {
        return 1;
    }
    declaration.values.core.contract_id = 2u;
    return vm_profile_resolver_resolve(&declaration, &catalog, &request, &resolved) ==
        TYPE_STATUS_OK;
}

static C_INT vm_model40_child_resolves_copy(C_VOID)
{
    vm_profile_resolver_declaration root;
    vm_profile_resolver_declaration child;
    vm_resolved_profile resolved;

    if (vm_profile_ibm_5170_root_declaration_create(&root) != TYPE_STATUS_OK ||
        vm_profile_model40_child_declaration_create(&root, &child) != TYPE_STATUS_OK ||
        vm_profile_model40_child_resolve(&resolved) != TYPE_STATUS_OK ||
        STD_STRCMP(resolved.identity, "compaq-deskpro-386-model-40") != 0 ||
        STD_STRCMP(resolved.parent_identity, "pc-at-5170") != 0 ||
        STD_STRCMP(resolved.field_owner[0], "compaq-deskpro-386-model-40") != 0 ||
        STD_STRCMP(resolved.field_owner[1], "pc-at-5170") != 0 ||
        STD_STRCMP(resolved.field_owner[6], "compaq-deskpro-386-model-40") != 0 ||
        resolved.values.core.configuration.cpu_profile !=
            CORE_MACHINE_CPU_PROFILE_80386 ||
        resolved.values.core.configuration.memory_bytes != 1024u * 1024u ||
        resolved.values.core.configuration.pic_topology !=
            CORE_MACHINE_PIC_TOPOLOGY_CASCADED ||
        resolved.values.core.configuration.dma_controller_count !=
            CORE_MACHINE_DMA_CONTROLLER_COUNT ||
        resolved.values.firmware_policy != VM_PROFILE_RESOLVER_FIRMWARE_POLICY_BYOB ||
        resolved.values.media_policy != VM_PROFILE_RESOLVER_MEDIA_POLICY_SESSION) {
        return 1;
    }
    child.values.core.configuration.memory_bytes = 512u * 1024u;
    return resolved.values.core.configuration.memory_bytes != 1024u * 1024u;
}

static C_INT vm_model40_child_rejects_invalid_parent(C_VOID)
{
    vm_profile_resolver_declaration invalid = {0};
    vm_profile_resolver_declaration child;

    invalid.identity = "not-5170";
    return vm_profile_model40_child_declaration_create(&invalid, &child) ==
        TYPE_STATUS_OK;
}

static C_INT vm_default_at_child_resolves_copy(C_VOID)
{
    const vm_profile_default_at_request request = {
        .requested_options = VM_PROFILE_DEFAULT_AT_SESSION_OPTION_CPU_FPU |
            VM_PROFILE_DEFAULT_AT_SESSION_OPTION_MEMORY,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_80387,
        .memory_bytes = 32u * 1024u * 1024u};
    vm_profile_resolver_declaration root;
    vm_profile_resolver_declaration child;
    vm_profile_default_pc_at_resolved_profile resolved;

    if (vm_profile_ibm_5170_root_declaration_create(&root) != TYPE_STATUS_OK ||
        vm_profile_default_at_child_declaration_create(&root, &request, &child) !=
            TYPE_STATUS_OK || vm_profile_default_at_child_resolve(&request, &resolved) !=
            TYPE_STATUS_OK || STD_STRCMP(resolved.resolved.identity, "default-at") != 0 ||
        STD_STRCMP(resolved.resolved.parent_identity, "pc-at-5170") != 0 ||
        STD_STRCMP(resolved.resolved.field_owner[0], "default-at") != 0 ||
        STD_STRCMP(resolved.resolved.field_owner[1], "default-at") != 0 ||
        STD_STRCMP(resolved.resolved.field_owner[2], "default-at") != 0 ||
        STD_STRCMP(resolved.resolved.field_owner[3], "default-at") != 0 ||
        STD_STRCMP(resolved.resolved.field_owner[4], "pc-at-5170") != 0 ||
        STD_STRCMP(resolved.resolved.field_owner[5], "pc-at-5170") != 0 ||
        STD_STRCMP(resolved.resolved.field_owner[6], "default-at") != 0 ||
        resolved.resolved.values.core.configuration.cpu_profile !=
            CORE_MACHINE_CPU_PROFILE_80386 ||
        resolved.resolved.values.core.configuration.fpu_profile !=
            CORE_MACHINE_FPU_PROFILE_80387 ||
        resolved.resolved.values.core.configuration.memory_bytes != 32u * 1024u * 1024u ||
        resolved.resolved.values.core.configuration.pic_topology !=
            CORE_MACHINE_PIC_TOPOLOGY_CASCADED ||
        resolved.resolved.values.core.configuration.dma_controller_count !=
            CORE_MACHINE_DMA_CONTROLLER_COUNT ||
        resolved.resolved.values.allowed_session_options !=
            (VM_PROFILE_DEFAULT_AT_SESSION_OPTION_CPU_FPU |
                VM_PROFILE_DEFAULT_AT_SESSION_OPTION_MEMORY |
                VM_PROFILE_DEFAULT_AT_SESSION_OPTION_FLOPPY) ||
        STD_STRCMP(resolved.descriptor.identity, "default-at") != 0 ||
        resolved.descriptor.cpu_profile != CORE_MACHINE_CPU_PROFILE_80386 ||
        resolved.descriptor.fpu_profile != CORE_MACHINE_FPU_PROFILE_80387 ||
        resolved.descriptor.default_memory_bytes != 32u * 1024u * 1024u ||
        !resolved.descriptor.hdc_present || !resolved.descriptor.ega_present ||
        resolved.topology.absent_memory_count != 1u ||
        !resolved.topology.planar_parity_present ||
        resolved.topology.planar_parity.memory_bytes != 0u ||
        !resolved.topology.display_present || !resolved.topology.display.ega_present ||
        resolved.topology.display.cga_vram_present ||
        !resolved.topology.dma_present ||
        resolved.topology.dma.fdc_channel != 2u ||
        !resolved.topology.rtc_cmos_present ||
        resolved.topology.rtc_cmos.timing.provenance != CORE_MACHINE_RTC_TIMING_L2_RATIO) {
        return 1;
    }
    child.values.core.configuration.memory_bytes = 512u * 1024u;
    return resolved.resolved.values.core.configuration.memory_bytes != 32u * 1024u * 1024u;
}

static C_INT vm_default_at_child_rejects_invalid_request(C_VOID)
{
    vm_profile_resolver_declaration invalid_parent = {0};
    vm_profile_resolver_declaration child;
    vm_profile_default_pc_at_resolved_profile resolved;
    const vm_profile_default_at_request bad_option = { .requested_options = 0x80u };
    const vm_profile_default_at_request bad_memory = {
        .requested_options = VM_PROFILE_DEFAULT_AT_SESSION_OPTION_MEMORY };

    invalid_parent.identity = "not-5170";
    return vm_profile_default_at_child_declaration_create(&invalid_parent,
        &bad_option, &child) == TYPE_STATUS_OK ||
        vm_profile_default_at_child_resolve(&bad_option, &resolved) == TYPE_STATUS_OK ||
        vm_profile_default_at_child_resolve(&bad_memory, &resolved) == TYPE_STATUS_OK;
}

static C_INT vm_default_at_80286_selection_keeps_generic_topology(C_VOID)
{
    const vm_profile_default_at_request request = {
        .requested_options = VM_PROFILE_DEFAULT_AT_SESSION_OPTION_CPU_FPU,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80286,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_80287 };
    vm_profile_default_pc_at_resolved_profile resolved;

    if (vm_profile_default_at_child_resolve(&request, &resolved) != TYPE_STATUS_OK ||
        resolved.descriptor.cpu_profile != CORE_MACHINE_CPU_PROFILE_80286 ||
        resolved.descriptor.fpu_profile != CORE_MACHINE_FPU_PROFILE_80287 ||
        resolved.descriptor.firmware_slot != VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_SLOT_GENERIC ||
        !resolved.topology.display_present || !resolved.topology.display.ega_present ||
        !resolved.topology.rtc_cmos_present) return 1;
    return !vm_profile_default_pc_at_descriptor_is_valid(&resolved.descriptor);
}

int main(void)
{
    if (vm_ibm_5170_root_is_copied_and_complete() ||
        vm_ibm_5170_root_rejects_invalid_declaration() ||
        vm_model40_child_resolves_copy() || vm_model40_child_rejects_invalid_parent() ||
        vm_default_at_child_resolves_copy() ||
        vm_default_at_child_rejects_invalid_request() ||
        vm_default_at_80286_selection_keeps_generic_topology()) {
        return 1;
    }
    STD_PRINTF("M5:T476:S2:IBM5170-ROOT-RESOLVER:OK\n");
    STD_PRINTF("M5:T476:S2:IBM5170-ROOT-NEGATIVE:OK\n");
    STD_PRINTF("M5:T477:S2:DESKPRO-CHILD-RESOLVER:OK\n");
    STD_PRINTF("M5:T477:S2:DESKPRO-CHILD-NEGATIVE:OK\n");
    STD_PRINTF("M5:T478:S2:DEFAULT-AT-CHILD-RESOLVER:OK\n");
    STD_PRINTF("M5:T478:S2:DEFAULT-AT-CHILD-NEGATIVE:OK\n");
    return 0;
}
