/* Copyright 2012-2014 Neko. */

#include "type.h"

#include "core/machine/machine_interface.h"
#include "vm/composition/session/block.h"
#include "vm/composition/session/machine_devices.h"
#include "vm/composition/session/session.h"
#include "vm/machine/cmos.h"
#include "vm/machine/fdc.h"
#include "vm/machine/fdd.h"
#include "vm/machine/hdd.h"

C_VOID vm_session_machine_devices_initialize_media(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_machine_fdd_initialize(&session->fdd);
    vm_machine_hdd_initialize(&session->hdd);
    vm_session_bind_block(session);
}

C_VOID vm_session_machine_devices_initialize_cmos(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_machine_cmos_initialize(&session->cmos,
        core_machine_configuration_cpu_borrow(session->core_machine),
        core_machine_configuration_port_borrow(session->core_machine));
}

C_VOID vm_session_machine_devices_initialize_fdc(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_machine_fdc_connect(&session->fdc, &session->fdd,
        core_machine_configuration_shared_dma_latch_borrow(session->core_machine),
        core_machine_configuration_shared_dma_primary_borrow(session->core_machine),
        core_machine_configuration_shared_dma_secondary_borrow(session->core_machine),
        core_machine_configuration_shared_pic_master_borrow(session->core_machine),
        core_machine_configuration_shared_pic_slave_borrow(session->core_machine),
        core_machine_configuration_port_borrow(session->core_machine));
    vm_machine_fdc_initialize(&session->fdc);
}

C_VOID vm_session_machine_devices_reset_cmos(vm_session *session)
{
    if (session != STD_NULL) vm_machine_cmos_reset(&session->cmos);
}

C_VOID vm_session_machine_devices_refresh_cmos(vm_session *session)
{
    if (session != STD_NULL) vm_machine_cmos_refresh(&session->cmos);
}

C_VOID vm_session_machine_devices_refresh(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_machine_fdd_refresh(&session->fdd);
    vm_machine_hdd_refresh(&session->hdd);
    vm_machine_cmos_refresh(&session->cmos);
    vm_machine_fdc_refresh(&session->fdc);
}

C_VOID vm_session_machine_devices_reset(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_machine_cmos_reset(&session->cmos);
    vm_machine_fdc_reset(&session->fdc);
    vm_machine_fdd_reset(&session->fdd);
    vm_machine_hdd_reset(&session->hdd);
}

C_VOID vm_session_machine_devices_finalize(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_machine_cmos_finalize(&session->cmos);
    vm_machine_fdc_finalize(&session->fdc);
    vm_machine_fdd_finalize(&session->fdd);
    vm_machine_hdd_finalize(&session->hdd);
}
