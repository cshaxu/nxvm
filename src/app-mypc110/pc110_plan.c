#include "app-mypc110/pc110_plan_interface.h"
#include "x86-nxvm/machine/preparation_interface.h"
#include "app-mypc110/pc110_clock.h"
#include "app-mypc110/pc110_audio.h"

#include "app-mypc110/pc110_config_interface.h"
#include "app-mypc110/pc110_display_interface.h"
#include "app-mypc110/pc110_ec_block_interface.h"
#include "app-mypc110/pc110_ec_mailbox_interface.h"
#include "app-mypc110/pc110_es488.h"
#include "app-mypc110/pc110_opl2.h"
#include "app-mypc110/pc110_firmware_interface.h"
#include "app-mypc110/pc110_font_interface.h"
#include "app-mypc110/pc110_inking_interface.h"
#include "app-mypc110/pc110_lpt_interface.h"
#include "app-mypc110/pc110_pcic_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "x86-nxvm/machine/media/media_interface.h"

#define MYPC110_POWER_INPUT_FAULT 0x50575200u
#define MYPC110_INKING_INPUT_FAULT 0x494e4b00u

typedef struct mypc110_machine_plan {
    mypc110_firmware firmware;
    mypc110_font_rom font;
    mypc110_lpt lpt;
    mypc110_ec_block ec_block;
    mypc110_ec_mailbox ec_mailbox;
    mypc110_es488 es488;
    mypc110_opl2 opl2;
    mypc110_audio audio;
    mypc110_inking inking;
    x86_vl82c420 *configuration_chip;
    x86_f65535 *display_controller;
    x86_core_machine *core;
    x86_rb5c396 *pcic_controller;
    mypc110_pcic pcic;
    mypc110_product_context *product;
    lib_i32 pen_x, pen_y;
} mypc110_machine_plan;

static lib_status mypc110_machine_apply_cards(mypc110_machine_plan *plan)
{
    for (lib_u8 slot = 0u; slot < MYPC110_CARD_SLOT_COUNT; ++slot) {
        mypc110_card_slot *entry = &plan->product->cards.slot[slot];
        lib_storage_medium *old_common, *old_attribute;
        lib_status status, close_status;
        if (lib_atomic_i32_load_explicit(&entry->pending, LIB_MEMORY_ORDER_SEQ_CST) == 0)
            continue;
        old_common = entry->active_common;
        old_attribute = entry->active_attribute;
        status = mypc110_pcic_set_memory_card(&plan->pcic, slot,
            entry->pending_common, entry->pending_attribute, entry->pending_readonly);
        /* The receiver adopts these pointers before it updates its IRQ/route. */
        entry->active_common = entry->pending_common;
        entry->active_attribute = entry->pending_attribute;
        entry->pending_common = LIB_NULL;
        entry->pending_attribute = LIB_NULL;
        lib_atomic_i32_store_explicit(&entry->pending, 0, LIB_MEMORY_ORDER_SEQ_CST);
        close_status = mypc110_product_card_close_pair(&old_common, &old_attribute);
        if (status != LIB_STATUS_OK) return status;
        if (close_status != LIB_STATUS_OK) return close_status;
    }
    return LIB_STATUS_OK;
}

