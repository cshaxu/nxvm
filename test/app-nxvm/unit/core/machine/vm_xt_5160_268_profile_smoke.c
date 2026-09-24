#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/devices/port.h"
#include "app-nxvm/machine/machine_interface.h"
#include "app-nxvm/machine/machine_private.h"
#include "app-nxvm/profiles/xt/xt_5160_268.h"

static core_machine_media_result vm_xt_5160_268_fdd_query(void *context,
    core_machine_media_info *out_info)
{
    (void)context;
    if (out_info == LIB_NULL) return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    lib_memory_set(out_info, 0u, sizeof(*out_info));
    out_info->generation = 1u;
    out_info->capabilities = CORE_MACHINE_MEDIA_CAPABILITY_REMOVABLE |
        CORE_MACHINE_MEDIA_CAPABILITY_GEOMETRY_KNOWN;
    out_info->present = LIB_TRUE;
    out_info->geometry = (core_machine_media_geometry) {
        720u, 512u, 40u, 2u, 9u};
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static const core_machine_media_provider vm_xt_5160_268_fdd_provider = {
    vm_xt_5160_268_fdd_query, LIB_NULL, LIB_NULL, LIB_NULL, LIB_NULL,
    LIB_NULL, LIB_NULL};

static lib_i32 vm_xt_5160_268_contract_is_fixed(void)
{
    vm_profile_xt_5160_268_plan_snapshot profile;

    if (vm_profile_xt_5160_268_values_create(&profile.values) != LIB_STATUS_OK ||
        vm_profile_xt_5160_268_plan_create(&profile, LIB_FALSE) != LIB_STATUS_OK ||
        profile.values.core.configuration.memory_bytes != 256u * 1024u ||
        profile.values.core.configuration.cpu_profile !=
            CORE_MACHINE_CPU_PROFILE_8088 ||
        profile.values.core.configuration.l1_compatibility_policy !=
            CORE_MACHINE_L1_COMPATIBILITY_BOUNDED_PROGRESS ||
        profile.topology.absent_memory_count != 3u ||
        profile.topology.absent_memory[0].physical_start != 0x00040000u ||
        profile.topology.absent_memory[0].bytes != 0x00060000u ||
        profile.topology.absent_memory[0].read_value != 0xffu ||
        profile.topology.absent_memory[1].physical_start != 0x000a0000u ||
        profile.topology.absent_memory[1].bytes != 0x00018000u ||
        profile.topology.absent_memory[1].read_value != 0xffu ||
        profile.topology.absent_memory[2].physical_start != 0x000c0000u ||
        profile.topology.absent_memory[2].bytes != 0x00030000u ||
        profile.topology.absent_memory[2].read_value != 0xffu ||
        profile.values.core.configuration.fpu_profile !=
            CORE_MACHINE_FPU_PROFILE_NONE ||
        profile.values.core.configuration.shared_pit_personality !=
            CORE_MACHINE_PIT_PERSONALITY_8253 ||
        profile.values.core.configuration.pic_topology !=
            CORE_MACHINE_PIC_TOPOLOGY_SINGLE ||
        profile.values.core.configuration.dma_controller_count != 1u ||
        profile.values.core.configuration.keyboard_topology !=
            CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI ||
        profile.values.core.configuration.xt_ppi_keyboard.port_a != 0x0060u ||
        profile.values.core.configuration.xt_ppi_keyboard.control_port != 0x0063u ||
        profile.values.core.configuration.xt_ppi_keyboard.irq != 1u ||
        profile.values.core.configuration.clock_plan.pit.numerator != 1u ||
        profile.values.core.configuration.clock_plan.pit.denominator != 4u ||
        profile.values.core.configuration.clock_plan.pit.reset_phase != 0u ||
        profile.values.allowed_session_options != 0u ||
        profile.values.port_leaf_count != 0u ||
        profile.values.memory_window_count != 0u ||
        profile.values.irq_route_count != 0u ||
        profile.values.drq_route_count != 0u ||
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
        profile.topology.rtc_cmos_present || !profile.topology.hdc_present ||
        profile.topology.hdc_media_id != VM_PROFILE_XT_5160_268_HDD_MEDIA_ID ||
        profile.topology.hdc_slave_media_id != CORE_MACHINE_MEDIA_ID_INVALID ||
        profile.topology.hdc.protocol != CORE_MACHINE_HDC_PROTOCOL_XEBEC_XT ||
        profile.topology.hdc.irq != 5u ||
        profile.topology.hdc.service.command_ticks != 250u ||
        profile.topology.hdc.service.next_sector_ticks != 0u ||
        profile.topology.hdc.bus.xebec.data_port != 0x0320u ||
        profile.topology.hdc.bus.xebec.hardware_status_reset_port != 0x0321u ||
        profile.topology.hdc.bus.xebec.jumpers_select_port != 0x0322u ||
        profile.topology.hdc.bus.xebec.dma_irq_mask_port != 0x0323u ||
        profile.topology.hdc.bus.xebec.dma_channel != 3u ||
        profile.topology.hdc.bus.xebec.drive_type !=
            CORE_MACHINE_XEBEC_DRIVE_TYPE_2 ||
        profile.topology.hdc.bus.xebec.expected_media_geometry.logical_sector_count !=
            CORE_MACHINE_XEBEC_TYPE_2_LOGICAL_SECTOR_COUNT ||
        profile.topology.hdc.bus.xebec.expected_media_geometry.bytes_per_sector !=
            CORE_MACHINE_XEBEC_TYPE_2_BYTES_PER_SECTOR ||
        profile.topology.hdc.bus.xebec.expected_media_geometry.cylinders !=
            CORE_MACHINE_XEBEC_TYPE_2_CYLINDERS ||
        profile.topology.hdc.bus.xebec.expected_media_geometry.heads !=
            CORE_MACHINE_XEBEC_TYPE_2_HEADS ||
        profile.topology.hdc.bus.xebec.expected_media_geometry.sectors_per_track !=
            CORE_MACHINE_XEBEC_TYPE_2_SECTORS_PER_TRACK) return 1;
    if (profile.topology.fdc_drives.media_id[0] !=
            VM_PROFILE_XT_5160_268_FDD_MEDIA_ID ||
        profile.topology.fdc_drives.media_id[1] != CORE_MACHINE_MEDIA_ID_INVALID ||
        profile.topology.fdc.dor_port != 0x03f2u ||
        profile.topology.fdc.status_port != 0x03f4u ||
        profile.topology.fdc.data_port != 0x03f5u ||
        profile.topology.fdc.direction_port != 0u ||
        profile.topology.fdc.control_port != 0u || profile.topology.fdc.irq != 6u ||
        profile.topology.fdc.dma_channel != 2u ||
        profile.topology.fdc.clock_ticks_per_second != 4772727u) return 1;
    return 0;
}

static lib_i32 vm_xt_5160_268_topology_constructs_one_xt_route(void)
{
    vm_profile_xt_5160_268_plan_snapshot profile;
    core_machine_plan *plan = LIB_NULL;
    core_machine *machine = LIB_NULL;
    core_machine_dma_request_binding binding = {0};
    core_machine_media_registry *media = LIB_NULL;
    core_machine_display_snapshot snapshot = {0};
    const lib_u8 cells[] = { 'X', 0x1fu };
    lib_u8 open_bus_byte = 0u;
    lib_i32 failed = 0;

    failed |= vm_profile_xt_5160_268_plan_create(&profile, LIB_FALSE) != LIB_STATUS_OK;
    failed |= !failed && core_machine_plan_create(
        &profile.values.core.configuration, &plan) != LIB_STATUS_OK;
    failed |= !failed && core_machine_media_registry_create(&media) != LIB_STATUS_OK;
    failed |= !failed && core_machine_media_registry_bind(media,
        VM_PROFILE_XT_5160_268_FDD_MEDIA_ID, LIB_NULL,
        &vm_xt_5160_268_fdd_provider) != LIB_STATUS_OK;
    failed |= !failed && core_machine_media_registry_freeze(media) != LIB_STATUS_OK;
    failed |= !failed && core_machine_plan_bind_media_registry(plan, media) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_plan_set_topology(plan, &profile.topology) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_create_from_plan(plan, &machine) != LIB_STATUS_OK;
    failed |= !failed && core_machine_get_fdc_dma_request_binding(machine, &binding) !=
        LIB_STATUS_OK;
    failed |= !failed && (binding.core_token == 0u || binding.channel != 2u ||
        machine->fdc.connect.config.irq != 6u ||
        machine->fdc.connect.config.dma_channel != 2u ||
        machine->fdc.connect.config.clock_ticks_per_second != 4772727u ||
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
        core_machine_port_has_read(&machine->executor_port, 0x03c0u) ||
        !machine->hdc_configured ||
        machine->hdc.connect.config.protocol != CORE_MACHINE_HDC_PROTOCOL_XEBEC_XT ||
        machine->hdc.connect.config.bus.xebec.drive_type !=
            CORE_MACHINE_XEBEC_DRIVE_TYPE_2 ||
        !core_machine_port_has_read(&machine->executor_port, 0x0320u) ||
        !core_machine_port_has_write(&machine->executor_port, 0x0320u) ||
        !core_machine_port_has_read(&machine->executor_port, 0x0321u) ||
        !core_machine_port_has_write(&machine->executor_port, 0x0321u) ||
        !core_machine_port_has_read(&machine->executor_port, 0x0322u) ||
        !core_machine_port_has_write(&machine->executor_port, 0x0322u) ||
        core_machine_port_has_read(&machine->executor_port, 0x0323u) ||
        !core_machine_port_has_write(&machine->executor_port, 0x0323u));
    failed |= !failed && core_machine_freeze_execution_providers(machine) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= !failed && (core_machine_memory_read(machine, 0x000c8000u,
        &open_bus_byte, sizeof(open_bus_byte)) != LIB_STATUS_OK ||
        open_bus_byte != 0xffu);
    core_machine_port_write(&machine->executor_port, 0x03d8u, 0x0du);
    failed |= !failed && core_machine_memory_write(machine, 0x000b8000u,
        cells, sizeof(cells)) != LIB_STATUS_OK;
    failed |= !failed && core_machine_capture_display_snapshot(machine, &snapshot) !=
        LIB_STATUS_OK;
    failed |= !failed && (snapshot.kind != CORE_MACHINE_DISPLAY_KIND_TEXT ||
        snapshot.characters[0] != 'X' || snapshot.attributes[0] != 0x1fu);
    core_machine_destroy(machine);
    core_machine_media_registry_destroy(media);
    core_machine_plan_destroy(plan);
    return failed;
}

static lib_i32 vm_xt_5160_268_byob_session_uses_one_xt_route(void)
{
    static lib_u8 system[VM_PROFILE_XT_5160_268_SYSTEM_ROM_BYTES];
    static lib_u8 xebec[VM_PROFILE_XT_5160_268_XEBEC_ROM_BYTES];
    static lib_u8 video[512] = {0x55u, 0xaau, 1u};
    vm_machine_config config = {
        .profile_kind = VM_MACHINE_PROFILE_IBM_5160_MODEL_268,
        .bios_count = 1u
    };
    vm_machine_assets assets = { .bios = {
        { system, sizeof(system) }, { xebec, sizeof(xebec) }
    }, .video = { video, sizeof(video) } };
    vm_machine *session = LIB_NULL;
    vm_machine_reset_vector vector;
    lib_u8 observed[2] = {0};
    lib_i32 failed = 0;

    failed |= lib_text_compare(vm_profile_name(config.profile_kind),
        "ibm-5160-model-268") != 0;
    failed |= vm_machine_create_from_assets(&config, &assets, &session) != LIB_STATUS_OK ||
        session == LIB_NULL;
    failed |= !failed && vm_machine_get_reset_vector(session, &vector) != LIB_STATUS_OK;
    failed |= !failed && (core_machine_memory_read(session->core_machine, 0x000c0000u,
        observed, sizeof(observed)) != LIB_STATUS_OK || observed[0u] != 0x55u ||
        observed[1u] != 0xaau);
    vm_machine_destroy(session);
    session = LIB_NULL;
    config.bios_count = 2u;
    failed |= vm_machine_create_from_assets(&config, &assets, &session) != LIB_STATUS_OK ||
        session == LIB_NULL;
    vm_machine_destroy(session);
    session = LIB_NULL;
    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_8086;
    failed |= vm_machine_create_from_assets(&config, &assets, &session) !=
        LIB_STATUS_INVALID_ARGUMENT;
    return failed;
}

int main(void)
{
    if (vm_xt_5160_268_contract_is_fixed() ||
        vm_xt_5160_268_topology_constructs_one_xt_route() ||
        vm_xt_5160_268_byob_session_uses_one_xt_route()) return 1;
    printf("M5:T484:S3:XT-FIXED-PROFILE:OK\n");
    printf("M5:T484:S5:XT-B2-SHARED-TOPOLOGY:OK\n");
    printf("M5:T484:S10:XT-FDC-PLAN:OK\n");
    printf("M5:T484:S10:XT-NO-AT-FDC-ALIAS:OK\n");
    printf("M5:T484:S11:XT-CGA-PLAN:OK\n");
    printf("M5:T484:S11:XT-NO-VIDEO-ALIAS:OK\n");
    printf("M5:T484:S16:XT-TYPE2:OK\n");
    printf("M5:T484:S21:XT-B6-BYOB-SESSION:OK\n");
    return 0;
}
