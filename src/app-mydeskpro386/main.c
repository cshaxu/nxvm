#include "x86-nxvm/product/startup_interface.h"
#include "app-mydeskpro386/product_binding_interface.h"

#define APP_PRODUCT_NAME "MyDeskPro386"

lib_i32 main(void)
{
    return x86_nxvm_product_run(APP_PRODUCT_NAME, APP_PRODUCT_INI_NAME, &vm_app_machine);
}
