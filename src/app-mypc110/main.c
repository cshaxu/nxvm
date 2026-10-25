#include "app-mypc110/product_binding_interface.h"
#include "x86-nxvm/product/startup_interface.h"

#define APP_PRODUCT_INI_NAME "mypc110.ini"

#ifndef MYPC110_PRODUCT_NAME
#error "MyPC110 build must select one fixed product name."
#endif

lib_i32 main(void)
{
    return x86_nxvm_product_run(MYPC110_PRODUCT_NAME, APP_PRODUCT_INI_NAME,
        &vm_app_machine);
}
