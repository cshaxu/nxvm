/* Copyright 2012-2014 Neko. */

#include "type.h"

#include "core/machine/machine_interface.h"
#include "core/product/utils.h"
#include "vm/composition/session/profile_firmware.h"
#include "vm/composition/session/session.h"
#include "core/machine/dma.h"
#include "core/machine/pic.h"
#include "core/machine/pit.h"
#include "vm/profile/default_profile/firmware/rtc_firmware.h"
#include "vm/profile/default_profile/firmware/fdc_firmware.h"
#include "vm/profile/default_profile/firmware/bios.h"
#include "vm/profile/default_profile/firmware/hdc.h"
#include "vm/profile/default_profile/firmware/qdcga.h"
#include "vm/profile/default_profile/firmware/qdkeyb.h"

static type_unsigned_16 vm_session_profile_firmware_assemble(const C_CHAR *statement,
    type_unsigned_8 **out_bytes)
{
    STD_SIZE_T length;
    STD_SIZE_T capacity;
    STD_SIZE_T index;
    type_unsigned_8 *bytes;

    if (out_bytes == STD_NULL || statement == STD_NULL) return 0u;
    *out_bytes = STD_NULL;
    capacity = 1u;
    for (index = 0u; statement[index] != '\0'; ++index) {
        if (statement[index] == '\n') capacity++;
    }
    if (capacity > TYPE_MAX_UNSIGNED_16 / 15u) return 0u;
    bytes = (type_unsigned_8 *)STD_MALLOC(capacity * 15u);
    if (bytes == STD_NULL) return 0u;
    if (core_product_utils_assemble_paragraph(statement, STD_STRLEN(statement),
            bytes, capacity * CORE_PRODUCT_UTILS_XASM_MAX_CODE_BYTES, &length,
            TYPE_FALSE) != TYPE_STATUS_OK) {
        STD_FREE(bytes);
        return 0u;
    }
    if (length == 0u || length > TYPE_MAX_UNSIGNED_16) {
        STD_FREE(bytes);
        return 0u;
    }
    *out_bytes = bytes;
    return (type_unsigned_16)length;
}

static C_VOID vm_session_profile_firmware_add_post(vm_session *session,
    const C_CHAR *statement)
{
    type_unsigned_8 *bytes;
    type_unsigned_16 length;

    if (session == STD_NULL) return;
    length = vm_session_profile_firmware_assemble(statement, &bytes);
    vm_profile_default_bios_add_post_code(&session->default_bios, bytes, length);
}

static C_VOID vm_session_profile_firmware_add_interrupt(vm_session *session,
    const C_CHAR *statement, type_unsigned_8 vector)
{
    type_unsigned_8 *bytes;
    type_unsigned_16 length;

    if (session == STD_NULL) return;
    length = vm_session_profile_firmware_assemble(statement, &bytes);
    vm_profile_default_bios_add_interrupt_code(&session->default_bios, bytes,
        length, vector);
}

static C_VOID vm_session_profile_firmware_set_boot(vm_session *session,
    const C_CHAR *statement)
{
    type_unsigned_8 *bytes;
    type_unsigned_16 length;

    if (session == STD_NULL) return;
    length = vm_session_profile_firmware_assemble(statement, &bytes);
    vm_profile_default_bios_set_boot_code(&session->default_bios, bytes, length);
}

static C_INT vm_session_profile_firmware_hook_is_valid(
    vm_profile_default_pc_at_firmware_hook hook)
{
    return hook >= VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_VIDEO_INT10 &&
        hook <= VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_PIC_POST;
}

static C_VOID vm_session_profile_firmware_apply(
    vm_session *session, vm_profile_default_pc_at_firmware_hook hook,
    type_unsigned_8 vector)
{
    if (session == STD_NULL) return;
    switch (hook) {
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_VIDEO_INT10:
        vm_session_profile_firmware_add_interrupt(session, VBIOS_INT_SOFT_VIDEO_10,
            vector);
        break;
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_CMOS_POST:
        vm_session_profile_firmware_add_post(session, VCMOS_POST);
        break;
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_TIMER_IRQ0:
        vm_session_profile_firmware_add_interrupt(session,
            VCMOS_INT_HARD_TIMER_08, vector);
        break;
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_TIMER_INT1A:
        vm_session_profile_firmware_add_interrupt(session,
            VCMOS_INT_SOFT_TIMER_1A, vector);
        break;
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_KEYBOARD_IRQ1:
        vm_session_profile_firmware_add_interrupt(session,
            VBIOS_INT_HARD_KEYBOARD_09, vector);
        break;
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_KEYBOARD_INT16:
        vm_session_profile_firmware_add_interrupt(session,
            VBIOS_INT_SOFT_KEYBOARD_16, vector);
        break;
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_DMA_POST:
        vm_session_profile_firmware_add_post(session, VDMA_POST);
        break;
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_FDC_POST:
        vm_session_profile_firmware_add_post(session, VFDC_POST);
        break;
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_FDC_IRQ6:
        vm_session_profile_firmware_add_interrupt(session,
            VFDC_INT_HARD_FDD_0E, vector);
        break;
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_FDC_INT13:
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_FDC_INT40:
        vm_session_profile_firmware_add_interrupt(session,
            VFDC_INT_SOFT_FDD_40, vector);
        break;
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_HDC_INT13:
        vm_session_profile_firmware_add_interrupt(session,
            VHDC_INT_SOFT_HDD_13, vector);
        break;
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_PIT_POST:
        vm_session_profile_firmware_add_post(session, VPIT_POST);
        break;
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_PIC_POST:
        vm_session_profile_firmware_add_post(session, VPIC_POST);
        break;
    }
}

