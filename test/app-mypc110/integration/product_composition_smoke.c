#include "app-mypc110/product_binding_interface.h"
#include "app-mypc110/pc110_plan_interface.h"
#include "app-mypc110/firmware_variants.h"
#include "lib/types/test.h"
#include "lib/types/file.h"
#include "lib/storage/file_interface.h"
#include "x86-nxvm/product/factory_interface.h"
#include "x86-nxvm/product/ini_interface.h"

int main(void)
{
    lib_u8 document[] = "[machine]\n"
        "memory_kib=20480\n"
        "[presentation]\n"
        "display=console\n"
        "console_control=0\n"
        "[media]\nfixed_disk0=pc110-vdisplay-direct.img|direct\n";
    x86_nxvm_product_request request = {0};
    x86_nxvm_product_machine machine = {0};
    x86_nxvm_product_information information = {0};
    x86_nxvm_product_machine_binding binding = vm_app_machine;
    mypc110_vdisplay display = {0};
    static const lib_u8 zeroes[4096u];
    lib_storage_file_writer *writer = LIB_NULL;
    lib_storage_medium *second = LIB_NULL;
    int failed = 0;

    lib_test_assert(lib_storage_file_writer_open("pc110-vdisplay-direct.img",
        LIB_STORAGE_FILE_WRITER_TRUNCATE, &writer) == LIB_STATUS_OK);
    for (lib_size i = 0u; i < 984u; ++i)
        lib_test_assert(lib_storage_file_writer_write(writer, zeroes, sizeof(zeroes)) == LIB_STATUS_OK);
    lib_test_assert(lib_storage_file_writer_close(writer) == LIB_STATUS_OK);

    mypc110_vdisplay_initialize(&display);
    display.firmware[0] = &mypc110_dstn_firmware;
    display.firmware[1] = &mypc110_tft_firmware;
    binding.context = &display;

    /* This integration owns no executable-adjacent user state. The process
     * Product integration tests exercise the actual file-backed binding. */
    binding.prepare = mypc110_machine_construction_create;
    binding.save_configuration = LIB_NULL;
    if (x86_nxvm_product_ini_parse((const lib_u8 *)".",
            (const lib_u8 *)"mypc110.ini", document, &request) != LIB_STATUS_OK ||
        request.memory_bytes != 20u * 1024u * 1024u ||
        x86_nxvm_product_compose_machine(&binding, &request, &machine) !=
            LIB_STATUS_OK) {
        return 1;
    }
    if (machine.composition.machine == LIB_NULL ||
        machine.composition.driver.copy_frame == LIB_NULL ||
        machine.information(machine.context, machine.composition.machine,
            &information) != LIB_STATUS_OK ||
        lib_text_compare(information.machine_name, "ibm-pc110") != 0 ||
        information.memory_bytes != 20u * 1024u * 1024u ||
        vm_app_machine.firmware == LIB_NULL || vm_app_machine.firmware->bios[0].data == LIB_NULL ||
        vm_app_machine.firmware->bios[0].bytes != 256u * 1024u ||
        vm_app_machine.firmware->bios[0].data[0x20af6u] != 0x15u) {
        failed = 2;
    }
    for (lib_size index = 0u; index < 4u; ++index) {
        const mypc110_display_variant selected = (index & 1u) == 0u ?
            MYPC110_DISPLAY_TFT : MYPC110_DISPLAY_DSTN;
        void *identity = machine.composition.machine;
        lib_test_assert(lib_storage_medium_open("pc110-vdisplay-direct.img",
            LIB_STORAGE_MEDIUM_DIRECT, &second) != LIB_STATUS_OK && second == LIB_NULL);
        lib_atomic_i32_store_explicit(&display.requested, selected, LIB_MEMORY_ORDER_SEQ_CST);
        lib_test_assert(machine.composition.driver.reset(machine.composition.driver.context));
        lib_test_assert(display.active == selected && machine.composition.machine == identity);
        lib_test_assert(display.present[1u - selected] && display.dirty[1u - selected]);
    }
    if (machine.composition.destroy(machine.composition.machine) != LIB_STATUS_OK)
        failed = 3;
    lib_test_assert(lib_storage_medium_open("pc110-vdisplay-direct.img",
        LIB_STORAGE_MEDIUM_DIRECT, &second) == LIB_STATUS_OK);
    lib_test_assert(lib_storage_medium_destroy(&second) == LIB_STATUS_OK);
    lib_test_assert(lib_c_remove("pc110-vdisplay-direct.img") == 0);
    return failed;
}
