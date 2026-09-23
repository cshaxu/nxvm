#include "lib/types/types_interface.h"
#include "type.h"

#include "app-nxvm/product/config.h"
#include "app-nxvm/product/profile_binding.h"

type_status vm_app_configure_machine(const vm_session_request *request,
    vm_machine_config *out_config)
{
    lib_size index;

    if (out_config == LIB_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    lib_memory_set(out_config, 0, sizeof(*out_config));
    if (request == LIB_NULL || (lib_c_strcmp(request->display, "console") &&
         lib_c_strcmp(request->display, "window"))) return TYPE_STATUS_INVALID_ARGUMENT;
    out_config->profile_kind = VM_APP_PROFILE_KIND;
    out_config->cpu_profile = VM_APP_PROFILE_CPU;
    out_config->fpu_profile = VM_APP_PROFILE_FPU;
    out_config->floppy_format = VM_APP_PROFILE_FLOPPY_FORMAT;
    out_config->memory_bytes = request->memory_bytes;
    out_config->bios_count = VM_APP_PROFILE_BIOS_COUNT;
    out_config->bios_path[0u] = VM_APP_PROFILE_BIOS_0;
    out_config->bios_path[1u] = VM_APP_PROFILE_BIOS_1;
    out_config->video_path = VM_APP_PROFILE_VIDEO;
    out_config->cmos_seed = VM_APP_PROFILE_CMOS;
    out_config->font_path = VM_APP_PROFILE_FONT;
    for (index = 0u; index < VM_MACHINE_FLOPPY_SLOT_COUNT; ++index) {
        out_config->floppy_image[index] = index < request->floppy_count ?
            request->floppy[index] : LIB_NULL;
        out_config->floppy_mode[index] = request->floppy_mode[index];
    }
    for (index = 0u; index < VM_MACHINE_FIXED_DISK_SLOT_COUNT; ++index) {
        out_config->fixed_disk_image[index] = index < request->fixed_disk_count ?
            request->fixed_disk[index] : LIB_NULL;
        out_config->fixed_disk_mode[index] = request->fixed_disk_mode[index];
    }
    return TYPE_STATUS_OK;
}
