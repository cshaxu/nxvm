#include "vm/composition/session/model40_composition.h"

#include "vm/composition/session/display.h"
#include "vm/composition/session/media.h"
#include "vm/composition/session/session.h"

static C_VOID vm_session_model40_storage_rollback(vm_session *session)
{
    if (session == STD_NULL) return;
    core_machine_display_provider_slot_finalize(&session->display_provider);
    core_machine_destroy(session->core_machine);
    session->core_machine = STD_NULL;
}

type_status vm_session_model40_storage_initialize(vm_session *session)
{
    core_machine_display_config display = {0};
    core_machine_dma_wiring dma = {2u};
    core_machine_d4_platform_config d4 = { CORE_MACHINE_PC_AT_PORT_B, 0u };
    core_machine_rtc_cmos_config rtc = {0};
    type_status status;

    if (session == STD_NULL || session->core_machine != STD_NULL ||
        !session->model40_private ||
        !vm_profile_model40_external_rom_is_valid(&session->model40_rom)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    status = core_machine_create(&session->core_machine_config, &session->core_machine);
    if (status != TYPE_STATUS_OK) return status;
    core_machine_display_provider_slot_initialize(&session->display_provider);
    vm_session_bind_display(session);
    display.text_timing = (core_machine_vadp_text_timing) {48u, 8u, 8u};
    display.cga_vram_present = TYPE_FALSE;
    display.ega_present = TYPE_TRUE;
    display.ega_personality = CORE_MACHINE_VADP_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR;
    display.cecg = (core_machine_vadp_cecg_config) {
        0x40u, 0x00u, 0x30u, 0x01u, TYPE_TRUE, TYPE_FALSE, TYPE_TRUE,
        0x06u, 0x01u, TYPE_FALSE, TYPE_FALSE };
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
    display.provider = &session->display_provider;
    rtc.index_port = 0x0070u;
    rtc.data_port = 0x0071u;
    rtc.irq = 8u;
    rtc.nmi_mask_bit = 0x80u;
    rtc.ticks_per_second = 32768u;
    rtc.defaults[0] = (core_machine_rtc_default_byte) { CORE_MACHINE_RTC_TYPE_DISK_FLOPPY, 0x20u };
    rtc.defaults[1] = (core_machine_rtc_default_byte) { CORE_MACHINE_RTC_TYPE_DISK_FIXED, 0x2fu };
    rtc.defaults[2] = (core_machine_rtc_default_byte) { CORE_MACHINE_RTC_TYPE_DISK_FIXED_EXTENDED_0, 0u };
    rtc.defaults[3] = (core_machine_rtc_default_byte) { CORE_MACHINE_RTC_EQUIPMENT, 0x21u };
    rtc.defaults[4] = (core_machine_rtc_default_byte) { CORE_MACHINE_RTC_BASEMEM_LSB, 0u };
    rtc.defaults[5] = (core_machine_rtc_default_byte) { CORE_MACHINE_RTC_BASEMEM_MSB, 0x04u };
    rtc.default_count = CORE_MACHINE_RTC_DEFAULT_COUNT;
    status = core_machine_configure_d4_platform(session->core_machine, &d4);
    if (status == TYPE_STATUS_OK) status = core_machine_configure_display(session->core_machine, &display);
    if (status == TYPE_STATUS_OK) status = core_machine_configure_dma(session->core_machine, &dma, &session->fdc_dma_request);
    if (status == TYPE_STATUS_OK) status = core_machine_configure_rtc_cmos(session->core_machine, &rtc);
    if (status != TYPE_STATUS_OK) { vm_session_model40_storage_rollback(session); return status; }
    core_machine_media_registry_initialize(&session->media_registry);
    core_platform_presentation_mailbox_initialize(&session->presentation_mailbox);
    core_product_debug_context_initialize(&session->debugger_context);
    return TYPE_STATUS_OK;
}

type_status vm_session_model40_configure_controllers(vm_session *session)
{
    const core_machine_fdc_drive_bindings drives = {{VM_SESSION_MEDIA_FDD_ID,
        CORE_MACHINE_MEDIA_ID_INVALID, CORE_MACHINE_MEDIA_ID_INVALID,
        CORE_MACHINE_MEDIA_ID_INVALID}};
    core_machine_fdc_topology fdc = {0};
    core_machine_hdc_topology hdc = {0};
    type_status status;

    if (session == STD_NULL || !session->model40_private ||
        session->core_machine == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    fdc.media_registry = &session->media_registry;
    fdc.drives = drives;
    fdc.dma_request = session->fdc_dma_request;
    fdc.config = (core_machine_fdc_config) { 0x03f2u, 0x03f4u, 0x03f5u,
        0x03f7u, 0x03f7u, 6u, 2u };
    status = core_machine_configure_fdc(session->core_machine, &fdc);
    if (status != TYPE_STATUS_OK) return status;
    hdc.media_registry = &session->media_registry;
    hdc.media_id = VM_SESSION_MEDIA_HDD_ID;
    hdc.config = (core_machine_hdc_config) { 0x01f0u, 0x01f1u, 0x01f2u,
        0x01f3u, 0x01f4u, 0x01f5u, 0x01f6u, 0x01f7u, 0x03f6u, 0x03f7u,
        14u, TYPE_FALSE, CORE_MACHINE_HDC_PROTOCOL_COMPAQ_WD_40MB };
    return core_machine_configure_hdc(session->core_machine, &hdc);
}