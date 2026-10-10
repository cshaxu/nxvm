#include "core/product/startup_interface.h"
#include "app-my5160/product/binding_interface.h"

#define APP_PRODUCT_NAME "My5160"

lib_i32 main(void)
{
    return vm_app_run(APP_PRODUCT_NAME, &vm_app_machine);
}
