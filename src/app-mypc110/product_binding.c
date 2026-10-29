#include "app-mypc110/product_binding_interface.h"
#include "app-mypc110/pc110_cmos.h"
#include "app-mypc110/pc110_plan_interface.h"
#include "app-mypc110/firmware_variants.h"
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
    status = mypc110_machine_construction_create(config, &selected, out);
    if (status == LIB_STATUS_OK && present && image[0x0du] == 0x80u) {
        lib_memory_copy(out->rtc_seed, image, sizeof(out->rtc_seed));
        out->rtc_seed_present = LIB_TRUE;
    }
    return status;
}

static const char *mypc110_cmos_names[] = {"mypc110-dstn.cmos", "mypc110-tft.cmos"};

lib_status mypc110_product_initialize_vdisplay(mypc110_vdisplay *display)
{
    lib_u8 path[1024];
    lib_status status;
    if (display == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *display = (mypc110_vdisplay){0};
    mypc110_vdisplay_initialize(display);
    display->firmware[MYPC110_DISPLAY_DSTN] = &mypc110_dstn_firmware;
    display->firmware[MYPC110_DISPLAY_TFT] = &mypc110_tft_firmware;
    for (lib_size index = 0u; index < 2u; ++index) {
        status = x86_nxvm_product_ini_executable_path(mypc110_cmos_names[index], path, sizeof(path));
        if (status == LIB_STATUS_OK)
            status = mypc110_cmos_load((const char *)path, display->cmos[index], &display->present[index]);
        if (status != LIB_STATUS_OK) return status;
    }
    if (!display->present[MYPC110_DISPLAY_TFT] &&
        mypc110_tft_firmware.cmos_seed.bytes == X86_NXVM_MACHINE_EXTENDED_CMOS_SEED_BYTES)
        lib_memory_copy(display->cmos[MYPC110_DISPLAY_TFT], mypc110_tft_firmware.cmos_seed.data,
            X86_NXVM_MACHINE_EXTENDED_CMOS_SEED_BYTES);
    return LIB_STATUS_OK;
}

static lib_bool mypc110_product_submit(void *context, emulator_machine *machine,
    emulator_session_machine_state state, const char *line, emulator_session_command_result *out)
{
    mypc110_vdisplay *display = context;
    if (mypc110_vdisplay_submit(display, machine, state, line, out)) return LIB_TRUE;
    return display->standard.submit(display->standard.context, machine, state, line, out);
}

static lib_status mypc110_product_configure_extensions(void *context,
    x86_nxvm_product_machine *machine, x86_ibmpc_product_command_extensions *out)
{
    mypc110_vdisplay *display = context;
    lib_status status;
    if (display == LIB_NULL || out == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_nxvm_product_configure_standard_extensions(machine, &display->standard);
    if (status != LIB_STATUS_OK) return status;
    if (display->standard.help.count + 1u > sizeof(display->help) / sizeof(display->help[0]))
        return LIB_STATUS_LIMIT_EXCEEDED;
    for (lib_size index = 0u; index < display->standard.help.count; ++index)
        display->help[index] = display->standard.help.rows[index];
    display->help[display->standard.help.count] = (emulator_product_help_row){
        "vdisplay=dstn|tft", "select display while stopped"};
    *out = (x86_ibmpc_product_command_extensions){.context = display,
        .submit = mypc110_product_submit,
        .help = {display->help, display->standard.help.count + 1u}};
    return LIB_STATUS_OK;
}

static lib_status mypc110_save_configuration(void *context, x86_nxvm_machine *machine)
{
    mypc110_vdisplay *display = context;
    lib_u8 path[1024];
    lib_status status;
    if (display == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_nxvm_machine_capture_cmos(machine, display->cmos[display->active],
        X86_NXVM_MACHINE_EXTENDED_CMOS_SEED_BYTES);
    if (status == LIB_STATUS_OK)
        status = x86_nxvm_machine_capture_rtc(machine, display->cmos[display->active],
            X86_NXVM_MACHINE_RTC_SEED_BYTES);
    if (status != LIB_STATUS_OK) return status;
    display->dirty[display->active] = LIB_TRUE;
    for (lib_size index = 0u; index < 2u; ++index) {
        if (!display->dirty[index]) continue;
        status = x86_nxvm_product_ini_executable_path(mypc110_cmos_names[index], path, sizeof(path));
        if (status == LIB_STATUS_OK)
            status = mypc110_cmos_save_image((const char *)path, display->cmos[index]);
        if (status != LIB_STATUS_OK) return status;
        display->dirty[index] = LIB_FALSE;
    }
    return LIB_STATUS_OK;
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
    .firmware = &mypc110_dstn_firmware,
    .prepare = mypc110_prepare,
    .attach_context = mypc110_machine_attach_vdisplay,
    .save_configuration = mypc110_save_configuration,
    .configure_extensions = mypc110_product_configure_extensions
};
