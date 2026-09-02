/* Copyright 2012-2014 Neko. */

#include "type.h"

#include "core/machine/machine_interface.h"
#include "core/product/utils.h"
#include "vm/composition/session/profile_firmware.h"
#include "vm/composition/session/session_private.h"
#include "vm/profile/default_profile/firmware/post_firmware.h"
#include "vm/profile/default_profile/firmware/rtc_firmware.h"
#include "vm/profile/default_profile/firmware/fdc_firmware.h"
#include "vm/profile/default_profile/firmware/bios.h"
#include "vm/profile/default_profile/firmware/hdc.h"
#include "vm/profile/default_profile/firmware/qdcga.h"
#include "vm/profile/default_profile/firmware/qdkeyb.h"

#define VM_SESSION_PROFILE_FIRMWARE_CHUNK(value) value,
#define VM_SESSION_PROFILE_FIRMWARE_CHUNKS(source) \
    ((const C_CHAR *const[]){ source(VM_SESSION_PROFILE_FIRMWARE_CHUNK) })
#define VM_SESSION_PROFILE_FIRMWARE_CHUNK_COUNT(source) \
    (sizeof(VM_SESSION_PROFILE_FIRMWARE_CHUNKS(source)) / sizeof(C_CHAR *))

static C_CHAR *vm_session_profile_firmware_materialize(
    const C_CHAR *const *chunks, STD_SIZE_T count)
{
    STD_SIZE_T length = 0u;
    STD_SIZE_T index;
    C_CHAR *statement;

    if (chunks == STD_NULL || count == 0u) return STD_NULL;
    for (index = 0u; index < count; ++index) {
        if (chunks[index] == STD_NULL || STD_STRLEN(chunks[index]) >
            TYPE_MAX_NATIVE_UNSIGNED - length - 1u) return STD_NULL;
        length += STD_STRLEN(chunks[index]);
    }
    statement = (C_CHAR *)STD_MALLOC(length + 1u);
    if (statement == STD_NULL) return STD_NULL;
    length = 0u;
    for (index = 0u; index < count; ++index) {
        STD_SIZE_T chunk_length = STD_STRLEN(chunks[index]);
        STD_MEMCPY(statement + length, chunks[index], chunk_length);
        length += chunk_length;
    }
    statement[length] = '\0';
    return statement;
}

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