static C_VOID vm_session_profile_firmware_apply_range(vm_session *session,
    vm_profile_default_pc_at_firmware_hook first,
    vm_profile_default_pc_at_firmware_hook last)
{
    STD_SIZE_T index;

    if (session == STD_NULL || session->profile == STD_NULL) return;
    for (index = 0u; index < session->profile->firmware_service_count; ++index) {
        const vm_profile_default_pc_at_firmware_service *service =
            &session->profile->firmware_services[index];
        if (service->hook >= first && service->hook <= last) {
            vm_session_profile_firmware_apply(session, service->hook,
                service->vector);
        }
    }
}

type_status vm_session_profile_firmware_initialize(vm_session *session)
{
    STD_SIZE_T index;

    if (session == STD_NULL || session->profile == STD_NULL ||
        (session->profile->firmware_service_count != 0u &&
        session->profile->firmware_services == STD_NULL)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    for (index = 0u; index < session->profile->firmware_service_count; ++index) {
        if (!vm_session_profile_firmware_hook_is_valid(
                session->profile->firmware_services[index].hook)) {
            return TYPE_STATUS_INVALID_ARGUMENT;
        }
    }
    vm_profile_default_bios_initialize(&session->default_bios);
    vm_session_profile_firmware_add_interrupt(session, VBIOS_INT_SOFT_MISC_11,
        0x11u);
    vm_session_profile_firmware_add_interrupt(session, VBIOS_INT_SOFT_MISC_12,
        0x12u);
    vm_session_profile_firmware_add_interrupt(session, VBIOS_INT_SOFT_MISC_15,
        0x15u);
    vm_session_profile_firmware_set_boot(session, VBIOS_POST_BOOT);
    vm_session_profile_firmware_apply_range(session,
        VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_VIDEO_INT10,
        VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_VIDEO_INT10);
    return TYPE_STATUS_OK;
}

static type_status vm_session_profile_firmware_configure(C_VOID *opaque,
    core_machine_firmware_context *firmware)
{
    vm_profile_default_context *context = (vm_profile_default_context *)opaque;

    return context == STD_NULL || context->bios == STD_NULL ||
        !vm_profile_default_bios_materialize(context->bios, firmware) ?
        TYPE_STATUS_FAULT : TYPE_STATUS_OK;
}

static type_status vm_session_profile_firmware_reset_callback(C_VOID *opaque,
    core_machine_firmware_context *firmware)
{
    vm_profile_default_context *context = (vm_profile_default_context *)opaque;

    if (context == STD_NULL || context->bios == STD_NULL) return TYPE_STATUS_FAULT;
    vm_profile_default_bios_reset(context->bios, firmware, context->media_registry,
        context->hdd_media_id);
    vm_profile_default_cga_reset(context, firmware);
    return TYPE_STATUS_OK;
}

static type_status vm_session_profile_firmware_after_run(C_VOID *opaque,
    core_machine_firmware_context *firmware)
{
    vm_profile_default_context *context = (vm_profile_default_context *)opaque;

    if (context == STD_NULL || context->bios == STD_NULL) return TYPE_STATUS_FAULT;
    return vm_profile_default_bios_take_boot_failure_report(firmware) ?
        core_machine_firmware_request_stop(firmware) : TYPE_STATUS_OK;
}

static const core_machine_firmware_provider vm_session_default_firmware_provider = {
    vm_session_profile_firmware_configure,
    vm_session_profile_firmware_reset_callback,
    vm_session_profile_firmware_after_run
};

const core_machine_firmware_provider *vm_session_profile_firmware_provider(C_VOID)
{
    return &vm_session_default_firmware_provider;
}

C_VOID vm_session_profile_firmware_register_cmos(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_session_profile_firmware_apply_range(session,
        VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_CMOS_POST,
        VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_TIMER_INT1A);
}

C_VOID vm_session_profile_firmware_register_keyboard(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_session_profile_firmware_apply_range(session,
        VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_KEYBOARD_IRQ1,
        VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_KEYBOARD_INT16);
}

C_VOID vm_session_profile_firmware_register_dma(vm_session *session)
{
    vm_session_profile_firmware_apply_range(session,
        VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_DMA_POST,
        VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_DMA_POST);
}

C_VOID vm_session_profile_firmware_register_fdc(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_session_profile_firmware_apply_range(session,
        VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_FDC_POST,
        VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_FDC_INT40);
}

C_VOID vm_session_profile_firmware_register_hdc(vm_session *session)
{
    vm_session_profile_firmware_apply_range(session,
        VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_HDC_INT13,
        VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_HDC_INT13);
}

C_VOID vm_session_profile_firmware_register_core_posts(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_session_profile_firmware_apply_range(session,
        VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_PIT_POST,
        VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_PIC_POST);
}

C_VOID vm_session_profile_firmware_finalize(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_profile_default_bios_finalize(&session->default_bios);
}
