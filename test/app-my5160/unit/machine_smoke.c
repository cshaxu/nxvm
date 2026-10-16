#include "app-my5160/machine_construction_interface.h"
#include "x86-nxvm/machine/machine_interface.h"
#include "lib/types/file.h"
#include "lib/types/types_interface.h"

#include "../../x86-nxvm/board-base/composition/bus_fixture.h"
#include "../../x86-nxvm/board-base/support/composition_fixture.h"
#include "../../x86-nxvm/board-base/support/controller_fixture.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "x86-nxvm/machine/machine_private.h"
#include "app-my5160/machine_config.h"

static lib_status vm_xt_test_create(const x86_nxvm_machine_config *config,
    const x86_nxvm_machine_assets *assets, x86_nxvm_machine **out_machine)
{
    x86_nxvm_machine_construction construction;
    lib_status status = vm_profile_machine_plan_create_xt(config, assets, &construction);

    *out_machine = LIB_NULL;
    return status == LIB_STATUS_OK ?
        x86_nxvm_machine_create(config, &construction, out_machine) : status;
}

static x86_nxvm_board_media_result vm_xt_5160_268_fdd_query(void *context,
    x86_nxvm_board_media_info *out_info)
{
    (void)context;
    if (out_info == LIB_NULL) return X86_NXVM_BOARD_MEDIA_RESULT_INVALID_RANGE;
    lib_memory_set(out_info, 0u, sizeof(*out_info));
    out_info->generation = 1u;
    out_info->capabilities = X86_NXVM_BOARD_MEDIA_CAPABILITY_REMOVABLE |
        X86_NXVM_BOARD_MEDIA_CAPABILITY_GEOMETRY_KNOWN;
    out_info->present = LIB_TRUE;
    out_info->geometry = (x86_nxvm_board_media_geometry) {
        720u, 512u, 40u, 2u, 9u};
    return X86_NXVM_BOARD_MEDIA_RESULT_OK;
}

static const x86_nxvm_board_media_provider vm_xt_5160_268_fdd_provider = {
    .query = vm_xt_5160_268_fdd_query,
};

