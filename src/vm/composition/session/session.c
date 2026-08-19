#include "type.h"

#include "vm/composition/session/session.h"

#include "core/machine/machine_interface.h"
#include "vm/composition/session/control.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/display.h"
#include "vm/composition/session/media.h"
#include "vm/machine/fdd.h"
#include "vm/machine/hdd.h"
#include "vm/profile/default_profile/firmware/bios.h"
#include "vm/profile/default_profile/keyboard_mapper.h"
#include "vm/profile/default_profile/mouse_mapper.h"

C_VOID vm_session_consume_request(
    C_VOID *opaque, const vm_platform_request *request)
{
    vm_session *session = (vm_session *)opaque;

    if (session == STD_NULL || !session->active || request == STD_NULL) return;
    if (request->kind == VM_PLATFORM_REQUEST_KEY_EVENT) {
        vm_profile_default_keyboard_sequence sequence;
        type_unsigned_8 native_scan_set;

        if (core_machine_keyboard_get_native_scan_set(session->core_machine,
                &native_scan_set) == TYPE_STATUS_OK &&
            vm_profile_default_keyboard_map_host_key_for_scan_set(
                request->data.key_event.scan_code,
                request->data.key_event.virtual_key, request->data.key_event.pressed,
                native_scan_set, &sequence) ==
            TYPE_STATUS_OK) {
            (C_VOID)core_machine_keyboard_receive_native_bytes(session->core_machine,
                sequence.bytes, sequence.count);
        }
    } else if (request->kind == VM_PLATFORM_REQUEST_MOUSE_EVENT) {
        vm_profile_default_mouse_report report;

        if (vm_profile_default_mouse_map_host_relative(
                request->data.mouse_event.delta_x,
                request->data.mouse_event.delta_y,
                request->data.mouse_event.buttons, &report) == TYPE_STATUS_OK) {
            (C_VOID)core_machine_mouse_receive_relative(session->core_machine,
                report.delta_x, report.delta_y, report.buttons);
        }
    }
}

static type_unsigned_16 vm_session_read_u16(const C_VOID *source)
{
    type_unsigned_16 value;

    STD_MEMCPY(&value, source, sizeof(value));
    return value;
}

static const vm_profile_default_pc_at_descriptor *
vm_session_profile_select(vm_session_profile_kind kind)
{
    if (kind == VM_SESSION_PROFILE_DEFAULT_PC_AT) {
        return vm_profile_default_pc_at_descriptor_get();
    }
    if (kind == VM_SESSION_PROFILE_IBM_5170_MODEL_339) {
        return vm_profile_ibm_5170_model_339_descriptor_get();
    }
    return STD_NULL;
}

const C_CHAR *vm_session_profile_name(vm_session_profile_kind kind)
{
    if (kind == VM_SESSION_PROFILE_DEFAULT_PC_AT) return "default-pc-at";
    if (kind == VM_SESSION_PROFILE_IBM_5170_MODEL_339) {
        return "ibm-5170-model-339";
    }
    if (kind == VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40) {
        return "compaq-deskpro-386-model-40";
    }
    return "unknown";
}

static C_INT vm_session_copy_path(C_CHAR *destination, STD_SIZE_T capacity,
    const C_CHAR *source)
{
    STD_SIZE_T length;

    if (destination == STD_NULL || capacity == 0u) return 0;
    destination[0] = '\0';
    if (source == STD_NULL) return 1;
    length = STD_STRLEN(source);
    if (length >= capacity) return 0;
    STD_MEMCPY(destination, source, length + 1u);
    return 1;
}

