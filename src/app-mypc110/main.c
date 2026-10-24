#include "app-mypc110/product_binding_interface.h"
#include "x86-nxvm/product/startup_interface.h"

#define APP_PRODUCT_NAME "MyPC110 TFT"
#define APP_PRODUCT_INI_NAME "mypc110.ini"

lib_i32 main(void)
{
    return x86_nxvm_product_run(APP_PRODUCT_NAME, APP_PRODUCT_INI_NAME,
        &vm_app_machine);
}
