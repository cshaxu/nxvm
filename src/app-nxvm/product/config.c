#include "lib/types/types_interface.h"

#include "app-nxvm/product/config.h"
#include "app-nxvm/product/profile_binding.h"

static lib_bool vm_app_config_text_equal(const lib_u8 *left, const char *right)
{
    while (*left != '\0' && *right != '\0') {
        if (*left++ != (lib_u8)*right++) return LIB_FALSE;
    }
    return *left == '\0' && *right == '\0';
}

lib_status vm_app_configure_machine(const vm_session_request *request,
    vm_machine_config *out_config)
{
    lib_size index;

    if (out_config == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    lib_memory_set(out_config, 0, sizeof(*out_config));
    if (request == LIB_NULL || (!vm_app_config_text_equal(request->display, "console") &&
         !vm_app_config_text_equal(request->display, "window"))) return LIB_STATUS_INVALID_ARGUMENT;
    out_config->profile_kind = VM_APP_PROFILE_KIND;
    out_config->cpu_profile = VM_APP_PROFILE_CPU;
    out_config->fpu_profile = VM_APP_PROFILE_FPU;
    out_config->floppy_format = VM_APP_PROFILE_FLOPPY_FORMAT;
    out_config->memory_bytes = request->memory_bytes;
    out_config->bios_count = VM_APP_PROFILE_BIOS_COUNT;
    for (index = 0u; index < VM_MACHINE_FLOPPY_SLOT_COUNT; ++index) {
        out_config->floppy_image[index] = index < request->floppy_count ?
            (const char *)request->floppy[index] : LIB_NULL;
        out_config->floppy_mode[index] = request->floppy_mode[index];
    }
    for (index = 0u; index < VM_MACHINE_FIXED_DISK_SLOT_COUNT; ++index) {
        out_config->fixed_disk_image[index] = index < request->fixed_disk_count ?
            (const char *)request->fixed_disk[index] : LIB_NULL;
        out_config->fixed_disk_mode[index] = request->fixed_disk_mode[index];
    }
    return LIB_STATUS_OK;
}

static lib_status vm_app_prepare(const void *context,
    const vm_session_request *request, void **out_machine,
    common_machine_driver *out_driver)
{
    vm_machine_config config;
    vm_machine *machine = LIB_NULL;
    lib_status status;

    *out_machine = LIB_NULL;
    status = vm_app_configure_machine(request, &config);
    if (status == LIB_STATUS_OK)
        status = vm_machine_create_from_assets(&config, context, &machine);
    if (status == LIB_STATUS_OK)
        status = vm_machine_describe_common_driver(machine, out_driver);
    if (status != LIB_STATUS_OK) {
        vm_machine_destroy(machine);
        return status;
    }
    *out_machine = machine;
    return LIB_STATUS_OK;
}

static lib_status vm_app_bind(void *machine, common_machine *common)
{ return vm_machine_bind_common_machine(machine, common); }

static void vm_app_release_machine(void *machine)
{ vm_machine_destroy(machine); }

static lib_status vm_app_read_information(const void *machine,
    vm_app_information *out_info)
{
    vm_machine_information info;
    lib_status status = vm_machine_get_information(machine, &info);

    if (status != LIB_STATUS_OK) return status;
    *out_info = (vm_app_information){
        .machine_name = vm_profile_name(info.profile_kind),
        .cpu_name = core_machine_cpu_profile_name(info.cpu_profile),
        .memory_bytes = info.memory_bytes,
        .floppy_image_bytes = info.floppy_image_bytes,
        .floppy_media_inserted = info.floppy_media_inserted ? LIB_TRUE : LIB_FALSE,
        .fixed_disk_present = info.fixed_disk_present ? LIB_TRUE : LIB_FALSE,
        .fixed_disk_cylinders = info.fixed_disk_cylinders,
        .fixed_disk_image_bytes = info.fixed_disk_image_bytes,
        .fixed_disk_media_connected = info.fixed_disk_media_connected ? LIB_TRUE : LIB_FALSE,
        .external_firmware = info.external_firmware ? LIB_TRUE : LIB_FALSE
    };
    return LIB_STATUS_OK;
}

static lib_status vm_app_read_speed(const void *machine, vm_app_speed *out_speed)
{
    vm_machine_speed speed;
    lib_status status = vm_machine_get_speed(machine, &speed);

    if (status == LIB_STATUS_OK) *out_speed = speed == VM_MACHINE_SPEED_TURBO ?
        VM_APP_SPEED_TURBO : VM_APP_SPEED_STANDARD;
    return status;
}

static lib_status vm_app_write_speed(void *machine, vm_app_speed speed)
{
    return vm_machine_set_speed(machine, speed == VM_APP_SPEED_TURBO ?
        VM_MACHINE_SPEED_TURBO : VM_MACHINE_SPEED_STANDARD);
}

void vm_app_configure_factory(const vm_machine_assets *firmware,
    vm_app_factory *out_factory)
{
    *out_factory = (vm_app_factory){
        .context = firmware, .prepare = vm_app_prepare, .bind = vm_app_bind,
        .destroy = vm_app_release_machine, .information = vm_app_read_information,
        .get_speed = vm_app_read_speed, .set_speed = vm_app_write_speed
    };
}
