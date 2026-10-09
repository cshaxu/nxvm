#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "ibmpc/nxvm/factory_interface.h"
#include "ibmpc/machine/machine_interface.h"

lib_status vm_app_configure_machine(const vm_app_machine_binding *binding,
    const vm_session_request *request,
    vm_machine_config *out_config)
{
    lib_size index;

    if (out_config == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    lib_memory_set(out_config, 0, sizeof(*out_config));
    if (binding == LIB_NULL || request == LIB_NULL || (lib_text_compare((const char *)request->display, "console") != 0 &&
         lib_text_compare((const char *)request->display, "window") != 0)) return LIB_STATUS_INVALID_ARGUMENT;
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

static void vm_app_extension_append(common_session_command_result *out,
    const char *format, ...)
{
    lib_size used;
    lib_i32 written;
    lib_c_va_list arguments;

    if (out == LIB_NULL || format == LIB_NULL) return;
    used = lib_text_length((const char *)out->text);
    if (used >= sizeof(out->text)) return;
    lib_c_va_start(arguments, format);
    written = lib_c_vsnprintf((char *)out->text + used,
        sizeof(out->text) - used, format, arguments);
    lib_c_va_end(arguments);
    if (written < 0) out->text[used] = '\0';
}

static lib_bool vm_app_extension_normalize(const char *source,
    lib_u8 *destination, lib_size capacity)
{
    lib_size length;
    lib_size begin = 0u;
    lib_size end;
    lib_size index;

    if (source == LIB_NULL || destination == LIB_NULL) return LIB_FALSE;
    length = lib_text_length(source);
    while (begin < length && (source[begin] == ' ' || source[begin] == '\t')) ++begin;
    end = length;
    while (end > begin && (source[end - 1u] == ' ' || source[end - 1u] == '\t')) --end;
    if (end - begin >= capacity) return LIB_FALSE;
    for (index = 0u; begin + index < end; ++index) {
        lib_u8 value = (lib_u8)source[begin + index];
        destination[index] = value >= 'A' && value <= 'Z' ?
            (lib_u8)(value + ('a' - 'A')) : value;
    }
    destination[index] = '\0';
    return LIB_TRUE;
}

static lib_bool vm_app_extension_floppy_mode(const char *text,
    lib_storage_medium_mode *out_mode)
{
    if (lib_text_compare(text, "direct") == 0)
        *out_mode = LIB_STORAGE_MEDIUM_DIRECT;
    else if (lib_text_compare(text, "readonly") == 0)
        *out_mode = LIB_STORAGE_MEDIUM_READONLY;
    else if (lib_text_compare(text, "overlay") == 0)
        *out_mode = LIB_STORAGE_MEDIUM_OVERLAY;
    else
        return LIB_FALSE;
    return LIB_TRUE;
}

static lib_bool vm_app_extension_floppy(common_machine *machine,
    common_session_machine_state state, char *command,
    common_session_command_result *out)
{
    char *operation = command;
    char *mode;
    char *path;
    lib_storage_medium_mode medium_mode;

    if (lib_text_compare(operation, "floppy") != 0)
        return LIB_FALSE;
    operation += 7u;
    while (*operation == ' ' || *operation == '\t')
        ++operation;
    if (state == COMMON_SESSION_MACHINE_RUNNING) {
        vm_app_extension_append(out, "Cannot change floppy media now.\r\n\r\n");
        return LIB_TRUE;
    }
    if (lib_text_compare(operation, "eject") == 0) {
        vm_app_extension_append(out,
            common_machine_set_removable_media(machine, LIB_NULL,
                LIB_STORAGE_MEDIUM_OVERLAY) ?
            "Floppy disk ejected.\r\n\r\n" : "Cannot eject floppy disk.\r\n\r\n");
        return LIB_TRUE;
    }
    if (lib_text_compare(operation, "insert ") != 0) {
        vm_app_extension_append(out,
            "Usage: floppy insert <readonly|direct|overlay> <image> | floppy eject\r\n\r\n");
        return LIB_TRUE;
    }
    mode = operation + 7u;
    path = mode;
    while (*path != '\0' && *path != ' ' && *path != '\t')
        ++path;
    if (*path == '\0') {
        vm_app_extension_append(out,
            "Usage: floppy insert <readonly|direct|overlay> <image> | floppy eject\r\n\r\n");
        return LIB_TRUE;
    }
    *path++ = '\0';
    while (*path == ' ' || *path == '\t')
        ++path;
    if (*path == '\0' || !vm_app_extension_floppy_mode(mode, &medium_mode)) {
        vm_app_extension_append(out,
            "Usage: floppy insert <readonly|direct|overlay> <image> | floppy eject\r\n\r\n");
        return LIB_TRUE;
    }
    vm_app_extension_append(out,
        common_machine_set_removable_media(machine, path, medium_mode) ?
        "Floppy disk inserted.\r\n\r\n" : "Cannot read floppy disk.\r\n\r\n");
    return LIB_TRUE;
}

static lib_bool vm_app_standard_extension(void *context,
    common_machine *machine, common_session_machine_state state,
    const char *line, common_session_command_result *out)
{
    vm_app *app = context;
    vm_app_information information;
    vm_app_speed speed;
    lib_u8 command[APP_COMMAND_PATH_CAPACITY];

    (void)machine;
    if (app == LIB_NULL || line == LIB_NULL || out == LIB_NULL) return LIB_FALSE;
    if (!vm_app_extension_normalize(line, command, sizeof(command))) return LIB_FALSE;
    *out = (common_session_command_result){0};
    if (lib_text_compare((const char *)command, "info") == 0) {
        if (vm_app_information_read(app, &information) != LIB_STATUS_OK) {
            vm_app_extension_append(out, "Machine information unavailable.\r\n\r\n");
            return LIB_TRUE;
        }
        vm_app_extension_append(out,
            "Device Info\r\n===========\r\nMachine:           %s\r\n"
            "CPU:               Intel %s\r\nRAM Size:          %u %s\r\n"
            "Floppy Disk Drive: %u bytes, %s\r\n",
            information.machine_name, information.cpu_name,
            (lib_u32)(information.memory_bytes < (1u << 20) ?
                information.memory_bytes >> 10 : information.memory_bytes >> 20),
            information.memory_bytes < (1u << 20) ? "KB" : "MB",
            (lib_u32)information.floppy_image_bytes,
            information.floppy_media_inserted ? "inserted" : "not inserted");
        if (information.fixed_disk_present)
            vm_app_extension_append(out,
                "Hard Disk Drive:   %u cylinders, %u bytes, %s\r\n",
                (lib_u32)information.fixed_disk_cylinders,
                (lib_u32)information.fixed_disk_image_bytes,
                information.fixed_disk_media_connected ? "connected" : "disconnected");
        vm_app_extension_append(out, "\r\nBIOS: %s\r\nRunning: %s\r\n\r\n",
            information.external_firmware ? "external ROM mapped at F0000h" :
                "profile ROM mapped",
            state == COMMON_SESSION_MACHINE_RUNNING ? "Yes" : "No");
        return LIB_TRUE;
    }
    if (lib_text_compare((const char *)command, "speed") == 0) {
        if (vm_app_speed_read(app, &speed) == LIB_STATUS_OK)
            vm_app_extension_append(out, "Speed: %s\r\n\r\n",
                speed == VM_APP_SPEED_TURBO ? "turbo" : "standard");
        return LIB_TRUE;
    }
    if (lib_text_compare((const char *)command, "speed standard") == 0 ||
        lib_text_compare((const char *)command, "speed turbo") == 0) {
        speed = lib_text_compare((const char *)command, "speed turbo") == 0 ?
            VM_APP_SPEED_TURBO : VM_APP_SPEED_STANDARD;
        if (vm_app_speed_write(app, speed) == LIB_STATUS_OK)
            vm_app_extension_append(out, "Speed: %s\r\n\r\n",
                speed == VM_APP_SPEED_TURBO ? "turbo" : "standard");
        else
            vm_app_extension_append(out,
                "Cannot change speed while session is running.\r\n\r\n");
        return LIB_TRUE;
    }
    return vm_app_extension_floppy(machine, state, (char *)command, out);
}

lib_status vm_app_configure_standard_extensions(vm_app *app,
    app_command_extensions *out_extensions)
{
    if (app == LIB_NULL || out_extensions == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_extensions = (app_command_extensions){
        .context = app,
        .submit = vm_app_standard_extension,
        .help_text = "  info           list device information\r\n"
            "  speed [standard|turbo]\r\n"
            "  floppy insert <mode> <image>\r\n"
            "                 insert drive A media while stopped/paused\r\n"
            "  floppy eject   eject drive A media while stopped/paused\r\n"
    };
    return LIB_STATUS_OK;
}
