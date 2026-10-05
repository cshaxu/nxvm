#include "lib/types/types_interface.h"

#include "ibmpc/product/factory_interface.h"
#include "ibmpc/machine/machine_interface.h"

static lib_bool vm_app_config_text_equal(const lib_u8 *left, const char *right)
{
    while (*left != '\0' && *right != '\0') {
        if (*left++ != (lib_u8)*right++) return LIB_FALSE;
    }
    return *left == '\0' && *right == '\0';
}

lib_status vm_app_configure_machine(const vm_app_machine_binding *binding,
    const vm_session_request *request,
    vm_machine_config *out_config)
{
    lib_size index;

    if (out_config == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    lib_memory_set(out_config, 0, sizeof(*out_config));
    if (binding == LIB_NULL || request == LIB_NULL || (!vm_app_config_text_equal(request->display, "console") &&
         !vm_app_config_text_equal(request->display, "window"))) return LIB_STATUS_INVALID_ARGUMENT;
    out_config->cpu_profile = binding->cpu;
    out_config->fpu_profile = binding->fpu;
    out_config->floppy_format = binding->floppy_format;
    out_config->memory_bytes = request->memory_bytes;
    out_config->bios_count = binding->bios_count;
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
    const vm_app_machine_binding *binding = context;
    vm_machine_config config;
    vm_machine_construction construction;
    vm_machine *machine = LIB_NULL;
    lib_status status;

    if (out_machine == LIB_NULL || out_driver == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_machine = LIB_NULL;
    *out_driver = (common_machine_driver){0};
    if (binding == LIB_NULL || binding->prepare == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    status = vm_app_configure_machine(binding, request, &config);
    if (status == LIB_STATUS_OK)
        status = binding->prepare(&config, binding->firmware, &construction);
    if (status == LIB_STATUS_OK)
        status = vm_machine_create(&config, &construction, &machine);
    if (status == LIB_STATUS_OK)
        status = vm_machine_describe_common_driver(machine, out_driver);
    if (status != LIB_STATUS_OK) {
        vm_machine_destroy(machine);
        *out_driver = (common_machine_driver){0};
        return status;
    }
    *out_machine = machine;
    return LIB_STATUS_OK;
}

static lib_status vm_app_bind(void *machine, common_machine *common)
{ return vm_machine_bind_common_machine(machine, common); }

static void vm_app_release_machine(void *machine)
{ vm_machine_destroy(machine); }

static lib_status vm_app_read_information(const void *context, const void *machine,
    vm_app_information *out_info)
{
    const vm_app_machine_binding *binding = context;
    vm_machine_information info;
    lib_status status = vm_machine_get_information(machine, &info);

    if (status != LIB_STATUS_OK) return status;
    *out_info = (vm_app_information){
        .machine_name = binding->name,
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

void vm_app_configure_factory(const vm_app_machine_binding *binding,
    vm_app_factory *out_factory)
{
    *out_factory = (vm_app_factory){
        .context = binding, .prepare = vm_app_prepare, .bind = vm_app_bind,
        .destroy = vm_app_release_machine, .information = vm_app_read_information,
        .get_speed = vm_app_read_speed, .set_speed = vm_app_write_speed
    };
}