static lib_i32 vm_xt_5160_268_contract_is_fixed(void)
{
    vm_profile_xt_5160_268_plan_snapshot profile;

    if (vm_profile_xt_5160_268_values_create(&profile.values) != LIB_STATUS_OK ||
        vm_profile_xt_5160_268_plan_create(&profile, LIB_FALSE) != LIB_STATUS_OK ||
        profile.values.core.configuration.memory_bytes != 256u * 1024u ||
        profile.values.core.configuration.cpu_profile !=
            X86_CORE_CPU_PROFILE_8088 ||
        profile.values.core.configuration.l1_compatibility_policy !=
            X86_CORE_MACHINE_L1_COMPATIBILITY_BOUNDED_PROGRESS ||
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
            X86_FPU_PROFILE_NONE ||
        profile.values.core.configuration.shared_pit_personality !=
            X86_PIT_PERSONALITY_8253 ||
        profile.values.core.configuration.pic_topology !=
            X86_NXVM_BOARD_PIC_TOPOLOGY_SINGLE ||
        profile.values.core.configuration.dma_controller_count != 1u ||
        profile.values.core.configuration.keyboard_topology !=
            X86_NXVM_BOARD_KEYBOARD_TOPOLOGY_XT_PPI ||
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
        profile.topology.hdc_slave_media_id != X86_NXVM_BOARD_MEDIA_ID_INVALID ||
        profile.topology.hdc.protocol != X86_NXVM_BOARD_HDC_PROTOCOL_XEBEC_XT ||
        profile.topology.hdc.irq != 5u ||
        profile.topology.hdc.service.command_ticks != 250u ||
        profile.topology.hdc.service.next_sector_ticks != 0u ||
        profile.topology.hdc.bus.xebec.data_port != 0x0320u ||
        profile.topology.hdc.bus.xebec.hardware_status_reset_port != 0x0321u ||
        profile.topology.hdc.bus.xebec.jumpers_select_port != 0x0322u ||
        profile.topology.hdc.bus.xebec.dma_irq_mask_port != 0x0323u ||
        profile.topology.hdc.bus.xebec.dma_channel != 3u ||
        profile.topology.hdc.bus.xebec.drive_type !=
            X86_NXVM_BOARD_XEBEC_DRIVE_TYPE_2 ||
        profile.topology.hdc.bus.xebec.expected_media_geometry.logical_sector_count !=
            X86_NXVM_BOARD_XEBEC_TYPE_2_LOGICAL_SECTOR_COUNT ||
        profile.topology.hdc.bus.xebec.expected_media_geometry.bytes_per_sector !=
            X86_NXVM_BOARD_XEBEC_TYPE_2_BYTES_PER_SECTOR ||
        profile.topology.hdc.bus.xebec.expected_media_geometry.cylinders !=
            X86_NXVM_BOARD_XEBEC_TYPE_2_CYLINDERS ||
        profile.topology.hdc.bus.xebec.expected_media_geometry.heads !=
            X86_NXVM_BOARD_XEBEC_TYPE_2_HEADS ||
        profile.topology.hdc.bus.xebec.expected_media_geometry.sectors_per_track !=
            X86_NXVM_BOARD_XEBEC_TYPE_2_SECTORS_PER_TRACK) return 1;
    if (profile.topology.fdc_drives.media_id[0] !=
            VM_PROFILE_XT_5160_268_FDD_MEDIA_ID ||
        profile.topology.fdc_drives.media_id[1] != X86_NXVM_BOARD_MEDIA_ID_INVALID ||
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
    x86_nxvm_board_plan *plan = LIB_NULL;
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_state *board = LIB_NULL;
    x86_nxvm_board_dma_request_binding binding = {0};
    x86_nxvm_board_media_registry *media = LIB_NULL;
    x86_video_snapshot snapshot = {0};
    test_board_composition_observation composition;
    const lib_u8 cells[] = { 'X', 0x1fu };
    lib_u8 open_bus_byte = 0u;
    lib_i32 failed = 1;

    if (vm_profile_xt_5160_268_plan_create(&profile, LIB_FALSE) != LIB_STATUS_OK ||
        x86_nxvm_board_plan_create(&profile.values.core.configuration, &plan) != LIB_STATUS_OK ||
        plan == LIB_NULL || x86_nxvm_board_media_registry_create(&media) != LIB_STATUS_OK ||
        media == LIB_NULL || x86_nxvm_board_media_registry_bind(media,
        VM_PROFILE_XT_5160_268_FDD_MEDIA_ID, LIB_NULL,
        &vm_xt_5160_268_fdd_provider) != LIB_STATUS_OK ||
        x86_nxvm_board_media_registry_freeze(media) != LIB_STATUS_OK ||
        x86_nxvm_board_plan_bind_media_registry(plan, media) != LIB_STATUS_OK ||
        x86_nxvm_board_plan_set_topology(plan, &profile.topology) != LIB_STATUS_OK ||
        x86_nxvm_board_create_from_plan(plan, &machine, &board) != LIB_STATUS_OK ||
        machine == LIB_NULL || board == LIB_NULL ||
        x86_nxvm_board_get_fdc_dma_request_binding(board, &binding) != LIB_STATUS_OK ||
        binding.core_token == 0u || binding.channel != 2u) goto done;
    composition = test_board_capture_composition(board);
    if (composition.fdc.irq != 6u || composition.fdc.dma_channel != 2u ||
        composition.fdc.clock_ticks_per_second != 4772727u ||
        test_x86_nxvm_machine_fixture_expect_port(machine, 0x03f4u, LIB_FALSE, LIB_TRUE) ||
        test_x86_nxvm_machine_fixture_expect_port(machine, 0x03f5u, LIB_FALSE, LIB_TRUE) ||
        test_x86_nxvm_machine_fixture_expect_port(machine, 0x03f2u, LIB_TRUE, LIB_TRUE) ||
        test_x86_nxvm_machine_fixture_expect_port(machine, 0x03f5u, LIB_TRUE, LIB_TRUE) ||
        test_x86_nxvm_machine_fixture_expect_port(machine, 0x03f7u, LIB_FALSE, LIB_FALSE) ||
        test_x86_nxvm_machine_fixture_expect_port(machine, 0x03f7u, LIB_TRUE, LIB_FALSE) ||
        test_x86_nxvm_machine_fixture_expect_port(machine, 0x03d4u, LIB_TRUE, LIB_TRUE) ||
        test_x86_nxvm_machine_fixture_expect_port(machine, 0x03d5u, LIB_FALSE, LIB_TRUE) ||
        test_x86_nxvm_machine_fixture_expect_port(machine, 0x03d8u, LIB_TRUE, LIB_TRUE) ||
        test_x86_nxvm_machine_fixture_expect_port(machine, 0x03d9u, LIB_TRUE, LIB_TRUE) ||
        test_x86_nxvm_machine_fixture_expect_port(machine, 0x03dau, LIB_FALSE, LIB_TRUE) ||
        test_x86_nxvm_machine_fixture_expect_port(machine, 0x03c0u, LIB_FALSE, LIB_FALSE) ||
        !composition.hdc_configured ||
        composition.hdc.protocol != X86_NXVM_BOARD_HDC_PROTOCOL_XEBEC_XT ||
        composition.hdc.bus.xebec.drive_type !=
            X86_NXVM_BOARD_XEBEC_DRIVE_TYPE_2 ||
        test_x86_nxvm_machine_fixture_expect_port(machine, 0x0320u, LIB_FALSE, LIB_TRUE) ||
        test_x86_nxvm_machine_fixture_expect_port(machine, 0x0320u, LIB_TRUE, LIB_TRUE) ||
        test_x86_nxvm_machine_fixture_expect_port(machine, 0x0321u, LIB_FALSE, LIB_TRUE) ||
        test_x86_nxvm_machine_fixture_expect_port(machine, 0x0321u, LIB_TRUE, LIB_TRUE) ||
        test_x86_nxvm_machine_fixture_expect_port(machine, 0x0322u, LIB_FALSE, LIB_TRUE) ||
        test_x86_nxvm_machine_fixture_expect_port(machine, 0x0322u, LIB_TRUE, LIB_TRUE) ||
        test_x86_nxvm_machine_fixture_expect_port(machine, 0x0323u, LIB_FALSE, LIB_FALSE) ||
        test_x86_nxvm_machine_fixture_expect_port(machine, 0x0323u, LIB_TRUE, LIB_TRUE) ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, 0x000c8000u,
        &open_bus_byte, sizeof(open_bus_byte)) != LIB_STATUS_OK ||
        open_bus_byte != 0xffu ||
        x86_core_machine_bus_write(machine, 0x03d8u, 0x0du) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x000b8000u, cells, sizeof(cells)) != LIB_STATUS_OK ||
        x86_nxvm_board_capture_display_snapshot(board, &snapshot) != LIB_STATUS_OK ||
        snapshot.kind != X86_VIDEO_KIND_TEXT ||
        snapshot.characters[0] != 'X' || snapshot.attributes[0] != 0x1fu) goto done;
    failed = 0;
