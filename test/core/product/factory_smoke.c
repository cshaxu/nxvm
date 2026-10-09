#include "lib/types/types_interface.h"
#include "core/product/factory_interface.h"
#include "core/machine/machine_interface.h"

static lib_u32 phase;
static lib_u32 releases;
static lib_u32 creations;
static lib_u32 descriptions;
static vm_machine_config observed;
static vm_machine_speed speed;
static lib_u8 candidate;

static void release_profile(void *context)
{ (void)context; ++releases; }

static lib_status prepare(const vm_machine_config *config,
    const vm_machine_assets *assets, vm_machine_construction *construction)
{
    (void)assets;
    observed = *config;
    if (phase == 1u) return LIB_STATUS_UNSUPPORTED;
    *construction = (vm_machine_construction){
        .profile = {.context = &candidate, .release = release_profile}
    };
    return LIB_STATUS_OK;
}

lib_status vm_machine_create(const vm_machine_config *config,
    const vm_machine_construction *construction, vm_machine **out_machine)
{
    ++creations;
    observed = *config;
    *out_machine = LIB_NULL;
    if (phase == 2u) {
        construction->profile.release(construction->profile.context);
        return LIB_STATUS_NO_MEMORY;
    }
    *out_machine = (vm_machine *)&candidate;
    return LIB_STATUS_OK;
}

void vm_machine_destroy(vm_machine *machine)
{ if (machine != LIB_NULL) ++releases; }

lib_status vm_machine_describe_emulator_driver(vm_machine *machine,
    emulator_machine_driver *driver)
{
    (void)machine;
    ++descriptions;
    driver->context = &candidate;
    if (phase == 3u) return LIB_STATUS_INVALID_STATE;
    *driver = (emulator_machine_driver){0};
    return LIB_STATUS_OK;
}

lib_status vm_machine_bind_emulator_machine(vm_machine *machine, emulator_machine *emulator)
{ (void)machine; (void)emulator; return LIB_STATUS_OK; }

lib_status vm_machine_get_information(const vm_machine *machine,
    vm_machine_information *information)
{
    (void)machine;
    *information = (vm_machine_information){.cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .memory_bytes = 1024u, .floppy_image_bytes = 360u, .floppy_media_inserted = LIB_TRUE,
        .fixed_disk_present = LIB_TRUE, .fixed_disk_cylinders = 40u,
        .fixed_disk_image_bytes = 4096u, .fixed_disk_media_connected = LIB_TRUE,
        .external_firmware = LIB_TRUE};
    return LIB_STATUS_OK;
}

const char *core_machine_cpu_profile_name(core_machine_cpu_profile profile)
{ (void)profile; return "80386"; }

lib_status vm_machine_get_speed(const vm_machine *machine, vm_machine_speed *out_speed)
{ (void)machine; *out_speed = speed; return LIB_STATUS_OK; }
lib_status vm_machine_set_speed(vm_machine *machine, vm_machine_speed value)
{ (void)machine; speed = value; return LIB_STATUS_OK; }

lib_i32 main(void)
{
    const vm_app_machine_binding binding = {.name = "fixture", .cpu = CORE_MACHINE_CPU_PROFILE_80386,
        .floppy_format = VM_MACHINE_FLOPPY_FORMAT_1200K, .bios_count = 2u, .prepare = prepare};
    vm_session_request request = {.display = "console", .floppy = {"disk"},
        .floppy_count = 1u, .floppy_mode = {LIB_STORAGE_MEDIUM_READONLY},
        .fixed_disk = {"hard"}, .fixed_disk_count = 1u,
        .fixed_disk_mode = {LIB_STORAGE_MEDIUM_OVERLAY}, .memory_bytes = 1024u};
    vm_machine_config config;
    const vm_machine_config empty = {0};
    app_composed_machine machine;
    product_surface_information info;
    product_surface_speed app_speed;
    const lib_status expected[] = {LIB_STATUS_OK, LIB_STATUS_UNSUPPORTED,
        LIB_STATUS_NO_MEMORY, LIB_STATUS_INVALID_STATE};

    lib_memory_set(&config, 0xff, sizeof(config));
    if (vm_app_configure_machine(&binding, LIB_NULL, &config) != LIB_STATUS_INVALID_ARGUMENT ||
        lib_memory_compare(&config, &empty, sizeof(config)) != 0 ||
        vm_app_configure_machine(LIB_NULL, &request, &config) != LIB_STATUS_INVALID_ARGUMENT ||
        vm_app_configure_machine(&binding, &request, LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT)
        return 1;
    for (phase = 0u; phase < 4u; ++phase) {
        releases = creations = descriptions = 0u;
        machine = (app_composed_machine){.machine = &candidate};
        if (vm_app_compose_machine(&binding, &request, &machine) != expected[phase])
            return 2;
        if (phase == 0u) machine.destroy(machine.machine);
        if ((phase != 0u && (machine.machine != LIB_NULL || machine.driver.context != LIB_NULL)) ||
            releases != (phase == 1u ? 0u : 1u) ||
            creations != (phase == 1u ? 0u : 1u) ||
            descriptions != (phase == 1u || phase == 2u ? 0u : 1u)) return 3;
    }
    if (observed.cpu_profile != binding.cpu || observed.bios_count != 2u ||
        observed.memory_bytes != request.memory_bytes ||
        observed.floppy_image[0] != (const char *)request.floppy[0] ||
        observed.floppy_image[1] != LIB_NULL ||
        observed.floppy_mode[0] != LIB_STORAGE_MEDIUM_READONLY ||
        observed.fixed_disk_image[0] != (const char *)request.fixed_disk[0] ||
        observed.fixed_disk_mode[0] != LIB_STORAGE_MEDIUM_OVERLAY) return 4;
    phase = 0u;
    if (vm_app_compose_machine(&binding, &request, &machine) != LIB_STATUS_OK ||
        machine.information(machine.context, machine.machine, &info) != LIB_STATUS_OK ||
        info.machine_name != binding.name || lib_text_compare(info.cpu_name, "80386") != 0 ||
        info.memory_bytes != 1024u || info.fixed_disk_cylinders != 40u ||
        info.fixed_disk_image_bytes != 4096u || info.floppy_image_bytes != 360u ||
        !info.floppy_media_inserted || !info.fixed_disk_present ||
        !info.fixed_disk_media_connected || !info.external_firmware) return 5;
    if (machine.set_speed(machine.machine, PRODUCT_SURFACE_SPEED_TURBO) != LIB_STATUS_OK ||
        speed != VM_MACHINE_SPEED_TURBO ||
        machine.get_speed(machine.machine, &app_speed) != LIB_STATUS_OK ||
        app_speed != PRODUCT_SURFACE_SPEED_TURBO ||
        machine.set_speed(machine.machine, PRODUCT_SURFACE_SPEED_STANDARD) != LIB_STATUS_OK ||
        speed != VM_MACHINE_SPEED_STANDARD) return 6;
    machine.destroy(machine.machine);
    return 0;
}
