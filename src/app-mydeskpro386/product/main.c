#include "core/product/startup_interface.h"
#include "app-mydeskpro386/product/binding_interface.h"

#define APP_PRODUCT_NAME "MyDeskPro386"

lib_i32 main(void)
{
    vm_app_print_banner(APP_PRODUCT_NAME);
    return vm_app_run(APP_PRODUCT_NAME, &vm_app_machine);
}
