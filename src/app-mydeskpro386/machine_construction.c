#include "lib/types/types_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "app-mydeskpro386/machine_construction_interface.h"

#include "x86-nxvm/board-base/vadp_interface.h"
#include "x86-nxvm/board-base/floppy_interface.h"
#include "x86-nxvm/board-base/at_assembly_interface.h"

static lib_status mydeskpro386_materialize_controllers(x86_nxvm_board_plan *plan,
    x86_nxvm_board_fdc_terminal_observation_provider terminal_observation)
{
    const x86_nxvm_board_fdc_drive_bindings drives = {
        .media_id = { 1u, 2u,
            X86_NXVM_BOARD_MEDIA_ID_INVALID, X86_NXVM_BOARD_MEDIA_ID_INVALID },
        /* The selected Model 40 topology has two 1.2 MiB mechanisms.
         * A: may be the only drive with inserted media. */
        .installed_mask = 0x03u,
        .double_sided_mask = 0x03u,
        .track_zero_active_low_mask = 0u,
        .cylinder_count = {80u, 80u, 0u, 0u},
        .channel = x86_nxvm_board_floppy_channel_get(X86_NXVM_BOARD_FLOPPY_525_1200K)
    };
    x86_nxvm_board_fdc_config fdc = {0};
    x86_nxvm_board_hdc_config hdc = {0};

    if (plan == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    fdc.ready_mask = 0x0fu;
    fdc.initial_ccr_configured = LIB_TRUE;
    fdc.initial_ccr = 0u; /* Board reset selects 500 kbit/s. */
    /* FDC deadlines use the Core source axis, not the controller crystal. */
    fdc.clock_ticks_per_second = 16000000u;
    if (x86_nxvm_board_at_fdc_materialize(x86_nxvm_board_at_port_leaves, X86_NXVM_BOARD_AT_PORT_LEAF_COUNT,
            x86_nxvm_board_at_routes_without_aux, 4u, &fdc) != LIB_STATUS_OK)
        return LIB_STATUS_INVALID_ARGUMENT;
    hdc = (x86_nxvm_board_hdc_config) {
        .protocol = X86_NXVM_BOARD_HDC_PROTOCOL_COMPAQ_WD_40MB,
        .irq = 14u, .service = {0u, 0u}, .bus.task_file = {
            .data_port = 0x01f0u, .error_features_port = 0x01f1u,
            .sector_count_port = 0x01f2u, .sector_number_port = 0x01f3u,
            .cylinder_low_port = 0x01f4u, .cylinder_high_port = 0x01f5u,
            .drive_head_port = 0x01f6u, .status_command_port = 0x01f7u,
            .alternate_status_device_control_port = 0x03f6u,
            .drive_address_port = 0x03f7u, .lba28_supported = LIB_FALSE}};
    if (x86_nxvm_board_plan_configure_fdc(plan, &drives, &fdc) != LIB_STATUS_OK ||
        x86_nxvm_board_plan_bind_fdc_terminal_observation(plan,
            terminal_observation) != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    return x86_nxvm_board_plan_configure_hdc(plan, 2u,
        X86_NXVM_BOARD_MEDIA_ID_INVALID, &hdc);
}

lib_status mydeskpro386_topology_materialize(
    x86_nxvm_board_plan_topology *out_topology)
{
    x86_nxvm_board_display_config display = {0};
    x86_nxvm_board_rtc_cmos_config rtc = {0};
    x86_nxvm_board_plan_topology topology = {0};
    if (out_topology == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    display.text_timing = (x86_video_text_timing) {48u, 8u, 8u};
    display.cga_vram_present = LIB_FALSE;
    display.ega_present = LIB_TRUE;
    display.vertical_irq = 9u;
    display.ega_personality = X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR;
    display.cecg = (x86_video_cecg_config) {
        0x41u, 0x0du, 0x30u, 0x01u, LIB_TRUE, LIB_FALSE, LIB_TRUE,
        0x06u, 0x01u, LIB_FALSE, LIB_FALSE, LIB_FALSE };
    display.ega_sequencer = (x86_video_ega_sequencer_config) {
        X86_NXVM_BOARD_VADP_EGA_APERTURE_BASE, X86_NXVM_BOARD_VADP_EGA_APERTURE_BYTES,
        0x03u, 0x00u, 0x0fu, 0x02u, LIB_TRUE };
    display.ega_controllers = (x86_video_ega_controller_config) {
        { 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x04u, 0x00u, 0xffu },
        { 0x00u, 0x01u, 0x02u, 0x03u, 0x04u, 0x05u, 0x06u, 0x07u,
          0x08u, 0x09u, 0x0au, 0x0bu, 0x0cu, 0x0du, 0x0eu, 0x0fu,
          0x00u, 0x00u, 0x0fu, 0x00u, 0x00u } };
    rtc.nmi_mask_bit = 0x80u;
    rtc.ticks_per_second = 32768u;
    rtc.timing = (x86_nxvm_board_rtc_timing_plan) {8u, 65u,
        X86_NXVM_BOARD_RTC_TIMING_L3_SOURCE};
    /* The selected D4 setup has 640 KiB conventional RAM and relocates the
     * remaining 384 KiB of the built-in first MiB to FA0000h--FFFFFFh.
     * Core owns the single RAM backing; the frozen profile declares only the
     * board decode. */
    topology.memory_alias_count = 2u;
    topology.memory_alias[0] = (x86_core_machine_memory_alias_config) {
        0x00fa0000u, 0x000a0000u, 0x00060000u };
    /* D4 enables this second alias at runtime when its mapping register
     * relocates FE0000h--FFFFFFh into E0000h--FFFFFh.  Aliases name backing
     * offsets, so this is the high range's already-decoded E0000h backing,
     * not a second physical hop through FE0000h. */
    topology.memory_alias[1] = (x86_core_machine_memory_alias_config) {
        0x000e0000u, 0x000e0000u, 0x00020000u };
    /* D4 decodes the installed 1 MiB upgrade through 1FFFFFh; the option
     * board range and the unselected F00000h--F9FFFFh bank decode open bus. */
    topology.absent_memory_count = 4u;
    topology.absent_memory[0] = (x86_nxvm_board_absent_memory_config) {
        0x00200000u, 0x00800000u, 0xffu };
    topology.absent_memory[1] = (x86_nxvm_board_absent_memory_config) {
        0x00f00000u, 0x000a0000u, 0xffu };
    /* The board relocates this B0000h window into the selected FB0000h
     * backing.  Keep the low unpopulated decode open while the high alias
     * continues to win as its own selected physical mapping. */
    topology.absent_memory[2] = (x86_nxvm_board_absent_memory_config) {
        0x000b0000u, 0x00008000u, 0xffu };
    /* Before D4 selects its 128 KiB relocated decode, the E0000h half is an
     * unpopulated board window; F0000h remains available to the system ROM. */
    topology.absent_memory[3] = (x86_nxvm_board_absent_memory_config) {
        0x000e0000u, 0x00010000u, 0xffu };
    topology.display = display;
    topology.rtc_cmos = rtc;
    if (x86_nxvm_board_at_topology_materialize(x86_nxvm_board_at_port_leaves, X86_NXVM_BOARD_AT_PORT_LEAF_COUNT,
            x86_nxvm_board_at_routes_without_aux, 4u, &topology) != LIB_STATUS_OK)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_topology = topology;
    return LIB_STATUS_OK;
}

static lib_status model40_selected_board_factory(
    const x86_nxvm_board_profile_services *services, void *context,
    x86_nxvm_board_profile_binding *out_binding)
{
    const mydeskpro386_d4_platform_config config = {X86_NXVM_BOARD_PC_AT_PORT_B, 0u, 2u, 18u};
    const mydeskpro386_d4_memory_config d4_memory = {
        LIB_TRUE, 0x8fu, 0xfdu, 0xfc42u };
    mydeskpro386_d4_platform *platform = LIB_NULL;
    mydeskpro386_d4_platform **construction_output = context;
    lib_status status;

    if (construction_output != LIB_NULL) *construction_output = LIB_NULL;
    status = mydeskpro386_d4_platform_create(services->core, services->pit,
        services->auxiliary_pit, &config, services->speaker,
        services->speaker_context, &platform);
    if (status != LIB_STATUS_OK) return status;
    status = mydeskpro386_d4_platform_configure_memory(platform, &d4_memory);
    if (status != LIB_STATUS_OK) {
        mydeskpro386_d4_platform_destroy(platform);
        return status;
    }
    *out_binding = mydeskpro386_d4_platform_binding(platform);
    out_binding->kbc_command_extension = mydeskpro386_d4_platform_kbc_command;
    if (construction_output != LIB_NULL) *construction_output = platform;
    return LIB_STATUS_OK;
}

lib_status mydeskpro386_materialize_plan(x86_nxvm_board_plan *plan,
    x86_nxvm_board_fdc_terminal_observation_provider terminal_observation,
    mydeskpro386_d4_platform **construction_output)
{
    if (plan == LIB_NULL || x86_nxvm_board_plan_bind_profile_factory(plan,
            model40_selected_board_factory, construction_output) != LIB_STATUS_OK)
        return LIB_STATUS_INVALID_ARGUMENT;
    return mydeskpro386_materialize_controllers(plan, terminal_observation);
}