static lib_status mypc110_machine_before_reset(void *opaque, x86_nxvm_board_state *board)
{
    mypc110_machine_plan *plan = opaque;
    mypc110_vdisplay *display;
    mypc110_display_variant selected;
    const x86_nxvm_machine_assets *assets;
    lib_u8 old_cmos[X86_NXVM_MACHINE_EXTENDED_CMOS_SEED_BYTES];
    lib_status status;

    if (plan == LIB_NULL || board == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (plan->audio.stream != LIB_NULL) {
        status = lib_audio_pcm_stream_clear(plan->audio.stream);
        if (status != LIB_STATUS_OK) return status;
    }
    plan->audio.count = plan->audio.read = plan->audio.tick_phase = 0u;
    plan->audio.clock_valid = LIB_FALSE;
    plan->audio.pcm_until = 0u;
    if (plan->product == LIB_NULL) return LIB_STATUS_OK;
    status = mypc110_machine_apply_cards(plan);
    if (status != LIB_STATUS_OK) return status;
    display = &plan->product->display;
    selected = (mypc110_display_variant)lib_atomic_i32_load_explicit(
        &display->requested, LIB_MEMORY_ORDER_SEQ_CST);
    if (selected == display->active) return LIB_STATUS_OK;
    if (selected != MYPC110_DISPLAY_DSTN && selected != MYPC110_DISPLAY_TFT)
        return LIB_STATUS_INVALID_ARGUMENT;
    assets = display->firmware[selected];
    if (assets == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_nxvm_board_capture_rtc(board, old_cmos, X86_NXVM_MACHINE_RTC_SEED_BYTES);
    if (status == LIB_STATUS_OK)
        status = x86_nxvm_board_capture_cmos(board, old_cmos + X86_NXVM_MACHINE_RTC_SEED_BYTES,
            sizeof(old_cmos) - X86_NXVM_MACHINE_RTC_SEED_BYTES);
    if (status != LIB_STATUS_OK) return status;
    status = mypc110_firmware_select(&plan->firmware, assets->bios[0].data, assets->bios[0].bytes);
    if (status != LIB_STATUS_OK) return status;
    status = x86_nxvm_board_restore_cmos(board,
        display->cmos[selected] + X86_NXVM_MACHINE_RTC_SEED_BYTES,
        sizeof(old_cmos) - X86_NXVM_MACHINE_RTC_SEED_BYTES);
    if (status == LIB_STATUS_OK && display->present[selected] && display->cmos[selected][0x0du] == 0x80u)
        status = x86_nxvm_board_restore_rtc(board, display->cmos[selected],
            X86_NXVM_MACHINE_RTC_SEED_BYTES);
    if (status != LIB_STATUS_OK) return status;
    lib_memory_copy(display->cmos[display->active], old_cmos, sizeof(old_cmos));
    display->present[display->active] = LIB_TRUE;
    display->dirty[display->active] = LIB_TRUE;
    display->active = selected;
    return LIB_STATUS_OK;
}

lib_status mypc110_machine_attach_product(void *context,
    x86_nxvm_machine_construction *construction)
{
    mypc110_machine_plan *plan;
    lib_status status;
    if (context == LIB_NULL || construction == LIB_NULL || construction->profile.context == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    plan = construction->profile.context;
    status = mypc110_audio_open(&plan->audio, &plan->es488, &plan->opl2);
    if (status != LIB_STATUS_OK) return status;
    construction->profile.output_tick_budget = plan->audio.stream != LIB_NULL ?
        MYPC110_CPU_CLOCK_HZ / 50u : 0u;
    plan->product = context;
    plan->product->plan = plan;
    return LIB_STATUS_OK;
}

lib_status mypc110_machine_release_cards(mypc110_product_context *context)
{
    mypc110_machine_plan *plan;
    if (context == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    plan = context->plan;
    if (plan == LIB_NULL) return LIB_STATUS_OK;
    for (lib_u8 slot = 0u; slot < MYPC110_CARD_SLOT_COUNT; ++slot) {
        mypc110_card_slot *entry = &context->cards.slot[slot];
        lib_status status = mypc110_pcic_set_memory_card(&plan->pcic, slot,
            LIB_NULL, LIB_NULL, LIB_FALSE);
        if (status != LIB_STATUS_OK) return status;
        status = mypc110_product_card_close_pair(&entry->active_common, &entry->active_attribute);
        if (status != LIB_STATUS_OK) return status;
    }
    return LIB_STATUS_OK;
}

lib_status mypc110_machine_close_audio(mypc110_product_context *context)
{
    mypc110_machine_plan *plan;
    if (context == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    plan = context->plan;
    return plan != LIB_NULL ? mypc110_audio_close(&plan->audio) : LIB_STATUS_OK;
}

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
 * not a PC/AT facade.  Board applies the selected external configuration
 * defaults while the TFT BIOS remains their ordinary consumer and writer. */
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
    mypc110_machine_plan *plan = context;
    lib_u64 ticks;
    lib_status status;

    if (plan == LIB_NULL || plan->core == LIB_NULL) return LIB_STATUS_INVALID_STATE;
    status = x86_core_machine_get_elapsed_ticks(plan->core, &ticks);
    if (status != LIB_STATUS_OK) return status;
    return x86_f65535_capture_snapshot(plan->display_controller,
        ticks / (MYPC110_CPU_CLOCK_HZ / MYPC110_VSYNC_HZ), out_snapshot);
}

/* The board owns reset ordering.  PCIC state is controller-local, while the
 * physical socket levels remain owned by the App's slot-input contract. */
static void mypc110_machine_profile_reset(void *opaque,
    x86_nxvm_board_reset_phase phase)
{
    mypc110_machine_plan *plan = opaque;

    if (plan != LIB_NULL && phase == X86_NXVM_BOARD_RESET_BEFORE_DEVICES) {
        x86_f65535_reset(plan->display_controller);
        mypc110_pcic_reset(&plan->pcic);
        mypc110_inking_reset(&plan->inking);
        mypc110_es488_reset(&plan->es488);
        mypc110_opl2_reset(&plan->opl2);
    }
}

static void mypc110_machine_profile_system_management_active(void *opaque,
    lib_bool active)
{
    mypc110_machine_plan *plan = opaque;

    if (plan != LIB_NULL)
        mypc110_firmware_system_management_active(&plan->firmware, active);
}

static lib_bool mypc110_machine_profile_next_deadline(void *opaque,
    lib_u64 now, lib_u64 *out_tick)
{
    mypc110_machine_plan *plan = opaque;
    lib_bool valid;
    lib_u64 audio_tick;
    if (plan == LIB_NULL || out_tick == LIB_NULL) return LIB_FALSE;
    valid = mypc110_es488_next_deadline(&plan->es488, out_tick);
    /* Bound HLT progress so the one executor can drain a finite mixer. */
    if (mypc110_audio_needs_deadline(&plan->audio, now) &&
        now <= LIB_UINT64_MAX - MYPC110_CPU_CLOCK_HZ / 50u) {
        audio_tick = now + MYPC110_CPU_CLOCK_HZ / 50u;
        if (!valid || audio_tick < *out_tick) { *out_tick = audio_tick; valid = LIB_TRUE; }
    }
    return valid;
}

static void mypc110_machine_profile_advance(void *opaque,
    lib_u64 source_ticks, lib_u64 due_tick)
{
    mypc110_machine_plan *plan = opaque;
    (void)source_ticks;
    if (plan != LIB_NULL) mypc110_es488_advance(&plan->es488, due_tick);
}

static lib_status mypc110_machine_profile_create(
    const x86_nxvm_board_profile_services *services, void *opaque,
    x86_nxvm_board_profile_binding *out_binding)
{
    mypc110_machine_plan *plan = opaque;
    lib_status status;

    if (plan == LIB_NULL || services == LIB_NULL || out_binding == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    plan->inking.configuration = plan->configuration_chip;
    plan->inking.pic_master = services->pic_master;
    plan->inking.pic_slave = services->pic_slave;
    status = mypc110_firmware_profile_create(services, &plan->firmware, out_binding);
    if (status != LIB_STATUS_OK) return status;
    status = mypc110_pcic_initialize(&plan->pcic, plan->pcic_controller,
        services->pic_master, services->pic_slave);
    if (status != LIB_STATUS_OK) return status;
    status = mypc110_es488_initialize(&plan->es488, services->core, services->dma,
        services->pic_master, services->pic_slave);
    if (status != LIB_STATUS_OK) return status;
    plan->pcic.firmware = &plan->firmware;
    plan->firmware.inking = &plan->inking;
    out_binding->context = plan;
    out_binding->reset = mypc110_machine_profile_reset;
    out_binding->next_deadline = mypc110_machine_profile_next_deadline;
    out_binding->advance = mypc110_machine_profile_advance;
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
    plan->core = core;
    status = mypc110_pcic_install_routes(core, &plan->pcic);
    if (status == LIB_STATUS_OK)
        status = mypc110_firmware_install_routes(core, &plan->firmware);
    if (status == LIB_STATUS_OK)
        status = mypc110_config_install_routes(core, &plan->firmware,
            plan->configuration_chip);
    if (status == LIB_STATUS_OK)
        status = mypc110_ec_mailbox_install_routes(core, &plan->ec_mailbox);
    if (status == LIB_STATUS_OK)
        status = mypc110_ec_block_install_routes(core, &plan->ec_block);
    if (status == LIB_STATUS_OK)
        status = mypc110_inking_install_routes(core, &plan->inking, 0x15e0u);
    if (status == LIB_STATUS_OK)
        status = mypc110_font_rom_install(core, &plan->font);
    if (status == LIB_STATUS_OK)
        status = mypc110_lpt_install_routes(core, &plan->lpt);
    if (status == LIB_STATUS_OK)
        status = mypc110_es488_install_routes(core, &plan->es488);
    if (status == LIB_STATUS_OK)
        status = mypc110_opl2_install_routes(core, &plan->opl2);
    if (status == LIB_STATUS_OK)
        status = mypc110_display_install_routes(core, plan->display_controller);
    return status;
}

static void mypc110_machine_release(void *opaque)
{
    mypc110_machine_plan *plan = opaque;

    if (plan == LIB_NULL) return;
    if (mypc110_audio_close(&plan->audio) != LIB_STATUS_OK) return;
    if (plan->product != LIB_NULL) plan->product->plan = LIB_NULL;
    x86_f65535_destroy(plan->display_controller);
    x86_rb5c396_destroy(plan->pcic_controller);
    x86_vl82c420_destroy(plan->configuration_chip);
    mypc110_firmware_destroy(&plan->firmware);
    lib_release(plan);
}

static lib_status mypc110_machine_audio_before_write(void *opaque, lib_u64 tick)
{ return mypc110_audio_advance(opaque, tick); }

static lib_status mypc110_machine_output_pump(void *opaque)
{
    mypc110_machine_plan *plan = opaque;
    lib_u64 tick = 0u;
    lib_status status = x86_core_machine_get_elapsed_ticks(plan->core, &tick);
    return status == LIB_STATUS_OK ? mypc110_audio_pump(&plan->audio, tick) : status;
}

static lib_status mypc110_machine_output_wait(void *opaque)
{ return mypc110_audio_wait(&((mypc110_machine_plan *)opaque)->audio); }

static lib_status mypc110_machine_output_active(void *opaque, lib_bool enabled)
{ return mypc110_audio_active(&((mypc110_machine_plan *)opaque)->audio, enabled); }

static void mypc110_machine_output_interrupt(void *opaque)
{ mypc110_audio_interrupt(&((mypc110_machine_plan *)opaque)->audio); }

static lib_bool mypc110_machine_handle_input(void *opaque, const kvm_input_event *event)
{
    mypc110_machine_plan *plan = opaque;

    if (event->type == KVM_EVENT_MOUSE) {
        const lib_i64 x = event->data.mouse.relative ?
            (lib_i64)plan->pen_x + event->data.mouse.delta_x : event->data.mouse.absolute_x;
        const lib_i64 y = event->data.mouse.relative ?
            (lib_i64)plan->pen_y + event->data.mouse.delta_y : event->data.mouse.absolute_y;
        plan->pen_x = x < 0 ? 0 : x > 255 ? 255 : (lib_i32)x;
        plan->pen_y = y < 0 ? 0 : y > 255 ? 255 : (lib_i32)y;
        if ((plan->inking.control & 0xfeu) == 0x38u) {
            const lib_status status = mypc110_inking_sample(&plan->inking,
                (lib_u8)plan->pen_x, (lib_u8)plan->pen_y,
                (event->data.mouse.buttons & BASE_MOUSE_BUTTON_LEFT) != 0u);
            if (status != LIB_STATUS_OK && status != LIB_STATUS_INVALID_STATE)
                (void)x86_core_machine_report_fault(plan->firmware.machine,
                    MYPC110_INKING_INPUT_FAULT | (lib_u32)status);
        }
        /* The pointing stick and signature pad remain separate guest devices. */
        return LIB_FALSE;
    }
    if (event->type != KVM_EVENT_KEY || event->data.key.key != BASE_KEY_F12)
        return LIB_FALSE;
    const lib_status status = mypc110_pcic_set_resume_button(&plan->pcic,
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
    x86_nxvm_machine_construction candidate = {0};
    x86_nxvm_machine_profile_binding profile;
    x86_nxvm_machine_assets generic_assets;
    lib_status status;

    if (out_construction == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_construction = (x86_nxvm_machine_construction){0};
    if (config == LIB_NULL || assets == LIB_NULL || assets->bios[0u].data == LIB_NULL ||
        assets->bios[0u].bytes != MYPC110_FIRMWARE_BYTES ||
        assets->font.data == LIB_NULL || assets->font.bytes != MYPC110_FONT_ROM_BYTES ||
        (config->memory_bytes != 0u &&
            config->memory_bytes != mypc110_machine_configuration.memory_bytes)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    plan = lib_allocate_zero(1u, sizeof(*plan));
    if (plan == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    plan->audio.device = &plan->es488;
    plan->audio.opl2 = &plan->opl2;
    plan->opl2.output_context = &plan->audio;
    plan->opl2.before_write = mypc110_machine_audio_before_write;
    lib_atomic_i32_initialize(&plan->audio.interrupt_failure, LIB_STATUS_OK);
    plan->pen_x = 128;
    plan->pen_y = 64;
    profile = (x86_nxvm_machine_profile_binding) {
        .context = plan, .configure = mypc110_machine_configure,
        .after_firmware = mypc110_machine_after_firmware,
        .before_reset = mypc110_machine_before_reset,
        .release = mypc110_machine_release,
        .handle_input = mypc110_machine_handle_input,
        .output_pump = mypc110_machine_output_pump,
        .output_wait = mypc110_machine_output_wait,
        .output_active = mypc110_machine_output_active,
        .output_interrupt = mypc110_machine_output_interrupt};
    status = mypc110_firmware_create(assets->bios[0u].data, assets->bios[0u].bytes,
        &plan->firmware);
    if (status == LIB_STATUS_OK)
        status = mypc110_font_rom_create(assets->font.data, assets->font.bytes, &plan->font);
    if (status == LIB_STATUS_OK) status = x86_vl82c420_create(&plan->configuration_chip);
    if (status == LIB_STATUS_OK)
        status = mypc110_config_initialize_cold_image(plan->configuration_chip);
    if (status == LIB_STATUS_OK) status = x86_f65535_create(&plan->display_controller);
    if (status == LIB_STATUS_OK) status = x86_rb5c396_create(&plan->pcic_controller);
    candidate = (x86_nxvm_machine_construction) {
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
        .display_snapshot_context = plan,
        .firmware_provider = mypc110_firmware_provider(),
        .firmware_context = &plan->firmware,
        .profile = profile};
    /* PC110's font slot is its 1 MiB Kanji ROM, not the optional generic
     * 8 KiB text generator.  The App-local font owner already consumed it. */
    generic_assets = *assets;
    generic_assets.font = (x86_nxvm_machine_asset_bytes) {LIB_NULL, 0u};
    return x86_nxvm_machine_construction_finish(&candidate, config, &generic_assets,
        status, out_construction);
}