type_status vm_session_submit_host_input(vm_session *session,
    const core_platform_input_event *event)
{
    if (session == STD_NULL || !session->active) return TYPE_STATUS_INVALID_STATE;
    if (event == STD_NULL || (event->kind != CORE_PLATFORM_INPUT_KEY &&
        event->kind != CORE_PLATFORM_INPUT_RELATIVE_MOUSE)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    return core_platform_input_source_submit(&session->input_source, event);
}

static C_INT vm_session_materialize_profile_core_config(vm_session *session,
    const vm_profile_default_pc_at_cpu_contract *contract)
{
    const vm_profile_default_pc_at_descriptor *profile;

    if (session == STD_NULL || session->profile == STD_NULL ||
        contract == STD_NULL) return 0;
    profile = session->profile;
    session->core_machine_config = (core_machine_config) {
        .memory_bytes = profile->default_memory_bytes,
        .cpu_profile = contract->cpu_profile,
        .fpu_profile = contract->fpu_profile,
        .ticks_per_instruction = contract->ticks_per_instruction,
        .instruction_timing = contract->instruction_timing,
        .clock_plan = contract->clock_plan,
        .kbc_typematic_initial_ticks = contract->kbc_typematic_initial_ticks,
        .kbc_typematic_repeat_ticks = contract->kbc_typematic_repeat_ticks,
        .kbc_command_response_ticks = contract->kbc_command_response_ticks
    };
    return 1;
}

static const vm_profile_default_pc_at_port_leaf *
vm_session_profile_port_leaf(const vm_profile_default_pc_at_descriptor *profile,
    vm_profile_default_pc_at_device_role device, STD_SIZE_T ordinal)
{
    return vm_profile_default_pc_at_port_leaf_at(profile, device, ordinal);
}

static C_VOID vm_session_apply_core_config_overrides(vm_session *session,
    const vm_session_config *config)
{
    if (session == STD_NULL || config == STD_NULL) return;
    session->core_machine_config.memory_bytes = config->memory_bytes;
}

static C_INT vm_session_cpu_contract_select(const vm_session *session,
    const vm_session_config *config,
    vm_profile_default_pc_at_cpu_contract *out_contract)
{
    core_machine_cpu_profile cpu_profile;
    core_machine_fpu_profile fpu_profile;

    if (session == STD_NULL || session->profile == STD_NULL ||
        out_contract == STD_NULL) return 0;
    cpu_profile = session->profile->cpu_profile;
    fpu_profile = session->profile->fpu_profile;
    if (config != STD_NULL &&
        session->profile == vm_profile_default_pc_at_descriptor_get()) {
        cpu_profile = config->cpu_profile;
        fpu_profile = config->fpu_profile;
    }
    return vm_profile_default_pc_at_cpu_contract_select(session->profile,
        cpu_profile, fpu_profile, out_contract);
}

static C_VOID vm_session_storage_rollback(vm_session *machine)
{
    if (machine == STD_NULL) return;
    core_machine_display_provider_slot_finalize(&machine->display_provider);
    core_machine_destroy(machine->core_machine);
    machine->core_machine = STD_NULL;
}

C_INT vm_session_insert_fdd(vm_session *session, const C_CHAR *path)
{
    C_CHAR candidate[sizeof(session->fdd_image_path)];

    if (session == STD_NULL || vm_session_control_is_running(&session->control) ||
        !vm_session_copy_path(candidate, sizeof(candidate), path) ||
        vm_machine_fdd_insert_for(&session->fdd, candidate) != 0 ||
        !vm_session_copy_path(session->fdd_image_path, sizeof(session->fdd_image_path),
            candidate)) return -1;
    session->retained_config.fdd_image = session->fdd_image_path;
    vm_session_apply_boot_preference(session);
    return 0;
}

C_INT vm_session_remove_fdd(vm_session *session, const C_CHAR *path)
{
    if (session == STD_NULL || vm_session_control_is_running(&session->control) ||
        vm_machine_fdd_remove_for(&session->fdd, path) != 0) return -1;
    session->fdd_image_path[0] = '\0';
    session->retained_config.fdd_image = STD_NULL;
    vm_session_apply_boot_preference(session);
    return 0;
}
static C_INT vm_session_insert_hdd_at_startup(vm_session *session,
    const C_CHAR *path)
{
    C_CHAR candidate[sizeof(session->hdd_image_path)];

    if (session == STD_NULL || session->model40_private || session->profile == STD_NULL ||
        !session->profile->hdc_present || !vm_session_copy_path(candidate,
            sizeof(candidate), path) || vm_machine_hdd_insert(&session->hdd, candidate) != 0 ||
        !vm_session_copy_path(session->hdd_image_path, sizeof(session->hdd_image_path),
            candidate)) return -1;
    session->retained_config.hdd_image = session->hdd_image_path;
    vm_session_apply_boot_preference(session);
    return 0;
}

C_INT vm_session_insert_hdd(vm_session *session, const C_CHAR *path)
{
    (C_VOID)session;
    (C_VOID)path;
    return -1;
}
C_VOID vm_session_apply_boot_preference(vm_session *session)
{
    C_INT boot_hdd;

    if (session == STD_NULL || session->profile == STD_NULL) return;
    if (!session->profile->hdc_present) {
        vm_profile_default_bios_set_boot_hdd(&session->default_bios, 0);
        return;
    }
    boot_hdd = session->boot_preference == VM_SESSION_BOOT_PREFERENCE_HDD ||
        (session->boot_preference == VM_SESSION_BOOT_PREFERENCE_AUTO &&
            !session->fdd.connect.flagDiskExist && session->hdd.connect.flagDiskExist);
    vm_profile_default_bios_set_boot_hdd(&session->default_bios, boot_hdd);
}

C_VOID vm_session_set_boot_hdd(vm_session *session, C_INT enabled)
{
    if (session == STD_NULL || session->profile == STD_NULL ||
        !session->profile->hdc_present) return;
    session->boot_preference = enabled ? VM_SESSION_BOOT_PREFERENCE_HDD :
        VM_SESSION_BOOT_PREFERENCE_FDD;
    session->retained_config.boot_hdd = enabled != 0;
    vm_session_apply_boot_preference(session);
}



type_status vm_session_storage_initialize(vm_session *machine)
{
    const vm_profile_default_pc_at_port_leaf *attribute_first;
    const vm_profile_default_pc_at_port_leaf *attribute_last;
    const vm_profile_default_pc_at_port_leaf *sequencer_first;
    const vm_profile_default_pc_at_port_leaf *sequencer_last;
    const vm_profile_default_pc_at_port_leaf *graphics_first;
    const vm_profile_default_pc_at_port_leaf *graphics_last;
    const vm_profile_default_pc_at_port_leaf *crtc_first;
    const vm_profile_default_pc_at_port_leaf *crtc_last;
    const vm_profile_default_pc_at_port_leaf *cmos_first;
    const vm_profile_default_pc_at_port_leaf *cmos_last;
    const vm_profile_default_pc_at_route *cmos_route;
    const vm_profile_default_pc_at_route *fdc_route;
    core_machine_display_config display_config = {0};
    core_machine_dma_wiring dma_wiring = {0};
    core_machine_rtc_cmos_config rtc_cmos_config = {0};
    type_status status;

    if (machine == STD_NULL || machine->core_machine != STD_NULL) {
        return TYPE_STATUS_INVALID_STATE;
    }
    if (machine->model40_private) {
        return vm_session_model40_storage_initialize(machine);
    }
    if (machine->profile == STD_NULL) {
        machine->profile = vm_profile_default_pc_at_descriptor_get();
    }
    if (!vm_profile_default_pc_at_descriptor_is_valid(machine->profile)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    status = core_machine_create(&machine->core_machine_config,
        &machine->core_machine);
    if (status != TYPE_STATUS_OK) return status;
    if (machine->profile->unpopulated_extended_memory) {
        const core_machine_absent_memory_config absent_memory = {
            0x00100000u, 0x00f00000u, 0xffu
        };

        status = core_machine_configure_absent_memory(machine->core_machine,
            &absent_memory);
        if (status != TYPE_STATUS_OK) {
            vm_session_storage_rollback(machine);
            return status;
        }
    }
    if (machine->profile->planar_parity_present) {
        const core_machine_planar_parity_config parity = {
            CORE_MACHINE_PC_AT_PORT_B, machine->profile->default_memory_bytes
        };

        status = core_machine_configure_planar_parity(machine->core_machine,
            &parity);
        if (status != TYPE_STATUS_OK) {
            vm_session_storage_rollback(machine);
            return status;
        }
    }
    core_machine_display_provider_slot_initialize(&machine->display_provider);
    vm_session_bind_display(machine);
    attribute_first = vm_session_profile_port_leaf(machine->profile,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_ATTRIBUTE, 0u);
    attribute_last = vm_session_profile_port_leaf(machine->profile,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_ATTRIBUTE, 1u);
    sequencer_first = vm_session_profile_port_leaf(machine->profile,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_SEQUENCER, 0u);
    sequencer_last = vm_session_profile_port_leaf(machine->profile,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_SEQUENCER, 1u);
    graphics_first = vm_session_profile_port_leaf(machine->profile,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_GRAPHICS, 0u);
    graphics_last = vm_session_profile_port_leaf(machine->profile,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_GRAPHICS, 1u);
    crtc_first = vm_session_profile_port_leaf(machine->profile,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP, 0u);
    crtc_last = vm_session_profile_port_leaf(machine->profile,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP, 4u);
    cmos_first = vm_session_profile_port_leaf(machine->profile,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_CMOS, 0u);
    cmos_last = vm_session_profile_port_leaf(machine->profile,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_CMOS, 1u);
    cmos_route = vm_profile_default_pc_at_route_find(machine->profile,
        VM_PROFILE_DEFAULT_PC_AT_ROUTE_CMOS_IRQ8);
    fdc_route = vm_profile_default_pc_at_route_find(machine->profile,
        VM_PROFILE_DEFAULT_PC_AT_ROUTE_FDC_IRQ6_DMA2);
    if ((machine->profile->ega_present && (attribute_first == STD_NULL ||
        attribute_last == STD_NULL || sequencer_first == STD_NULL ||
        sequencer_last == STD_NULL || graphics_first == STD_NULL ||
        graphics_last == STD_NULL)) || crtc_first == STD_NULL ||
        crtc_last == STD_NULL || cmos_first == STD_NULL ||
        cmos_last == STD_NULL || cmos_route == STD_NULL || fdc_route == STD_NULL) {
        vm_session_storage_rollback(machine);
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    display_config.text_timing = machine->profile->cga_text_timing;
    display_config.cga_vram_present = machine->profile->cga_vram_present;
    display_config.ega_present = machine->profile->ega_present;
    display_config.ega_sequencer = machine->profile->ega_sequencer;
    display_config.ega_controllers = machine->profile->ega_controllers;
    if (machine->profile->ega_present) {
        display_config.ports.attribute_first = attribute_first->port;
        display_config.ports.attribute_last = attribute_last->port;
        display_config.ports.sequencer_first = sequencer_first->port;
        display_config.ports.sequencer_last = sequencer_last->port;
        display_config.ports.graphics_first = graphics_first->port;
        display_config.ports.graphics_last = graphics_last->port;
    }
    display_config.ports.crtc_first = crtc_first->port;
    display_config.ports.crtc_last = crtc_last->port;
    display_config.provider = &machine->display_provider;
    dma_wiring.fdc_channel = fdc_route->dma_channel;
    dma_wiring.controller_count = CORE_MACHINE_DMA_CONTROLLER_COUNT;
    dma_wiring.cascade_channel = CORE_MACHINE_DMA_CASCADE_CHANNEL;
    rtc_cmos_config.index_port = cmos_first->port;
    rtc_cmos_config.data_port = cmos_last->port;
    rtc_cmos_config.irq = cmos_route->irq;
    rtc_cmos_config.nmi_mask_bit = 0x80u;
    rtc_cmos_config.ticks_per_second = machine->profile->rtc_ticks_per_second;
    rtc_cmos_config.defaults[0].index = CORE_MACHINE_RTC_TYPE_DISK_FLOPPY;
    rtc_cmos_config.defaults[0].value = machine->profile->cmos.floppy_type;
    rtc_cmos_config.defaults[1].index = CORE_MACHINE_RTC_TYPE_DISK_FIXED;
    rtc_cmos_config.defaults[1].value = machine->profile->cmos.fixed_disk_type;
    rtc_cmos_config.defaults[2].index = CORE_MACHINE_RTC_TYPE_DISK_FIXED_EXTENDED_0;
    rtc_cmos_config.defaults[2].value = machine->profile->cmos.fixed_disk_type_extended_0;
    rtc_cmos_config.defaults[3].index = CORE_MACHINE_RTC_EQUIPMENT;
    rtc_cmos_config.defaults[3].value = machine->profile->cmos.equipment;
    rtc_cmos_config.defaults[4].index = CORE_MACHINE_RTC_BASEMEM_LSB;
    rtc_cmos_config.defaults[4].value = TYPE_MASK_UNSIGNED_8(
        machine->profile->cmos.base_memory_kib);
    rtc_cmos_config.defaults[5].index = CORE_MACHINE_RTC_BASEMEM_MSB;
    rtc_cmos_config.defaults[5].value = TYPE_MASK_UNSIGNED_8(
        machine->profile->cmos.base_memory_kib >> 8);
    rtc_cmos_config.default_count = CORE_MACHINE_RTC_DEFAULT_COUNT;
    status = core_machine_configure_display(machine->core_machine, &display_config);
    if (status == TYPE_STATUS_OK) {
        status = core_machine_configure_dma(machine->core_machine, &dma_wiring,
            &machine->fdc_dma_request);
    }
    if (status == TYPE_STATUS_OK) {
        status = core_machine_configure_rtc_cmos(machine->core_machine,
            &rtc_cmos_config);
    }
    if (status != TYPE_STATUS_OK) {
        vm_session_storage_rollback(machine);
        return status;
    }
    core_machine_media_registry_initialize(&machine->media_registry);
    vm_profile_default_context_initialize(&machine->default_profile_context,
        &machine->default_bios, &machine->media_registry, VM_SESSION_MEDIA_HDD_ID,
        machine->profile->firmware_slot);
    core_platform_presentation_mailbox_initialize(&machine->presentation_mailbox);
    core_product_debug_context_initialize(&machine->debugger_context);
    machine->display_generation = 0u;
    return TYPE_STATUS_OK;
}

C_VOID vm_session_storage_finalize(vm_session *machine)
{
    if (machine == STD_NULL || machine->core_machine == STD_NULL) return;
    core_platform_presentation_mailbox_finalize(&machine->presentation_mailbox);
    core_machine_media_registry_finalize(&machine->media_registry);
    core_machine_display_provider_slot_finalize(&machine->display_provider);
    core_machine_destroy(machine->core_machine);
    machine->core_machine = STD_NULL;
}

static type_status vm_session_create_model40_byob(const vm_session_config *config,
    vm_session **out_session)
{
    vm_session *session;
    type_status status;

    if (config == STD_NULL || out_session == STD_NULL ||
        !vm_profile_model40_byob_manifest_is_valid(&config->model40_firmware)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    *out_session = STD_NULL;
    session = (vm_session *)STD_CALLOC(1u, sizeof(*session));
    if (session == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    session->model40_private = 1;
    session->floppy_kind = VM_PROFILE_FLOPPY_525_1200K;
    session->core_machine_config = (core_machine_config) {
        .memory_bytes = 1024u * 1024u,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
        .cpu_80386_cr_mov_ignores_mod = TYPE_TRUE,
        .a20_wrap_policy = CORE_MACHINE_A20_WRAP_FIRST_TO_SECOND_MIB,
        .ticks_per_instruction = 1u,
        .instruction_timing = {1u, 0u, 0u, 0u, 0u, 0u},
        /* D4 selects 2 KiB miss/hit policy; Core requires explicit prefetch overlap,
         * not an inferred D4 row/bank PAL waveform. */        .external_cycle_timing = {.page_bytes = 2048u, .page_miss_ticks = 2u,
            .page_hit_ticks = 0u,
            .overlap_policy = CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_EXPLICIT_SEQUENTIAL,
            .first_eligible_address = 0x00000000u,
            .last_eligible_address = 0x0009ffffu},        /* Generic-AT virtual-time wait skeleton. Original CECG material only
         * establishes this 8-bit port surface, not NOWS/BUSRDY behavior. */
        .external_access_wait_windows = {
            {CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x03b4u, 0x03bau, 1u},
            {CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x03c0u, 0x03cfu, 1u},
            {CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x03d4u, 0x03dcu, 1u},
            {CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x07c6u, 0x07c6u, 1u},
            {CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x0bc6u, 0x0bc6u, 1u},
            {CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x0fc6u, 0x0fc6u, 1u},
            {CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, 0x000a0000u,
                0x000affffu, 1u}},
        .retirement_time_contract = CORE_MACHINE_RETIREMENT_TIME_DETERMINISTIC,
        .kbc_serial_delivery_ticks = 1u, /* Generic-AT virtual-time cadence, not a Compaq measurement. */
        .clock_plan = {{1u, 1u, 0u}, {1u, 1u, 0u}, {1u, 1u, 0u},
            {1u, 1u, 0u}, {1u, 1u, 0u}, {1u, 1u, 0u}},
        /* D3PE: one DCLK wait each DMA cycle; discrete bridge, not 250 ns. */
        .dma_cycle_wait_quanta = 1u,
        /* D3PE: BUSRDY may extend a DMA cycle; reset level is ready. */
        .dma_cycle_bus_ready_gate_enabled = TYPE_TRUE,
        .cpu_cycle_bus_ready_gate_enabled = TYPE_TRUE,
        .cpu_prefetch_reservation_enabled = TYPE_TRUE,
        .auxiliary_pit_present = TYPE_TRUE,
        .auxiliary_pit_base_port = 0x0048u,
        .kbc_aux_absent = TYPE_TRUE
    };
    status = vm_profile_model40_byob_manifest_load(&config->model40_firmware,
        session->model40_even_rom, session->model40_odd_rom, &session->model40_rom);
    if (status != TYPE_STATUS_OK) { STD_FREE(session); return status; }
    session->retained_config = *config;
    STD_MEMSET(&session->retained_config.model40_firmware, 0,
        sizeof(session->retained_config.model40_firmware));
    status = vm_session_initialize(session);
    if (status != TYPE_STATUS_OK) { STD_FREE(session); return status; }
    if ((config->fdd_image != STD_NULL && vm_session_insert_fdd(session, config->fdd_image)) ||
        (config->hdd_image != STD_NULL && vm_session_model40_insert_hdd_at_startup(session, config->hdd_image))) {
        vm_session_destroy(session);
        return TYPE_STATUS_FAULT;
    }
    if (config->boot_hdd) session->boot_preference = VM_SESSION_BOOT_PREFERENCE_HDD;
    vm_session_apply_boot_preference(session);
    vm_session_control_reset(&session->control);
    *out_session = session;
    return TYPE_STATUS_OK;
}
type_status vm_session_create_model40_private(
    const vm_profile_model40_external_rom *rom, vm_session **out_session)
{
    vm_session *session;
    type_status status;

    if (out_session == STD_NULL || !vm_profile_model40_external_rom_is_valid(rom)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    *out_session = STD_NULL;
    session = (vm_session *)STD_CALLOC(1u, sizeof(*session));
    if (session == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    session->model40_private = 1;
    session->floppy_kind = VM_PROFILE_FLOPPY_525_1200K;
    session->model40_rom = *rom;
    session->core_machine_config = (core_machine_config) {
        .memory_bytes = 1024u * 1024u,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
        .cpu_80386_cr_mov_ignores_mod = TYPE_TRUE,
        .a20_wrap_policy = CORE_MACHINE_A20_WRAP_FIRST_TO_SECOND_MIB,
        .ticks_per_instruction = 1u,
        .instruction_timing = {1u, 0u, 0u, 0u, 0u, 0u},
        /* D4 selects 2 KiB miss/hit policy; Core requires explicit prefetch overlap,
         * not an inferred D4 row/bank PAL waveform. */        .external_cycle_timing = {.page_bytes = 2048u, .page_miss_ticks = 2u,
            .page_hit_ticks = 0u,
            .overlap_policy = CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_EXPLICIT_SEQUENTIAL,
            .first_eligible_address = 0x00000000u,
            .last_eligible_address = 0x0009ffffu},        /* Generic-AT virtual-time wait skeleton. Original CECG material only
         * establishes this 8-bit port surface, not NOWS/BUSRDY behavior. */
        .external_access_wait_windows = {
            {CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x03b4u, 0x03bau, 1u},
            {CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x03c0u, 0x03cfu, 1u},
            {CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x03d4u, 0x03dcu, 1u},
            {CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x07c6u, 0x07c6u, 1u},
            {CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x0bc6u, 0x0bc6u, 1u},
            {CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x0fc6u, 0x0fc6u, 1u},
            {CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, 0x000a0000u,
                0x000affffu, 1u}},
        .retirement_time_contract = CORE_MACHINE_RETIREMENT_TIME_DETERMINISTIC,
        .kbc_serial_delivery_ticks = 1u, /* Generic-AT virtual-time cadence, not a Compaq measurement. */
        .clock_plan = {{1u, 1u, 0u}, {1u, 1u, 0u}, {1u, 1u, 0u},
            {1u, 1u, 0u}, {1u, 1u, 0u}, {1u, 1u, 0u}},
        /* D3PE: one DCLK wait each DMA cycle; discrete bridge, not 250 ns. */
        .dma_cycle_wait_quanta = 1u,
        /* D3PE: BUSRDY may extend a DMA cycle; reset level is ready. */
        .dma_cycle_bus_ready_gate_enabled = TYPE_TRUE,
        .cpu_cycle_bus_ready_gate_enabled = TYPE_TRUE,
        .cpu_prefetch_reservation_enabled = TYPE_TRUE,
        .auxiliary_pit_present = TYPE_TRUE,
        .auxiliary_pit_base_port = 0x0048u,
        .kbc_aux_absent = TYPE_TRUE
    };
    status = vm_session_initialize(session);
    if (status != TYPE_STATUS_OK) { STD_FREE(session); return status; }
    vm_session_control_reset(&session->control);
    *out_session = session;
    return TYPE_STATUS_OK;
}

C_INT vm_session_create(const vm_session_config *config, vm_session **out_session)
{
    vm_session *session;
    vm_profile_default_pc_at_cpu_contract cpu_contract;

    if (config != STD_NULL && config->profile_kind == VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40) {
        return vm_session_create_model40_byob(config, out_session);
    }
    if (out_session == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_session = STD_NULL;
    session = (vm_session *)STD_CALLOC(1u, sizeof(*session));
    if (session == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    session->profile = vm_session_profile_select(config == STD_NULL ?
        VM_SESSION_PROFILE_DEFAULT_PC_AT : config->profile_kind);
    if (session->profile == STD_NULL) {
        STD_FREE(session);
        return TYPE_STATUS_FAULT;
    }
    if (!vm_session_cpu_contract_select(session, config, &cpu_contract)) {
        STD_FREE(session);
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (!vm_session_materialize_profile_core_config(session, &cpu_contract)) {
        STD_FREE(session);
        return TYPE_STATUS_FAULT;
    }
    if (config != STD_NULL) {
        if (config->virtual_time_source != STD_NULL &&
            config->virtual_time_source->next == STD_NULL) {
            STD_FREE(session);
            return TYPE_STATUS_INVALID_ARGUMENT;
        }
        session->retained_config = *config;
        if (config->virtual_time_source != STD_NULL) {
            session->virtual_time_source = *config->virtual_time_source;
            session->retained_config.virtual_time_source =
                &session->virtual_time_source;
        }
        if (!session->profile->hdc_present && (config->hdd_image != STD_NULL ||
            config->create_hdd_cylinders != 0u || config->boot_hdd)) {
            STD_FREE(session);
            return TYPE_STATUS_INVALID_ARGUMENT;
        }
        if (session->profile == vm_profile_default_pc_at_descriptor_get()) {
            vm_session_apply_core_config_overrides(session, config);
        }
    }
    if (session->profile == vm_profile_ibm_5170_model_339_descriptor_get() &&
        session->virtual_time_source.next == STD_NULL &&
        vm_platform_virtual_time_source_initialize(
            &session->model_339_virtual_time_source, 8000000u,
            &session->virtual_time_source) != TYPE_STATUS_OK) {
        STD_FREE(session);
        return TYPE_STATUS_FAULT;
    }
    {
        type_status status = vm_session_initialize(session);

        if (status != TYPE_STATUS_OK) {
            STD_FREE(session);
            return status;
        }
    }
    if (session->core_machine == STD_NULL) {
        STD_FREE(session);
        return TYPE_STATUS_FAULT;
    }
    if (config != STD_NULL &&
        ((config->fdd_image != STD_NULL && vm_session_insert_fdd(session,
            config->fdd_image)) ||
         (config->hdd_image != STD_NULL && vm_session_insert_hdd_at_startup(session, config->hdd_image)))) {
        vm_session_destroy(session);
        return TYPE_STATUS_FAULT;
    }
    if (config != STD_NULL && config->create_fdd) vm_machine_fdd_create_for(&session->fdd);
    if (config != STD_NULL && session->profile->hdc_present &&
        config->create_hdd_cylinders != 0u) {
        vm_machine_hdd_create(&session->hdd, config->create_hdd_cylinders);
    }
    if (config != STD_NULL && session->profile->hdc_present && config->boot_hdd) {
        session->boot_preference = VM_SESSION_BOOT_PREFERENCE_HDD;
    }
    vm_session_apply_boot_preference(session);
    vm_session_control_reset(&session->control);
    *out_session = session;
    return TYPE_STATUS_OK;
}

type_status vm_session_reconfigure_memory(vm_session *session,
    STD_SIZE_T memory_bytes)
{
    if (session == STD_NULL ||
        vm_session_control_is_running(&session->control) ||
        vm_platform_run_handle_is_active(&session->platform_run_handle)) {
        return TYPE_STATUS_INVALID_STATE;
    }
    if (core_machine_reconfigure_memory(session->core_machine, memory_bytes) !=
        TYPE_STATUS_OK) return TYPE_STATUS_INVALID_STATE;
    session->retained_config.memory_bytes = memory_bytes;
    session->core_machine_config.memory_bytes = memory_bytes;
    vm_machine_debug_reset(&session->debug);
    vm_session_publish_display(session, 1);
    return TYPE_STATUS_OK;
}

C_VOID vm_session_destroy(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_session_finalize(session);
    STD_FREE(session);
}

type_status vm_session_get_reset_vector(const vm_session *session,
    vm_session_reset_vector *out_vector)
{
    core_machine_observation observation;

    if (session == STD_NULL || session->core_machine == STD_NULL ||
        out_vector == STD_NULL) return TYPE_STATUS_INVALID_STATE;
    if (core_machine_capture_observation(session->core_machine, &observation) !=
        TYPE_STATUS_OK) return TYPE_STATUS_INVALID_STATE;
    out_vector->cs = observation.cpu.cs;
    out_vector->ip = (type_unsigned_16)observation.cpu.eip;
    return TYPE_STATUS_OK;
}
