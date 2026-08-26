#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/machine_interface.h"
#include "core/machine/port.h"
#include "core/product/session/session_provider.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/provider.h"
#include "vm/product/session_catalog.h"
#include "vm/profile/xt/xt_5160_268.h"

static core_machine_media_result vm_xt_5160_268_fdd_query(C_VOID *context,
    core_machine_media_info *out_info)
{
    (C_VOID)context;
    if (out_info == STD_NULL) return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    STD_MEMSET(out_info, TYPE_ZERO_8, sizeof(*out_info));
    out_info->generation = 1u;
    out_info->capabilities = CORE_MACHINE_MEDIA_CAPABILITY_REMOVABLE |
        CORE_MACHINE_MEDIA_CAPABILITY_GEOMETRY_KNOWN;
    out_info->present = TYPE_TRUE;
    out_info->geometry = (core_machine_media_geometry) {
        720u, 512u, 40u, 2u, 9u};
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static const core_machine_media_provider vm_xt_5160_268_fdd_provider = {
    vm_xt_5160_268_fdd_query, STD_NULL, STD_NULL, STD_NULL, STD_NULL,
    STD_NULL, STD_NULL};

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
        profile.resolved.values.core.configuration.keyboard_topology !=
            CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI ||
        profile.resolved.values.core.configuration.xt_ppi_keyboard.port_a != 0x0060u ||
        profile.resolved.values.core.configuration.xt_ppi_keyboard.control_port != 0x0063u ||
        profile.resolved.values.core.configuration.xt_ppi_keyboard.irq != 1u ||
        profile.resolved.values.allowed_session_options != 0u ||
        profile.resolved.values.port_leaf_count != 0u ||
        profile.resolved.values.memory_window_count != 0u ||
        profile.resolved.values.irq_route_count != 0u ||
        profile.resolved.values.drq_route_count != 0u ||
        !profile.topology.dma_present ||
        !profile.topology.display_present ||
        !profile.topology.display.cga_vram_present ||
        profile.topology.display.ega_present ||
        profile.topology.display.ports.crtc_first != 0x03d4u ||
        profile.topology.display.ports.crtc_last != 0x03dau ||
        profile.topology.dma.controller_count != 1u ||
        profile.topology.dma.cascade_channel != 0u ||
        !profile.topology.fdc_present ||
        profile.topology.dma.fdc_channel != 2u ||
        profile.topology.rtc_cmos_present) return 1;
    if (profile.topology.fdc_drives.media_id[0] !=
            VM_PROFILE_XT_5160_268_FDD_MEDIA_ID ||
        profile.topology.fdc_drives.media_id[1] != CORE_MACHINE_MEDIA_ID_INVALID ||
        profile.topology.fdc.dor_port != 0x03f2u ||
        profile.topology.fdc.status_port != 0x03f4u ||
        profile.topology.fdc.data_port != 0x03f5u ||
        profile.topology.fdc.direction_port != 0u ||
        profile.topology.fdc.control_port != 0u || profile.topology.fdc.irq != 6u ||
        profile.topology.fdc.dma_channel != 2u) return 1;
    declaration.values.core.configuration.memory_bytes = 512u * 1024u;
    return profile.resolved.values.core.configuration.memory_bytes != 256u * 1024u;
}

static C_INT vm_xt_5160_268_topology_constructs_one_xt_fdc_route(C_VOID)
{
    vm_profile_xt_5160_268_resolved_profile profile;
    core_machine_plan *plan = STD_NULL;
    core_machine *machine = STD_NULL;
    core_machine_dma_request_binding binding = {0};
    core_machine_media_registry *media = STD_NULL;
    core_machine_display_snapshot snapshot = {0};
    const type_unsigned_8 cells[] = { 'X', 0x1fu };
    C_INT failed = 0;

    failed |= vm_profile_xt_5160_268_resolve(&profile) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_plan_create(
        &profile.resolved.values.core.configuration, &plan) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_media_registry_create(&media) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_media_registry_bind(media,
        VM_PROFILE_XT_5160_268_FDD_MEDIA_ID, STD_NULL,
        &vm_xt_5160_268_fdd_provider) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_media_registry_freeze(media) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_plan_bind_media_registry(plan, media) !=
        TYPE_STATUS_OK;
    failed |= !failed && core_machine_plan_set_topology(plan, &profile.topology) !=
        TYPE_STATUS_OK;
    failed |= !failed && core_machine_create_from_plan(plan, &machine) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_get_fdc_dma_request_binding(machine, &binding) !=
        TYPE_STATUS_OK;
    failed |= !failed && (binding.core_token == 0u || binding.channel != 2u ||
        machine->fdc.connect.config.irq != 6u ||
        machine->fdc.connect.config.dma_channel != 2u ||
        !core_machine_port_has_read(&machine->executor_port, 0x03f4u) ||
        !core_machine_port_has_read(&machine->executor_port, 0x03f5u) ||
        !core_machine_port_has_write(&machine->executor_port, 0x03f2u) ||
        !core_machine_port_has_write(&machine->executor_port, 0x03f5u) ||
        core_machine_port_has_read(&machine->executor_port, 0x03f7u) ||
        core_machine_port_has_write(&machine->executor_port, 0x03f7u) ||
        !core_machine_port_has_write(&machine->executor_port, 0x03d4u) ||
        !core_machine_port_has_read(&machine->executor_port, 0x03d5u) ||
        !core_machine_port_has_write(&machine->executor_port, 0x03d8u) ||
        !core_machine_port_has_write(&machine->executor_port, 0x03d9u) ||
        !core_machine_port_has_read(&machine->executor_port, 0x03dau) ||
        core_machine_port_has_read(&machine->executor_port, 0x03c0u));
    failed |= !failed && core_machine_freeze_execution_providers(machine) !=
        TYPE_STATUS_OK;
    failed |= !failed && core_machine_reset(machine) != TYPE_STATUS_OK;
    core_machine_port_write(&machine->executor_port, 0x03d8u, 0x0du);
    failed |= !failed && core_machine_memory_write(machine, 0x000b8000u,
        cells, sizeof(cells)) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_capture_display_snapshot(machine, &snapshot) !=
        TYPE_STATUS_OK;
    failed |= !failed && (snapshot.kind != CORE_MACHINE_DISPLAY_KIND_TEXT ||
        snapshot.characters[0] != 'X' || snapshot.attributes[0] != 0x1fu);
    core_machine_destroy(machine);
    core_machine_media_registry_destroy(media);
    core_machine_plan_destroy(plan);
    return failed;
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
        vm_xt_5160_268_topology_constructs_one_xt_fdc_route() ||
        vm_xt_5160_268_is_not_a_runnable_at_alias() ||
        vm_xt_5160_268_request_is_fixed()) return 1;
    STD_PRINTF("M5:T484:S3:XT-FIXED-PROFILE:OK\n");
    STD_PRINTF("M5:T484:S5:XT-B2-SHARED-TOPOLOGY:OK\n");
    STD_PRINTF("M5:T484:S10:XT-FDC-PLAN:OK\n");
    STD_PRINTF("M5:T484:S10:XT-NO-AT-FDC-ALIAS:OK\n");
    STD_PRINTF("M5:T484:S11:XT-CGA-PLAN:OK\n");
    STD_PRINTF("M5:T484:S11:XT-NO-VIDEO-ALIAS:OK\n");
    return 0;
}
