/* Copyright 2012-2014 Neko. */

#include "type.h"

#include "core/machine/machine_interface.h"
#include "vm/composition/session/profile_firmware.h"
#include "vm/composition/session/session.h"
#include "core/machine/dma.h"
#include "core/machine/pic.h"
#include "core/machine/pit.h"
#include "vm/machine/cmos.h"
#include "vm/machine/fdc.h"
#include "vm/profile/default_profile/firmware/bios.h"
#include "vm/profile/default_profile/firmware/hdc.h"
#include "vm/profile/default_profile/firmware/qdcga.h"
#include "vm/profile/default_profile/firmware/qdkeyb.h"

static C_VOID vm_session_profile_firmware_apply(
    vm_session *session, vm_profile_default_pc_at_firmware_hook hook,
    uint8_t vector)
{
    if (session == STD_NULL) return;
    switch (hook) {
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_VIDEO_INT10:
        vm_profile_default_bios_add_interrupt(&session->default_bios,
            "int f2\niret", vector);
        break;
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_CMOS_POST:
        vm_profile_default_bios_add_post(&session->default_bios, VCMOS_POST);
        break;
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_CMOS_IRQ8:
        vm_profile_default_bios_add_interrupt(&session->default_bios,
            VCMOS_INT_HARD_RTC_08, vector);
        break;
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_CMOS_INT1A:
        vm_profile_default_bios_add_interrupt(&session->default_bios,
            VCMOS_INT_SOFT_RTC_1A, vector);
        break;
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_KEYBOARD_IRQ1:
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_KEYBOARD_INT16:
        vm_profile_default_bios_add_interrupt(&session->default_bios,
            hook == VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_KEYBOARD_IRQ1 ?
                "int f1\niret" : "int f3\niret", vector);
        break;
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_DMA_POST:
        vm_profile_default_bios_add_post(&session->default_bios, VDMA_POST);
        break;
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_FDC_POST:
        vm_profile_default_bios_add_post(&session->default_bios, VFDC_POST);
        break;
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_FDC_IRQ6:
        vm_profile_default_bios_add_interrupt(&session->default_bios,
            VFDC_INT_HARD_FDD_0E, vector);
        break;
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_FDC_INT13:
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_FDC_INT40:
        vm_profile_default_bios_add_interrupt(&session->default_bios,
            VFDC_INT_SOFT_FDD_40, vector);
        break;
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_HDC_INT13:
        vm_profile_default_bios_add_interrupt(&session->default_bios,
            VHDC_INT_SOFT_HDD_13, vector);
        break;
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_PIT_POST:
        vm_profile_default_bios_add_post(&session->default_bios, VPIT_POST);
        break;
    case VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_PIC_POST:
        vm_profile_default_bios_add_post(&session->default_bios, VPIC_POST);
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

C_VOID vm_session_profile_firmware_initialize(vm_session *session)
{
    if (session == STD_NULL) return;
    (C_VOID)core_machine_memory_register_mapping(
        vm_profile_default_context_memory(&session->default_profile_context),
        session->profile->rom.linear_start, session->profile->rom.physical_start,
        session->profile->rom.bytes);
    vm_profile_default_bios_initialize(&session->default_bios);
    vm_session_profile_firmware_apply_range(session,
        VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_VIDEO_INT10,
        VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_VIDEO_INT10);
}

C_VOID vm_session_profile_firmware_register_cmos(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_session_profile_firmware_apply_range(session,
        VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_CMOS_POST,
        VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_CMOS_INT1A);
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

C_VOID vm_session_profile_firmware_refresh(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_profile_default_bios_refresh(&session->default_bios);
}

C_VOID vm_session_profile_firmware_reset(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_profile_default_bios_reset(&session->default_bios,
        vm_profile_default_context_memory(&session->default_profile_context),
        &session->block_provider);
    vm_profile_default_keyboard_reset(&session->default_profile_context);
    vm_profile_default_cga_reset(&session->default_profile_context);
}

C_VOID vm_session_profile_firmware_finalize(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_profile_default_bios_finalize(&session->default_bios);
}
