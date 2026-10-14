#include "core/product/startup_interface.h"
#include "app-mydeskpro386/product_binding_interface.h"

#define APP_PRODUCT_NAME "MyDeskPro386"

lib_i32 main(void)
{
    return vm_app_run(APP_PRODUCT_NAME, APP_PRODUCT_INI_NAME, &vm_app_machine);
}
