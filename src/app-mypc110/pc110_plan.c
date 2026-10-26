#include "app-mypc110/pc110_plan_interface.h"
#include "app-mypc110/pc110_clock.h"

#include "app-mypc110/pc110_config_interface.h"
#include "app-mypc110/pc110_display_interface.h"
#include "app-mypc110/pc110_ec_block_interface.h"
#include "app-mypc110/pc110_ec_mailbox_interface.h"
#include "app-mypc110/pc110_firmware_interface.h"
#include "app-mypc110/pc110_font_interface.h"
#include "app-mypc110/pc110_lpt_interface.h"
#include "app-mypc110/pc110_pcic_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "x86-nxvm/machine/media/media_interface.h"

#define MYPC110_POWER_INPUT_FAULT 0x50575200u

typedef struct mypc110_machine_plan {
    mypc110_firmware firmware;
    mypc110_font_rom font;
    mypc110_lpt lpt;
    mypc110_ec_block ec_block;
    mypc110_ec_mailbox ec_mailbox;
    x86_vl82c420 *configuration_chip;
    x86_f65535 *display_controller;
    x86_rb5c396 *pcic_controller;
} mypc110_machine_plan;

static const x86_nxvm_board_config mypc110_machine_configuration = {
    .memory_bytes = 20u * 1024u * 1024u,
    .cpu_profile = X86_CORE_CPU_PROFILE_80486SX,
    .fpu_profile = X86_FPU_PROFILE_NONE,
    .protected_data_reload_policy =
        X86_CORE_CPU_PROTECTED_DATA_RELOAD_VISIBLE_CACHE_OR_REAL_ADDRESS,
    .ticks_per_instruction = 1u,
    .instruction_timing = {1u, 0u, 0u, 0u, 0u, 0u},
    .time_axis = {X86_CORE_MACHINE_TIME_AXIS_MACRO_PROPORTIONAL, MYPC110_CPU_CLOCK_HZ},
    .clock_plan = {
        .dma = {1u, 1u, 0u}, .pit = {MYPC110_PIT_INPUT_HZ, MYPC110_CPU_CLOCK_HZ, 0u},
        .auxiliary_pit = {1u, 1u, 0u},
        .rtc = {MYPC110_RTC_INPUT_HZ, MYPC110_CPU_CLOCK_HZ, 0u},
        .vadp = {1u, 1u, 0u}, .kbc = {1u, 1u, 0u}, .provider = {1u, 1u, 0u}},
    .shared_pit_personality = X86_PIT_PERSONALITY_8254,
    .pic_topology = X86_NXVM_BOARD_PIC_TOPOLOGY_CASCADED,
    .dma_controller_count = X86_NXVM_BOARD_DMA_CONTROLLER_COUNT,
    .dma_page_ports_plain_scratch = LIB_TRUE,
    .keyboard_topology = X86_NXVM_BOARD_KEYBOARD_TOPOLOGY_8042,
    /* Let completed-write polling precede ACK, without requiring a guest
     * status read to release an IRQ-driven reply. */
    .kbc_command_response_ticks = MYPC110_KBC_COMMAND_RESPONSE_TICKS,
    .kbc_reset_output_port_configured = 1u,
    .kbc_reset_output_port = 0x03u,
    .kbc_reset_command_byte_configured = 1u,
    .kbc_reset_command_byte = 0x65u,
    .display_topology = X86_NXVM_BOARD_DISPLAY_TOPOLOGY_NONE,
    .system_facade = X86_NXVM_BOARD_SYSTEM_FACADE_NONE,
    .system_control_port_present = LIB_TRUE
};

/* The PC110 exposes its MC146818-compatible CMOS through the conventional
 * 70h/71h pair and routes its interrupt through IRQ8.  This is board decode,
 * not a PC/AT facade: the TFT BIOS itself owns all CMOS setup values. */
