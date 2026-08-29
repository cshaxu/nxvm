#include "vm/composition/session/model40_composition.h"

#include "vm/composition/session/display.h"
#include "vm/composition/session/media.h"
#include "vm/composition/session/session_private.h"

static C_VOID vm_session_model40_capture_fdc_terminal(C_VOID *opaque,
    const core_machine_fdc_terminal_observation *observation)
{
    vm_session *session = (vm_session *)opaque;

    if (session == STD_NULL || observation == STD_NULL) return;
    session->model40_fdc_terminal_observation = *observation;
    session->model40_fdc_terminal_observation_valid = TYPE_TRUE;
}

static C_VOID vm_session_model40_storage_rollback(vm_session *session)
{
    if (session == STD_NULL) return;
    core_platform_presentation_mailbox_destroy(session->presentation_mailbox);
    session->presentation_mailbox = STD_NULL;
    core_product_debugger_destroy(session->debugger);
    session->debugger = STD_NULL;
    core_machine_destroy(session->core_machine);
    session->core_machine = STD_NULL;
    core_machine_display_provider_slot_destroy(session->display_provider);
    session->display_provider = STD_NULL;
    core_machine_media_registry_destroy(session->media_registry);
    session->media_registry = STD_NULL;
    core_machine_plan_destroy(session->core_machine_plan);
    session->core_machine_plan = STD_NULL;
}

static type_status vm_session_model40_materialize_controllers(vm_session *session,
    core_machine_plan *plan)
{
    const core_machine_fdc_drive_bindings drives = {
        .media_id = { VM_SESSION_MEDIA_FDD_ID, CORE_MACHINE_MEDIA_ID_INVALID,
            CORE_MACHINE_MEDIA_ID_INVALID, CORE_MACHINE_MEDIA_ID_INVALID },
        .installed_mask = 0x03u,
        .double_sided_mask = 0x03u,
        .track_zero_active_low_mask = 0x03u,
        .cylinder_count = {80u, 80u, 0u, 0u}
    };
    core_machine_fdc_config fdc = {0};
    core_machine_hdc_config hdc = {0};

    if (session == STD_NULL || plan == STD_NULL || !session->model40_private) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    fdc = (core_machine_fdc_config) { 0x03f2u, 0x03f4u, 0x03f5u,
        0x03f7u, 0x03f7u, 6u, 2u, CORE_MACHINE_FDC_UNREADY_READ_DESKPRO_REFERENCE,
        8u };
    hdc = (core_machine_hdc_config) {
        .protocol = CORE_MACHINE_HDC_PROTOCOL_COMPAQ_WD_40MB,
        .irq = 14u, .bus.task_file = {
            .data_port = 0x01f0u, .error_features_port = 0x01f1u,
            .sector_count_port = 0x01f2u, .sector_number_port = 0x01f3u,
            .cylinder_low_port = 0x01f4u, .cylinder_high_port = 0x01f5u,
            .drive_head_port = 0x01f6u, .status_command_port = 0x01f7u,
            .alternate_status_device_control_port = 0x03f6u,
            .drive_address_port = 0x03f7u, .lba28_supported = TYPE_FALSE}};
    if (core_machine_plan_configure_fdc(plan, &drives, &fdc) != TYPE_STATUS_OK ||
        core_machine_plan_bind_fdc_terminal_observation(plan,
            (core_machine_fdc_terminal_observation_provider) {
                vm_session_model40_capture_fdc_terminal, session }) != TYPE_STATUS_OK) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    return core_machine_plan_configure_hdc(plan, VM_SESSION_MEDIA_HDD_ID,
        session->retained_config.hdd_slave_image == STD_NULL ?
            CORE_MACHINE_MEDIA_ID_INVALID : VM_SESSION_MEDIA_HDD_SLAVE_ID, &hdc);
}

