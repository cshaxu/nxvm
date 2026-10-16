#include "lib/types/types_interface.h"
#include "x86-nxvm/product/factory_interface.h"
#include "x86-nxvm/machine/machine_interface.h"

static lib_u32 phase;
static lib_u32 releases;
static lib_u32 creations;
static lib_u32 descriptions;
static x86_nxvm_machine_config observed;
static x86_nxvm_machine_speed speed;
static lib_status speed_status;
static lib_u8 candidate;

static void release_profile(void *context)
{ (void)context; ++releases; }

static lib_status prepare(const x86_nxvm_machine_config *config,
    const x86_nxvm_machine_assets *assets, x86_nxvm_machine_construction *construction)
{
    (void)assets;
    observed = *config;
    if (phase == 1u) return LIB_STATUS_UNSUPPORTED;
    *construction = (x86_nxvm_machine_construction){
        .profile = {.context = &candidate, .release = release_profile}
    };
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_machine_create(const x86_nxvm_machine_config *config,
    const x86_nxvm_machine_construction *construction, x86_nxvm_machine **out_machine)
{
    ++creations;
    observed = *config;
    *out_machine = LIB_NULL;
    if (phase == 2u) {
        construction->profile.release(construction->profile.context);
        return LIB_STATUS_NO_MEMORY;
    }
    *out_machine = (x86_nxvm_machine *)&candidate;
    return LIB_STATUS_OK;
}

void x86_nxvm_machine_destroy(x86_nxvm_machine *machine)
{ if (machine != LIB_NULL) ++releases; }

lib_status x86_nxvm_machine_describe_emulator_driver(x86_nxvm_machine *machine,
    emulator_machine_driver *driver)
{
    (void)machine;
    ++descriptions;
    driver->context = &candidate;
    if (phase == 3u) return LIB_STATUS_INVALID_STATE;
    *driver = (emulator_machine_driver){0};
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_machine_bind_emulator_machine(x86_nxvm_machine *machine, emulator_machine *emulator)
{ (void)machine; (void)emulator; return LIB_STATUS_OK; }

lib_status x86_nxvm_machine_get_information(const x86_nxvm_machine *machine,
    x86_nxvm_machine_information *information)
{
    (void)machine;
    *information = (x86_nxvm_machine_information){.cpu_profile = X86_CORE_CPU_PROFILE_80386,
        .memory_bytes = 1024u, .floppy_image_bytes = 360u, .floppy_media_inserted = LIB_TRUE,
        .fixed_disk_present = LIB_TRUE, .fixed_disk_cylinders = 40u,
        .fixed_disk_image_bytes = 4096u, .fixed_disk_media_connected = LIB_TRUE,
        .external_firmware = LIB_TRUE};
    return LIB_STATUS_OK;
}

const char *x86_core_cpu_profile_name(x86_core_cpu_profile profile)
{ (void)profile; return "80386"; }

lib_status x86_nxvm_machine_get_speed(const x86_nxvm_machine *machine, x86_nxvm_machine_speed *out_speed)
{ (void)machine; if (speed_status != LIB_STATUS_OK) return speed_status;
  *out_speed = speed; return LIB_STATUS_OK; }
lib_status x86_nxvm_machine_set_speed(x86_nxvm_machine *machine, x86_nxvm_machine_speed value)
{ (void)machine; if (speed_status != LIB_STATUS_OK) return speed_status;
  speed = value; return LIB_STATUS_OK; }

lib_i32 main(void)
{
    const x86_nxvm_product_machine_binding binding = {.name = "fixture", .cpu = X86_CORE_CPU_PROFILE_80386,
        .floppy_format = X86_NXVM_MACHINE_FLOPPY_FORMAT_1200K, .bios_count = 2u, .prepare = prepare};
    x86_nxvm_product_request request = {.display = "console", .floppy = {"disk"},
        .floppy_count = 1u, .floppy_mode = {LIB_STORAGE_MEDIUM_READONLY},
        .fixed_disk = {"hard"}, .fixed_disk_count = 1u,
        .fixed_disk_mode = {LIB_STORAGE_MEDIUM_OVERLAY}, .memory_bytes = 1024u};
    x86_nxvm_machine_config config;
    const x86_nxvm_machine_config empty = {0};
    x86_nxvm_product_machine machine;
    x86_nxvm_product_information info;
    x86_nxvm_machine_speed app_speed;
    x86_ibmpc_product_command_extensions extensions;
    emulator_session_command_result result;
    const lib_status expected[] = {LIB_STATUS_OK, LIB_STATUS_UNSUPPORTED,
        LIB_STATUS_NO_MEMORY, LIB_STATUS_INVALID_STATE};

    lib_memory_set(&config, 0xff, sizeof(config));
    if (x86_nxvm_product_configure_machine(&binding, LIB_NULL, &config) != LIB_STATUS_INVALID_ARGUMENT ||
        lib_memory_compare(&config, &empty, sizeof(config)) != 0 ||
        x86_nxvm_product_configure_machine(LIB_NULL, &request, &config) != LIB_STATUS_INVALID_ARGUMENT ||
        x86_nxvm_product_configure_machine(&binding, &request, LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT)
        return 1;
    for (phase = 0u; phase < 4u; ++phase) {
        releases = creations = descriptions = 0u;
        machine = (x86_nxvm_product_machine){.composition.machine = &candidate};
        if (x86_nxvm_product_compose_machine(&binding, &request, &machine) != expected[phase])
            return 2;
        if (phase == 0u) machine.composition.destroy(machine.composition.machine);
        if ((phase != 0u && (machine.composition.machine != LIB_NULL ||
            machine.composition.driver.context != LIB_NULL)) ||
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
    if (x86_nxvm_product_compose_machine(&binding, &request, &machine) != LIB_STATUS_OK ||
        machine.information(machine.context, machine.composition.machine, &info) != LIB_STATUS_OK ||
        info.machine_name != binding.name || lib_text_compare(info.cpu_name, "80386") != 0 ||
        info.memory_bytes != 1024u || info.fixed_disk_cylinders != 40u ||
        info.fixed_disk_image_bytes != 4096u || info.floppy_image_bytes != 360u ||
        !info.floppy_media_inserted || !info.fixed_disk_present ||
        !info.fixed_disk_media_connected || !info.external_firmware) return 5;
    if (machine.set_speed(machine.composition.machine, X86_NXVM_MACHINE_SPEED_TURBO) != LIB_STATUS_OK ||
        speed != X86_NXVM_MACHINE_SPEED_TURBO ||
        machine.get_speed(machine.composition.machine, &app_speed) != LIB_STATUS_OK ||
        app_speed != X86_NXVM_MACHINE_SPEED_TURBO ||
        machine.set_speed(machine.composition.machine, X86_NXVM_MACHINE_SPEED_STANDARD) != LIB_STATUS_OK ||
        speed != X86_NXVM_MACHINE_SPEED_STANDARD) return 6;
    speed_status = LIB_STATUS_INVALID_STATE;
    app_speed = X86_NXVM_MACHINE_SPEED_TURBO;
    if (machine.get_speed(machine.composition.machine, &app_speed) != speed_status ||
        app_speed != X86_NXVM_MACHINE_SPEED_TURBO ||
        machine.set_speed(machine.composition.machine, X86_NXVM_MACHINE_SPEED_TURBO) != speed_status ||
        speed != X86_NXVM_MACHINE_SPEED_STANDARD) return 8;
    speed_status = LIB_STATUS_OK;
    if (x86_nxvm_product_configure_standard_extensions(&machine, &extensions) != LIB_STATUS_OK ||
        extensions.submit == LIB_NULL ||
        !extensions.submit(extensions.context, LIB_NULL,
            EMULATOR_SESSION_MACHINE_STOPPED, "info", &result) ||
        !result.arm_prompt || lib_text_compare_n(result.text, "Device Info\r\n",
            sizeof("Device Info\r\n") - 1u) != 0)
        return 7;
    machine.composition.destroy(machine.composition.machine);
    return 0;
}
