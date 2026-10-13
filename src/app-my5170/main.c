#include "core/product/startup_interface.h"
#include "app-my5170/product/binding_interface.h"

#define APP_PRODUCT_NAME "My5170"

lib_i32 main(void)
{
    return vm_app_run(APP_PRODUCT_NAME, &vm_app_machine);
}