type_status vm_session_model40_storage_initialize(vm_session *session)
{
    core_machine_display_config display = {0};
    core_machine_dma_wiring dma = { .fdc_channel = 2u,
        .controller_count = CORE_MACHINE_DMA_CONTROLLER_COUNT,
        .cascade_channel = CORE_MACHINE_DMA_CASCADE_CHANNEL };
    core_machine_d4_platform_config d4 = { CORE_MACHINE_PC_AT_PORT_B, 0u };
    core_machine_rtc_cmos_config rtc = {0};
    core_machine_plan_topology topology = {0};
    core_machine_d4_memory_config d4_memory = {0};
    type_status status;

    if (session == STD_NULL || session->core_machine != STD_NULL ||
        !session->model40_private ||
        !vm_profile_model40_external_rom_is_valid(&session->model40_rom)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    status = core_machine_display_provider_slot_create(&session->display_provider);
    if (status != TYPE_STATUS_OK) return status;
    vm_session_bind_display(session);
    status = core_machine_media_registry_create(&session->media_registry);
    if (status != TYPE_STATUS_OK) {
        vm_session_model40_storage_rollback(session);
        return status;
    }
    status = core_machine_plan_create(&session->core_machine_config,
        &session->core_machine_plan);
    if (status != TYPE_STATUS_OK) {
        vm_session_model40_storage_rollback(session);
        return status;
    }
    d4_memory = (core_machine_d4_memory_config) { TYPE_TRUE, 0x8fu, 0xc1u, 0xfc42u };
    status = core_machine_plan_configure_d4_memory(session->core_machine_plan,
        &d4_memory);
    if (status != TYPE_STATUS_OK) {
        vm_session_model40_storage_rollback(session);
        return status;
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
    rtc.defaults[0] = (core_machine_rtc_default_byte) { CORE_MACHINE_RTC_TYPE_DISK_FLOPPY, 0x22u };
    rtc.defaults[1] = (core_machine_rtc_default_byte) { CORE_MACHINE_RTC_TYPE_DISK_FIXED,
        session->retained_config.hdd_image == STD_NULL ? 0u : 0x2fu };
    rtc.defaults[2] = (core_machine_rtc_default_byte) { CORE_MACHINE_RTC_TYPE_DISK_FIXED_EXTENDED_0, 0u };
    /* The selected board has two 1.2 MiB mechanisms; only A: is initially
     * supplied with media. Presence remains solely an FDC concern. */
    rtc.defaults[3] = (core_machine_rtc_default_byte) { CORE_MACHINE_RTC_EQUIPMENT, 0x41u };
    rtc.defaults[4] = (core_machine_rtc_default_byte) { CORE_MACHINE_RTC_BASEMEM_LSB, 0x80u };
    rtc.defaults[5] = (core_machine_rtc_default_byte) { CORE_MACHINE_RTC_BASEMEM_MSB, 0x02u };
    rtc.defaults[6] = (core_machine_rtc_default_byte) { CORE_MACHINE_RTC_EXTMEM_LSB, 0u };
    rtc.defaults[7] = (core_machine_rtc_default_byte) { CORE_MACHINE_RTC_EXTMEM_MSB, 0x04u };
    rtc.default_count = CORE_MACHINE_RTC_DEFAULT_CAPACITY;
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
    topology.absent_memory_count = 2u;
    topology.absent_memory[0] = (core_machine_absent_memory_config) {
        0x00200000u, 0x00800000u, 0xffu };
    topology.absent_memory[1] = (core_machine_absent_memory_config) {
        0x00f00000u, 0x000a0000u, 0xffu };
    topology.display_present = TYPE_TRUE;
    topology.display = display;
    topology.dma_present = TYPE_TRUE;
    topology.dma = dma;
    topology.rtc_cmos_present = TYPE_TRUE;
    topology.rtc_cmos = rtc;
    status = core_machine_plan_set_topology(session->core_machine_plan, &topology);
    if (status == TYPE_STATUS_OK) {
        status = core_machine_plan_bind_media_registry(session->core_machine_plan,
            session->media_registry);
    }
    if (status == TYPE_STATUS_OK) {
        status = core_machine_plan_bind_display_provider(session->core_machine_plan,
            session->display_provider);
    }
    if (status != TYPE_STATUS_OK) {
        vm_session_model40_storage_rollback(session);
        return status;
    }
    status = vm_session_model40_materialize_controllers(session,
        session->core_machine_plan);
    if (status != TYPE_STATUS_OK) {
        vm_session_model40_storage_rollback(session);
        return status;
    }
    status = core_machine_create_from_plan(session->core_machine_plan,
        &session->core_machine);
    if (status == TYPE_STATUS_OK) {
        status = core_machine_get_fdc_dma_request_binding(session->core_machine,
            &session->fdc_dma_request);
    }
    if (status != TYPE_STATUS_OK) { vm_session_model40_storage_rollback(session); return status; }
    status = core_platform_presentation_mailbox_create(&session->presentation_mailbox);
    if (status != TYPE_STATUS_OK) { vm_session_model40_storage_rollback(session); return status; }
    status = core_product_debugger_create(&session->debugger);
    if (status != TYPE_STATUS_OK) { vm_session_model40_storage_rollback(session); return status; }
    return TYPE_STATUS_OK;
}

C_INT vm_session_model40_insert_hdd_at_startup(vm_session *session, const C_CHAR *path)
{
    const STD_SIZE_T expected_bytes = 925u * 5u * 17u * 512u;
    STD_SIZE_T path_length;

    if (session == STD_NULL || !session->model40_private || path == STD_NULL ||
        vm_machine_hdd_has_media(&session->hdd) ||
        vm_machine_hdd_insert(&session->hdd, path) != 0 ||
        vm_machine_hdd_raw_byte_count(&session->hdd) != expected_bytes) return -1;
    path_length = STD_STRLEN(path);
    if (path_length >= sizeof(session->hdd_image_path)) return -1;
    STD_MEMCPY(session->hdd_image_path, path, path_length + 1u);
    if (vm_machine_hdd_set_geometry(&session->hdd, 925u, 5u, 17u) != TYPE_FALSE) {
        return -1;
    }
    session->retained_config.hdd_image = session->hdd_image_path;
    return 0;
}

C_INT vm_session_model40_insert_hdd_slave_at_startup(vm_session *session, const C_CHAR *path)
{
    const STD_SIZE_T expected_bytes = 925u * 5u * 17u * 512u;
    STD_SIZE_T path_length;

    if (session == STD_NULL || !session->model40_private || path == STD_NULL ||
        vm_machine_hdd_has_media(&session->hdd_slave) ||
        vm_machine_hdd_insert(&session->hdd_slave, path) != 0 ||
        vm_machine_hdd_raw_byte_count(&session->hdd_slave) != expected_bytes) return -1;
    path_length = STD_STRLEN(path);
    if (path_length >= sizeof(session->hdd_slave_image_path)) return -1;
    STD_MEMCPY(session->hdd_slave_image_path, path, path_length + 1u);
    if (vm_machine_hdd_set_geometry(&session->hdd_slave, 925u, 5u, 17u) != TYPE_FALSE) {
        return -1;
    }
    session->retained_config.hdd_slave_image = session->hdd_slave_image_path;
    return 0;
}
