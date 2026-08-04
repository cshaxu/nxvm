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
#include "vm/profile/default_profile/firmware/qdx.h"

C_VOID vm_session_profile_firmware_initialize(vm_session *session)
{
    if (session == STD_NULL) return;
    (C_VOID)core_machine_memory_register_mapping(
        session->default_profile_context.ram, 0xfffffff0u, 0x000ffff0u, 16u);
    vm_profile_default_bios_initialize(&session->default_bios);
    vm_profile_default_bios_add_interrupt(&session->default_bios,
        "qdx 10\niret", 0x10);
}

C_VOID vm_session_profile_firmware_register_cmos(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_profile_default_bios_add_post(&session->default_bios, VCMOS_POST);
    vm_profile_default_bios_add_interrupt(&session->default_bios,
        VCMOS_INT_HARD_RTC_08, 0x08);
    vm_profile_default_bios_add_interrupt(&session->default_bios,
        VCMOS_INT_SOFT_RTC_1A, 0x1a);
}

C_VOID vm_session_profile_firmware_register_keyboard(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_profile_default_bios_add_interrupt(&session->default_bios, "qdx 09\niret", 0x09);
    vm_profile_default_bios_add_interrupt(&session->default_bios, "qdx 16\niret", 0x16);
}

C_VOID vm_session_profile_firmware_register_dma(vm_session *session)
{
    if (session != STD_NULL) vm_profile_default_bios_add_post(&session->default_bios,
        VDMA_POST);
}

C_VOID vm_session_profile_firmware_register_fdc(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_profile_default_bios_add_post(&session->default_bios, VFDC_POST);
    vm_profile_default_bios_add_interrupt(&session->default_bios,
        VFDC_INT_HARD_FDD_0E, 0x0e);
    /* Overwritten below by the retained hard-disk INT 13 service. */
    vm_profile_default_bios_add_interrupt(&session->default_bios,
        VFDC_INT_SOFT_FDD_40, 0x13);
    vm_profile_default_bios_add_interrupt(&session->default_bios,
        VFDC_INT_SOFT_FDD_40, 0x40);
}

C_VOID vm_session_profile_firmware_register_hdc(vm_session *session)
{
    if (session != STD_NULL) vm_profile_default_bios_add_interrupt(&session->default_bios,
        VHDC_INT_SOFT_HDD_13, 0x13);
}

C_VOID vm_session_profile_firmware_register_core_posts(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_profile_default_bios_add_post(&session->default_bios, VPIT_POST);
    vm_profile_default_bios_add_post(&session->default_bios, VPIC_POST);
}

C_VOID vm_session_profile_firmware_initialize_qdx(vm_session *session)
{
    if (session != STD_NULL) vm_profile_default_qdx_initialize(
        &session->default_profile_context,
        core_machine_configuration_cpu_execution_borrow(session->core_machine));
}

C_VOID vm_session_profile_firmware_refresh(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_profile_default_qdx_refresh(&session->default_profile_context);
    vm_profile_default_bios_refresh(&session->default_bios);
}

C_VOID vm_session_profile_firmware_reset(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_profile_default_bios_reset(&session->default_bios,
        session->default_profile_context.ram,
        session->block_provider);
    vm_profile_default_qdx_reset(&session->default_profile_context);
}

C_VOID vm_session_profile_firmware_finalize(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_profile_default_qdx_finalize(&session->default_profile_context);
    vm_profile_default_bios_finalize(&session->default_bios);
}
