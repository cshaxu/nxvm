#include "app-mypc110/product_binding_interface.h"
#include "x86-nxvm/product/factory_interface.h"
#include "x86-nxvm/product/ini_interface.h"

#ifndef MYPC110_EXPECTED_MACHINE_NAME
#error "The MyPC110 product-composition test needs one fixed machine identity."
#endif

#ifndef MYPC110_EXPECTED_PANEL_VALUE
#error "The MyPC110 product-composition test needs one fixed panel byte."
#endif

int main(void)
{
    lib_u8 document[] = "[machine]\n"
        "memory_kib=20480\n"
        "[presentation]\n"
        "display=console\n"
        "console_control=0\n"
        "[media]\n";
    x86_nxvm_product_request request = {0};
    x86_nxvm_product_machine machine = {0};
    x86_nxvm_product_information information = {0};
    int failed = 0;

    if (x86_nxvm_product_ini_parse((const lib_u8 *)".",
            (const lib_u8 *)"mypc110.ini", document, &request) != LIB_STATUS_OK ||
        request.memory_bytes != 20u * 1024u * 1024u ||
        x86_nxvm_product_compose_machine(&vm_app_machine, &request, &machine) !=
            LIB_STATUS_OK) {
        return 1;
    }
    if (machine.composition.machine == LIB_NULL ||
        machine.composition.driver.copy_frame == LIB_NULL ||
        machine.information(machine.context, machine.composition.machine,
            &information) != LIB_STATUS_OK ||
        lib_text_compare(information.machine_name, MYPC110_EXPECTED_MACHINE_NAME) != 0 ||
        information.memory_bytes != 20u * 1024u * 1024u ||
        vm_app_machine.firmware == LIB_NULL || vm_app_machine.firmware->bios[0].data == LIB_NULL ||
        vm_app_machine.firmware->bios[0].bytes != 256u * 1024u ||
        vm_app_machine.firmware->bios[0].data[0x20af6u] != MYPC110_EXPECTED_PANEL_VALUE) {
        failed = 2;
    }
    if (machine.composition.destroy(machine.composition.machine) != LIB_STATUS_OK)
        failed = 3;
    return failed;
}
