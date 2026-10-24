#include "app-mypc110/product_binding_interface.h"
#include "x86-nxvm/product/factory_interface.h"
#include "x86-nxvm/product/ini_interface.h"

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
        lib_text_compare(information.machine_name, "ibm-pc110-tft") != 0 ||
        information.memory_bytes != 20u * 1024u * 1024u) {
        failed = 2;
    }
    if (machine.composition.destroy(machine.composition.machine) != LIB_STATUS_OK)
        failed = 3;
    return failed;
}
