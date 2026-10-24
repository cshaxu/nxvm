#include "app-mypc110/pc110_plan_interface.h"

#include "app-mypc110/pc110_config_interface.h"
#include "app-mypc110/pc110_display_interface.h"
#include "app-mypc110/pc110_firmware_interface.h"
#include "app-mypc110/pc110_font_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"

typedef struct mypc110_machine_plan {
    mypc110_tft_firmware firmware;
    mypc110_font_rom font;
    x86_vl82c420 *configuration_chip;
    x86_f65535 *display_controller;
} mypc110_machine_plan;

static const x86_nxvm_board_config mypc110_machine_configuration = {
    .memory_bytes = 20u * 1024u * 1024u,
    .cpu_profile = X86_CORE_CPU_PROFILE_80486SX,
    .fpu_profile = X86_FPU_PROFILE_NONE,
    .protected_data_reload_policy =
        X86_CORE_CPU_PROTECTED_DATA_RELOAD_VISIBLE_CACHE_OR_REAL_ADDRESS,
    .ticks_per_instruction = 1u,
    .instruction_timing = {1u, 0u, 0u, 0u, 0u, 0u},
    .time_axis = {X86_CORE_MACHINE_TIME_AXIS_MACRO_PROPORTIONAL, 33000000u},
    .clock_plan = {
        .dma = {1u, 1u, 0u}, .pit = {1u, 1u, 0u},
        .auxiliary_pit = {1u, 1u, 0u}, .rtc = {1u, 1u, 0u},
        .vadp = {1u, 1u, 0u}, .kbc = {1u, 1u, 0u}, .provider = {1u, 1u, 0u}},
    .shared_pit_personality = X86_PIT_PERSONALITY_8254,
    .pic_topology = X86_NXVM_BOARD_PIC_TOPOLOGY_CASCADED,
    .dma_controller_count = X86_NXVM_BOARD_DMA_CONTROLLER_COUNT,
    .keyboard_topology = X86_NXVM_BOARD_KEYBOARD_TOPOLOGY_8042,
    .kbc_reset_output_port_configured = 1u,
    .kbc_reset_output_port = 0x03u,
    .kbc_reset_system_flag_configured = 1u,
    .kbc_reset_system_flag = 1u,
    .display_topology = X86_NXVM_BOARD_DISPLAY_TOPOLOGY_NONE,
    .system_facade = X86_NXVM_BOARD_SYSTEM_FACADE_NONE
};

/* The PC110 exposes its MC146818-compatible CMOS through the conventional
 * 70h/71h pair and routes its interrupt through IRQ8.  This is board decode,
 * not a PC/AT facade: the TFT BIOS itself owns all CMOS setup values. */
static const x86_nxvm_board_plan_topology mypc110_machine_topology = {
    /* The admitted BIOS probes the first 64 KiB beyond installed RAM to find
     * the limit.  Model only that observed probe window as L2 open bus. */
    .absent_memory_count = 1u,
    .absent_memory = {{0x01400000u, 0x00010000u, 0xffu}},
    /* PC110 firmware polls the conventional system-control timer status at
     * 61h.  This is only the port/timer/speaker wiring: zero memory bytes
     * deliberately excludes the PC/AT parity-memory behavior. */
    .planar_parity_present = LIB_TRUE,
    .planar_parity = {
        .port = X86_NXVM_BOARD_PC_AT_PORT_B,
        .memory_bytes = 0u,
        .refresh_status_source =
            X86_NXVM_BOARD_PLANAR_PARITY_REFRESH_STATUS_PIT_COUNTER_1},
    .rtc_cmos_present = LIB_TRUE,
    .rtc_cmos = {
        .index_port = 0x0070u,
        .data_port = 0x0071u,
        .irq = 8u,
        .nmi_mask_bit = 0x80u,
        .ticks_per_second = 32768u,
        .timing = {8u, 65u, X86_NXVM_BOARD_RTC_TIMING_L3_SOURCE}}
};

static lib_status mypc110_machine_capture_display(void *context,
    x86_video_snapshot *out_snapshot)
{
    return x86_f65535_capture_snapshot((x86_f65535 *)context, out_snapshot);
}

/* The PC110 firmware deliberately enters CPU shutdown as its warm-reset
 * handoff.  Core already models a board-accepted shutdown as a processor-only
 * reset, preserving RAM and board state.  This binding selects that existing
 * electrical route without adding a BIOS or device special case. */
