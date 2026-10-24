#include "app-mypc110/pc110_machine_interface.h"

#include "app-mypc110/pc110_config_interface.h"
#include "app-mypc110/pc110_firmware_interface.h"

static const x86_nxvm_board_config mypc110_base_configuration = {
    .memory_bytes = 20u * 1024u * 1024u,
    .cpu_profile = X86_CORE_CPU_PROFILE_80486SX,
    .fpu_profile = X86_FPU_PROFILE_NONE,
    .ticks_per_instruction = 1u,
    .instruction_timing = {1u, 0u, 0u, 0u, 0u, 0u},
    /* 33 MHz is a source-backed CPU fact; these controller ratios are only
     * neutral L2 construction units until the PC110 device receivers qualify
     * their own clocks and routes. */
    .time_axis = {X86_CORE_MACHINE_TIME_AXIS_MACRO_PROPORTIONAL, 33000000u},
    .clock_plan = {
        .dma = {1u, 1u, 0u},
        .pit = {1u, 1u, 0u},
        .auxiliary_pit = {1u, 1u, 0u},
        .rtc = {1u, 1u, 0u},
        .vadp = {1u, 1u, 0u},
        .kbc = {1u, 1u, 0u},
        .provider = {1u, 1u, 0u}},
    .shared_pit_personality = X86_PIT_PERSONALITY_8254,
    .pic_topology = X86_NXVM_BOARD_PIC_TOPOLOGY_CASCADED,
    .dma_controller_count = X86_NXVM_BOARD_DMA_CONTROLLER_COUNT,
    .keyboard_topology = X86_NXVM_BOARD_KEYBOARD_TOPOLOGY_8042_NEUTRAL,
    .kbc_reset_system_flag_configured = 1u,
    .kbc_reset_system_flag = 1u,
    .display_topology = X86_NXVM_BOARD_DISPLAY_TOPOLOGY_NONE,
    .system_facade = X86_NXVM_BOARD_SYSTEM_FACADE_NONE
};

static const x86_nxvm_board_plan_topology mypc110_base_topology = {
    .rtc_cmos_present = LIB_TRUE,
    .rtc_cmos = {
        .index_port = 0x0070u,
        .data_port = 0x0071u,
        .irq = 8u,
        .nmi_mask_bit = 0x80u,
        .ticks_per_second = 32768u,
        .timing = {8u, 65u, X86_NXVM_BOARD_RTC_TIMING_L3_SOURCE}}
};

void mypc110_base_composition_destroy(mypc110_base_composition *composition)
{
    if (composition == LIB_NULL) return;
    x86_core_machine_destroy(composition->core);
    x86_f65535_destroy(composition->display_controller);
    x86_vl82c420_destroy(composition->configuration_chip);
    *composition = (mypc110_base_composition) {0};
}

lib_status mypc110_base_composition_create(const lib_u8 *tft_bios,
    lib_size tft_bios_bytes, const lib_u8 *font_rom, lib_size font_rom_bytes,
    mypc110_base_composition *out_composition)
{
    mypc110_base_composition *composition;
    mypc110_tft_firmware firmware;
    x86_nxvm_board_plan *plan = LIB_NULL;
    lib_status status;

    if (out_composition == LIB_NULL || mypc110_tft_firmware_create(tft_bios,
            tft_bios_bytes, &firmware) != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_composition = (mypc110_base_composition) {0};
    composition = out_composition;
    status = mypc110_font_rom_create(font_rom, font_rom_bytes, &composition->font);
    if (status == LIB_STATUS_OK) {
        status = x86_vl82c420_create(&composition->configuration_chip);
    }
    if (status == LIB_STATUS_OK) {
        status = x86_f65535_create(&composition->display_controller);
    }
    if (status == LIB_STATUS_OK) {
    status = x86_nxvm_board_plan_create(&mypc110_base_configuration, &plan);
    if (status == LIB_STATUS_OK) {
        status = x86_nxvm_board_plan_set_topology(plan, &mypc110_base_topology);
    }
    }
    if (status == LIB_STATUS_OK) {
        status = x86_nxvm_board_create_from_plan(plan, &composition->core,
            &composition->board);
    }
    if (status == LIB_STATUS_OK) {
        status = x86_core_machine_bind_firmware_provider(composition->core,
            mypc110_tft_firmware_provider(), &firmware);
    }
    if (status == LIB_STATUS_OK) {
        status = mypc110_config_install_routes(composition->core,
            composition->configuration_chip);
    }
    if (status == LIB_STATUS_OK) {
        status = mypc110_font_rom_install(composition->core, &composition->font);
    }
    if (status == LIB_STATUS_OK) {
        status = mypc110_display_install_routes(composition->core,
            composition->display_controller);
    }
    if (status == LIB_STATUS_OK) {
        status = x86_core_machine_freeze_execution_providers(composition->core);
    }
    x86_nxvm_board_plan_destroy(plan);
    if (status != LIB_STATUS_OK) {
        mypc110_base_composition_destroy(composition);
        return status;
    }
    return LIB_STATUS_OK;
}
