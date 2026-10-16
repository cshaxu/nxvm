#include "x86-nxvm/product/startup_interface.h"
#include "app-my5170/product_binding_interface.h"

#define APP_PRODUCT_NAME "My5170"

lib_i32 main(void)
{
    return x86_nxvm_product_run(APP_PRODUCT_NAME, APP_PRODUCT_INI_NAME, &vm_app_machine);
}
