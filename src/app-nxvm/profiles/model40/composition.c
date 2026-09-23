#include "app-nxvm/profiles/model40/composition_interface.h"

#include "app-nxvm/devices/vadp.h"

static type_status vm_profile_model40_materialize_controllers(core_machine_plan *plan,
    core_machine_fdc_terminal_observation_provider terminal_observation)
{
    const core_machine_fdc_drive_bindings drives = {
        .media_id = { 1u, 2u,
            CORE_MACHINE_MEDIA_ID_INVALID, CORE_MACHINE_MEDIA_ID_INVALID },
        /* The selected Model 40 topology has two 1.2 MiB mechanisms.
         * A: may be the only drive with inserted media. */
        .installed_mask = 0x03u,
        .double_sided_mask = 0x03u,
        .track_zero_active_low_mask = 0u,
        .cylinder_count = {80u, 80u, 0u, 0u}
    };
    core_machine_fdc_config fdc = {0};
    core_machine_hdc_config hdc = {0};

    if (plan == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    fdc = (core_machine_fdc_config) { 0x03f2u, 0x03f4u, 0x03f5u,
        0x03f7u, 0x03f7u, 6u, 2u, CORE_MACHINE_FDC_UNREADY_READ_DESKPRO_REFERENCE,
        0x0fu, 8000000u, 0u, 0u };
    hdc = (core_machine_hdc_config) {
        .protocol = CORE_MACHINE_HDC_PROTOCOL_COMPAQ_WD_40MB,
        .irq = 14u, .service = {0u, 0u}, .bus.task_file = {
            .data_port = 0x01f0u, .error_features_port = 0x01f1u,
            .sector_count_port = 0x01f2u, .sector_number_port = 0x01f3u,
            .cylinder_low_port = 0x01f4u, .cylinder_high_port = 0x01f5u,
            .drive_head_port = 0x01f6u, .status_command_port = 0x01f7u,
            .alternate_status_device_control_port = 0x03f6u,
            .drive_address_port = 0x03f7u, .lba28_supported = TYPE_FALSE}};
    if (core_machine_plan_configure_fdc(plan, &drives, &fdc) != TYPE_STATUS_OK ||
        core_machine_plan_bind_fdc_terminal_observation(plan,
            terminal_observation) != TYPE_STATUS_OK) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    return core_machine_plan_configure_hdc(plan, 2u,
        CORE_MACHINE_MEDIA_ID_INVALID, &hdc);
}

type_status vm_profile_model40_topology_materialize(
    core_machine_plan_topology *out_topology)
{
    core_machine_display_config display = {0};
    core_machine_dma_wiring dma = { .fdc_channel = 2u,
        .controller_count = CORE_MACHINE_DMA_CONTROLLER_COUNT,
        .cascade_channel = CORE_MACHINE_DMA_CASCADE_CHANNEL };
    core_machine_d4_platform_config d4 = { CORE_MACHINE_PC_AT_PORT_B, 0u };
    core_machine_rtc_cmos_config rtc = {0};
    core_machine_plan_topology topology = {0};
    if (out_topology == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    display.text_timing = (core_machine_vadp_text_timing) {48u, 8u, 8u};
    display.cga_vram_present = TYPE_FALSE;
    display.ega_present = TYPE_TRUE;
    display.ega_personality = CORE_MACHINE_VADP_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR;
    display.cecg = (core_machine_vadp_cecg_config) {
        0x40u, 0x00u, 0x30u, 0x01u, TYPE_TRUE, TYPE_FALSE, TYPE_TRUE,
        0x06u, 0x01u, TYPE_FALSE, TYPE_FALSE, TYPE_FALSE };
    display.ega_sequencer = (core_machine_vadp_ega_sequencer_config) {
        CORE_MACHINE_VADP_EGA_APERTURE_BASE, CORE_MACHINE_VADP_EGA_APERTURE_BYTES,
        0x03u, 0x00u, 0x0fu, 0x02u, TYPE_TRUE };
    display.ega_controllers = (core_machine_vadp_ega_controller_config) {
        { 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x05u, 0x00u, 0xffu },
        { 0x00u, 0x01u, 0x02u, 0x03u, 0x04u, 0x05u, 0x06u, 0x07u,
          0x08u, 0x09u, 0x0au, 0x0bu, 0x0cu, 0x0du, 0x0eu, 0x0fu,
          0x01u, 0x00u, 0x0fu, 0x00u, 0x00u } };
    display.ports = (core_machine_display_port_topology) {
        0x03c0u, 0x03c1u, 0x03c4u, 0x03c5u, 0x03ceu, 0x03cfu,
        0x03d4u, 0x03dau };
    rtc.index_port = 0x0070u;
    rtc.data_port = 0x0071u;
    rtc.irq = 8u;
    rtc.nmi_mask_bit = 0x80u;
    rtc.ticks_per_second = 32768u;
    rtc.timing = (core_machine_rtc_timing_plan) {8u, 65u,
        CORE_MACHINE_RTC_TIMING_L3_SOURCE};
    topology.d4_platform_present = TYPE_TRUE;
    topology.d4_platform = d4;
    /* The selected D4 setup has 640 KiB conventional RAM and relocates the
     * remaining 384 KiB of the built-in first MiB to FA0000h--FFFFFFh.
     * Core owns the single RAM backing; the frozen profile declares only the
     * board decode. */
    topology.memory_alias_count = 1u;
    topology.memory_alias[0] = (core_machine_memory_alias_config) {
        0x00fa0000u, 0x000a0000u, 0x00060000u };
    /* D4 decodes the installed 1 MiB upgrade through 1FFFFFh; the option
     * board range and the unselected F00000h--F9FFFFh bank decode open bus. */
    topology.absent_memory_count = 3u;
    topology.absent_memory[0] = (core_machine_absent_memory_config) {
        0x00200000u, 0x00800000u, 0xffu };
    topology.absent_memory[1] = (core_machine_absent_memory_config) {
        0x00f00000u, 0x000a0000u, 0xffu };
    /* The board relocates this B0000h window into the selected FB0000h
     * backing.  Keep the low unpopulated decode open while the high alias
     * continues to win as its own selected physical mapping. */
    topology.absent_memory[2] = (core_machine_absent_memory_config) {
        0x000b0000u, 0x00008000u, 0xffu };
    topology.display_present = TYPE_TRUE;
    topology.display = display;
    topology.dma_present = TYPE_TRUE;
    topology.dma = dma;
    topology.rtc_cmos_present = TYPE_TRUE;
    topology.rtc_cmos = rtc;
    *out_topology = topology;
    return TYPE_STATUS_OK;
}

type_status vm_profile_model40_materialize_plan(core_machine_plan *plan,
    core_machine_fdc_terminal_observation_provider terminal_observation)
{
    const core_machine_d4_memory_config d4_memory = {
        TYPE_TRUE, 0x8fu, 0xfdu, 0xfc42u };

    if (plan == STD_NULL || core_machine_plan_configure_d4_memory(plan,
            &d4_memory) != TYPE_STATUS_OK) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    return vm_profile_model40_materialize_controllers(plan, terminal_observation);
}