done:
    x86_core_machine_destroy(machine);
    x86_nxvm_board_media_registry_destroy(media);
    x86_nxvm_board_plan_destroy(plan);
    return failed;
}

static lib_i32 vm_xt_5160_268_byob_session_uses_one_xt_route(void)
{
    static lib_u8 system[VM_PROFILE_XT_5160_268_SYSTEM_ROM_BYTES];
    static lib_u8 xebec[VM_PROFILE_XT_5160_268_XEBEC_ROM_BYTES];
    static lib_u8 video[512] = {0x55u, 0xaau, 1u};
    x86_nxvm_machine_config config = {
        .bios_count = 1u
    };
    x86_nxvm_machine_assets assets = { .bios = {
        { system, sizeof(system) }, { xebec, sizeof(xebec) }
    }, .video = { video, sizeof(video) } };
    x86_nxvm_machine *session = LIB_NULL;
    x86_nxvm_machine_reset_vector vector;
    lib_u8 observed[2] = {0};
    lib_i32 failed = 1;

    if (vm_xt_test_create(
            &config, &assets, &session) != LIB_STATUS_OK ||
        session == LIB_NULL || session->x86_core_machine == LIB_NULL ||
        x86_nxvm_machine_get_reset_vector(session, &vector) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(session->x86_core_machine, 0x000c0000u,
        observed, sizeof(observed)) != LIB_STATUS_OK || observed[0u] != 0x55u ||
        observed[1u] != 0xaau) goto done;
    x86_nxvm_machine_destroy(session);
    session = LIB_NULL;
    config.bios_count = 2u;
    if (vm_xt_test_create(
        &config, &assets, &session) != LIB_STATUS_OK ||
        session == LIB_NULL) goto done;
    x86_nxvm_machine_destroy(session);
    session = LIB_NULL;
    config.cpu_profile = X86_CORE_CPU_PROFILE_8086;
    failed = vm_xt_test_create(
        &config, &assets, &session) !=
        LIB_STATUS_INVALID_ARGUMENT || session != LIB_NULL;
done:
    x86_nxvm_machine_destroy(session);
    return failed;
}

int main(void)
{
    if (vm_xt_5160_268_contract_is_fixed() ||
        vm_xt_5160_268_topology_constructs_one_xt_route() ||
        vm_xt_5160_268_byob_session_uses_one_xt_route()) return 1;
    lib_c_printf("XT-FIXED-PROFILE:OK\n");
    lib_c_printf("XT-B2-SHARED-TOPOLOGY:OK\n");
    lib_c_printf("XT-FDC-PLAN:OK\n");
    lib_c_printf("XT-NO-AT-FDC-ALIAS:OK\n");
    lib_c_printf("XT-CGA-PLAN:OK\n");
    lib_c_printf("XT-NO-VIDEO-ALIAS:OK\n");
    lib_c_printf("XT-TYPE2:OK\n");
    lib_c_printf("XT-B6-BYOB-SESSION:OK\n");
    return 0;
}
