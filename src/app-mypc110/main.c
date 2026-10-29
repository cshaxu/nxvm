#include "app-mypc110/product_binding_interface.h"
#include "x86-nxvm/product/startup_interface.h"

#define APP_PRODUCT_INI_NAME "mypc110.ini"

#ifndef MYPC110_PRODUCT_NAME
#error "MyPC110 build must select one fixed product name."
#endif

lib_i32 main(void)
{
    mypc110_vdisplay display;
    x86_nxvm_product_machine_binding binding = vm_app_machine;
    if (mypc110_product_initialize_vdisplay(&display) != LIB_STATUS_OK) return 1;
    binding.context = &display;
    return x86_nxvm_product_run(MYPC110_PRODUCT_NAME, APP_PRODUCT_INI_NAME,
        &binding);
}
