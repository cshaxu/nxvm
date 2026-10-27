#include "app-mypc110/product_binding_interface.h"
#include "app-mypc110/pc110_cmos.h"
#include "app-mypc110/pc110_plan_interface.h"
#include "x86-nxvm/product/startup_interface.h"

#ifndef MYPC110_MACHINE_KEY
#error "MyPC110 build must select one fixed machine key."
#endif

static lib_status mypc110_prepare(const x86_nxvm_machine_config *config,
    const x86_nxvm_machine_assets *assets, x86_nxvm_machine_construction *out)
{
    lib_u8 path[1024];
    lib_u8 image[X86_NXVM_MACHINE_EXTENDED_CMOS_SEED_BYTES];
    x86_nxvm_machine_assets selected;
    lib_bool present = LIB_FALSE;
    lib_status status;

    if (config == LIB_NULL || assets == LIB_NULL || out == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out = (x86_nxvm_machine_construction){0};
    status = x86_nxvm_product_ini_executable_path(MYPC110_CMOS_NAME, path, sizeof(path));
    if (status == LIB_STATUS_OK)
        status = mypc110_cmos_load((const char *)path, image, &present);
    if (status != LIB_STATUS_OK) return status;
    selected = *assets;
    if (present) selected.cmos_seed = (x86_nxvm_machine_asset_bytes){image, sizeof(image)};
    return mypc110_machine_construction_create(config, &selected, out);
}

static lib_status mypc110_save_configuration(x86_nxvm_machine *machine)
{
    lib_u8 path[1024];
    lib_status status = x86_nxvm_product_ini_executable_path(MYPC110_CMOS_NAME,
        path, sizeof(path));
    return status == LIB_STATUS_OK ? mypc110_cmos_save((const char *)path, machine) : status;
}

const x86_nxvm_product_machine_binding vm_app_machine = {
    .name = MYPC110_MACHINE_KEY,
    .cpu = X86_CORE_CPU_PROFILE_80486SX,
    .fpu = X86_FPU_PROFILE_NONE,
    /* The selected PC110 firmware performs long, intentional port-61 delay
     * loops during POST.  Reuse the existing non-pacing runner policy rather
     * than special-casing BIOS code; the user may still select standard. */
    .initial_speed = X86_NXVM_MACHINE_SPEED_TURBO,
    .floppy_format = X86_NXVM_MACHINE_FLOPPY_FORMAT_1440K,
    .bios_count = 1u,
    .firmware = &vm_app_firmware,
    .prepare = mypc110_prepare,
    .save_configuration = mypc110_save_configuration
};
