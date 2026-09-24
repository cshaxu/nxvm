/* Copyright 2012-2014 Neko. */
#include "lib/types/types_interface.h"


#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/machine/machine_private.h"
#include "app-nxvm/machine/media/fdd.h"
#include "app-nxvm/machine/media/hdd.h"
#include "app-nxvm/machine/control.h"
#include "app-nxvm/machine/fault.h"

lib_status vm_machine_get_information(const vm_machine *session,
    vm_machine_information *out_information)
{
    core_machine_cpu_profile cpu_profile;
    const vm_machine_fault_outcome *fault;
    lib_size memory_bytes = 0u;
    lib_status status;

    if (session == LIB_NULL || out_information == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (session->core_machine == LIB_NULL) return LIB_STATUS_INVALID_STATE;
    status = core_machine_get_memory_bytes(session->core_machine, &memory_bytes);
    if (status != LIB_STATUS_OK) return status;
    status = core_machine_get_cpu_profile(session->core_machine, &cpu_profile);
    if (status != LIB_STATUS_OK) return status;
    lib_memory_set(out_information, 0, sizeof(*out_information));
    out_information->profile_kind = session->retained_config.profile_kind;
    out_information->cpu_profile = cpu_profile;
    out_information->memory_bytes = memory_bytes;
    out_information->floppy_image_bytes = vm_machine_fdd_image_size(&session->fdd);
    out_information->floppy_media_inserted = vm_machine_fdd_has_media(&session->fdd);
    out_information->fixed_disk_present = vm_profile_machine_plan_hdc_present(
        session->profile_plan);
    out_information->fixed_disk_cylinders = vm_machine_hdd_cylinders(&session->hdd);
    out_information->fixed_disk_image_bytes = vm_machine_hdd_image_size(&session->hdd);
    out_information->fixed_disk_media_connected = vm_machine_hdd_has_media(&session->hdd);
    out_information->external_firmware = session->profile_plan != LIB_NULL;
    out_information->active = vm_machine_executor_state_is_active(
        session->control.state);
    fault = &session->fault_outcome;
    out_information->fault_valid = fault->valid;
    if (fault->valid) {
        out_information->fault_detail = fault->run.detail;
        out_information->fault_linear_pc = fault->run.linear_pc;
        out_information->fault_exception_valid = fault->diagnostic.first_fault.valid;
        out_information->fault_exception_mask = fault->diagnostic.first_fault.exception_mask;
        out_information->fault_exception_code = fault->diagnostic.first_fault.exception_code;
        out_information->fault_exception_cs = fault->diagnostic.first_fault.point.cs;
        out_information->fault_exception_eip = fault->diagnostic.first_fault.point.eip;
    }
    return LIB_STATUS_OK;
}
