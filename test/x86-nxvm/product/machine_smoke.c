#include "lib/types/test.h"
#include "x86-nxvm/product/machine_interface.h"

static x86_nxvm_machine_speed current = X86_NXVM_MACHINE_SPEED_STANDARD;

static lib_status information(const void *context, const void *machine,
    x86_nxvm_product_information *out)
{
    lib_test_assert(context == machine);
    *out = (x86_nxvm_product_information){
        .machine_name = "fixture", .cpu_name = "fixture-cpu",
        .memory_bytes = 1024u, .floppy_image_bytes = 512u,
        .floppy_media_inserted = LIB_TRUE, .fixed_disk_present = LIB_TRUE,
        .fixed_disk_cylinders = 2u, .fixed_disk_image_bytes = 2048u,
        .fixed_disk_media_connected = LIB_TRUE, .external_firmware = LIB_TRUE};
    return LIB_STATUS_OK;
}

static lib_status get_speed(const void *machine, x86_nxvm_machine_speed *out)
{
    lib_test_assert(machine == &current);
    *out = current;
    return LIB_STATUS_OK;
}

static lib_status set_speed(void *machine, x86_nxvm_machine_speed speed)
{
    lib_test_assert(machine == &current);
    current = speed;
    return LIB_STATUS_OK;
}

lib_i32 main(void)
{
    x86_nxvm_product_machine app = {
        .composition = {.machine = &current}, .context = &current,
        .information = information, .get_speed = get_speed, .set_speed = set_speed};
    x86_nxvm_product_information info = {0};
    x86_nxvm_machine_speed speed;
    lib_test_assert(app.information(app.context, app.composition.machine, &info) == LIB_STATUS_OK);
    lib_test_assert(info.machine_name && info.cpu_name && info.memory_bytes == 1024u);
    lib_test_assert(info.floppy_image_bytes == 512u && info.floppy_media_inserted);
    lib_test_assert(info.fixed_disk_present && info.fixed_disk_cylinders == 2u);
    lib_test_assert(info.fixed_disk_image_bytes == 2048u && info.fixed_disk_media_connected);
    lib_test_assert(info.external_firmware);
    lib_test_assert(app.get_speed(app.composition.machine, &speed) == LIB_STATUS_OK);
    lib_test_assert(speed == X86_NXVM_MACHINE_SPEED_STANDARD);
    lib_test_assert(app.set_speed(app.composition.machine, X86_NXVM_MACHINE_SPEED_TURBO) == LIB_STATUS_OK);
    lib_test_assert(app.get_speed(app.composition.machine, &speed) == LIB_STATUS_OK);
    lib_test_assert(speed == X86_NXVM_MACHINE_SPEED_TURBO);
    return 0;
}