static type_unsigned_16 vm_session_profile_firmware_assemble_chunks(
    const C_CHAR *const *chunks, STD_SIZE_T count, type_unsigned_8 **out_bytes)
{
    C_CHAR *statement = vm_session_profile_firmware_materialize(chunks, count);
    type_unsigned_16 length = vm_session_profile_firmware_assemble(statement,
        out_bytes);

    STD_FREE(statement);
    return length;
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

/* AH=88h reports KiB physically installed above the first MiB.  Compose the
 * one generic BIOS service from the already-frozen Core memory value; neither
 * a profile-specific handler nor a mutable runtime memory authority is added. */
static C_VOID vm_session_profile_firmware_add_int15_interrupt(
    vm_session *session, type_unsigned_8 vector)
{
    C_CHAR extended_kib[5];
    const C_CHAR *chunks[] = {
        VBIOS_INT_SOFT_MISC_15_PREFIX, extended_kib,
        VBIOS_INT_SOFT_MISC_15_SUFFIX
    };
    STD_SIZE_T bytes;
    type_unsigned_8 *code;
    type_unsigned_16 length;

    if (session == STD_NULL) return;
    bytes = session->core_machine_config.memory_bytes;
    if (bytes > 1024u * 1024u) bytes = (bytes - 1024u * 1024u) / 1024u;
    else bytes = 0u;
    if (bytes > 0xffffu || STD_SNPRINTF(extended_kib, sizeof(extended_kib),
            "%04x", (unsigned int)bytes) != 4) return;
    length = vm_session_profile_firmware_assemble_chunks(chunks,
        sizeof(chunks) / sizeof(chunks[0]), &code);
    vm_profile_default_bios_add_interrupt_code(&session->default_bios, code,
        length, vector);
}

static C_VOID vm_session_profile_firmware_add_interrupt_chunks(vm_session *session,
    const C_CHAR *const *chunks, STD_SIZE_T count, type_unsigned_16 offset,
    type_unsigned_8 vector)
{
    type_unsigned_8 *bytes;
    type_unsigned_16 length;

    if (session == STD_NULL) return;
    length = vm_session_profile_firmware_assemble_chunks(chunks, count, &bytes);
    vm_profile_default_bios_add_interrupt_code_at(&session->default_bios, bytes,
        length, offset, vector);
}

/* The FDC service is one firmware program, but its DMA-boundary bounce page is
 * profile-owned conventional memory.  Substitute only its two executable
 * segment operands; the source remains one shared service rather than a
 * Model-339-specific boot path. */
static C_VOID vm_session_profile_firmware_add_fdc_interrupt(vm_session *session,
    type_unsigned_16 offset, type_unsigned_8 vector)
{
    STD_SIZE_T length;
    STD_SIZE_T index;
    C_CHAR replacement[5];
    C_CHAR *statement;
    type_unsigned_8 *bytes;
    type_unsigned_16 assembled;

    if (session == STD_NULL || session->profile == STD_NULL ||
        session->profile->fdc_bounce_segment == 0u) return;
    if (STD_SNPRINTF(replacement, sizeof(replacement), "%04x",
            session->profile->fdc_bounce_segment) != 4) return;
    statement = vm_session_profile_firmware_materialize(
        VM_SESSION_PROFILE_FIRMWARE_CHUNKS(VFDC_INT_SOFT_FDD_40),
        VM_SESSION_PROFILE_FIRMWARE_CHUNK_COUNT(VFDC_INT_SOFT_FDD_40));
    if (statement == STD_NULL) return;
    length = STD_STRLEN(statement);
    for (index = 0u; index + 4u <= length; ++index) {
        if (STD_MEMCMP(statement + index, "9fc0", 4u) == 0) {
            STD_MEMCPY(statement + index, replacement, 4u);
            index += 3u;
        }
    }
    assembled = vm_session_profile_firmware_assemble(statement, &bytes);
    STD_FREE(statement);
    vm_profile_default_bios_add_interrupt_code_at(&session->default_bios, bytes,
        assembled, offset, vector);
}

static C_VOID vm_session_profile_firmware_add_interrupt_at(vm_session *session,
    const C_CHAR *statement, type_unsigned_16 offset, type_unsigned_8 vector)
{
    type_unsigned_8 *bytes;
    type_unsigned_16 length;

    if (session == STD_NULL) return;
    length = vm_session_profile_firmware_assemble(statement, &bytes);
    vm_profile_default_bios_add_interrupt_code_at(&session->default_bios, bytes,
        length, offset, vector);
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
        vm_session_profile_firmware_add_interrupt_chunks(session,
            VM_SESSION_PROFILE_FIRMWARE_CHUNKS(VBIOS_INT_SOFT_VIDEO_10),
            VM_SESSION_PROFILE_FIRMWARE_CHUNK_COUNT(VBIOS_INT_SOFT_VIDEO_10),
            VBIOS_ADDR_VIDEO_SERVICE, vector);
        break;
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_CMOS_POST:
        vm_session_profile_firmware_add_post(session, VCMOS_POST);
        break;
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_TIMER_IRQ0:
        vm_session_profile_firmware_add_interrupt(session,
            VCMOS_INT_HARD_TIMER_08, vector);
        break;
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_TIMER_INT1A:
        vm_session_profile_firmware_add_interrupt_chunks(session,
            VM_SESSION_PROFILE_FIRMWARE_CHUNKS(VCMOS_INT_SOFT_TIMER_1A),
            VM_SESSION_PROFILE_FIRMWARE_CHUNK_COUNT(VCMOS_INT_SOFT_TIMER_1A),
            TYPE_MAX_UNSIGNED_16, vector);
        break;
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_KEYBOARD_IRQ1:
        vm_session_profile_firmware_add_interrupt_chunks(session,
            VM_SESSION_PROFILE_FIRMWARE_CHUNKS(VBIOS_INT_HARD_KEYBOARD_09),
            VM_SESSION_PROFILE_FIRMWARE_CHUNK_COUNT(VBIOS_INT_HARD_KEYBOARD_09),
            TYPE_MAX_UNSIGNED_16, vector);
        break;
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_KEYBOARD_INT16:
        vm_session_profile_firmware_add_interrupt_chunks(session,
            VM_SESSION_PROFILE_FIRMWARE_CHUNKS(VBIOS_INT_SOFT_KEYBOARD_16),
            VM_SESSION_PROFILE_FIRMWARE_CHUNK_COUNT(VBIOS_INT_SOFT_KEYBOARD_16),
            TYPE_MAX_UNSIGNED_16, vector);
        break;
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_DMA_POST:
        vm_session_profile_firmware_add_post(session, VM_PROFILE_DEFAULT_DMA_POST);
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
        vm_session_profile_firmware_add_fdc_interrupt(session,
            VBIOS_ADDR_FDC_SERVICE, vector);
        break;
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_HDC_INT13:
        vm_session_profile_firmware_add_interrupt_chunks(session,
            VM_SESSION_PROFILE_FIRMWARE_CHUNKS(VHDC_INT_SOFT_HDD_13),
            VM_SESSION_PROFILE_FIRMWARE_CHUNK_COUNT(VHDC_INT_SOFT_HDD_13),
            TYPE_MAX_UNSIGNED_16, vector);
        break;
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_PIT_POST:
        vm_session_profile_firmware_add_post(session, VM_PROFILE_DEFAULT_PIT_POST);
        break;
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_PIC_POST:
        vm_session_profile_firmware_add_post(session, VM_PROFILE_DEFAULT_PIC_POST);
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
    session->default_bios.data.base_memory_kib = session->profile->cmos.base_memory_kib;
    vm_session_profile_firmware_add_interrupt(session, VBIOS_INT_SOFT_MISC_11,
        0x11u);
    vm_session_profile_firmware_add_interrupt(session, VBIOS_INT_SOFT_MISC_12,
        0x12u);
    vm_session_profile_firmware_add_int15_interrupt(session, 0x15u);
    vm_session_profile_firmware_set_boot(session, VBIOS_POST_BOOT);
    vm_session_profile_firmware_apply_range(session,
        VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_VIDEO_INT10,
        VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_VIDEO_INT10);
    return TYPE_STATUS_OK;
}

C_INT vm_session_profile_firmware_is_external(const vm_session *session)
{
    return session != STD_NULL &&
        session->default_profile_context.external_rom != STD_NULL;
}

static type_status vm_session_profile_firmware_configure(C_VOID *opaque,
    core_machine_firmware_context *firmware)
{
    vm_profile_default_context *context = (vm_profile_default_context *)opaque;

    if (context == STD_NULL || context->bios == STD_NULL) return TYPE_STATUS_FAULT;
    if (context->external_rom != STD_NULL) {
        return core_machine_firmware_register_immutable_rom(firmware, 0x000f0000u,
            context->external_rom, VM_SESSION_PC_AT_ROM_BYTES);
    }
    return vm_profile_default_bios_materialize(context->bios, firmware) ?
        TYPE_STATUS_OK : TYPE_STATUS_FAULT;
}

static type_status vm_session_profile_firmware_reset_callback(C_VOID *opaque,
    core_machine_firmware_context *firmware)
{
    vm_profile_default_context *context = (vm_profile_default_context *)opaque;

    if (context == STD_NULL || context->bios == STD_NULL) return TYPE_STATUS_FAULT;
    if (context->external_rom != STD_NULL) return TYPE_STATUS_OK;
    vm_profile_default_bios_reset(context->bios, firmware, context->media_registry,
        context->fdd_media_id, context->hdd_media_id);
    vm_profile_default_cga_reset(context, firmware);
    return TYPE_STATUS_OK;
}

static type_status vm_session_profile_firmware_after_run(C_VOID *opaque,
    core_machine_firmware_context *firmware)
{
    vm_profile_default_context *context = (vm_profile_default_context *)opaque;

    if (context == STD_NULL || context->bios == STD_NULL) return TYPE_STATUS_FAULT;
    if (context->external_rom != STD_NULL) return TYPE_STATUS_OK;
    return vm_profile_default_bios_take_boot_failure_report(firmware) ?
        core_machine_firmware_request_stop(firmware) : TYPE_STATUS_OK;
}

static type_status vm_session_profile_firmware_software_interrupt(C_VOID *opaque,
    core_machine_firmware_context *firmware, type_unsigned_8 vector,
    type_unsigned_16 target_segment, type_unsigned_16 target_offset,
    const core_machine_firmware_interrupt_frame *input,
    core_machine_firmware_interrupt_result *output, type_bool *out_handled)
{
    vm_profile_default_context *context = (vm_profile_default_context *)opaque;

    if (out_handled == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_handled = TYPE_FALSE;
    if (context == STD_NULL || context->bios == STD_NULL ||
        context->external_rom != STD_NULL || vector != 0x15u) {
        return TYPE_STATUS_OK;
    }
    *out_handled = vm_profile_default_bios_handle_int15_block_move(context->bios,
        firmware, target_segment, target_offset, input, output) ? TYPE_TRUE : TYPE_FALSE;
    return TYPE_STATUS_OK;
}

static const core_machine_firmware_provider vm_session_default_firmware_provider = {
    vm_session_profile_firmware_configure,
    vm_session_profile_firmware_reset_callback,
    vm_session_profile_firmware_after_run,
    vm_session_profile_firmware_software_interrupt
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
    if (session == STD_NULL || session->profile == STD_NULL ||
        !session->profile->hdc_present) return;
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