static const x86_nxvm_board_plan_topology mypc110_machine_topology = {
    .dma_present = LIB_TRUE,
    .dma = {2u, X86_NXVM_BOARD_DMA_CONTROLLER_COUNT,
        X86_NXVM_BOARD_DMA_CASCADE_CHANNEL},
    /* L2 U22 standard-compatible FDC view, independent of the absent dock
     * drive. SMC Table 19 fixes the legacy ST3 READY/TS positions high. */
    .fdc_present = LIB_TRUE,
    .fdc = {
        .dor_port = 0x03f2u,
        .status_port = 0x03f4u,
        .data_port = 0x03f5u,
        .direction_port = 0x03f7u,
        .control_port = 0x03f7u,
        .irq = 6u,
        .dma_channel = 2u,
        .ready_mask = 0x0fu,
        .clock_ticks_per_second = MYPC110_CPU_CLOCK_HZ},
    .fdc_drives = {.double_sided_mask = 0x0fu},
    /* The admitted BIOS probes the first 64 KiB beyond installed RAM to find
     * the limit.  Model only that observed probe window as L2 open bus. */
    .absent_memory_count = 2u,
    .absent_memory = {{0x01400000u, 0x00010000u, 0xffu},
        /* External D window is unpopulated; selected font/shadow routes win. */
        {0x000d0000u, 0x00010000u, 0xffu}},
    /* L2 nominal refresh period: 18 PIT inputs at 1.193182 MHz, rounded to
     * 498 ticks on the 33 MHz Core axis. The firmware observes this signal
     * before programming channel 1 and later compares it with PIT counter 0.
     * Keep both consumers on the same declared time axis.
     * Zero memory bytes deliberately excludes PC/AT parity-memory behavior. */
    .planar_parity_present = LIB_TRUE,
    .planar_parity = {
        .port = X86_NXVM_BOARD_PC_AT_PORT_B,
        .memory_bytes = 0u,
        .refresh_status_source =
            X86_NXVM_BOARD_PLANAR_PARITY_REFRESH_STATUS_ELAPSED_TICK_TOGGLE,
        .refresh_status_toggle_ticks = MYPC110_REFRESH_SOURCE_TICKS},
    .rtc_cmos_present = LIB_TRUE,
    .rtc_cmos = {
        .index_port = 0x0070u,
        .data_port = 0x0071u,
        .irq = 8u,
        .nmi_mask_bit = 0x80u,
        /* L2: unchanged BIOS and both reference models address independent
         * upper CMOS bytes; do not alias them onto classic RTC registers. */
        .extended_registers = LIB_TRUE,
        .ticks_per_second = 32768u,
        .timing = {8u, 65u, X86_NXVM_BOARD_RTC_TIMING_L3_SOURCE}}
};

/* PC110's internal CF/PC-Card storage reaches a standard ATA task file at
 * 1F0h--1F7h and IRQ14. The selected firmware is CHS-only, but later software
 * may issue IDENTIFY/LBA commands directly; this describes the controller
 * interface, not a BIOS policy or inserted medium. No source gives an ATA
 * service duration on the Core tick axis, so zero retains the existing
 * immediate-owner model rather than inventing a mechanical time. */
static const x86_nxvm_board_hdc_config mypc110_machine_hdc = {
    .protocol = X86_NXVM_BOARD_HDC_PROTOCOL_ATA_PIO,
    .irq = 14u,
    .bus.task_file = {
        .data_port = 0x01f0u,
        .error_features_port = 0x01f1u,
        .sector_count_port = 0x01f2u,
        .sector_number_port = 0x01f3u,
        .cylinder_low_port = 0x01f4u,
        .cylinder_high_port = 0x01f5u,
        .drive_head_port = 0x01f6u,
        .status_command_port = 0x01f7u,
        .alternate_status_device_control_port = 0x03f6u,
        .lba28_supported = LIB_TRUE}};

static lib_status mypc110_machine_capture_display(void *context,
    x86_video_snapshot *out_snapshot)
{
    return x86_f65535_capture_snapshot((x86_f65535 *)context, out_snapshot);
}

/* The board owns reset ordering.  PCIC state is controller-local, while the
 * physical socket levels remain owned by the App's slot-input contract. */
static void mypc110_machine_profile_reset(void *opaque,
    x86_nxvm_board_reset_phase phase)
{
    mypc110_machine_plan *plan = opaque;

    if (plan != LIB_NULL && phase == X86_NXVM_BOARD_RESET_BEFORE_DEVICES)
        x86_rb5c396_reset(plan->pcic_controller);
}

static void mypc110_machine_profile_system_management_active(void *opaque,
    lib_bool active)
{
    mypc110_machine_plan *plan = opaque;

    if (plan != LIB_NULL)
        mypc110_firmware_system_management_active(&plan->firmware, active);
}