static void mypc110_machine_profile_reset(void *context,
    x86_nxvm_board_reset_phase phase)
{
    (void)context;
    (void)phase;
}

static void mypc110_machine_profile_refresh_nmi(void *context)
{
    (void)context;
}

static lib_bool mypc110_machine_profile_refresh_request(void *context,
    lib_u8 *out_address)
{
    (void)context;
    (void)out_address;
    return LIB_FALSE;
}

static void mypc110_machine_profile_refresh_complete(void *context)
{
    (void)context;
}

static lib_bool mypc110_machine_profile_next_deadline(void *context,
    lib_u64 now, lib_u64 *out_tick)
{
    (void)context;
    (void)now;
    (void)out_tick;
    return LIB_FALSE;
}

static void mypc110_machine_profile_finalize(void *context)
{
    (void)context;
}

static lib_status mypc110_machine_profile_create(
    const x86_nxvm_board_profile_services *services, void *context,
    x86_nxvm_board_profile_binding *out_binding)
{
    if (services == LIB_NULL || services->core == LIB_NULL || context == LIB_NULL ||
        out_binding == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_binding = (x86_nxvm_board_profile_binding) {
        .context = context,
        .reset = mypc110_machine_profile_reset,
        .refresh_nmi = mypc110_machine_profile_refresh_nmi,
        .refresh_request = mypc110_machine_profile_refresh_request,
        .refresh_complete = mypc110_machine_profile_refresh_complete,
        .next_deadline = mypc110_machine_profile_next_deadline,
        .finalize = mypc110_machine_profile_finalize,
        .shutdown_resets = LIB_TRUE};
    return LIB_STATUS_OK;
}

static lib_status mypc110_machine_configure(void *opaque,
    x86_nxvm_board_plan *board_plan)
{
    return x86_nxvm_board_plan_bind_profile_factory(board_plan,
        mypc110_machine_profile_create, opaque);
}

static lib_status mypc110_machine_after_firmware(void *opaque, x86_core_machine *core)
{
    mypc110_machine_plan *plan = opaque;
    lib_status status;

    if (plan == LIB_NULL || core == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = mypc110_tft_firmware_install_routes(core, &plan->firmware);
    if (status == LIB_STATUS_OK)
        status = mypc110_config_install_routes(core, &plan->firmware,
            plan->configuration_chip);
    if (status == LIB_STATUS_OK)
        status = mypc110_font_rom_install(core, &plan->font);
    if (status == LIB_STATUS_OK)
        status = mypc110_display_install_routes(core, plan->display_controller);
    return status;
}

static void mypc110_machine_release(void *opaque)
{
    mypc110_machine_plan *plan = opaque;

    if (plan == LIB_NULL) return;
    x86_f65535_destroy(plan->display_controller);
    x86_vl82c420_destroy(plan->configuration_chip);
    mypc110_tft_firmware_destroy(&plan->firmware);
    lib_release(plan);
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
        assets->bios[0u].bytes != MYPC110_TFT_BIOS_BYTES ||
        assets->font.data == LIB_NULL || assets->font.bytes != MYPC110_FONT_ROM_BYTES) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    plan = lib_allocate_zero(1u, sizeof(*plan));
    if (plan == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    status = mypc110_tft_firmware_create(assets->bios[0u].data, assets->bios[0u].bytes,
        &plan->firmware);
    if (status == LIB_STATUS_OK)
        status = mypc110_font_rom_create(assets->font.data, assets->font.bytes, &plan->font);
    if (status == LIB_STATUS_OK) status = x86_vl82c420_create(&plan->configuration_chip);
    if (status == LIB_STATUS_OK) status = x86_f65535_create(&plan->display_controller);
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
        .hdc_present = LIB_FALSE,
        .memory_reconfigurable = LIB_FALSE,
        .display_snapshot_provider = mypc110_machine_capture_display,
        .display_snapshot_context = plan->display_controller,
        .firmware_provider = mypc110_tft_firmware_provider(),
        .firmware_context = &plan->firmware,
        .profile = {.context = plan, .configure = mypc110_machine_configure,
            .after_firmware = mypc110_machine_after_firmware,
            .release = mypc110_machine_release}};
    return LIB_STATUS_OK;
}