static lib_status mypc110_machine_profile_create(
    const x86_nxvm_board_profile_services *services, void *opaque,
    x86_nxvm_board_profile_binding *out_binding)
{
    mypc110_machine_plan *plan = opaque;
    lib_status status;

    if (plan == LIB_NULL || out_binding == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = mypc110_firmware_profile_create(services, &plan->firmware, out_binding);
    if (status != LIB_STATUS_OK) return status;
    out_binding->context = plan;
    out_binding->reset = mypc110_machine_profile_reset;
    out_binding->system_management_active =
        mypc110_machine_profile_system_management_active;
    return LIB_STATUS_OK;
}

static lib_status mypc110_machine_configure(void *opaque,
    x86_nxvm_board_plan *board_plan)
{
    mypc110_machine_plan *plan = opaque;
    lib_status status;

    status = x86_nxvm_board_plan_bind_profile_factory(board_plan,
        mypc110_machine_profile_create, plan);
    if (status == LIB_STATUS_OK) {
        status = x86_nxvm_board_plan_configure_hdc(board_plan,
            X86_NXVM_MACHINE_MEDIA_HDD_ID, X86_NXVM_MACHINE_MEDIA_HDD_SECONDARY_ID,
            &mypc110_machine_hdc);
    }
    return status;
}

static lib_status mypc110_machine_after_firmware(void *opaque, x86_core_machine *core)
{
    mypc110_machine_plan *plan = opaque;
    lib_status status;

    if (plan == LIB_NULL || core == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = mypc110_firmware_install_routes(core, &plan->firmware);
    if (status == LIB_STATUS_OK)
        status = mypc110_config_install_routes(core, &plan->firmware,
            plan->configuration_chip);
    if (status == LIB_STATUS_OK)
        status = mypc110_ec_mailbox_install_routes(core, &plan->ec_mailbox);
    if (status == LIB_STATUS_OK)
        status = mypc110_ec_block_install_routes(core, &plan->ec_block);
    if (status == LIB_STATUS_OK)
        status = mypc110_font_rom_install(core, &plan->font);
    if (status == LIB_STATUS_OK)
        status = mypc110_lpt_install_routes(core, &plan->lpt);
    if (status == LIB_STATUS_OK)
        status = mypc110_display_install_routes(core, plan->display_controller);
    if (status == LIB_STATUS_OK)
        status = mypc110_pcic_install_routes(core, plan->pcic_controller);
    return status;
}

static void mypc110_machine_release(void *opaque)
{
    mypc110_machine_plan *plan = opaque;

    if (plan == LIB_NULL) return;
    x86_f65535_destroy(plan->display_controller);
    x86_rb5c396_destroy(plan->pcic_controller);
    x86_vl82c420_destroy(plan->configuration_chip);
    mypc110_firmware_destroy(&plan->firmware);
    lib_release(plan);
}

static lib_bool mypc110_machine_handle_key(void *opaque, const kvm_input_event *event)
{
    mypc110_machine_plan *plan = opaque;

    if (event->data.key.key != BASE_KEY_F12) return LIB_FALSE;
    const lib_status status = mypc110_firmware_set_resume_button(&plan->firmware,
        event->data.key.pressed);
    if (status != LIB_STATUS_OK)
        (void)x86_core_machine_report_fault(plan->firmware.machine,
            MYPC110_POWER_INPUT_FAULT | (lib_u32)status);
    return LIB_TRUE;
}

lib_status mypc110_machine_construction_create(const x86_nxvm_machine_config *config,
    const x86_nxvm_machine_assets *assets,
    x86_nxvm_machine_construction *out_construction)
{
    mypc110_machine_plan *plan;
    lib_status status;

    if (out_construction == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_construction = (x86_nxvm_machine_construction){0};
    if (config == LIB_NULL || assets == LIB_NULL || assets->bios[0u].data == LIB_NULL ||
        assets->bios[0u].bytes != MYPC110_FIRMWARE_BYTES ||
        assets->font.data == LIB_NULL || assets->font.bytes != MYPC110_FONT_ROM_BYTES) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    plan = lib_allocate_zero(1u, sizeof(*plan));
    if (plan == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    status = mypc110_firmware_create(assets->bios[0u].data, assets->bios[0u].bytes,
        &plan->firmware);
    if (status == LIB_STATUS_OK)
        status = mypc110_font_rom_create(assets->font.data, assets->font.bytes, &plan->font);
    if (status == LIB_STATUS_OK) status = x86_vl82c420_create(&plan->configuration_chip);
    if (status == LIB_STATUS_OK)
        status = mypc110_config_initialize_cold_image(plan->configuration_chip);
    if (status == LIB_STATUS_OK) status = x86_f65535_create(&plan->display_controller);
    if (status == LIB_STATUS_OK) status = x86_rb5c396_create(&plan->pcic_controller);
    if (status != LIB_STATUS_OK) {
        mypc110_machine_release(plan);
        return status;
    }
    *out_construction = (x86_nxvm_machine_construction) {
        .core_config = mypc110_machine_configuration,
        .topology = mypc110_machine_topology,
        .floppy_kind = X86_NXVM_BOARD_FLOPPY_35_1440K,
        .media_kind = X86_NXVM_BOARD_FLOPPY_35_1440K,
        .floppy_slot_count = 0u,
        .fixed_disk_slot_count = 2u,
        .hdc_present = LIB_TRUE,
        /* L2: Open-Source-PC110's live INT 13h and ATA IDENTIFY captures
         * record the soldered primary SunDisk as 123 cylinders, two heads
         * and 32 sectors/track.  This consumes Machine's existing
         * master-only exact-geometry contract; the removable slave is not
         * assigned a guessed geometry. */
        .fixed_geometry = LIB_TRUE,
        .cylinders = 123u,
        .heads = 2u,
        .sectors = 32u,
        .memory_reconfigurable = LIB_FALSE,
        .display_snapshot_provider = mypc110_machine_capture_display,
        .display_snapshot_context = plan->display_controller,
        .firmware_provider = mypc110_firmware_provider(),
        .firmware_context = &plan->firmware,
        .profile = {.context = plan, .configure = mypc110_machine_configure,
            .after_firmware = mypc110_machine_after_firmware,
            .release = mypc110_machine_release,
            .handle_key = mypc110_machine_handle_key}};
    return LIB_STATUS_OK;
}
